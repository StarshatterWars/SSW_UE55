/*  Project Starshatter Wars
    Fractal Dev Studios
    Copyright (c) 2025-2026.

    SUBSYSTEM:    Stars.exe
    FILE:         MenuScreen.h
    AUTHOR:       Carlos Bott

    OVERVIEW
    ========
    UMenuScreen (simplified)
    - Owns top-level menu dialogs + mission editor dialogs.
    - Owns a single UOptionsScreen that manages ALL options subdialogs (Audio/Video/Controls/etc).
    - MenuScreen no longer knows about UAudioDlg/UVideoDlg/UOptDlg/UControlOptionsDlg/etc.
*/

#pragma once

#include "CoreMinimal.h"
#include "BaseScreen.h"
#include "MenuScreen.generated.h"

// Top-level dialogs:
class UMenuDlg;
class UExitDlg;
class UConfirmDlg;

class UFirstTimeDlg;
class UPlayerDlg;
class UAwardShowDlg;

class UMissionSelectDlg;
class UCampaignSelectDlg;

class UMissionEditorDlg;
class UMissionElementDlg;
class UMissionEventDlg;
class UMissionEditorNavDlg;

class ULoadDlg;
class UTacRefDlg;

// NEW: Options hub (manager of all option subdialogs)
class UOptionsScreen;

UCLASS()
class STARSHATTERWARS_API UMenuScreen : public UBaseScreen
{
    GENERATED_BODY()

public:
    UMenuScreen(const FObjectInitializer& ObjectInitializer);

    void Setup();
    void TearDown();

    bool CloseTopmost();

    bool IsShown() const { return bIsShown; }
    void Show();
    void Hide();

    virtual void ExecFrame(float DeltaTime);

    // ------------------------------------------------------------
    // Primary routing
    // ------------------------------------------------------------

    void ShowMenuDlg();

    void ShowCampaignSelectDlg();
    void ShowMissionSelectDlg();

    void ShowMissionEditorDlg();

    void ShowMsnElemDlg();
    void HideMsnElemDlg();

    void ShowMissionEventDlg();
    void HideMsnEventDlg();

    void ShowNavDlg();
    void HideNavDlg();
    bool IsNavShown() const;

    void ShowFirstTimeDlg();
    void ShowPlayerDlg();
    void ShowTacRefDlg();
    void ShowAwardDlg();

    void ShowExitDlg();

    void ShowConfirmDlg();
    void HideConfirmDlg();

    void ShowLoadDlg();
    void HideLoadDlg();

    // ------------------------------------------------------------
    // Options hub (single entry point)
    // ------------------------------------------------------------
    void ShowOptionsScreen();
    void HideOptionsScreen();

    // Optional: if OptionsScreen wants to return to menu:
    void ReturnFromOptions();

    // ------------------------------------------------------------
    // Getters (used by other UI)
    // ------------------------------------------------------------
    UMissionEditorDlg* GetMsnEditDlg()  const { return MsnEditDlg; }
    UMissionElementDlg* GetMsnElemDlg()  const { return MsnElemDlg; }
    UMissionEventDlg* GetMsnEventDlg() const { return MsnEventDlg; }
    UMissionEditorNavDlg* GetNavDlg()      const { return MsnEditNavDlg; }
    ULoadDlg* GetLoadDlg()     const { return LoadDlg; }

protected:
    virtual void NativeConstruct() override;

    virtual void HandleAccept() override;
    virtual void HandleCancel() override;

private:
    void HideAll();

    void ShowDialog(UBaseScreen* Dialog, bool bTopMost);
    void HideDialog(UBaseScreen* Dialog);

    template<typename TDialog>
    TDialog* EnsureDialog(TSubclassOf<UBaseScreen> ClassToSpawn, TDialog*& Storage);

private:
    // Spawn classes
    UPROPERTY(EditDefaultsOnly, Category = "Menu|Classes") TSubclassOf<UBaseScreen> MenuDlgClass;
    UPROPERTY(EditDefaultsOnly, Category = "Menu|Classes") TSubclassOf<UBaseScreen> ExitDlgClass;
    UPROPERTY(EditDefaultsOnly, Category = "Menu|Classes") TSubclassOf<UBaseScreen> ConfirmDlgClass;

    UPROPERTY(EditDefaultsOnly, Category = "Menu|Classes") TSubclassOf<UBaseScreen> FirstTimeDlgClass;
    UPROPERTY(EditDefaultsOnly, Category = "Menu|Classes") TSubclassOf<UBaseScreen> PlayerDlgClass;
    UPROPERTY(EditDefaultsOnly, Category = "Menu|Classes") TSubclassOf<UBaseScreen> AwardDlgClass;

    UPROPERTY(EditDefaultsOnly, Category = "Menu|Classes") TSubclassOf<UBaseScreen> MsnSelectDlgClass;
    UPROPERTY(EditDefaultsOnly, Category = "Menu|Classes") TSubclassOf<UBaseScreen> CmpSelectDlgClass;
    UPROPERTY(EditDefaultsOnly, Category = "Menu|Classes") TSubclassOf<UBaseScreen> MsnEditDlgClass;
    UPROPERTY(EditDefaultsOnly, Category = "Menu|Classes") TSubclassOf<UBaseScreen> MsnElemDlgClass;
    UPROPERTY(EditDefaultsOnly, Category = "Menu|Classes") TSubclassOf<UBaseScreen> MsnEventDlgClass;
    UPROPERTY(EditDefaultsOnly, Category = "Menu|Classes") TSubclassOf<UBaseScreen> MsnEditNavDlgClass;

    UPROPERTY(EditDefaultsOnly, Category = "Menu|Classes") TSubclassOf<UBaseScreen> LoadDlgClass;
    UPROPERTY(EditDefaultsOnly, Category = "Menu|Classes") TSubclassOf<UBaseScreen> TacRefDlgClass;

    // NEW: Options hub class
    UPROPERTY(EditDefaultsOnly, Category = "Menu|Classes") TSubclassOf<UBaseScreen> OptionsScreenClass;

private:
    // Instances (raw pointers; rooted)
    UMenuDlg* MenuDlg = nullptr;
    UExitDlg* ExitDlg = nullptr;
    UConfirmDlg* ConfirmDlg = nullptr;

    UFirstTimeDlg* FirstTimeDlg = nullptr;
    UPlayerDlg* PlayerDlg = nullptr;
    UAwardShowDlg* AwardDlg = nullptr;

    UMissionSelectDlg* MissionSelectDlg = nullptr;
    UCampaignSelectDlg* CmpSelectDlg = nullptr;

    UMissionEditorDlg* MsnEditDlg = nullptr;
    UMissionElementDlg* MsnElemDlg = nullptr;
    UMissionEventDlg* MsnEventDlg = nullptr;
    UMissionEditorNavDlg* MsnEditNavDlg = nullptr;

    ULoadDlg* LoadDlg = nullptr;
    UTacRefDlg* TacRefDlg = nullptr;

    // NEW: Options hub instance
    UOptionsScreen* OptionsScreen = nullptr;

    // Tracks active “front-most” dialog:
    UBaseScreen* CurrentDialog = nullptr;

    bool  bIsShown = false;
    int32 ZCounter = 100;
};
