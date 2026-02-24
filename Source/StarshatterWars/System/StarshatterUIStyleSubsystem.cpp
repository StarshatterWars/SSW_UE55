/*=============================================================================
    Project:        Starshatter Wars (Unreal Engine Port)
    Studio:         Fractal Dev Studios
    Copyright:      (C) 2025-2026. All Rights Reserved.

    SUBSYSTEM:      UI Style
    FILE:           StarshatterUIStyleSubsystem.cpp
    AUTHOR:         Carlos Bott
=============================================================================*/

#include "StarshatterUIStyleSubsystem.h"

#include "StarshatterUIStyleSettings.h"

#include "Blueprint/WidgetTree.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/EditableTextBox.h"
#include "Components/PanelWidget.h"
#include "Components/ContentWidget.h"

#include "Engine/Font.h"
#include "Engine/Texture2D.h"

DEFINE_LOG_CATEGORY(LogStarshatterUIStyle);

void UStarshatterUIStyleSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    ReloadFromSettings(true);
    UE_LOG(LogStarshatterUIStyle, Log, TEXT("[UISTYLE] Initialize"));
}

void UStarshatterUIStyleSubsystem::Deinitialize()
{
    UE_LOG(LogStarshatterUIStyle, Log, TEXT("[UISTYLE] Deinitialize"));
    Super::Deinitialize();
}

void UStarshatterUIStyleSubsystem::ReloadFromSettings(bool bLoadNow)
{
    const UStarshatterUIStyleSettings* S = GetDefault<UStarshatterUIStyleSettings>();
    if (!S)
    {
        UE_LOG(LogStarshatterUIStyle, Error, TEXT("[UISTYLE] Settings missing"));
        return;
    }

    auto LoadTex = [bLoadNow](const TSoftObjectPtr<UTexture2D>& Ptr) -> UTexture2D*
        {
            if (Ptr.IsNull()) return nullptr;
            return bLoadNow ? Ptr.LoadSynchronous() : Ptr.Get();
        };

    auto LoadFont = [bLoadNow](const TSoftObjectPtr<UFont>& Ptr) -> UFont*
        {
            if (Ptr.IsNull()) return nullptr;
            return bLoadNow ? Ptr.LoadSynchronous() : Ptr.Get();
        };

    DefaultUIFont = LoadFont(S->DefaultUIFont);
    DefaultTextColor = S->DefaultTextColor;

    TitleUIFont = LoadFont(S->TitleUIFont);
    TitleTextColor = S->TitleTextColor;

    Btn_NormalTex = LoadTex(S->Btn_NormalTex);
    Btn_HoverTex = LoadTex(S->Btn_HoverTex);
    Btn_PressedTex = LoadTex(S->Btn_PressedTex);
    Btn_DisabledTex = LoadTex(S->Btn_DisabledTex);
    Btn_9SliceMargin = S->Btn_9SliceMargin;
    Btn_ImageSize = S->Btn_ImageSize;

    MenuBtn_NormalTex = LoadTex(S->MenuBtn_NormalTex);
    MenuBtn_HoverTex = LoadTex(S->MenuBtn_HoverTex);
    MenuBtn_PressedTex = LoadTex(S->MenuBtn_PressedTex);
    MenuBtn_DisabledTex = LoadTex(S->MenuBtn_DisabledTex);
    MenuBtn_9SliceMargin = S->MenuBtn_9SliceMargin;

    MenuButtonFontSize = S->MenuButtonFontSize;
    MenuButtonTextColor = S->MenuButtonTextColor;

    UE_LOG(LogStarshatterUIStyle, Log, TEXT("[UISTYLE] Reloaded from Project Settings"));
}

FSlateBrush UStarshatterUIStyleSubsystem::MakeBrush(UTexture2D* Tex, const FMargin& Margin, const FVector2D& ImageSize)
{
    FSlateBrush B;

    if (Tex)
    {
        B.SetResourceObject(Tex);
        B.DrawAs = ESlateBrushDrawType::Box; // 9-slice
        B.Margin = Margin;

        if (!ImageSize.IsNearlyZero())
            B.ImageSize = ImageSize;
    }
    else
    {
        B.DrawAs = ESlateBrushDrawType::Box;
        B.TintColor = FSlateColor(FLinearColor(0.18f, 0.18f, 0.18f, 1.f));
        B.Margin = FMargin(4.f / 16.f);
    }

    return B;
}

UTextBlock* UStarshatterUIStyleSubsystem::FindFirstTextBlockRecursive(const UWidget* Root)
{
    if (!Root)
        return nullptr;

    if (const UTextBlock* TB = Cast<UTextBlock>(Root))
        return const_cast<UTextBlock*>(TB);

    if (const UContentWidget* CW = Cast<UContentWidget>(Root))
        return FindFirstTextBlockRecursive(CW->GetContent());

    if (const UPanelWidget* PW = Cast<UPanelWidget>(Root))
    {
        const int32 N = PW->GetChildrenCount();
        for (int32 i = 0; i < N; ++i)
        {
            if (UTextBlock* Found = FindFirstTextBlockRecursive(PW->GetChildAt(i)))
                return Found;
        }
    }

    return nullptr;
}

void UStarshatterUIStyleSubsystem::ApplyButtonStyleInternal(
    UButton* Button,
    UTexture2D* Normal,
    UTexture2D* Hover,
    UTexture2D* Pressed,
    UTexture2D* Disabled,
    const FMargin& SliceMargin,
    const FVector2D& ImageSize,
    UFont* Font,
    int32 FontSize,
    const FLinearColor& TextColor
)
{
    if (!Button)
        return;

    const FSlateBrush NormalB = MakeBrush(Normal, SliceMargin, ImageSize);
    const FSlateBrush HoverB = MakeBrush(Hover, SliceMargin, ImageSize);
    const FSlateBrush PressB = MakeBrush(Pressed, SliceMargin, ImageSize);
    const FSlateBrush DisableB = MakeBrush(Disabled ? Disabled : Normal, SliceMargin, ImageSize);

    FButtonStyle Style = Button->WidgetStyle;
    Style.SetNormal(NormalB);
    Style.SetHovered(HoverB);
    Style.SetPressed(PressB);
    Style.SetDisabled(DisableB);

#if ENGINE_MAJOR_VERSION >= 5
    Button->SetStyle(Style);
#else
    Button->WidgetStyle = Style;
    Button->SynchronizeProperties();
#endif

    UTextBlock* TextChild = FindFirstTextBlockRecursive(Button);
    if (!TextChild)
        return;

    TextChild->SetJustification(ETextJustify::Center);
    TextChild->SetColorAndOpacity(FSlateColor(TextColor));

    if (Font)
    {
        FSlateFontInfo FI = TextChild->GetFont();
        FI.FontObject = Font;
        FI.Size = FontSize;
        TextChild->SetFont(FI);
    }

    TextChild->SynchronizeProperties();
}

void UStarshatterUIStyleSubsystem::ApplyDefaultButtonStyle(UButton* Button, int32 FontSize)
{
    ApplyButtonStyleInternal(
        Button,
        Btn_NormalTex,
        Btn_HoverTex,
        Btn_PressedTex,
        Btn_DisabledTex,
        Btn_9SliceMargin,
        Btn_ImageSize,
        DefaultUIFont,
        FontSize,
        DefaultTextColor
    );
}

void UStarshatterUIStyleSubsystem::ApplyDefaultButtonStyle_Override(
    UButton* Button,
    int32 FontSize,
    bool bOverrideTextColor,
    FLinearColor OverrideTextColor,
    UFont* OverrideFont
)
{
    const FLinearColor FinalColor = bOverrideTextColor ? OverrideTextColor : DefaultTextColor;
    UFont* FinalFont = OverrideFont ? OverrideFont : DefaultUIFont.Get();

    ApplyButtonStyleInternal(
        Button,
        Btn_NormalTex,
        Btn_HoverTex,
        Btn_PressedTex,
        Btn_DisabledTex,
        Btn_9SliceMargin,
        Btn_ImageSize,
        FinalFont,
        FontSize,
        FinalColor
    );
}

void UStarshatterUIStyleSubsystem::ApplyMenuButtonStyle(UButton* Button)
{
    // Menu uses TitleUIFont if set, else DefaultUIFont.
    UFont* FontToUse = TitleUIFont ? TitleUIFont : DefaultUIFont;

    ApplyButtonStyleInternal(
        Button,
        MenuBtn_NormalTex,
        MenuBtn_HoverTex,
        MenuBtn_PressedTex,
        MenuBtn_DisabledTex,
        MenuBtn_9SliceMargin,
        Btn_ImageSize,
        FontToUse,
        MenuButtonFontSize,
        MenuButtonTextColor
    );
}

void UStarshatterUIStyleSubsystem::ApplyDefaultTextStyle(UTextBlock* Text, int32 FontSize)
{
    if (!Text)
        return;

    Text->SetColorAndOpacity(FSlateColor(DefaultTextColor));

    if (DefaultUIFont)
    {
        FSlateFontInfo FI = Text->GetFont();
        FI.FontObject = DefaultUIFont;
        FI.Size = FontSize;
        Text->SetFont(FI);
    }
}

void UStarshatterUIStyleSubsystem::ApplyTitleTextStyle(UTextBlock* Text, int32 FontSize)
{
    if (!Text)
        return;

    Text->SetColorAndOpacity(FSlateColor(TitleTextColor));

    UFont* FontToUse = TitleUIFont ? TitleUIFont : DefaultUIFont;
    if (FontToUse)
        Text->SetFont(FSlateFontInfo(FontToUse, FontSize));
}

void UStarshatterUIStyleSubsystem::ApplyDefaultEditBoxStyle(UEditableTextBox* Edit, int32 FontSize)
{
    if (!Edit)
        return;

    FEditableTextBoxStyle& Style = Edit->WidgetStyle;

    if (DefaultUIFont)
    {
        Style.TextStyle.SetFont(FSlateFontInfo(DefaultUIFont, FontSize));
        Style.TextStyle.SetColorAndOpacity(FSlateColor(DefaultTextColor));
    }

    FSlateBrush Bg;
    Bg.DrawAs = ESlateBrushDrawType::Box;
    Bg.TintColor = FSlateColor(FLinearColor(0.18f, 0.18f, 0.18f, 1.f));
    Bg.Margin = FMargin(4.f / 16.f);

    Style.SetBackgroundImageNormal(Bg);
    Style.SetBackgroundImageHovered(Bg);
    Style.SetBackgroundImageFocused(Bg);
    Style.SetBackgroundImageReadOnly(Bg);

    Edit->SynchronizeProperties();
}