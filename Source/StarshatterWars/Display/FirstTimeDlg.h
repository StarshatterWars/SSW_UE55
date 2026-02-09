/*  Project Starshatter Wars
    Fractal Dev Studios
    Copyright (c) 2025–2026.

    DIALOG:      FirstTimeDlg
    FILE:        FirstTimeDlg.h
    AUTHOR:      Carlos Bott

    OVERVIEW
    ========
    First-time player setup dialog.

    Unreal UMG replacement for legacy FirstTimeDlg.frm.
    Handles:
      - Player name creation
      - Play style selection (Arcade / Standard)
      - Experience level (Cadet / Admiral)
      - Initial key bindings
      - Player save
*/

#pragma once 

#include "CoreMinimal.h"
#include "BaseScreen.h"
#include "FirstTimeDlg.generated.h"

class UButton;
class UEditableTextBox;
class UComboBoxString;
class UMenuScreen;

/**
 * UFirstTimeDlg
 * Unreal UMG replacement for legacy FirstTimeDlg (FirstTimeDlg.frm + OnApply).
 */
UCLASS(BlueprintType, Blueprintable)
class STARSHATTERWARS_API UFirstTimeDlg : public UBaseScreen
{
    GENERATED_BODY()

public:
    UFirstTimeDlg(const FObjectInitializer& ObjectInitializer);

    // Set by MenuScreen after CreateWidget (like MenuDlg->Manager)
    UPROPERTY() TObjectPtr<UMenuScreen> Manager;

protected:
    virtual void NativeConstruct() override;

    UFUNCTION()
    void OnAcceptClicked();

    void PopulateDefaultsIfNeeded();

    // --- BindWidget controls (must match names in WBP_FirstTimeDlg) ---
    UPROPERTY(meta = (BindWidgetOptional)) TObjectPtr<UEditableTextBox> NameEdit;
    UPROPERTY(meta = (BindWidgetOptional)) TObjectPtr<UComboBoxString>  PlayStyleCombo;
    UPROPERTY(meta = (BindWidgetOptional)) TObjectPtr<UComboBoxString>  ExperienceCombo;
    UPROPERTY(meta = (BindWidgetOptional)) TObjectPtr<UButton>          AcceptBtn;
};