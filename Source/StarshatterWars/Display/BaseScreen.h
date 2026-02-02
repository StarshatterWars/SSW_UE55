/*  Project Starshatter Wars
    Fractal Dev Studios
    Copyright (c) 2025-2026.

    SUBSYSTEM:    Stars.exe
    FILE:         BaseScreen.h
    AUTHOR:       Carlos Bott

    OVERVIEW
    ========
    UBaseScreen
    - Unreal base widget for Starshatter menu/dialog screens.
    - Provides FORM-style ID binding + helper ops (SetLabelText, SetVisible, etc).
    - Includes legacy .frm parse structures (form/ctrl/defctrl) and merge support.
    - Adds unified dialog input hooks: HandleAccept() / HandleCancel()
      so every dialog behaves identically (Enter/Escape).
    - Adds RichText support for legacy FORM type: text (URichTextBlock).
    - Adds FORM font mapping support (LegacyName -> UFont + size).
    - Adds slider active_color support (FORM: active_color).
*/

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"

// Input:
#include "Input/Reply.h"
#include "InputCoreTypes.h"

// Styling / Fonts:
#include "Fonts/SlateFontInfo.h"
#include "Styling/SlateTypes.h"
#include "Engine/Font.h"

// UMG:
#include "Components/Button.h"
#include "Components/ComboBoxString.h"
#include "Components/EditableTextBox.h"
#include "Components/Image.h"
#include "Components/ListView.h"
#include "Components/RichTextBlock.h"
#include "Components/Slider.h"
#include "Components/TextBlock.h"

#include "BaseScreen.generated.h"

// ====================================================================
//  LEGACY FORM PARSE TYPES
// ====================================================================

USTRUCT(BlueprintType)
struct FFormFontMapEntry
{
    GENERATED_BODY()

    // Legacy FORM font name, e.g. "Limerick12", "Verdana", "HUD", "GUI"
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    FString LegacyName;

    // Unreal font asset (UFont).
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    TObjectPtr<UFont> Font = nullptr;

    // If bOverrideSize is true and Size > 0, force this size.
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    int32 Size = 0;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    bool bOverrideSize = false;
};

UENUM(BlueprintType)
enum class EFormCtrlType : uint8
{
    None,
    Label,
    Button,
    Image,
    Edit,
    Combo,
    List,
    Slider,
    Panel,
    Background,

    // Legacy "text" control (treated as RichText in Unreal):
    Text
};

UENUM(BlueprintType)
enum class EFormAlign : uint8
{
    None,
    Left,
    Center,
    Right
};

USTRUCT(BlueprintType)
struct FFormIntRect
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly) int32 A = 0;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) int32 B = 0;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) int32 C = 0;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) int32 D = 0;

    FFormIntRect() = default;
    FFormIntRect(int32 InA, int32 InB, int32 InC, int32 InD)
        : A(InA), B(InB), C(InC), D(InD) {
    }
};

USTRUCT(BlueprintType)
struct FFormLayout
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly) TArray<int32> XMins;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) TArray<float> XWeights;

    UPROPERTY(EditAnywhere, BlueprintReadOnly) TArray<int32> YMins;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) TArray<float> YWeights;

    bool IsEmpty() const
    {
        return XMins.Num() == 0 && XWeights.Num() == 0 && YMins.Num() == 0 && YWeights.Num() == 0;
    }
};

USTRUCT(BlueprintType)
struct FFormColumnDef
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly) FString Title;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) int32 Width = 0;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) EFormAlign Align = EFormAlign::Left;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) int32 Sort = 0;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) FLinearColor Color = FLinearColor::Transparent;
};

USTRUCT(BlueprintType)
struct FParsedCtrl
{
    GENERATED_BODY()

    // ---------------- Identity / hierarchy ----------------
    UPROPERTY(EditAnywhere, BlueprintReadOnly) int32 Id = -1;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) int32 ParentId = -1; // pid:
    UPROPERTY(EditAnywhere, BlueprintReadOnly) EFormCtrlType Type = EFormCtrlType::None;

    // ---------------- Visuals / text ----------------
    UPROPERTY(EditAnywhere, BlueprintReadOnly) FString Text;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) FString Texture;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) FString Font;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) EFormAlign Align = EFormAlign::None;

    UPROPERTY(EditAnywhere, BlueprintReadOnly) FLinearColor BackColor = FLinearColor::Transparent;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) FLinearColor ForeColor = FLinearColor::Transparent;

    // Slider colors (legacy):
    UPROPERTY(EditAnywhere, BlueprintReadOnly) FLinearColor ActiveColor = FLinearColor::Transparent;

    UPROPERTY(EditAnywhere, BlueprintReadOnly) bool bTransparent = false;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) bool bSticky = false;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) bool bBorder = false;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) bool bHidePartial = false;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) bool bShowHeadings = false;

    // ---------------- Images / bevel ----------------
    UPROPERTY(EditAnywhere, BlueprintReadOnly) FString StandardImage;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) FString ActivatedImage;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) FString TransitionImage;

    UPROPERTY(EditAnywhere, BlueprintReadOnly) int32 BevelWidth = 0;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) int32 BevelDepth = 0;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) FLinearColor BorderColor = FLinearColor::Transparent;

    // ---------------- Sizing ----------------
    UPROPERTY(EditAnywhere, BlueprintReadOnly) int32 FixedWidth = 0;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) int32 FixedHeight = 0;

    // ---------------- Grid placement ----------------
    UPROPERTY(EditAnywhere, BlueprintReadOnly) FFormIntRect Cells;       // (x,y,w,h)
    UPROPERTY(EditAnywhere, BlueprintReadOnly) FFormIntRect CellInsets;  // (l,t,r,b)

    // ---------------- Panel/list margins ----------------
    UPROPERTY(EditAnywhere, BlueprintReadOnly) FFormIntRect Margins;     // (l,t,r,b)

    // ---------------- List style ----------------
    UPROPERTY(EditAnywhere, BlueprintReadOnly) int32 ScrollBar = 0;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) int32 Style = 0;

    // ---------------- Per-ctrl layout block ----------------
    UPROPERTY(EditAnywhere, BlueprintReadOnly) FFormLayout Layout;

    // ---------------- List columns ----------------
    UPROPERTY(EditAnywhere, BlueprintReadOnly) TArray<FFormColumnDef> Columns;

    // -----------------------------------------------------------------
    // Internal "explicitness" flags (NOT UPROPERTY; not for Blueprint).
    // Used to merge defctrl defaults correctly in BaseScreen.cpp.
    // -----------------------------------------------------------------
    bool bHasText = false;
    bool bHasTexture = false;
    bool bHasFont = false;
    bool bHasAlign = false;
    bool bHasBackColor = false;
    bool bHasForeColor = false;
    bool bHasActiveColor = false;

    bool bHasTransparent = false;
    bool bHasSticky = false;
    bool bHasBorder = false;
    bool bHasHidePartial = false;
    bool bHasShowHeadings = false;

    bool bHasStandardImage = false;
    bool bHasActivatedImage = false;
    bool bHasTransitionImage = false;

    bool bHasBevelWidth = false;
    bool bHasBevelDepth = false;
    bool bHasBorderColor = false;

    bool bHasFixedWidth = false;
    bool bHasFixedHeight = false;

    bool bHasCells = false;
    bool bHasCellInsets = false;
    bool bHasMargins = false;

    bool bHasScrollBar = false;
    bool bHasStyle = false;
    bool bHasLayout = false;
};

USTRUCT(BlueprintType)
struct FParsedForm
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly) FLinearColor BackColor = FLinearColor::Black;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) FLinearColor ForeColor = FLinearColor::White;

    UPROPERTY(EditAnywhere, BlueprintReadOnly) FString Font;

    UPROPERTY(EditAnywhere, BlueprintReadOnly) FString Texture;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) FFormIntRect Margins;

    UPROPERTY(EditAnywhere, BlueprintReadOnly) FFormLayout Layout;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) TArray<FParsedCtrl> Controls;
};

// ====================================================================
//  OPTIONAL COMPILED FORM DEF TYPES (DOCUMENTATION / FUTURE USE)
// ====================================================================

USTRUCT(BlueprintType)
struct FFormControlDef
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly) int32 Id = -1;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) EFormCtrlType Type = EFormCtrlType::None;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) FString Text;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) FString Texture;

    // Legacy grid info (documentation / future auto-layout)
    UPROPERTY(EditAnywhere, BlueprintReadOnly) int32 CellX = 0;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) int32 CellY = 0;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) int32 CellW = 1;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) int32 CellH = 1;
};

USTRUCT(BlueprintType)
struct FFormDef
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly) FName Name = NAME_None;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) TArray<FFormControlDef> Controls;
};

// ====================================================================
//  WIDGET MAP (ID -> UMG WIDGET)
// ====================================================================

USTRUCT()
struct FFormWidgetMap
{
    GENERATED_BODY()

    UPROPERTY(Transient) TMap<int32, TObjectPtr<UTextBlock>>       Labels;
    UPROPERTY(Transient) TMap<int32, TObjectPtr<URichTextBlock>>   Texts;
    UPROPERTY(Transient) TMap<int32, TObjectPtr<UButton>>          Buttons;
    UPROPERTY(Transient) TMap<int32, TObjectPtr<UImage>>           Images;
    UPROPERTY(Transient) TMap<int32, TObjectPtr<UEditableTextBox>> Edits;
    UPROPERTY(Transient) TMap<int32, TObjectPtr<UComboBoxString>>  Combos;
    UPROPERTY(Transient) TMap<int32, TObjectPtr<UListView>>        Lists;
    UPROPERTY(Transient) TMap<int32, TObjectPtr<USlider>>          Sliders;

    void Reset()
    {
        Labels.Reset();
        Texts.Reset();
        Buttons.Reset();
        Images.Reset();
        Edits.Reset();
        Combos.Reset();
        Lists.Reset();
        Sliders.Reset();
    }
};

// ====================================================================
//  UBaseScreen
// ====================================================================

UCLASS()
class STARSHATTERWARS_API UBaseScreen : public UUserWidget
{
    GENERATED_BODY()

public:
    UBaseScreen(const FObjectInitializer& ObjectInitializer);

    virtual void ExecFrame(double DeltaTime);
    
    // Visibility / flow:
    virtual bool IsShown()  const { return bIsShown; }
    virtual void Show();
    virtual void Hide();
    virtual void HideAll();

    // ----------------------------------------------------------------
    // Optional common widgets (bind if your UMG has them)
    // ----------------------------------------------------------------
    UPROPERTY(meta = (BindWidgetOptional)) TObjectPtr<UTextBlock> Title = nullptr;
    UPROPERTY(meta = (BindWidgetOptional)) TObjectPtr<UTextBlock> CancelButtonText = nullptr;
    UPROPERTY(meta = (BindWidgetOptional)) TObjectPtr<UTextBlock> ApplyButtonText = nullptr;

    // Used by HandleAccept / HandleCancel (optional)
    UPROPERTY(meta = (BindWidgetOptional)) TObjectPtr<UButton> ApplyButton = nullptr;
    UPROPERTY(meta = (BindWidgetOptional)) TObjectPtr<UButton> CancelButton = nullptr;

    // ----------------------------------------------------------------
    // FORM-style API (override per-screen as needed)
    // ----------------------------------------------------------------
    virtual void BindFormWidgets() {}
    virtual const FFormDef* GetFormDef() const { return nullptr; }
    virtual FString GetLegacyFormText() const { return FString(); }

    // ----------------------------------------------------------------
    // Dialog input hooks (Enter/Escape)
    // Override in derived dialogs.
    // ----------------------------------------------------------------
    virtual void HandleAccept();
    virtual void HandleCancel();

    UFUNCTION(BlueprintCallable, Category = "Dialog")
    void SetDialogInputEnabled(bool bEnabled) { bDialogInputEnabled = bEnabled; }

    // ----------------------------------------------------------------
    // Fonts: LegacyName -> UFont (+ optional size override)
    // ----------------------------------------------------------------
    UPROPERTY(EditAnywhere, Category = "FORM|Fonts")
    TArray<FFormFontMapEntry> FontMappings;

    UPROPERTY(EditAnywhere, Category = "FORM|Fonts")
    TObjectPtr<UFont> DefaultFont = nullptr;

    UPROPERTY(EditAnywhere, Category = "FORM|Fonts")
    int32 DefaultFontSize = 12;

protected:
    // ----------------------------------------------------------------
    // Binding helpers (call from BindFormWidgets)
    // ----------------------------------------------------------------
    void BindLabel(int32 Id, UTextBlock* Widget);
    void BindText(int32 Id, URichTextBlock* Widget);
    void BindButton(int32 Id, UButton* Widget);
    void BindImage(int32 Id, UImage* Widget);
    void BindEdit(int32 Id, UEditableTextBox* Widget);
    void BindCombo(int32 Id, UComboBoxString* Widget);
    void BindList(int32 Id, UListView* Widget);
    void BindSlider(int32 Id, USlider* Widget);

protected:
    // ----------------------------------------------------------------
    // Operations helpers
    // ----------------------------------------------------------------
    void SetLabelText(int32 Id, const FText& Text);
    void SetEditText(int32 Id, const FText& Text);
    void SetVisible(int32 Id, bool bVisible);
    void SetEnabled(int32 Id, bool bEnabled);

protected:
    // ----------------------------------------------------------------
    // Lookup helpers
    // ----------------------------------------------------------------
    UTextBlock* GetLabel(int32 Id) const;
    URichTextBlock* GetText(int32 Id) const;
    UButton* GetButton(int32 Id) const;
    UImage* GetImage(int32 Id) const;
    UEditableTextBox* GetEdit(int32 Id) const;
    UComboBoxString* GetCombo(int32 Id) const;
    UListView* GetList(int32 Id) const;
    USlider* GetSlider(int32 Id) const;

protected:
    // ----------------------------------------------------------------
    // Defaults application
    // ----------------------------------------------------------------
    void ApplyFormDefaults();

protected:
    // ----------------------------------------------------------------
    // Legacy FORM parse + defaults application (supports defctrl)
    // ----------------------------------------------------------------
    bool ParseLegacyForm(const FString& InText, FParsedForm& OutForm, FString& OutError) const;
    void ApplyLegacyFormDefaults(const FParsedForm& Parsed);

protected:
    // ----------------------------------------------------------------
    // Optional mapping hooks
    // ----------------------------------------------------------------
    virtual bool ResolveFont(const FString& InFontName, FSlateFontInfo& OutFont) const;
    virtual bool ResolveTextJustification(EFormAlign InAlign, ETextJustify::Type& OutJustify) const;

protected:
    // ----------------------------------------------------------------
    // UUserWidget lifecycle
    // ----------------------------------------------------------------
    virtual void NativeOnInitialized() override;
    virtual void NativePreConstruct() override;
    virtual void NativeConstruct() override;
    virtual void NativeDestruct() override;
    virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

protected:
    // ----------------------------------------------------------------
    // Centralized key handling (Enter/Escape)
    // ----------------------------------------------------------------
    virtual FReply NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;

protected:
    // ----------------------------------------------------------------
    // Dialog policy
    // ----------------------------------------------------------------
    UPROPERTY(EditAnywhere, Category = "Dialog")
    bool bDialogInputEnabled = true;

    // If you need the old "exit latch" behavior, flip true on show and clear next tick.
    UPROPERTY(Transient)
    bool bExitLatch = false;

protected:
    // ----------------------------------------------------------------
    // Internal state
    // ----------------------------------------------------------------
    UPROPERTY(Transient) FFormWidgetMap FormMap;
    UPROPERTY(Transient) FParsedForm ParsedForm;

private:
    UPROPERTY()
    bool  bIsShown;
};
