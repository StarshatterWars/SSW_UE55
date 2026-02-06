/*  Project Starshatter Wars
    Fractal Dev Studios
    Copyright (c) 2025-2026.

    SUBSYSTEM:    Stars.exe
    FILE:         MenuDlg.h
    AUTHOR:       Carlos Bott

    OVERVIEW
    ========
    Main Menu dialog (legacy MenuDlg) adapted for Unreal UMG.
    - Pure dialog widget: button wiring + hover description.
    - Delegates navigation to UMenuScreen (the router/manager).
*/

#pragma once

#include "CoreMinimal.h"
#include "BaseScreen.h"
#include "MenuDlg.generated.h"

// Forward declarations (UMG)
class UButton;
class UTextBlock;

// Manager/router
class UMenuScreen;

// Legacy
class Starshatter;
class Campaign;

UCLASS()
class STARSHATTERWARS_API UMenuDlg : public UBaseScreen
{
    GENERATED_BODY()

public:
    UMenuDlg(const FObjectInitializer& ObjectInitializer);

    virtual void NativeConstruct() override;

    // Manager wiring:
    void SetManager(UMenuScreen* InManager) { Manager = InManager; }
    UMenuScreen* GetManager() const { return Manager; }

    // Legacy parity:
    void RegisterControls();
    void Show();
    void ExecFrame();

    UFUNCTION(BlueprintCallable)
    void EnableMenuButtons(bool bEnable);

    // Click handlers:
    UFUNCTION() void OnStart();
    UFUNCTION() void OnCampaign();
    UFUNCTION() void OnMission();
    UFUNCTION() void OnPlayer();
    UFUNCTION() void OnMultiplayer();
    UFUNCTION() void OnMod();
    UFUNCTION() void OnTacReference();

    UFUNCTION() void OnVideo();    // now routes to Options hub
    UFUNCTION() void OnOptions();  // now routes to Options hub
    UFUNCTION() void OnControls(); // now routes to Options hub
    UFUNCTION() void OnQuit();

    // Hover handlers:
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
    UPROPERTY(Transient)
    TObjectPtr<UMenuScreen> Manager = nullptr;

    // Bound widgets:
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

    // Legacy pointers:
    Starshatter* Stars = nullptr;
    Campaign* CampaignPtr = nullptr;

    // Hover text:
    FString AltStart;
    FString AltCampaign;
    FString AltMission;
    FString AltPlayer;
    FString AltMulti;
    FString AltOptions;
    FString AltTac;
    FString AltQuit;

    bool bFirstRun_NoPlayerSave = false;
    bool bHasCampaignSelected = false;

    void ApplyMenuGating();

private:
    void ClearDescription();
    void SetDescription(const FString& Text);
    void SetButtonEnabled(UButton* Button, bool bEnable);
};
