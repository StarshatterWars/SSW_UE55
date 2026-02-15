/*=============================================================================
    Project:        Starshatter Wars
    Studio:         Fractal Dev Studios
    Copyright:      (c) 2025-2026.
    All Rights Reserved.

    SUBSYSTEM:      StarshatterWars (Unreal Engine)
    FILE:           KeyDlg.h
    AUTHOR:         Carlos Bott

    OVERVIEW
    ========
    UE-only key binding dialog.
    - Captures a key via NativeOnKeyDown.
    - Writes to UStarshatterKeyboardSettings (config-backed CDO).
    - Runtime apply via UStarshatterKeyboardSubsystem.
    - Routes back to UOptionsScreen (Manager) on apply/cancel.

    IMPORTANT
    =========
    - Uses AddUniqueDynamic (NO RemoveAll) to prevent delegate ensure failures.
    - KeyDlg never manipulates page visibility directly; OptionsScreen owns routing.
=============================================================================*/

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

    // Legacy parity:
    void ExecFrame();

    // Compatibility: treat this as enum index for EStarshatterInputAction
    int  GetKeyMapIndex() const { return KeyIndex; }
    void SetKeyMapIndex(int i);

    // Preferred:
    void SetEditingAction(EStarshatterInputAction InAction);

    // Manager:
    void SetOptionsManager(UOptionsScreen* InManager);

    // Capture flow:
    void BeginCapture();

protected:
    virtual void HandleAccept() override;
    virtual void HandleCancel() override;

private:
    void BindDelegatesOnce();
    void RefreshDisplayFromSettings();
    void SetTextBlock(UTextBlock* Block, const FString& Text);

    static FString ActionToDisplayString(EStarshatterInputAction Action);
    static FString KeyToDisplayString(const FKey& Key);

    void CommitPendingToSettings();
    void ClearFromSettings();

private:

    bool bDelegatesBound = false;

    // Compatibility:
    int KeyIndex = 0;

    // UE-only binding state:
    EStarshatterInputAction EditingAction = EStarshatterInputAction::Pause;

    FKey CurrentKey;
    FKey PendingKey;

    bool bCapturing = false;
    bool bKeyClear = false;

protected:
    UPROPERTY(meta = (BindWidgetOptional)) TObjectPtr<UTextBlock> CommandText;     // 201
    UPROPERTY(meta = (BindWidgetOptional)) TObjectPtr<UTextBlock> CurrentKeyText; // 202
    UPROPERTY(meta = (BindWidgetOptional)) TObjectPtr<UTextBlock> NewKeyText;     // 203

    UPROPERTY(meta = (BindWidgetOptional)) TObjectPtr<UButton> ClearButton;       // 300
    UPROPERTY(meta = (BindWidgetOptional)) TObjectPtr<UButton> ApplyBtn;
    UPROPERTY(meta = (BindWidgetOptional)) TObjectPtr<UButton> CancelBtn;

private:
    UFUNCTION() void OnApplyClicked();
    UFUNCTION() void OnCancelClicked();
    UFUNCTION() void OnClearClicked();
};
