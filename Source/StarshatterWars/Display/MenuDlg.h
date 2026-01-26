/*  Project Starshatter Wars
    Fractal Dev Studios
    Copyright (c) 2025-2026.

    SUBSYSTEM:    Stars.exe
    FILE:         MenuDlg.h
    AUTHOR:       Carlos Bott

    ORIGINAL AUTHOR AND STUDIO
    ==========================
    John DiCamillo / Destroyer Studios LLC

    OVERVIEW
    ========
    Main Menu dialog (legacy MenuDlg) adapted for Unreal UMG.
*/

#pragma once

#include "CoreMinimal.h"
#include "BaseScreen.h"
#include "MenuDlg.generated.h"

// Forward declarations (UMG)
class UButton;
class UTextBlock;

class UMenuScreen;
class Starshatter;
class Campaign;

// --------------------------------------------------------------------

UCLASS()
class STARSHATTERWARS_API UMenuDlg : public UBaseScreen
{
    GENERATED_BODY()

public:
    UMenuDlg(const FObjectInitializer& ObjectInitializer);

    virtual void NativeConstruct() override;

    // Legacy parity:
    void RegisterControls();
    void Show();         // semantic: you may call AddToViewport / SetVisibility externally
    void ExecFrame();    // semantic: tick-driven in UMG; kept for parity

    // Operations (legacy event handlers):
    UFUNCTION() void OnStart();
    UFUNCTION() void OnCampaign();
    UFUNCTION() void OnMission();
    UFUNCTION() void OnPlayer();
    UFUNCTION() void OnMultiplayer();
    UFUNCTION() void OnMod();
    UFUNCTION() void OnTacReference();

    UFUNCTION() void OnVideo();
    UFUNCTION() void OnOptions();
    UFUNCTION() void OnControls();
    UFUNCTION() void OnQuit();

    // UMG hover callbacks:
    UFUNCTION() void OnButtonEnter_Start();
    UFUNCTION() void OnButtonExit_Start();

    UFUNCTION() void OnButtonEnter_Campaign();
    UFUNCTION() void OnButtonExit_Campaign();

    UFUNCTION() void OnButtonEnter_Mission();
    UFUNCTION() void OnButtonExit_Mission();

    UFUNCTION() void OnButtonEnter_Player();
    UFUNCTION() void OnButtonExit_Player();

    UFUNCTION() void OnButtonEnter_Multi();
    UFUNCTION() void OnButtonExit_Multi();

    UFUNCTION() void OnButtonEnter_Options();
    UFUNCTION() void OnButtonExit_Options();

    UFUNCTION() void OnButtonEnter_Tac();
    UFUNCTION() void OnButtonExit_Tac();

    UFUNCTION() void OnButtonEnter_Quit();
    UFUNCTION() void OnButtonExit_Quit();

protected:
    // ----------------------------------------------------------------
    // External owner/screen (optional; depends on how you structured screens)
    // ----------------------------------------------------------------
    UPROPERTY()
    UMenuScreen* Manager = nullptr;

    // ----------------------------------------------------------------
    // Bound UMG widgets (match these names in your Widget Blueprint)
    // ----------------------------------------------------------------
    UPROPERTY(meta = (BindWidgetOptional)) UButton* BtnStart = nullptr;
    UPROPERTY(meta = (BindWidgetOptional)) UButton* BtnCampaign = nullptr;
    UPROPERTY(meta = (BindWidgetOptional)) UButton* BtnMission = nullptr;
    UPROPERTY(meta = (BindWidgetOptional)) UButton* BtnPlayer = nullptr;
    UPROPERTY(meta = (BindWidgetOptional)) UButton* BtnMulti = nullptr;
    UPROPERTY(meta = (BindWidgetOptional)) UButton* BtnMod = nullptr;
    UPROPERTY(meta = (BindWidgetOptional)) UButton* BtnTac = nullptr;

    UPROPERTY(meta = (BindWidgetOptional)) UButton* BtnVideo = nullptr;
    UPROPERTY(meta = (BindWidgetOptional)) UButton* BtnOptions = nullptr;
    UPROPERTY(meta = (BindWidgetOptional)) UButton* BtnControls = nullptr;
    UPROPERTY(meta = (BindWidgetOptional)) UButton* BtnQuit = nullptr;

    UPROPERTY(meta = (BindWidgetOptional)) UTextBlock* VersionText = nullptr;
    UPROPERTY(meta = (BindWidgetOptional)) UTextBlock* DescriptionText = nullptr;

    // ----------------------------------------------------------------
    // Starshatter singletons (legacy)
    // ----------------------------------------------------------------
    Starshatter* Stars = nullptr;
    Campaign* CampaignPtr = nullptr;

    // ----------------------------------------------------------------
    // Hover text (legacy "alt" strings)
    // ----------------------------------------------------------------
    FString AltStart;
    FString AltCampaign;
    FString AltMission;
    FString AltPlayer;
    FString AltMulti;
    FString AltOptions;
    FString AltTac;
    FString AltQuit;

private:
    void ClearDescription();
    void SetDescription(const FString& Text);
};
