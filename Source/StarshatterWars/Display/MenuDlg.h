/*  Project Starshatter Wars
    Fractal Dev Studios
    Copyright (c) 2025-2026.

    SUBSYSTEM:    Stars.exe
    FILE:         MenuDlg.h
    AUTHOR:       Carlos Bott

    OVERVIEW
    ========
    Main Menu dialog (legacy MenuDlg) implementation for Unreal UMG.
    - 100% UMG-driven input (OnClicked/OnHovered)
    - UFUNCTION bindings for dynamic delegates
*/

#pragma once

#include "CoreMinimal.h"
#include "BaseScreen.h"
#include "MenuDlg.generated.h"

class UButton;
class UTextBlock;

// Legacy forward decls:
class Starshatter;
class Campaign;
class UMenuScreen;

UCLASS()
class STARSHATTERWARS_API UMenuDlg : public UBaseScreen
{
    GENERATED_BODY()

public:
    UMenuDlg(const FObjectInitializer& ObjectInitializer);

protected:
    // Bind once (preferred for delegates)
    virtual void NativeOnInitialized() override;

    // Use for per-show refresh / gating re-eval
    virtual void NativeConstruct() override;

public:
    virtual void ExecFrame(double DeltaTime) override;

    // Manager/router (set this when you create the widget)
    UPROPERTY(BlueprintReadWrite, Category = "Menu", meta = (ExposeOnSpawn = "true"))
    TObjectPtr<UMenuScreen> Manager = nullptr;

protected:
    // ------------------------------------------------------------
    // UMG Widgets (names must match Blueprint widget names exactly)
    // ------------------------------------------------------------

    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UButton> BtnStart;

    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UButton> BtnCampaign;

    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UButton> BtnMission;

    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UButton> BtnPlayer;

    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UButton> BtnMulti;

    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UButton> BtnVideo;

    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UButton> BtnOptions;

    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UButton> BtnControls;

    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UButton> BtnMod;

    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UButton> BtnTac;

    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UButton> BtnQuit;

    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UTextBlock> DescriptionText;

    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UTextBlock> VersionText;

    // ------------------------------------------------------------
    // Runtime state
    // ------------------------------------------------------------

    Starshatter* Stars = nullptr;
    Campaign* CampaignPtr = nullptr;

    bool bFirstRun_NoPlayerSave = false;
    bool bHasCampaignSelected = true;
    bool bDelegatesBound = false;

    // Hover “alt text”
    FString AltStart;
    FString AltCampaign;
    FString AltMission;
    FString AltMulti;
    FString AltPlayer;
    FString AltOptions;
    FString AltTac;
    FString AltQuit;

protected:
    // ------------------------------------------------------------
    // Binding + lifecycle helpers
    // ------------------------------------------------------------

    void BindUMGDelegates();
    void RefreshFromPlayerState();
    void ApplyMenuGating();
    void EnableMenuButtons(bool bEnable);
    void SetButtonEnabled(UButton* Button, bool bEnable);
    void Show();

    void ClearDescription();
    void SetDescription(const FString& Text);

    // ------------------------------------------------------------
    // Click handlers (MUST be UFUNCTION for AddDynamic)
    // ------------------------------------------------------------

    UFUNCTION()
    void OnStart();

    UFUNCTION()
    void OnCampaign();

    UFUNCTION()
    void OnMission();

    UFUNCTION()
    void OnPlayer();

    UFUNCTION()
    void OnMultiplayer();

    UFUNCTION()
    void OnMod();

    UFUNCTION()
    void OnVideo();

    UFUNCTION()
    void OnOptions();

    UFUNCTION()
    void OnControls();

    UFUNCTION()
    void OnTacReference();

    UFUNCTION()
    void OnQuit();

    // ------------------------------------------------------------
    // Hover handlers (MUST be UFUNCTION for AddDynamic)
    // ------------------------------------------------------------

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
};
