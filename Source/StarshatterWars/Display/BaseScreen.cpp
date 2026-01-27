/*  Project Starshatter Wars
    Fractal Dev Studios
    Copyright (c) 2025-2026.

    SUBSYSTEM:    Stars.exe
    FILE:         BaseScreen.cpp
    AUTHOR:       Carlos Bott
*/

#include "BaseScreen.h"

// Core:
#include "Misc/Char.h"
#include "Logging/LogMacros.h"

// Input:
#include "Input/Reply.h"
#include "InputCoreTypes.h"

// UMG:
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Components/TextBlock.h"
#include "Components/RichTextBlock.h"
#include "Components/Button.h"
#include "Components/Image.h"
#include "Components/EditableTextBox.h"
#include "Components/ComboBoxString.h"
#include "Components/ListView.h"
#include "Components/Slider.h"

// Slate styling:
#include "Styling/SlateColor.h"
#include "Fonts/SlateFontInfo.h"
#include "Styling/SlateTypes.h"

DEFINE_LOG_CATEGORY_STATIC(LogBaseScreen, Log, All);

namespace
{
    enum class ETok : uint8
    {
        End,
        Ident,
        String,
        Number,
        LBrace,
        RBrace,
        LParen,
        RParen,
        Colon,
        Comma,
        Hash
    };

    struct FTok
    {
        ETok Type = ETok::End;
        FString Text;
        double Number = 0.0;
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
        while (true)
        {
            const int32 Start = S.Find(TEXT("/*"));
            if (Start == INDEX_NONE)
                break;

            const int32 End = S.Find(TEXT("*/"), ESearchCase::IgnoreCase, ESearchDir::FromStart, Start + 2);
            if (End == INDEX_NONE)
            {
                S = S.Left(Start);
                break;
            }

            const int32 RemoveLen = (End + 2) - Start;
            S.RemoveAt(Start, RemoveLen, false);
        }
    }

    class FFormLexer
    {
    public:
        explicit FFormLexer(const FString& In)
            : Src(In), Len(In.Len())
        {
        }

        FTok Next()
        {
            SkipWs();

            if (Pos >= Len)
                return { ETok::End };

            const TCHAR C = Src[Pos];

            if (C == '{') { ++Pos; return { ETok::LBrace, TEXT("{") }; }
            if (C == '}') { ++Pos; return { ETok::RBrace, TEXT("}") }; }
            if (C == '(') { ++Pos; return { ETok::LParen, TEXT("(") }; }
            if (C == ')') { ++Pos; return { ETok::RParen, TEXT(")") }; }
            if (C == ':') { ++Pos; return { ETok::Colon, TEXT(":") }; }
            if (C == ',') { ++Pos; return { ETok::Comma, TEXT(",") }; }
            if (C == '#') { ++Pos; return { ETok::Hash, TEXT("#") }; }

            // "string"
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

            // 0x... as ident
            if (C == '0' && (Pos + 1) < Len && (Src[Pos + 1] == 'x' || Src[Pos + 1] == 'X'))
            {
                const int32 Start = Pos;
                Pos += 2;
                while (Pos < Len && FChar::IsHexDigit(Src[Pos]))
                    ++Pos;

                FTok T;
                T.Type = ETok::Ident;
                T.Text = Src.Mid(Start, Pos - Start);
                return T;
            }

            // number
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

            // identifier
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
            if (!MatchIdent(TEXT("form")))
            {
                OutError = TEXT("Expected 'form'");
                return false;
            }
            if (!Consume(ETok::Colon, OutError, TEXT("Expected ':' after 'form'"))) return false;
            if (!Consume(ETok::LBrace, OutError, TEXT("Expected '{' after 'form:'"))) return false;

            FParsedCtrl CurrentDefaults;
            CurrentDefaults.Type = EFormCtrlType::None;

            while (!Is(ETok::RBrace) && !Is(ETok::End))
            {
                if (IsIdent(TEXT("back_color")))
                {
                    ConsumeIdent();
                    if (!Consume(ETok::Colon, OutError, TEXT("Expected ':' after back_color"))) return false;
                    Out.BackColor = ParseColor(OutError);
                    if (!OutError.IsEmpty()) return false;
                    EatOptionalComma();
                    continue;
                }

                if (IsIdent(TEXT("fore_color")))
                {
                    ConsumeIdent();
                    if (!Consume(ETok::Colon, OutError, TEXT("Expected ':' after fore_color"))) return false;
                    Out.ForeColor = ParseColor(OutError);
                    if (!OutError.IsEmpty()) return false;
                    EatOptionalComma();
                    continue;
                }

                // form-level font
                if (IsIdent(TEXT("font")))
                {
                    ConsumeIdent();
                    if (!Consume(ETok::Colon, OutError, TEXT("Expected ':' after font"))) return false;
                    Out.Font = ParseStringOrIdent(OutError);
                    if (!OutError.IsEmpty()) return false;
                    EatOptionalComma();
                    continue;
                }

                if (IsIdent(TEXT("texture")))
                {
                    ConsumeIdent();
                    if (!Consume(ETok::Colon, OutError, TEXT("Expected ':' after texture"))) return false;
                    Out.Texture = ParseStringOrIdent(OutError);
                    if (!OutError.IsEmpty()) return false;
                    EatOptionalComma();
                    continue;
                }

                if (IsIdent(TEXT("margins")))
                {
                    ConsumeIdent();
                    if (!Consume(ETok::Colon, OutError, TEXT("Expected ':' after margins"))) return false;
                    Out.Margins = ParseRect(OutError);
                    if (!OutError.IsEmpty()) return false;
                    EatOptionalComma();
                    continue;
                }

                if (IsIdent(TEXT("layout")))
                {
                    ConsumeIdent();
                    if (!Consume(ETok::Colon, OutError, TEXT("Expected ':' after layout"))) return false;
                    if (!ParseLayoutBlock(Out.Layout, OutError)) return false;
                    EatOptionalComma();
                    continue;
                }

                if (IsIdent(TEXT("defctrl")))
                {
                    ConsumeIdent();
                    if (!Consume(ETok::Colon, OutError, TEXT("Expected ':' after defctrl"))) return false;
                    if (!Consume(ETok::LBrace, OutError, TEXT("Expected '{' after defctrl:"))) return false;
                    if (!ParseCtrlBody(CurrentDefaults, true, OutError)) return false;
                    if (!Consume(ETok::RBrace, OutError, TEXT("Expected '}' after defctrl body"))) return false;
                    EatOptionalComma();
                    continue;
                }

                if (IsIdent(TEXT("ctrl")))
                {
                    ConsumeIdent();
                    if (!Consume(ETok::Colon, OutError, TEXT("Expected ':' after ctrl"))) return false;
                    if (!Consume(ETok::LBrace, OutError, TEXT("Expected '{' after ctrl:"))) return false;

                    FParsedCtrl Ctrl;
                    if (!ParseCtrlBody(Ctrl, false, OutError)) return false;
                    if (!Consume(ETok::RBrace, OutError, TEXT("Expected '}' after ctrl body"))) return false;

                    ApplyDefaults(Ctrl, CurrentDefaults);
                    Out.Controls.Add(Ctrl);

                    EatOptionalComma();
                    continue;
                }

                Tok = Lex.Next();
            }

            if (!Consume(ETok::RBrace, OutError, TEXT("Expected closing '}' for form"))) return false;
            return true;
        }

    private:
        bool Is(ETok T) const { return Tok.Type == T; }

        bool IsIdent(const TCHAR* S) const
        {
            return Tok.Type == ETok::Ident && Tok.Text.Equals(S, ESearchCase::IgnoreCase);
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
            return S.Len() > 2 && S[0] == TCHAR('0') && (S[1] == TCHAR('x') || S[1] == TCHAR('X'));
        }

        int32 ParseIntFlexible(FString& OutError)
        {
            if (Tok.Type == ETok::Number)
            {
                const int32 V = (int32)Tok.Number;
                Tok = Lex.Next();
                return V;
            }

            if (Tok.Type == ETok::Ident && IsHexLiteral(Tok.Text))
            {
                const FString HexDigits = Tok.Text.Mid(2);
                const int32 V = FCString::Strtoi(*HexDigits, nullptr, 16);
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
                const FString S = Tok.Text;
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
                const bool bTrue =
                    Tok.Text.Equals(TEXT("true"), ESearchCase::IgnoreCase) ||
                    Tok.Text.Equals(TEXT("1"), ESearchCase::IgnoreCase);

                const bool bFalse =
                    Tok.Text.Equals(TEXT("false"), ESearchCase::IgnoreCase) ||
                    Tok.Text.Equals(TEXT("0"), ESearchCase::IgnoreCase);

                if (!bTrue && !bFalse)
                {
                    OutError = TEXT("Expected boolean");
                    return false;
                }

                Tok = Lex.Next();
                return bTrue;
            }

            if (Tok.Type == ETok::Number)
            {
                const bool b = Tok.Number != 0.0;
                Tok = Lex.Next();
                return b;
            }

            OutError = TEXT("Expected boolean");
            return false;
        }

        FFormIntRect ParseRect(FString& OutError)
        {
            if (!Consume(ETok::LParen, OutError, TEXT("Expected '(' for rect"))) return {};
            const int32 A = ParseIntFlexible(OutError); if (!OutError.IsEmpty()) return {};
            if (!Consume(ETok::Comma, OutError, TEXT("Expected ',' in rect"))) return {};
            const int32 B = ParseIntFlexible(OutError); if (!OutError.IsEmpty()) return {};
            if (!Consume(ETok::Comma, OutError, TEXT("Expected ',' in rect"))) return {};
            const int32 C = ParseIntFlexible(OutError); if (!OutError.IsEmpty()) return {};
            if (!Consume(ETok::Comma, OutError, TEXT("Expected ',' in rect"))) return {};
            const int32 D = ParseIntFlexible(OutError); if (!OutError.IsEmpty()) return {};
            if (!Consume(ETok::RParen, OutError, TEXT("Expected ')' for rect"))) return {};
            return FFormIntRect(A, B, C, D);
        }

        FLinearColor ParseColor(FString& OutError)
        {
            if (!Consume(ETok::LParen, OutError, TEXT("Expected '(' for color"))) return FLinearColor::Transparent;

            const int32 R = ParseIntFlexible(OutError); if (!OutError.IsEmpty()) return FLinearColor::Transparent;
            if (!Consume(ETok::Comma, OutError, TEXT("Expected ',' in color"))) return FLinearColor::Transparent;
            const int32 G = ParseIntFlexible(OutError); if (!OutError.IsEmpty()) return FLinearColor::Transparent;
            if (!Consume(ETok::Comma, OutError, TEXT("Expected ',' in color"))) return FLinearColor::Transparent;
            const int32 B = ParseIntFlexible(OutError); if (!OutError.IsEmpty()) return FLinearColor::Transparent;

            int32 A = 255;
            if (Tok.Type == ETok::Comma)
            {
                Tok = Lex.Next();
                A = ParseIntFlexible(OutError);
                if (!OutError.IsEmpty()) return FLinearColor::Transparent;
            }

            if (!Consume(ETok::RParen, OutError, TEXT("Expected ')' for color"))) return FLinearColor::Transparent;

            return FLinearColor(
                FMath::Clamp(R / 255.0f, 0.0f, 1.0f),
                FMath::Clamp(G / 255.0f, 0.0f, 1.0f),
                FMath::Clamp(B / 255.0f, 0.0f, 1.0f),
                FMath::Clamp(A / 255.0f, 0.0f, 1.0f)
            );
        }

        EFormAlign ParseAlign(FString& OutError)
        {
            if (Tok.Type != ETok::Ident)
            {
                OutError = TEXT("Expected align identifier");
                return EFormAlign::None;
            }

            const FString S = Tok.Text;
            Tok = Lex.Next();

            if (S.Equals(TEXT("left"), ESearchCase::IgnoreCase))   return EFormAlign::Left;
            if (S.Equals(TEXT("center"), ESearchCase::IgnoreCase)) return EFormAlign::Center;
            if (S.Equals(TEXT("right"), ESearchCase::IgnoreCase))  return EFormAlign::Right;

            OutError = TEXT("Unknown align value");
            return EFormAlign::None;
        }

        EFormCtrlType ParseCtrlType(FString& OutError)
        {
            if (Tok.Type != ETok::Ident)
            {
                OutError = TEXT("Expected ctrl type identifier");
                return EFormCtrlType::None;
            }

            const FString S = Tok.Text;
            Tok = Lex.Next();

            if (S.Equals(TEXT("label"), ESearchCase::IgnoreCase))      return EFormCtrlType::Label;
            if (S.Equals(TEXT("text"), ESearchCase::IgnoreCase))       return EFormCtrlType::Text;
            if (S.Equals(TEXT("button"), ESearchCase::IgnoreCase))     return EFormCtrlType::Button;
            if (S.Equals(TEXT("image"), ESearchCase::IgnoreCase))      return EFormCtrlType::Image;
            if (S.Equals(TEXT("edit"), ESearchCase::IgnoreCase))       return EFormCtrlType::Edit;
            if (S.Equals(TEXT("combo"), ESearchCase::IgnoreCase))      return EFormCtrlType::Combo;
            if (S.Equals(TEXT("list"), ESearchCase::IgnoreCase))       return EFormCtrlType::List;
            if (S.Equals(TEXT("slider"), ESearchCase::IgnoreCase))     return EFormCtrlType::Slider;
            if (S.Equals(TEXT("panel"), ESearchCase::IgnoreCase))      return EFormCtrlType::Panel;
            if (S.Equals(TEXT("background"), ESearchCase::IgnoreCase)) return EFormCtrlType::Background;

            OutError = TEXT("Unknown ctrl type");
            return EFormCtrlType::None;
        }

        TArray<int32> ParseIntArray(FString& OutError)
        {
            TArray<int32> Out;
            if (!Consume(ETok::LParen, OutError, TEXT("Expected '(' for int array"))) return Out;

            while (!Is(ETok::RParen) && !Is(ETok::End))
            {
                Out.Add(ParseIntFlexible(OutError));
                if (!OutError.IsEmpty()) return Out;

                if (Tok.Type == ETok::Comma) { Tok = Lex.Next(); continue; }
                break;
            }

            if (!Consume(ETok::RParen, OutError, TEXT("Expected ')' for int array"))) return Out;
            return Out;
        }

        TArray<float> ParseFloatArray(FString& OutError)
        {
            TArray<float> Out;
            if (!Consume(ETok::LParen, OutError, TEXT("Expected '(' for float array"))) return Out;

            while (!Is(ETok::RParen) && !Is(ETok::End))
            {
                if (Tok.Type != ETok::Number)
                {
                    OutError = TEXT("Expected number in float array");
                    return Out;
                }

                Out.Add((float)Tok.Number);
                Tok = Lex.Next();

                if (Tok.Type == ETok::Comma) { Tok = Lex.Next(); continue; }
                break;
            }

            if (!Consume(ETok::RParen, OutError, TEXT("Expected ')' for float array"))) return Out;
            return Out;
        }

        bool ParseLayoutBlock(FFormLayout& Out, FString& OutError)
        {
            if (!Consume(ETok::LBrace, OutError, TEXT("Expected '{' for layout block"))) return false;

            while (!Is(ETok::RBrace) && !Is(ETok::End))
            {
                if (IsIdent(TEXT("x_mins")))
                {
                    ConsumeIdent();
                    if (!Consume(ETok::Colon, OutError, TEXT("Expected ':' after x_mins"))) return false;
                    Out.XMins = ParseIntArray(OutError);
                    if (!OutError.IsEmpty()) return false;
                    EatOptionalComma();
                    continue;
                }

                if (IsIdent(TEXT("x_weights")))
                {
                    ConsumeIdent();
                    if (!Consume(ETok::Colon, OutError, TEXT("Expected ':' after x_weights"))) return false;
                    Out.XWeights = ParseFloatArray(OutError);
                    if (!OutError.IsEmpty()) return false;
                    EatOptionalComma();
                    continue;
                }

                if (IsIdent(TEXT("y_mins")))
                {
                    ConsumeIdent();
                    if (!Consume(ETok::Colon, OutError, TEXT("Expected ':' after y_mins"))) return false;
                    Out.YMins = ParseIntArray(OutError);
                    if (!OutError.IsEmpty()) return false;
                    EatOptionalComma();
                    continue;
                }

                if (IsIdent(TEXT("y_weights")))
                {
                    ConsumeIdent();
                    if (!Consume(ETok::Colon, OutError, TEXT("Expected ':' after y_weights"))) return false;
                    Out.YWeights = ParseFloatArray(OutError);
                    if (!OutError.IsEmpty()) return false;
                    EatOptionalComma();
                    continue;
                }

                Tok = Lex.Next();
            }

            if (!Consume(ETok::RBrace, OutError, TEXT("Expected '}' closing layout block"))) return false;
            return true;
        }

        bool ParseCtrlBody(FParsedCtrl& OutCtrl, bool bIsDefctrl, FString& OutError)
        {
            while (!Is(ETok::RBrace) && !Is(ETok::End))
            {
                if (Tok.Type != ETok::Ident)
                {
                    Tok = Lex.Next();
                    continue;
                }

                const FString Key = Tok.Text;
                Tok = Lex.Next();

                if (!Consume(ETok::Colon, OutError, TEXT("Expected ':' in ctrl block"))) return false;

                if (Key.Equals(TEXT("id"), ESearchCase::IgnoreCase))
                {
                    OutCtrl.Id = ParseIntFlexible(OutError);
                    if (!OutError.IsEmpty()) return false;
                    EatOptionalComma();
                    continue;
                }

                if (Key.Equals(TEXT("pid"), ESearchCase::IgnoreCase))
                {
                    OutCtrl.ParentId = ParseIntFlexible(OutError);
                    if (!OutError.IsEmpty()) return false;
                    EatOptionalComma();
                    continue;
                }

                if (Key.Equals(TEXT("type"), ESearchCase::IgnoreCase))
                {
                    OutCtrl.Type = ParseCtrlType(OutError);
                    if (!OutError.IsEmpty()) return false;
                    EatOptionalComma();
                    continue;
                }

                if (Key.Equals(TEXT("text"), ESearchCase::IgnoreCase))
                {
                    OutCtrl.Text = ParseStringOrIdent(OutError);
                    if (!OutError.IsEmpty()) return false;
                    OutCtrl.bHasText = !bIsDefctrl;
                    EatOptionalComma();
                    continue;
                }

                if (Key.Equals(TEXT("texture"), ESearchCase::IgnoreCase))
                {
                    OutCtrl.Texture = ParseStringOrIdent(OutError);
                    if (!OutError.IsEmpty()) return false;
                    OutCtrl.bHasTexture = true;
                    EatOptionalComma();
                    continue;
                }

                if (Key.Equals(TEXT("font"), ESearchCase::IgnoreCase))
                {
                    OutCtrl.Font = ParseStringOrIdent(OutError);
                    if (!OutError.IsEmpty()) return false;
                    OutCtrl.bHasFont = true;
                    EatOptionalComma();
                    continue;
                }

                if (Key.Equals(TEXT("align"), ESearchCase::IgnoreCase))
                {
                    OutCtrl.Align = ParseAlign(OutError);
                    if (!OutError.IsEmpty()) return false;
                    OutCtrl.bHasAlign = true;
                    EatOptionalComma();
                    continue;
                }

                if (Key.Equals(TEXT("back_color"), ESearchCase::IgnoreCase))
                {
                    OutCtrl.BackColor = ParseColor(OutError);
                    if (!OutError.IsEmpty()) return false;
                    OutCtrl.bHasBackColor = true;
                    EatOptionalComma();
                    continue;
                }

                if (Key.Equals(TEXT("fore_color"), ESearchCase::IgnoreCase))
                {
                    OutCtrl.ForeColor = ParseColor(OutError);
                    if (!OutError.IsEmpty()) return false;
                    OutCtrl.bHasForeColor = true;
                    EatOptionalComma();
                    continue;
                }

                if (Key.Equals(TEXT("active_color"), ESearchCase::IgnoreCase))
                {
                    OutCtrl.ActiveColor = ParseColor(OutError);
                    if (!OutError.IsEmpty()) return false;
                    OutCtrl.bHasActiveColor = true;
                    EatOptionalComma();
                    continue;
                }

                if (Key.Equals(TEXT("transparent"), ESearchCase::IgnoreCase))
                {
                    OutCtrl.bTransparent = ParseBool(OutError);
                    if (!OutError.IsEmpty()) return false;
                    OutCtrl.bHasTransparent = true;
                    EatOptionalComma();
                    continue;
                }

                if (Key.Equals(TEXT("sticky"), ESearchCase::IgnoreCase))
                {
                    OutCtrl.bSticky = ParseBool(OutError);
                    if (!OutError.IsEmpty()) return false;
                    OutCtrl.bHasSticky = true;
                    EatOptionalComma();
                    continue;
                }

                if (Key.Equals(TEXT("border"), ESearchCase::IgnoreCase))
                {
                    OutCtrl.bBorder = ParseBool(OutError);
                    if (!OutError.IsEmpty()) return false;
                    OutCtrl.bHasBorder = true;
                    EatOptionalComma();
                    continue;
                }

                if (Key.Equals(TEXT("hide_partial"), ESearchCase::IgnoreCase))
                {
                    OutCtrl.bHidePartial = ParseBool(OutError);
                    if (!OutError.IsEmpty()) return false;
                    OutCtrl.bHasHidePartial = true;
                    EatOptionalComma();
                    continue;
                }

                if (Key.Equals(TEXT("show_headings"), ESearchCase::IgnoreCase))
                {
                    OutCtrl.bShowHeadings = ParseBool(OutError);
                    if (!OutError.IsEmpty()) return false;
                    OutCtrl.bHasShowHeadings = true;
                    EatOptionalComma();
                    continue;
                }

                if (Key.Equals(TEXT("fixed_width"), ESearchCase::IgnoreCase))
                {
                    OutCtrl.FixedWidth = ParseIntFlexible(OutError);
                    if (!OutError.IsEmpty()) return false;
                    OutCtrl.bHasFixedWidth = true;
                    EatOptionalComma();
                    continue;
                }

                if (Key.Equals(TEXT("fixed_height"), ESearchCase::IgnoreCase))
                {
                    OutCtrl.FixedHeight = ParseIntFlexible(OutError);
                    if (!OutError.IsEmpty()) return false;
                    OutCtrl.bHasFixedHeight = true;
                    EatOptionalComma();
                    continue;
                }

                if (Key.Equals(TEXT("cells"), ESearchCase::IgnoreCase))
                {
                    OutCtrl.Cells = ParseRect(OutError);
                    if (!OutError.IsEmpty()) return false;
                    OutCtrl.bHasCells = true;
                    EatOptionalComma();
                    continue;
                }

                if (Key.Equals(TEXT("cell_insets"), ESearchCase::IgnoreCase))
                {
                    OutCtrl.CellInsets = ParseRect(OutError);
                    if (!OutError.IsEmpty()) return false;
                    OutCtrl.bHasCellInsets = true;
                    EatOptionalComma();
                    continue;
                }

                if (Key.Equals(TEXT("margins"), ESearchCase::IgnoreCase))
                {
                    OutCtrl.Margins = ParseRect(OutError);
                    if (!OutError.IsEmpty()) return false;
                    OutCtrl.bHasMargins = true;
                    EatOptionalComma();
                    continue;
                }

                if (Key.Equals(TEXT("scroll_bar"), ESearchCase::IgnoreCase))
                {
                    OutCtrl.ScrollBar = ParseIntFlexible(OutError);
                    if (!OutError.IsEmpty()) return false;
                    OutCtrl.bHasScrollBar = true;
                    EatOptionalComma();
                    continue;
                }

                if (Key.Equals(TEXT("style"), ESearchCase::IgnoreCase))
                {
                    OutCtrl.Style = ParseIntFlexible(OutError);
                    if (!OutError.IsEmpty()) return false;
                    OutCtrl.bHasStyle = true;
                    EatOptionalComma();
                    continue;
                }

                if (Key.Equals(TEXT("layout"), ESearchCase::IgnoreCase))
                {
                    if (!ParseLayoutBlock(OutCtrl.Layout, OutError)) return false;
                    OutCtrl.bHasLayout = true;
                    EatOptionalComma();
                    continue;
                }

                Tok = Lex.Next();
            }

            return true;
        }

        static void ApplyDefaults(FParsedCtrl& Ctrl, const FParsedCtrl& Def)
        {
            if (!Ctrl.bHasTexture && Def.bHasTexture)          Ctrl.Texture = Def.Texture;
            if (!Ctrl.bHasFont && Def.bHasFont)               Ctrl.Font = Def.Font;
            if (!Ctrl.bHasAlign && Def.bHasAlign)             Ctrl.Align = Def.Align;
            if (!Ctrl.bHasBackColor && Def.bHasBackColor)     Ctrl.BackColor = Def.BackColor;
            if (!Ctrl.bHasForeColor && Def.bHasForeColor)     Ctrl.ForeColor = Def.ForeColor;

            if (!Ctrl.bHasActiveColor && Def.bHasActiveColor) Ctrl.ActiveColor = Def.ActiveColor;

            if (!Ctrl.bHasTransparent && Def.bHasTransparent) Ctrl.bTransparent = Def.bTransparent;
            if (!Ctrl.bHasSticky && Def.bHasSticky)           Ctrl.bSticky = Def.bSticky;
            if (!Ctrl.bHasBorder && Def.bHasBorder)           Ctrl.bBorder = Def.bBorder;
            if (!Ctrl.bHasHidePartial && Def.bHasHidePartial) Ctrl.bHidePartial = Def.bHidePartial;
            if (!Ctrl.bHasShowHeadings && Def.bHasShowHeadings) Ctrl.bShowHeadings = Def.bShowHeadings;

            if (!Ctrl.bHasFixedWidth && Def.bHasFixedWidth)   Ctrl.FixedWidth = Def.FixedWidth;
            if (!Ctrl.bHasFixedHeight && Def.bHasFixedHeight) Ctrl.FixedHeight = Def.FixedHeight;

            if (!Ctrl.bHasCells && Def.bHasCells)             Ctrl.Cells = Def.Cells;
            if (!Ctrl.bHasCellInsets && Def.bHasCellInsets)   Ctrl.CellInsets = Def.CellInsets;
            if (!Ctrl.bHasMargins && Def.bHasMargins)         Ctrl.Margins = Def.Margins;

            if (!Ctrl.bHasScrollBar && Def.bHasScrollBar)     Ctrl.ScrollBar = Def.ScrollBar;
            if (!Ctrl.bHasStyle && Def.bHasStyle)             Ctrl.Style = Def.Style;

            if (!Ctrl.bHasLayout && Def.bHasLayout)           Ctrl.Layout = Def.Layout;
        }

        FFormLexer Lex;
        FTok Tok;
    };
} // namespace

// --------------------------------------------------------------------
// UBaseScreen lifecycle
// --------------------------------------------------------------------

UBaseScreen::UBaseScreen(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
}

void UBaseScreen::NativeOnInitialized()
{
    Super::NativeOnInitialized();

    FormMap.Reset();
    BindFormWidgets();

    ApplyFormDefaults();

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
            UE_LOG(LogBaseScreen, Warning, TEXT("BaseScreen: ParseLegacyForm failed: %s"), *Err);
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
    if (ApplyButton && ApplyButton->GetIsEnabled())
    {
        ApplyButton->OnClicked.Broadcast();
        return;
    }
}

void UBaseScreen::HandleCancel()
{
    if (CancelButton && CancelButton->GetIsEnabled())
    {
        CancelButton->OnClicked.Broadcast();
        return;
    }
}

// --------------------------------------------------------------------
// Bind helpers
// --------------------------------------------------------------------

void UBaseScreen::BindLabel(int32 Id, UTextBlock* Widget) { if (Widget) FormMap.Labels.Add(Id, Widget); }
void UBaseScreen::BindText(int32 Id, URichTextBlock* Widget) { if (Widget) FormMap.Texts.Add(Id, Widget); }
void UBaseScreen::BindButton(int32 Id, UButton* Widget) { if (Widget) FormMap.Buttons.Add(Id, Widget); }
void UBaseScreen::BindImage(int32 Id, UImage* Widget) { if (Widget) FormMap.Images.Add(Id, Widget); }
void UBaseScreen::BindEdit(int32 Id, UEditableTextBox* Widget) { if (Widget) FormMap.Edits.Add(Id, Widget); }
void UBaseScreen::BindCombo(int32 Id, UComboBoxString* Widget) { if (Widget) FormMap.Combos.Add(Id, Widget); }
void UBaseScreen::BindList(int32 Id, UListView* Widget) { if (Widget) FormMap.Lists.Add(Id, Widget); }
void UBaseScreen::BindSlider(int32 Id, USlider* Widget) { if (Widget) FormMap.Sliders.Add(Id, Widget); }

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

    if (UTextBlock* L = GetLabel(Id))          L->SetVisibility(Vis);
    if (URichTextBlock* T = GetText(Id))       T->SetVisibility(Vis);
    if (UButton* B = GetButton(Id))           B->SetVisibility(Vis);
    if (UImage* I = GetImage(Id))             I->SetVisibility(Vis);
    if (UEditableTextBox* E = GetEdit(Id))    E->SetVisibility(Vis);
    if (UComboBoxString* C = GetCombo(Id))    C->SetVisibility(Vis);
    if (UListView* LV = GetList(Id))          LV->SetVisibility(Vis);
    if (USlider* S = GetSlider(Id))           S->SetVisibility(Vis);
}

void UBaseScreen::SetEnabled(int32 Id, bool bEnabled)
{
    if (UButton* B = GetButton(Id))           B->SetIsEnabled(bEnabled);
    if (UEditableTextBox* E = GetEdit(Id))    E->SetIsEnabled(bEnabled);
    if (UComboBoxString* C = GetCombo(Id))    C->SetIsEnabled(bEnabled);
    if (UListView* LV = GetList(Id))          LV->SetIsEnabled(bEnabled);
    if (USlider* S = GetSlider(Id))           S->SetIsEnabled(bEnabled);
}

// --------------------------------------------------------------------
// Optional compiled defaults hook
// --------------------------------------------------------------------

void UBaseScreen::ApplyFormDefaults()
{
    // Intentionally empty.
}

// --------------------------------------------------------------------
// Legacy FORM parse + defaults application
// --------------------------------------------------------------------

bool UBaseScreen::ParseLegacyForm(const FString& InText, FParsedForm& OutForm, FString& OutError) const
{
    FFormParser Parser(InText);
    return Parser.ParseForm(OutForm, OutError);
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

bool UBaseScreen::ResolveFont(const FString& InFontName, FSlateFontInfo& OutFont) const
{
    for (const FFormFontMapEntry& E : FontMappings)
    {
        if (E.LegacyName.Equals(InFontName, ESearchCase::IgnoreCase))
        {
            if (!E.Font)
                break;

            OutFont = FSlateFontInfo(E.Font, (E.bOverrideSize && E.Size > 0) ? E.Size : DefaultFontSize);
            return true;
        }
    }

    if (DefaultFont)
    {
        OutFont = FSlateFontInfo(DefaultFont, DefaultFontSize);
        return true;
    }

    return false;
}

void UBaseScreen::ApplyLegacyFormDefaults(const FParsedForm& Parsed)
{
    FSlateFontInfo FormFont;
    bool bHasFormFont = false;

    if (!Parsed.Font.IsEmpty())
    {
        bHasFormFont = ResolveFont(Parsed.Font, FormFont);
    }

    for (const FParsedCtrl& C : Parsed.Controls)
    {
        // LABEL
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

            if (!C.Font.IsEmpty())
            {
                FSlateFontInfo CtrlFont;
                if (ResolveFont(C.Font, CtrlFont))
                    L->SetFont(CtrlFont);
            }
            else if (bHasFormFont)
            {
                L->SetFont(FormFont);
            }

            continue;
        }

        // TEXT (RichText)
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
                FSlateFontInfo CtrlFont;
                if (ResolveFont(C.Font, CtrlFont))
                {
                    FTextBlockStyle Style = T->GetDefaultTextStyle();
                    Style.SetFont(CtrlFont);
                    T->SetDefaultTextStyle(Style);
                }
            }
            else if (bHasFormFont)
            {
                FTextBlockStyle Style = T->GetDefaultTextStyle();
                Style.SetFont(FormFont);
                T->SetDefaultTextStyle(Style);
            }

            continue;
        }

        // SLIDER
        if (C.Type == EFormCtrlType::Slider)
        {
            USlider* S = GetSlider(C.Id);
            if (!S) continue;

            if (C.bHasActiveColor && C.ActiveColor != FLinearColor::Transparent)
            {
                S->SetSliderBarColor(C.ActiveColor);
                S->SetSliderHandleColor(C.ActiveColor);
            }

            continue;
        }
    }
}
