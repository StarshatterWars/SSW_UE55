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
    - Provides FORM-style ID binding and operations helpers.
    - Includes data structures for legacy .frm parsing (form/ctrl/defctrl).
    - defctrl behavior is implemented in BaseScreen.cpp (current-defaults merge).
    - Adds unified dialog input helpers: HandleAccept() / HandleCancel()
      so every dialog behaves identically (Enter/Escape).
    - Adds RichText (FORM type: text) support using URichTextBlock.
*/

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"

// UMG components:
#include "Components/Image.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/RichTextBlock.h"
#include "Components/ComboBoxString.h"
#include "Components/EditableTextBox.h"
#include "Components/ListView.h"
#include "Components/Slider.h"

// Input:
#include "Input/Reply.h"
#include "InputCoreTypes.h"

#include "Engine/Font.h"

#include "BaseScreen.generated.h"

// ====================================================================
//  FORM PARSE TYPES
// ====================================================================

USTRUCT(BlueprintType)
struct FFormFontMapEntry
{
    GENERATED_BODY()

    // Legacy FORM font name, e.g. "Limerick12", "Verdana", "HUD", "GUI"
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    FString LegacyName;

    // Unreal font asset (UFont). You can use a Font asset from your content.
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    TObjectPtr<UFont> Font = nullptr;

    // If > 0 and bOverrideSize is true, force this size.
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
    FFormIntRect(int32 InA, int32 InB, int32 InC, int32 InD) : A(InA), B(InB), C(InC), D(InD) {}
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
    // Internal “explicitness” flags (NOT UPROPERTY, not for Blueprint)
    // Used to merge defctrl defaults correctly in BaseScreen.cpp.
    // -----------------------------------------------------------------
    bool bHasText = false;
    bool bHasTexture = false;
    bool bHasFont = false;
    bool bHasAlign = false;
    bool bHasBackColor = false;
    bool bHasForeColor = false;

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

    UPROPERTY(EditAnywhere, BlueprintReadOnly) FString Texture;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) FFormIntRect Margins;

    UPROPERTY(EditAnywhere, BlueprintReadOnly) FFormLayout Layout;

    UPROPERTY(EditAnywhere, BlueprintReadOnly) TArray<FParsedCtrl> Controls;
};

// ====================================================================
//  OPTIONAL COMPILED FORM DEF TYPES
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
    // ----------------------------------------------------------------
    //  Optional common widgets (bind if your UMG has them)
    // ----------------------------------------------------------------
    UPROPERTY(meta = (BindWidgetOptional)) UTextBlock* Title = nullptr;
    UPROPERTY(meta = (BindWidgetOptional)) UTextBlock* CancelButtonText = nullptr;
    UPROPERTY(meta = (BindWidgetOptional)) UTextBlock* ApplyButtonText = nullptr;

    // These are used by HandleAccept/HandleCancel in BaseScreen.cpp:
    UPROPERTY(meta = (BindWidgetOptional)) UButton* ApplyButton = nullptr;
    UPROPERTY(meta = (BindWidgetOptional)) UButton* CancelButton = nullptr;

public:
    // ----------------------------------------------------------------
    //  FORM-style API
    // ----------------------------------------------------------------
    /** Override in derived screens to bind legacy FORM IDs to widgets */
    virtual void BindFormWidgets() {}

    /** Optional: provide compiled FORM defaults (labels, etc.) */
    virtual const FFormDef* GetFormDef() const { return nullptr; }

    /** Optional: provide raw legacy .frm text to parse and apply defaults */
    virtual FString GetLegacyFormText() const { return FString(); }

public:
    // ----------------------------------------------------------------
    //  Dialog input helpers (Enter/Escape)
    // ----------------------------------------------------------------
    virtual void HandleAccept();
    virtual void HandleCancel();

    UFUNCTION(BlueprintCallable, Category = "Dialog")
    void SetDialogInputEnabled(bool bEnabled) { bDialogInputEnabled = bEnabled; }

    // Optional: fill this in Defaults to map FORM font names to assets.
    UPROPERTY(EditAnywhere, Category = "FORM|Fonts")
    TArray<FFormFontMapEntry> FontMappings;

    // Fallback if a FORM font name is not mapped.
    UPROPERTY(EditAnywhere, Category = "FORM|Fonts")
    TObjectPtr<UFont> DefaultFont = nullptr;

    // Fallback size if not inferable from name and not overridden.
    UPROPERTY(EditAnywhere, Category = "FORM|Fonts")
    int32 DefaultFontSize = 12;

protected:
    // ----------------------------------------------------------------
    //  Binding helpers (call from BindFormWidgets)
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
    //  Operations helpers
    // ----------------------------------------------------------------
    void SetLabelText(int32 Id, const FText& Text);
    void SetEditText(int32 Id, const FText& Text);
    void SetVisible(int32 Id, bool bVisible);
    void SetEnabled(int32 Id, bool bEnabled);

protected:
    // ----------------------------------------------------------------
    //  Lookup helpers
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
    //  Default application
    // ----------------------------------------------------------------
    void ApplyFormDefaults();

protected:
    // ----------------------------------------------------------------
    //  Legacy FORM parse + defaults application (supports defctrl)
    // ----------------------------------------------------------------
    bool ParseLegacyForm(const FString& InText, FParsedForm& OutForm, FString& OutError) const;
    void ApplyLegacyFormDefaults(const FParsedForm& Parsed);

protected:
    // ----------------------------------------------------------------
    //  Optional mapping hooks
    // ----------------------------------------------------------------
    virtual bool ResolveFont(const FString& InFontName, FSlateFontInfo& OutFont) const;
    virtual bool ResolveTextJustification(EFormAlign InAlign, ETextJustify::Type& OutJustify) const;

protected:
    // ----------------------------------------------------------------
    //  Internal state
    // ----------------------------------------------------------------
    UPROPERTY(Transient)
    FFormWidgetMap FormMap;

    UPROPERTY(Transient)
    FParsedForm ParsedForm;

protected:
    // ----------------------------------------------------------------
    //  Dialog policy
    // ----------------------------------------------------------------
    UPROPERTY(EditAnywhere, Category = "Dialog")
    bool bDialogInputEnabled = true;

    UPROPERTY(Transient)
    bool bExitLatch = false;

protected:
    // ----------------------------------------------------------------
    //  UUserWidget lifecycle
    // ----------------------------------------------------------------
    virtual void NativeOnInitialized() override;
    virtual void NativeConstruct() override;
    virtual void NativePreConstruct() override;
    virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

protected:
    // ----------------------------------------------------------------
    //  Centralized key handling (Enter/Escape)
    // ----------------------------------------------------------------
    virtual FReply NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;
};
