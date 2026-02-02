/*  Project Starshatter Wars
    Fractal Dev Studios
    Copyright (c) 2025-2026.

    SUBSYSTEM:    Stars.exe
    FILE:         MenuScreen.h
    AUTHOR:       Carlos Bott

    OVERVIEW
    ========
    UMenuScreen (C++ ONLY)
    - Unreal replacement for legacy MenuScreen.
    - Inherits from UBaseScreen.
    - Owns dialog widgets as standard raw pointers (no TObjectPtr, no Transient UPROPERTY).
    - Enter/Escape handled by UBaseScreen::NativeOnKeyDown -> HandleAccept/HandleCancel.
*/

#pragma once

#include "CoreMinimal.h"
#include "BaseScreen.h"
#include "MenuScreen.generated.h"

// Dialog forward declares (all should inherit from UBaseScreen)
class UMenuDlg;
class UAudioDlg;
class UVideoDlg;
class UOptDlg;
class UControlOptionsDlg;
class UJoyDlg;
class UKeyDlg;
class UExitDlg;
class UConfirmDlg;

class UFirstTimeDlg;
class UPlayerDlg;
class UAwardShowDlg;

class UMsnSelectDlg;
class UCampaignSelectDlg;

class UMissionEditorDlg;
class UMissionElementDlg;
class UMissionEventDlg;
class UMissionEditorNavDlg;

class ULoadDlg;
class UTacRefDlg;

UCLASS()
class STARSHATTERWARS_API UMenuScreen : public UBaseScreen
{
    GENERATED_BODY()

public:
    UMenuScreen(const FObjectInitializer& ObjectInitializer);

    // Setup / teardown
    void Setup();
    void TearDown();

    bool CloseTopmost();

    bool IsShown() const { return bIsShown; }
    void Show();
    void Hide();

    virtual void ExecFrame(float DeltaTime);

    // Dialog routing
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

    void ShowAudDlg();
    void ShowVidDlg();
    void ShowOptDlg();
    void ShowCtlDlg();
    void ShowJoyDlg();
    void ShowKeyDlg();

    void ShowExitDlg();

    void ShowConfirmDlg();
    void HideConfirmDlg();

    void ShowLoadDlg();
    void HideLoadDlg();

    void ApplyOptions();
    void CancelOptions();

    // Getters
    UMissionEditorDlg*      GetMsnEditDlg()  const { return MsnEditDlg; }
    UMissionElementDlg*     GetMsnElemDlg()  const { return MsnElemDlg; }
    UMissionEventDlg*       GetMsnEventDlg() const { return MsnEventDlg; }
    UMissionEditorNavDlg*   GetNavDlg()      const { return MsnEditNavDlg; }
    ULoadDlg*               GetLoadDlg()     const { return LoadDlg; }

protected:
    virtual void NativeConstruct() override;

    // BaseScreen dialog hooks
    virtual void HandleAccept() override;
    virtual void HandleCancel() override;

private:
    void HideAll();

    // “Topmost” via viewport Z-order
    void ShowDialog(UBaseScreen* Dialog, bool bTopMost);
    void HideDialog(UBaseScreen* Dialog);

    template<typename TDialog>
    TDialog* EnsureDialog(TSubclassOf<UBaseScreen> ClassToSpawn, TDialog*& Storage);

private:
    // Spawn classes (set in defaults or constructor)
    UPROPERTY(EditDefaultsOnly, Category = "Menu|Classes") TSubclassOf<UBaseScreen> MenuDlgClass;
    UPROPERTY(EditDefaultsOnly, Category = "Menu|Classes") TSubclassOf<UBaseScreen> ExitDlgClass;
    UPROPERTY(EditDefaultsOnly, Category = "Menu|Classes") TSubclassOf<UBaseScreen> AudDlgClass;
    UPROPERTY(EditDefaultsOnly, Category = "Menu|Classes") TSubclassOf<UBaseScreen> VidDlgClass;
    UPROPERTY(EditDefaultsOnly, Category = "Menu|Classes") TSubclassOf<UBaseScreen> OptDlgClass;
    UPROPERTY(EditDefaultsOnly, Category = "Menu|Classes") TSubclassOf<UBaseScreen> CtlDlgClass;
    UPROPERTY(EditDefaultsOnly, Category = "Menu|Classes") TSubclassOf<UBaseScreen> JoyDlgClass;
    UPROPERTY(EditDefaultsOnly, Category = "Menu|Classes") TSubclassOf<UBaseScreen> KeyDlgClass;
    UPROPERTY(EditDefaultsOnly, Category = "Menu|Classes") TSubclassOf<UBaseScreen> ConfirmDlgClass;

    UPROPERTY(EditDefaultsOnly, Category = "Menu|Classes") TSubclassOf<UBaseScreen> FirstTimeDlgClass;
    UPROPERTY(EditDefaultsOnly, Category = "Menu|Classes") TSubclassOf<UBaseScreen> PlayerDlgClass;
    UPROPERTY(EditDefaultsOnly, Category = "Menu|Classes") TSubclassOf<UBaseScreen> AwardDlgClass;

    UPROPERTY(EditDefaultsOnly, Category = "Menu|Classes") TSubclassOf<UBaseScreen> MsnSelectDlgClass;
    UPROPERTY(EditDefaultsOnly, Category = "Menu|Classes") TSubclassOf<UBaseScreen> CmpSelectDlgClass;
    UPROPERTY(EditDefaultsOnly, Category = "Menu|Classes") TSubclassOf<UBaseScreen> ModDlgClass;
    UPROPERTY(EditDefaultsOnly, Category = "Menu|Classes") TSubclassOf<UBaseScreen> ModInfoDlgClass;
    UPROPERTY(EditDefaultsOnly, Category = "Menu|Classes") TSubclassOf<UBaseScreen> MsnEditDlgClass;
    UPROPERTY(EditDefaultsOnly, Category = "Menu|Classes") TSubclassOf<UBaseScreen> MsnElemDlgClass;
    UPROPERTY(EditDefaultsOnly, Category = "Menu|Classes") TSubclassOf<UBaseScreen> MsnEventDlgClass;
    UPROPERTY(EditDefaultsOnly, Category = "Menu|Classes") TSubclassOf<UBaseScreen> MsnEditNavDlgClass;

    UPROPERTY(EditDefaultsOnly, Category = "Menu|Classes") TSubclassOf<UBaseScreen> LoadDlgClass;
    UPROPERTY(EditDefaultsOnly, Category = "Menu|Classes") TSubclassOf<UBaseScreen> TacRefDlgClass;

private:
    // Instances (standard raw pointers, NOT UPROPERTY, NOT TObjectPtr)
    UMenuDlg* MenuDlg = nullptr;
    UExitDlg* ExitDlg = nullptr;
    UAudioDlg* AudDlg = nullptr;
    UVideoDlg* VidDlg = nullptr;
    UOptDlg* OptDlg = nullptr;
    UControlOptionsDlg* CtlDlg = nullptr;
    UJoyDlg* JoyDlg = nullptr;
    UKeyDlg* KeyDlg = nullptr;
    UConfirmDlg* ConfirmDlg = nullptr;

    UFirstTimeDlg* FirstTimeDlg = nullptr;
    UPlayerDlg* PlayerDlg = nullptr;
    UAwardShowDlg* AwardDlg = nullptr;

    UMsnSelectDlg* MsnSelectDlg = nullptr;
    UCampaignSelectDlg* CmpSelectDlg = nullptr;
    UMissionEditorDlg* MsnEditDlg = nullptr;
    UMissionElementDlg* MsnElemDlg = nullptr;
    UMissionEventDlg* MsnEventDlg = nullptr;
    UMissionEditorNavDlg* MsnEditNavDlg = nullptr;

    ULoadDlg* LoadDlg = nullptr;
    UTacRefDlg* TacRefDlg = nullptr;

    UBaseScreen* CurrentDialog = nullptr;

    bool  bIsShown = false;
    int32 ZCounter = 100;
};
