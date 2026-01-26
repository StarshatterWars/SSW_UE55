/*  Project Starshatter Wars
    Fractal Dev Studios
    Copyright (c) 2025-2026.

    SUBSYSTEM:    Stars.exe
    FILE:         KeyDlg.h
    AUTHOR:       Carlos Bott

    ORIGINAL AUTHOR AND STUDIO
    ==========================
    John DiCamillo / Destroyer Studios LLC

    OVERVIEW
    ========
    Key Binding dialog (legacy KeyDlg) adapted for Unreal UMG.
*/

#pragma once

#include "CoreMinimal.h"
#include "BaseScreen.h"
#include "MenuScreen.h"


// Minimal Unreal includes requested for headers:
#include "Math/UnrealMathUtility.h"
#include "Math/Vector.h"
#include "Math/Color.h"

#include "KeyDlg.generated.h"
// ------------------------------------------------------------
// Forward declarations
// ------------------------------------------------------------
class UButton;
class UTextBlock;

// ------------------------------------------------------------

UCLASS()
class STARSHATTERWARS_API UKeyDlg : public UBaseScreen
{
    GENERATED_BODY()

public:
    UKeyDlg(const FObjectInitializer& ObjectInitializer);

    virtual void NativeOnInitialized() override;
    virtual void NativeConstruct() override;
    virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

    // Legacy parity:
    void ExecFrame();

    int  GetKeyMapIndex() const { return KeyIndex; }
    void SetKeyMapIndex(int i);

    void SetManager(UMenuScreen* InManager);

protected:
    UFUNCTION()
    void OnApplyClicked();

    UFUNCTION()
    void OnCancelClicked();

    UFUNCTION()
    void OnClearClicked();

protected:
    // --------------------------------------------------------
    // Bound UMG widgets (matching legacy IDs conceptually)
    // --------------------------------------------------------
    UPROPERTY(meta = (BindWidgetOptional))
    UTextBlock* CommandText = nullptr;      // id 201

    UPROPERTY(meta = (BindWidgetOptional))
    UTextBlock* CurrentKeyText = nullptr;   // id 202

    UPROPERTY(meta = (BindWidgetOptional))
    UTextBlock* NewKeyText = nullptr;       // id 203

    UPROPERTY(meta = (BindWidgetOptional))
    UButton* ClearButton = nullptr;         // id 300

protected:
    // Manager screen:
    UMenuScreen* Manager = nullptr;

    // Legacy fields:
    int  KeyIndex = 0;
    int  KeyKey = 0;
    int  KeyShift = 0;
    int  KeyJoy = 0;
    bool bKeyClear = false;

private:
    void RefreshDisplayFromCurrentBinding();
    void SetTextBlock(UTextBlock* Block, const char* AnsiText);
    void SetTextBlock(UTextBlock* Block, const FString& Text);
};
