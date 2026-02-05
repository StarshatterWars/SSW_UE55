/*  Project Starshatter Wars
    Fractal Dev Studios
    Copyright (c) 2025-2026.

    SUBSYSTEM:    StarshatterWars (Unreal Engine)
    FILE:         KeyDlg.h
    AUTHOR:       Carlos Bott

    OVERVIEW
    ========
    UE-only key binding dialog.
    - Captures a key via NativeOnKeyDown.
    - Writes to UStarshatterKeyboardSettings (config-backed CDO).
    - Runtime apply via UStarshatterKeyboardSubsystem.
    - Routes back to UOptionsScreen (Manager) on apply/cancel.
*/

#pragma once

#include "CoreMinimal.h"
#include "BaseScreen.h"
#include "InputCoreTypes.h"

#include "GameStructs.h" // EStarshatterInputAction

#include "KeyDlg.generated.h"

class UButton;
class UTextBlock;
class UOptionsScreen;

UCLASS()
class STARSHATTERWARS_API UKeyDlg : public UBaseScreen
{
    GENERATED_BODY()

public:
    UKeyDlg(const FObjectInitializer& ObjectInitializer);

    virtual void NativeOnInitialized() override;
    virtual void NativeConstruct() override;
    virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

    // Key capture:
    virtual FReply NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;

    // Keep legacy parity signature for now:
    void ExecFrame();

    // Compatibility: treat this as enum index for EStarshatterInputAction
    int  GetKeyMapIndex() const { return KeyIndex; }
    void SetKeyMapIndex(int i);

    // New preferred API:
    void SetEditingAction(EStarshatterInputAction InAction);

    // Manager:
    void SetManager(UOptionsScreen* InManager);

    // Capture flow:
    void BeginCapture();

protected:
    UFUNCTION() void OnApplyClicked();
    UFUNCTION() void OnCancelClicked();
    UFUNCTION() void OnClearClicked();

private:
    void RefreshDisplayFromSettings();
    void SetTextBlock(UTextBlock* Block, const FString& Text);

    static FString ActionToDisplayString(EStarshatterInputAction Action);
    static FString KeyToDisplayString(const FKey& Key);

    void CommitPendingToSettings();
    void ClearFromSettings();

private:
    UPROPERTY(Transient)
    TObjectPtr<UOptionsScreen> Manager = nullptr;

    // Compatibility:
    int KeyIndex = 0;

    // UE-only binding state:
    EStarshatterInputAction EditingAction = EStarshatterInputAction::Pause;

    FKey CurrentKey;
    FKey PendingKey;

    bool bCapturing = false;
    bool bKeyClear = false;

protected:
    UPROPERTY(meta = (BindWidgetOptional)) UTextBlock* CommandText = nullptr;     // 201
    UPROPERTY(meta = (BindWidgetOptional)) UTextBlock* CurrentKeyText = nullptr; // 202
    UPROPERTY(meta = (BindWidgetOptional)) UTextBlock* NewKeyText = nullptr;     // 203

    UPROPERTY(meta = (BindWidgetOptional)) UButton* ClearButton = nullptr;       // 300

    UPROPERTY(meta = (BindWidgetOptional)) UButton* ApplyBtn = nullptr;
    UPROPERTY(meta = (BindWidgetOptional)) UButton* CancelBtn = nullptr;
};
