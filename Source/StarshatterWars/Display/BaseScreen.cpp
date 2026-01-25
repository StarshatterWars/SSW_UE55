/*  Project Starshatter Wars
    Fractal Dev Studios
    Copyright (c) 2025-2026.

    ORIGINAL AUTHOR AND STUDIO
    ==========================
    John DiCamillo / Destroyer Studios LLC

    SUBSYSTEM:    Stars.exe
    FILE:         BaseScreen.cpp
    AUTHOR:       Carlos Bott

    OVERVIEW
    ========
    UBaseScreen implementation.
    - FORM-style ID binding helpers (Label/Button/Image/Edit/Combo/List/Slider/Text).
    - Legacy .frm parser:
        * Parses: form:{}, ctrl:{}, defctrl:{}, layout:{}, column:{}.
        * defctrl is treated as the "current defaults" applied to subsequent ctrl blocks.
        * Supports hex ints like 0x02 (style fields).
        * Robustly skips unknown keys by consuming whole value expressions:
          scalar, (tuples), {blocks}, nested.
    - Unified dialog key handling: Enter/Escape -> HandleAccept/HandleCancel.
*/

#include "BaseScreen.h"

// UMG:
#include "Components/TextBlock.h"
#include "Components/RichTextBlock.h"
#include "Components/Button.h"
#include "Components/Image.h"
#include "Components/EditableTextBox.h"
#include "Components/ComboBoxString.h"
#include "Components/ListView.h"
#include "Components/Slider.h"

#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Framework/Application/SlateApplication.h"

namespace
{
    // -----------------------------------------------------------------
    // Minimal tokenizer for Starshatter FORM syntax
    // -----------------------------------------------------------------

    enum class ETok : uint8
    {
        End,
        Ident,
        String,
        Number,
        LBrace,   // {
        RBrace,   // }
        LParen,   // (
        RParen,   // )
        Colon,    // :
        Comma,    // ,
        Hash,     // # (ignored)
    };

    struct FTok
    {
        ETok    Type = ETok::End;
        FString Text;
        double  Number = 0.0;
    };

    static bool IsIdentStart(TCHAR C)
    {
        return FChar::IsAlpha(C) || C == '_' || C == '$';
    }

    static bool IsIdentChar(TCHAR C)
    {
        return FChar::IsAlnum(C) || C == '_' || C == '.' || C == '-' || C == '$';
    }

    static void StripLineComments(FString& S)
    {
        // Remove // comments (FORM files use //).
        TArray<FString> Lines;
        S.ParseIntoArrayLines(Lines, false);

        for (FString& L : Lines)
        {
            for (int32 i = 0; i < L.Len() - 1; ++i)
            {
                if (L[i] == TEXT('/') && L[i + 1] == TEXT('/'))
                {
                    L = L.Left(i);
                    break;
                }
            }
        }

        S = FString::Join(Lines, TEXT("\n"));
    }

    static void StripBlockComments(FString& S)
    {
        // Remove /* ... */ style blocks (your FORM uses /*** ... ***/).
        // Simple non-nested removal.
        while (true)
        {
            int32 Start = S.Find(TEXT("/*"));
            if (Start == INDEX_NONE)
                break;

            int32 End = S.Find(TEXT("*/"), ESearchCase::IgnoreCase, ESearchDir::FromStart, Start + 2);
            if (End == INDEX_NONE)
            {
                // If unclosed, drop remainder
                S = S.Left(Start);
                break;
            }

            const int32 RemoveLen = (End + 2) - Start;
            S.RemoveAt(Start, RemoveLen, /*bAllowShrinking*/false);
        }
    }

    class FFormLexer
    {
    public:
        explicit FFormLexer(const FString& In)
            : Src(In)
            , Len(In.Len())
        {
        }

        FTok Next()
        {
            SkipWs();

            if (Pos >= Len)
                return { ETok::End };

            const TCHAR C = Src[Pos];

            // Single char tokens:
            if (C == '{') { ++Pos; return { ETok::LBrace, TEXT("{") }; }
            if (C == '}') { ++Pos; return { ETok::RBrace, TEXT("}") }; }
            if (C == '(') { ++Pos; return { ETok::LParen, TEXT("(") }; }
            if (C == ')') { ++Pos; return { ETok::RParen, TEXT(")") }; }
            if (C == ':') { ++Pos; return { ETok::Colon,  TEXT(":") }; }
            if (C == ',') { ++Pos; return { ETok::Comma,  TEXT(",") }; }
            if (C == '#') { ++Pos; return { ETok::Hash,   TEXT("#") }; }

            // String:
            if (C == '"')
            {
                ++Pos;
                FString Out;
                while (Pos < Len)
                {
                    const TCHAR D = Src[Pos++];
                    if (D == '"') break;
                    Out.AppendChar(D);
                }

                FTok T;
                T.Type = ETok::String;
                T.Text = Out;
                return T;
            }

            // Hex integer: 0xNN (treat as Ident so parser can interpret)
            if (C == '0' && (Pos + 1) < Len && (Src[Pos + 1] == 'x' || Src[Pos + 1] == 'X'))
            {
                const int32 Start = Pos;
                Pos += 2; // skip 0x
                while (Pos < Len && FChar::IsHexDigit(Src[Pos]))
                    ++Pos;

                FTok T;
                T.Type = ETok::Ident;
                T.Text = Src.Mid(Start, Pos - Start); // "0x02"
                return T;
            }

            // Number (int/float):
            if (FChar::IsDigit(C) || C == '-' || C == '+')
            {
                const int32 Start = Pos;
                ++Pos;
                while (Pos < Len)
                {
                    const TCHAR D = Src[Pos];
                    if (!(FChar::IsDigit(D) || D == '.' || D == 'e' || D == 'E' || D == '-' || D == '+'))
                        break;
                    ++Pos;
                }
                const FString NumStr = Src.Mid(Start, Pos - Start);

                FTok T;
                T.Type = ETok::Number;
                T.Text = NumStr;
                T.Number = FCString::Atod(*NumStr);
                return T;
            }

            // Identifier:
            if (IsIdentStart(C))
            {
                const int32 Start = Pos;
                ++Pos;
                while (Pos < Len && IsIdentChar(Src[Pos]))
                    ++Pos;

                FTok T;
                T.Type = ETok::Ident;
                T.Text = Src.Mid(Start, Pos - Start);
                return T;
            }

            // Unknown: skip and continue
            ++Pos;
            return Next();
        }

    private:
        void SkipWs()
        {
            while (Pos < Len)
            {
                const TCHAR C = Src[Pos];
                if (C == ' ' || C == '\t' || C == '\r' || C == '\n')
                {
                    ++Pos;
                    continue;
                }
                break;
            }
        }

        const FString& Src;
        int32 Len = 0;
        int32 Pos = 0;
    };

    class FFormParser
    {
    public:
        explicit FFormParser(const FString& InText)
            : Lex(InText)
        {
            Tok = Lex.Next();
        }

        bool ParseForm(FParsedForm& Out, FString& OutError)
        {
            // Expect: form : { ... }
            if (!MatchIdent(TEXT("form")))
            {
                OutError = TEXT("Expected 'form'");
                return false;
            }

            if (!Consume(ETok::Colon, OutError, TEXT("Expected ':' after 'form'")))
                return false;

            if (!Consume(ETok::LBrace, OutError, TEXT("Expected '{' after 'form:'")))
                return false;

            // Current defctrl defaults
            FParsedCtrl CurrentDefaults;
            CurrentDefaults.Type = EFormCtrlType::None;

            while (!Is(ETok::RBrace) && !Is(ETok::End))
            {
                // Form-level keys

                if (IsIdent(TEXT("back_color")))
                {
                    ConsumeIdent();
                    if (!Consume(ETok::Colon, OutError, TEXT("Expected ':' after back_color")))
                        return false;
                    Out.BackColor = ParseColor(OutError);
                    if (!OutError.IsEmpty()) return false;
                    EatOptionalComma();
                    continue;
                }

                if (IsIdent(TEXT("fore_color")))
                {
                    ConsumeIdent();
                    if (!Consume(ETok::Colon, OutError, TEXT("Expected ':' after fore_color")))
                        return false;
                    Out.ForeColor = ParseColor(OutError);
                    if (!OutError.IsEmpty()) return false;
                    EatOptionalComma();
                    continue;
                }

                if (IsIdent(TEXT("texture")))
                {
                    ConsumeIdent();
                    if (!Consume(ETok::Colon, OutError, TEXT("Expected ':' after texture")))
                        return false;
                    Out.Texture = ParseStringOrIdent(OutError);
                    if (!OutError.IsEmpty()) return false;
                    EatOptionalComma();
                    continue;
                }

                if (IsIdent(TEXT("margins")))
                {
                    ConsumeIdent();
                    if (!Consume(ETok::Colon, OutError, TEXT("Expected ':' after margins")))
                        return false;
                    Out.Margins = ParseRect(OutError);
                    if (!OutError.IsEmpty()) return false;
                    EatOptionalComma();
                    continue;
                }

                if (IsIdent(TEXT("layout")))
                {
                    ConsumeIdent();
                    if (!Consume(ETok::Colon, OutError, TEXT("Expected ':' after layout")))
                        return false;
                    if (!ParseLayoutBlock(Out.Layout, OutError))
                        return false;
                    EatOptionalComma();
                    continue;
                }

                // defctrl

                if (IsIdent(TEXT("defctrl")))
                {
                    ConsumeIdent();
                    if (!Consume(ETok::Colon, OutError, TEXT("Expected ':' after defctrl")))
                        return false;
                    if (!Consume(ETok::LBrace, OutError, TEXT("Expected '{' after defctrl")))
                        return false;
                    if (!ParseCtrlBody(CurrentDefaults, true, OutError))
                        return false;
                    if (!Consume(ETok::RBrace, OutError, TEXT("Expected '}' after defctrl")))
                        return false;
                    EatOptionalComma();
                    continue;
                }

                // ctrl

                if (IsIdent(TEXT("ctrl")))
                {
                    ConsumeIdent();
                    if (!Consume(ETok::Colon, OutError, TEXT("Expected ':' after ctrl")))
                        return false;
                    if (!Consume(ETok::LBrace, OutError, TEXT("Expected '{' after ctrl")))
                        return false;

                    FParsedCtrl Ctrl;
                    if (!ParseCtrlBody(Ctrl, false, OutError))
                        return false;

                    if (!Consume(ETok::RBrace, OutError, TEXT("Expected '}' after ctrl")))
                        return false;

                    ApplyDefaults(Ctrl, CurrentDefaults);
                    Out.Controls.Add(Ctrl);

                    EatOptionalComma();
                    continue;
                }

                // Unknown key, skip token
                Tok = Lex.Next();
            }

            if (!Consume(ETok::RBrace, OutError, TEXT("Expected closing '}' for form")))
                return false;

            return true;
        }

    private:
        bool Is(ETok T) const
        {
            return Tok.Type == T;
        }

        bool IsIdent(const TCHAR* S) const
        {
            return Tok.Type == ETok::Ident &&
                Tok.Text.Equals(S, ESearchCase::IgnoreCase);
        }

        bool MatchIdent(const TCHAR* S)
        {
            if (!IsIdent(S))
                return false;
            Tok = Lex.Next();
            return true;
        }

        void ConsumeIdent()
        {
            Tok = Lex.Next();
        }

        bool Consume(ETok T, FString& OutError, const TCHAR* Msg)
        {
            if (Tok.Type != T)
            {
                OutError = Msg;
                return false;
            }
            Tok = Lex.Next();
            return true;
        }

        void EatOptionalComma()
        {
            if (Tok.Type == ETok::Comma)
                Tok = Lex.Next();
        }

        static bool IsHexLiteral(const FString& S)
        {
            return S.Len() > 2 &&
                S[0] == '0' &&
                (S[1] == 'x' || S[1] == 'X');
        }

        int32 ParseIntFlexible(FString& OutError)
        {
            if (Tok.Type == ETok::Number)
            {
                int32 V = (int32)Tok.Number;
                Tok = Lex.Next();
                return V;
            }

            if (Tok.Type == ETok::Ident && IsHexLiteral(Tok.Text))
            {
                int32 V = FCString::Strtoi(*Tok.Text.Mid(2), nullptr, 16);
                Tok = Lex.Next();
                return V;
            }

            OutError = TEXT("Expected integer");
            return 0;
        }

        FString ParseStringOrIdent(FString& OutError)
        {
            if (Tok.Type == ETok::String || Tok.Type == ETok::Ident)
            {
                FString S = Tok.Text;
                Tok = Lex.Next();
                return S;
            }

            OutError = TEXT("Expected string or identifier");
            return FString();
        }

        bool ParseBool(FString& OutError)
        {
            if (Tok.Type == ETok::Ident)
            {
                bool b =
                    Tok.Text.Equals(TEXT("true"), ESearchCase::IgnoreCase) ||
                    Tok.Text.Equals(TEXT("1"));

                Tok = Lex.Next();
                return b;
            }

            if (Tok.Type == ETok::Number)
            {
                bool b = Tok.Number != 0.0;
                Tok = Lex.Next();
                return b;
            }

            OutError = TEXT("Expected boolean");
            return false;
        }

        FFormIntRect ParseRect(FString& OutError)
        {
            if (!Consume(ETok::LParen, OutError, TEXT("Expected '('")))
                return {};

            int32 A = ParseIntFlexible(OutError);
            if (!Consume(ETok::Comma, OutError, TEXT("Expected ','")))
                return {};

            int32 B = ParseIntFlexible(OutError);
            if (!Consume(ETok::Comma, OutError, TEXT("Expected ','")))
                return {};

            int32 C = ParseIntFlexible(OutError);
            if (!Consume(ETok::Comma, OutError, TEXT("Expected ','")))
                return {};

            int32 D = ParseIntFlexible(OutError);
            if (!Consume(ETok::RParen, OutError, TEXT("Expected ')'")))
                return {};

            return FFormIntRect(A, B, C, D);
        }

        FLinearColor ParseColor(FString& OutError)
        {
            FFormIntRect R = ParseRect(OutError);
            return FLinearColor(
                R.A / 255.0f,
                R.B / 255.0f,
                R.C / 255.0f,
                R.D / 255.0f
            );
        }

        bool ParseLayoutBlock(FFormLayout& Out, FString& OutError);
        bool ParseCtrlBody(FParsedCtrl& OutCtrl, bool bIsDefctrl, FString& OutError);

        static void ApplyDefaults(FParsedCtrl& Ctrl, const FParsedCtrl& Def)
        {
            if (!Ctrl.bHasTexture && Def.bHasTexture)
                Ctrl.Texture = Def.Texture;
            if (!Ctrl.bHasFont && Def.bHasFont)
                Ctrl.Font = Def.Font;
            if (!Ctrl.bHasAlign && Def.bHasAlign)
                Ctrl.Align = Def.Align;
            if (!Ctrl.bHasBackColor && Def.bHasBackColor)
                Ctrl.BackColor = Def.BackColor;
            if (!Ctrl.bHasForeColor && Def.bHasForeColor)
                Ctrl.ForeColor = Def.ForeColor;
        }

    private:
        FFormLexer Lex;
        FTok       Tok;
    };
} // namespace

// --------------------------------------------------------------------
// UBaseScreen lifecycle
// --------------------------------------------------------------------

void UBaseScreen::NativeOnInitialized()
{
    Super::NativeOnInitialized();

    FormMap.Reset();
    BindFormWidgets();

    // Optional compiled defaults bridge:
    ApplyFormDefaults();

    // Parse embedded .frm (if provided) and apply defaults:
    const FString Frm = GetLegacyFormText();
    if (!Frm.IsEmpty())
    {
        FString Clean = Frm;
        StripLineComments(Clean);
        StripBlockComments(Clean);

        FString Err;
        FParsedForm Parsed;
        if (ParseLegacyForm(Clean, Parsed, Err))
        {
            ParsedForm = Parsed;
            ApplyLegacyFormDefaults(ParsedForm);
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("BaseScreen: ParseLegacyForm failed: %s"), *Err);
        }
    }
}

void UBaseScreen::NativeConstruct()
{
    Super::NativeConstruct();
}

void UBaseScreen::NativePreConstruct()
{
    Super::NativePreConstruct();
}

void UBaseScreen::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
    Super::NativeTick(MyGeometry, InDeltaTime);
}

// --------------------------------------------------------------------
// Centralized dialog input (Enter/Escape)
// --------------------------------------------------------------------

FReply UBaseScreen::NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
    if (!bDialogInputEnabled)
        return Super::NativeOnKeyDown(InGeometry, InKeyEvent);

    const FKey Key = InKeyEvent.GetKey();

    if (Key == EKeys::Enter || Key == EKeys::Virtual_Accept)
    {
        HandleAccept();
        return FReply::Handled();
    }

    if (Key == EKeys::Escape || Key == EKeys::Virtual_Back)
    {
        HandleCancel();
        return FReply::Handled();
    }

    return Super::NativeOnKeyDown(InGeometry, InKeyEvent);
}

void UBaseScreen::HandleAccept()
{
    // Default: click ApplyButton if present/enabled, otherwise do nothing.
    if (ApplyButton && ApplyButton->GetIsEnabled())
    {
        ApplyButton->OnClicked.Broadcast();
        return;
    }
}

void UBaseScreen::HandleCancel()
{
    // Default: click CancelButton if present/enabled, otherwise do nothing.
    if (CancelButton && CancelButton->GetIsEnabled())
    {
        CancelButton->OnClicked.Broadcast();
        return;
    }
}

// --------------------------------------------------------------------
// Bind helpers
// --------------------------------------------------------------------

void UBaseScreen::BindLabel(int32 Id, UTextBlock* Widget)
{
    if (Widget) FormMap.Labels.Add(Id, Widget);
}

void UBaseScreen::BindText(int32 Id, URichTextBlock* Widget)
{
    if (Widget) FormMap.Texts.Add(Id, Widget);
}

void UBaseScreen::BindButton(int32 Id, UButton* Widget)
{
    if (Widget) FormMap.Buttons.Add(Id, Widget);
}

void UBaseScreen::BindImage(int32 Id, UImage* Widget)
{
    if (Widget) FormMap.Images.Add(Id, Widget);
}

void UBaseScreen::BindEdit(int32 Id, UEditableTextBox* Widget)
{
    if (Widget) FormMap.Edits.Add(Id, Widget);
}

void UBaseScreen::BindCombo(int32 Id, UComboBoxString* Widget)
{
    if (Widget) FormMap.Combos.Add(Id, Widget);
}

void UBaseScreen::BindList(int32 Id, UListView* Widget)
{
    if (Widget) FormMap.Lists.Add(Id, Widget);
}

void UBaseScreen::BindSlider(int32 Id, USlider* Widget)
{
    if (Widget) FormMap.Sliders.Add(Id, Widget);
}

// --------------------------------------------------------------------
// Lookup helpers
// --------------------------------------------------------------------

UTextBlock* UBaseScreen::GetLabel(int32 Id) const
{
    if (const TObjectPtr<UTextBlock>* Found = FormMap.Labels.Find(Id))
        return Found->Get();
    return nullptr;
}

URichTextBlock* UBaseScreen::GetText(int32 Id) const
{
    if (const TObjectPtr<URichTextBlock>* Found = FormMap.Texts.Find(Id))
        return Found->Get();
    return nullptr;
}

UButton* UBaseScreen::GetButton(int32 Id) const
{
    if (const TObjectPtr<UButton>* Found = FormMap.Buttons.Find(Id))
        return Found->Get();
    return nullptr;
}

UImage* UBaseScreen::GetImage(int32 Id) const
{
    if (const TObjectPtr<UImage>* Found = FormMap.Images.Find(Id))
        return Found->Get();
    return nullptr;
}

UEditableTextBox* UBaseScreen::GetEdit(int32 Id) const
{
    if (const TObjectPtr<UEditableTextBox>* Found = FormMap.Edits.Find(Id))
        return Found->Get();
    return nullptr;
}

UComboBoxString* UBaseScreen::GetCombo(int32 Id) const
{
    if (const TObjectPtr<UComboBoxString>* Found = FormMap.Combos.Find(Id))
        return Found->Get();
    return nullptr;
}

UListView* UBaseScreen::GetList(int32 Id) const
{
    if (const TObjectPtr<UListView>* Found = FormMap.Lists.Find(Id))
        return Found->Get();
    return nullptr;
}

USlider* UBaseScreen::GetSlider(int32 Id) const
{
    if (const TObjectPtr<USlider>* Found = FormMap.Sliders.Find(Id))
        return Found->Get();
    return nullptr;
}

// --------------------------------------------------------------------
// Operations helpers
// --------------------------------------------------------------------

void UBaseScreen::SetLabelText(int32 Id, const FText& Text)
{
    if (UTextBlock* L = GetLabel(Id))
        L->SetText(Text);
}

void UBaseScreen::SetEditText(int32 Id, const FText& Text)
{
    if (UEditableTextBox* E = GetEdit(Id))
        E->SetText(Text);
}

void UBaseScreen::SetVisible(int32 Id, bool bVisible)
{
    const ESlateVisibility Vis = bVisible ? ESlateVisibility::Visible : ESlateVisibility::Hidden;

    if (UTextBlock* L = GetLabel(Id))        L->SetVisibility(Vis);
    if (URichTextBlock* T = GetText(Id))    T->SetVisibility(Vis);
    if (UButton* B = GetButton(Id))         B->SetVisibility(Vis);
    if (UImage* I = GetImage(Id))           I->SetVisibility(Vis);
    if (UEditableTextBox* E = GetEdit(Id))  E->SetVisibility(Vis);
    if (UComboBoxString* C = GetCombo(Id))  C->SetVisibility(Vis);
    if (UListView* LV = GetList(Id))        LV->SetVisibility(Vis);
    if (USlider* S = GetSlider(Id))         S->SetVisibility(Vis);
}

void UBaseScreen::SetEnabled(int32 Id, bool bEnabled)
{
    if (UButton* B = GetButton(Id))         B->SetIsEnabled(bEnabled);
    if (UEditableTextBox* E = GetEdit(Id))  E->SetIsEnabled(bEnabled);
    if (UComboBoxString* C = GetCombo(Id))  C->SetIsEnabled(bEnabled);
    if (UListView* LV = GetList(Id))        LV->SetIsEnabled(bEnabled);
    if (USlider* S = GetSlider(Id))         S->SetIsEnabled(bEnabled);
}

// --------------------------------------------------------------------
// Optional compiled defaults hook (no-op by default)
// --------------------------------------------------------------------

void UBaseScreen::ApplyFormDefaults()
{
    // Intentionally empty in this version.
    // If you later add compiled form defs, this is where you apply them.
}

// --------------------------------------------------------------------
// Legacy FORM parse + defaults application
// --------------------------------------------------------------------

bool UBaseScreen::ParseLegacyForm(const FString& InText, FParsedForm& OutForm, FString& OutError) const
{
    FFormParser Parser(InText);
    return Parser.ParseForm(OutForm, OutError);
}

static int32 ParseTrailingInt(const FString& S)
{
    // Extract trailing digits, e.g. "Limerick12" -> 12, "Verdana" -> 0
    int32 i = S.Len() - 1;
    while (i >= 0 && FChar::IsDigit(S[i]))
        --i;

    const int32 StartDigits = i + 1;
    if (StartDigits < S.Len())
    {
        const FString Digits = S.Mid(StartDigits);
        return FCString::Atoi(*Digits);
    }

    return 0;
}

static bool EqualsNoCase(const FString& A, const FString& B)
{
    return A.Equals(B, ESearchCase::IgnoreCase);
}

bool UBaseScreen::ResolveFont(const FString& InFontName, FSlateFontInfo& OutFont) const
{
    // 1) Try explicit mapping first
    for (const FFormFontMapEntry& Entry : FontMappings)
    {
        if (!Entry.Font)
            continue;

        if (EqualsNoCase(Entry.LegacyName, InFontName))
        {
            int32 Size = DefaultFontSize;

            if (Entry.bOverrideSize && Entry.Size > 0)
            {
                Size = Entry.Size;
            }
            else
            {
                const int32 Inferred = ParseTrailingInt(InFontName);
                if (Inferred > 0)
                    Size = Inferred;
            }

            OutFont = FSlateFontInfo(Entry.Font, Size);
            return true;
        }
    }

    // 2) Fallback: infer size from name and use DefaultFont if present
    if (DefaultFont)
    {
        int32 Size = DefaultFontSize;
        const int32 Inferred = ParseTrailingInt(InFontName);
        if (Inferred > 0)
            Size = Inferred;

        OutFont = FSlateFontInfo(DefaultFont, Size);
        return true;
    }

    // 3) No mapping and no default
    return false;
}

bool UBaseScreen::ResolveTextJustification(EFormAlign InAlign, ETextJustify::Type& OutJustify) const
{
    switch (InAlign)
    {
    case EFormAlign::Left:   OutJustify = ETextJustify::Left;   return true;
    case EFormAlign::Center: OutJustify = ETextJustify::Center; return true;
    case EFormAlign::Right:  OutJustify = ETextJustify::Right;  return true;
    default: break;
    }
    return false;
}

void UBaseScreen::ApplyLegacyFormDefaults(const FParsedForm& Parsed)
{
    // Apply to Label (TextBlock) and Text (RichTextBlock).
    for (const FParsedCtrl& C : Parsed.Controls)
    {
        // ---------------- LABEL ----------------
        if (C.Type == EFormCtrlType::Label)
        {
            UTextBlock* L = GetLabel(C.Id);
            if (!L) continue;

            if (!C.Text.IsEmpty())
                L->SetText(FText::FromString(C.Text));

            if (C.ForeColor != FLinearColor::Transparent)
                L->SetColorAndOpacity(FSlateColor(C.ForeColor));

            ETextJustify::Type Just;
            if (ResolveTextJustification(C.Align, Just))
                L->SetJustification(Just);

            FSlateFontInfo Font;
            if (!C.Font.IsEmpty() && ResolveFont(C.Font, Font))
                L->SetFont(Font);

            continue;
        }

        // ---------------- TEXT (Rich) ----------------
        if (C.Type == EFormCtrlType::Text)
        {
            URichTextBlock* T = GetText(C.Id);
            if (!T) continue;

            if (!C.Text.IsEmpty())
                T->SetText(FText::FromString(C.Text));

            if (C.ForeColor != FLinearColor::Transparent)
                T->SetDefaultColorAndOpacity(C.ForeColor);

            ETextJustify::Type Just;
            if (ResolveTextJustification(C.Align, Just))
                T->SetJustification(Just);

            if (!C.Font.IsEmpty())
            {
                FSlateFontInfo Font;
                if (ResolveFont(C.Font, Font))
                {
                    FTextBlockStyle Style = T->GetDefaultTextStyle();
                    Style.SetFont(Font);
                    T->SetDefaultTextStyle(Style);
                }
            }

            continue;
        }
    }
}
