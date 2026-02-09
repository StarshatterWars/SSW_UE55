#include "MenuScreen.h"

// UE:
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"

// Dialogs:
#include "MenuDlg.h"
#include "ExitDlg.h"
#include "ConfirmDlg.h"
#include "FirstTimeDlg.h"
#include "PlayerDlg.h"
#include "AwardShowDlg.h"
#include "MissionSelectDlg.h"
#include "CampaignSelectDlg.h"
#include "MissionEditorDlg.h"
#include "MissionElementDlg.h"
#include "MissionEventDlg.h"
#include "MissionEditorNavDlg.h"
#include "LoadDlg.h"
#include "TacRefDlg.h"
#include "Blueprint/UserWidget.h"
// NEW:
#include "OptionsScreen.h"

// ------------------------------------------------------------

UMenuScreen::UMenuScreen(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
}

void UMenuScreen::NativeConstruct()
{
    Super::NativeConstruct();

    // Optional: eagerly build everything once
    // Setup();
}

// ------------------------------------------------------------
// Dialog creation (GC-safe via UPROPERTY storage)
// ------------------------------------------------------------

template<typename TDialog>
TDialog* UMenuScreen::EnsureDialog(TSubclassOf<TDialog> ClassToSpawn, TObjectPtr<TDialog>& Storage)
{
    if (Storage)
        return Storage;

    if (!ClassToSpawn)
        return nullptr;

    APlayerController* PC = GetOwningPlayer();
    if (!PC)
    {
        UE_LOG(LogTemp, Warning, TEXT("EnsureDialog: OwningPlayer is null (%s)"), *GetName());
        return nullptr;
    }

    Storage = CreateWidget<TDialog>(PC, ClassToSpawn);
    if (!Storage)
        return nullptr;

    // Keep it around; hide until needed
    Storage->AddToViewport(0);
    Storage->SetVisibility(ESlateVisibility::Hidden);
    Storage->SetDialogInputEnabled(false);

    return Storage;
}

// ------------------------------------------------------------
// Show/Hide helpers
// ------------------------------------------------------------

void UMenuScreen::ShowDialog(UBaseScreen* Dialog, bool bTopMost)
{
    if (!Dialog)
        return;

    int32 Z = 0;
    if (bTopMost)
    {
        ++ZCounter;
        Z = ZCounter;
    }

    if (Dialog->IsInViewport())
        Dialog->RemoveFromParent();

    Dialog->AddToViewport(Z);
    Dialog->SetVisibility(ESlateVisibility::Visible);
    Dialog->SetDialogInputEnabled(true);

    CurrentDialog = Dialog;
}

void UMenuScreen::HideDialog(UBaseScreen* Dialog)
{
    if (!Dialog)
        return;

    Dialog->SetDialogInputEnabled(false);
    Dialog->SetVisibility(ESlateVisibility::Hidden);

    if (CurrentDialog == Dialog)
        CurrentDialog = nullptr;
}

void UMenuScreen::HideAll()
{
    HideDialog(MenuDlg);
    HideDialog(ExitDlg);
    HideDialog(ConfirmDlg);

    HideDialog(FirstTimeDlg);
    HideDialog(PlayerDlg);
    HideDialog(AwardDlg);

    HideDialog(MissionSelectDlg);
    HideDialog(CmpSelectDlg);

    HideDialog(MsnEditDlg);
    HideDialog(MsnElemDlg);
    HideDialog(MsnEventDlg);
    HideDialog(MsnEditNavDlg);

    HideDialog(LoadDlg);
    HideDialog(TacRefDlg);

    HideDialog(OptionsScreen);

    CurrentDialog = nullptr;
}

// ------------------------------------------------------------
// Setup / TearDown
// ------------------------------------------------------------

void UMenuScreen::Setup()
{
    EnsureDialog<UMenuDlg>(MenuDlgClass, MenuDlg);
    EnsureDialog<UExitDlg>(ExitDlgClass, ExitDlg);
    EnsureDialog<UConfirmDlg>(ConfirmDlgClass, ConfirmDlg);

    EnsureDialog<UFirstTimeDlg>(FirstTimeDlgClass, FirstTimeDlg);
    EnsureDialog<UPlayerDlg>(PlayerDlgClass, PlayerDlg);
    EnsureDialog<UAwardShowDlg>(AwardDlgClass, AwardDlg);

    EnsureDialog<UMissionSelectDlg>(MsnSelectDlgClass, MissionSelectDlg);
    EnsureDialog<UCampaignSelectDlg>(CmpSelectDlgClass, CmpSelectDlg);

    EnsureDialog<UMissionEditorDlg>(MsnEditDlgClass, MsnEditDlg);
    EnsureDialog<UMissionElementDlg>(MsnElemDlgClass, MsnElemDlg);
    EnsureDialog<UMissionEventDlg>(MsnEventDlgClass, MsnEventDlg);
    EnsureDialog<UMissionEditorNavDlg>(MsnEditNavDlgClass, MsnEditNavDlg);

    EnsureDialog<ULoadDlg>(LoadDlgClass, LoadDlg);
    EnsureDialog<UTacRefDlg>(TacRefDlgClass, TacRefDlg);

    EnsureDialog<UOptionsScreen>(OptionsScreenClass, OptionsScreen);
    if (OptionsScreen)
    {
        OptionsScreen->SetMenuManager(this);
    }

    ShowMenuDlg();
}

void UMenuScreen::TearDown()
{
    HideAll();

    // If you truly want to destroy them, RemoveFromParent and null them.
    // Usually unnecessary; a menu screen lifetime is short.
    auto Destroy = [](UBaseScreen*& W)
        {
            if (W)
            {
                if (W->IsInViewport())
                    W->RemoveFromParent();
                W = nullptr;
            }
        };

    Destroy(reinterpret_cast<UBaseScreen*&>(MenuDlg));
    Destroy(reinterpret_cast<UBaseScreen*&>(ExitDlg));
    Destroy(reinterpret_cast<UBaseScreen*&>(ConfirmDlg));

    Destroy(reinterpret_cast<UBaseScreen*&>(FirstTimeDlg));
    Destroy(reinterpret_cast<UBaseScreen*&>(PlayerDlg));
    Destroy(reinterpret_cast<UBaseScreen*&>(AwardDlg));

    Destroy(reinterpret_cast<UBaseScreen*&>(MissionSelectDlg));
    Destroy(reinterpret_cast<UBaseScreen*&>(CmpSelectDlg));

    Destroy(reinterpret_cast<UBaseScreen*&>(MsnEditDlg));
    Destroy(reinterpret_cast<UBaseScreen*&>(MsnElemDlg));
    Destroy(reinterpret_cast<UBaseScreen*&>(MsnEventDlg));
    Destroy(reinterpret_cast<UBaseScreen*&>(MsnEditNavDlg));

    Destroy(reinterpret_cast<UBaseScreen*&>(LoadDlg));
    Destroy(reinterpret_cast<UBaseScreen*&>(TacRefDlg));

    Destroy(reinterpret_cast<UBaseScreen*&>(OptionsScreen));

    CurrentDialog = nullptr;
    bIsShown = false;
}

// ------------------------------------------------------------

void UMenuScreen::ExecFrame(double DeltaTime)
{
    (void)DeltaTime;
}

// ------------------------------------------------------------
// CloseTopmost
// ------------------------------------------------------------

bool UMenuScreen::CloseTopmost()
{
    // Mission editor overlays (top-most first)
    if (MsnElemDlg && MsnElemDlg->GetVisibility() == ESlateVisibility::Visible) { HideMsnElemDlg(); return true; }
    if (MsnEventDlg && MsnEventDlg->GetVisibility() == ESlateVisibility::Visible) { HideMsnEventDlg(); return true; }

    // Options hub
    if (OptionsScreen && OptionsScreen->GetVisibility() == ESlateVisibility::Visible)
    {
        ReturnFromOptions();
        return true;
    }

    if (CmpSelectDlg && CmpSelectDlg->GetVisibility() == ESlateVisibility::Visible)
    {
        ShowMenuDlg();
        return true;
    }

    if (MenuDlg && MenuDlg->GetVisibility() != ESlateVisibility::Visible)
    {
        ShowMenuDlg();
        return true;
    }

    return false;
}

// ------------------------------------------------------------

void UMenuScreen::Show()
{
    if (bIsShown)
        return;

    const bool bHasPlayerConfig = true;

    if (CurrentDialog == MissionSelectDlg)
    {
        ShowMissionSelectDlg();
    }
    else if (bHasPlayerConfig)
    {
        ShowMenuDlg();
    }
    else
    {
        ShowFirstTimeDlg();
    }

    bIsShown = true;
}

void UMenuScreen::Hide()
{
    if (!bIsShown)
        return;

    HideAll();
    bIsShown = false;
}

// ------------------------------------------------------------
// Show* routing
// ------------------------------------------------------------

void UMenuScreen::ShowMenuDlg()
{
    // Optional: single-dialog policy
    HideAll();

    if (!MenuDlgClass)
    {
        UE_LOG(LogTemp, Error, TEXT("UMenuScreen::ShowMenuDlg: MenuDlgClass is NULL (set it in WBP_MenuScreen defaults)"));
        return;
    }

    APlayerController* PC = GetOwningPlayer();
    if (!PC)
    {
        UE_LOG(LogTemp, Error, TEXT("UMenuScreen::ShowMenuDlg: OwningPlayer is NULL"));
        return;
    }

    // Lazy create once
    if (!MenuDlg)
    {
        MenuDlg = CreateWidget<UMenuDlg>(PC, MenuDlgClass);
        if (!MenuDlg)
        {
            UE_LOG(LogTemp, Error, TEXT("UMenuScreen::ShowMenuDlg: Failed to create MenuDlg"));
            return;
        }

        MenuDlg->Manager = this;
        MenuDlg->AddToViewport(0);                    // added once; Z handled below
        MenuDlg->SetDialogInputEnabled(false);
        MenuDlg->SetVisibility(ESlateVisibility::Hidden);
    }

    // Always re-assert manager in case of re-instancing
    MenuDlg->Manager = this;

    // Bring to front (no SetZOrderInViewport dependency)
    ++ZCounter;
    if (MenuDlg->IsInViewport())
        MenuDlg->RemoveFromParent();
    MenuDlg->AddToViewport(ZCounter);

    MenuDlg->SetVisibility(ESlateVisibility::Visible);
    MenuDlg->SetDialogInputEnabled(true);

    CurrentDialog = MenuDlg;
}


void UMenuScreen::ShowCampaignSelectDlg()
{
    HideAll();
    EnsureDialog<UCampaignSelectDlg>(CmpSelectDlgClass, CmpSelectDlg);
    ShowDialog(CmpSelectDlg, true);
}

void UMenuScreen::ShowMissionSelectDlg()
{
    HideAll();
    EnsureDialog<UMissionSelectDlg>(MsnSelectDlgClass, MissionSelectDlg);
    ShowDialog(MissionSelectDlg, true);
}

void UMenuScreen::ShowMissionEditorDlg()
{
    HideAll();
    EnsureDialog<UMissionEditorDlg>(MsnEditDlgClass, MsnEditDlg);
    ShowDialog(MsnEditDlg, true);
}

void UMenuScreen::ShowMsnElemDlg()
{
    EnsureDialog<UMissionElementDlg>(MsnElemDlgClass, MsnElemDlg);
    if (!MsnElemDlg) return;

    // Keep editor + nav visible behind it (non-topmost)
    if (MsnEditDlg && MsnEditDlg->GetVisibility() == ESlateVisibility::Visible)
        ShowDialog(MsnEditDlg, false);

    if (MsnEditNavDlg && MsnEditNavDlg->GetVisibility() == ESlateVisibility::Visible)
        ShowDialog(MsnEditNavDlg, false);

    ShowDialog(MsnElemDlg, true);
}

void UMenuScreen::HideMsnElemDlg()
{
    HideDialog(MsnElemDlg);

    if (MsnEditDlg && MsnEditDlg->GetVisibility() == ESlateVisibility::Visible)
        ShowDialog(MsnEditDlg, true);

    if (MsnEditNavDlg && MsnEditNavDlg->GetVisibility() == ESlateVisibility::Visible)
        ShowDialog(MsnEditNavDlg, true);
}

void UMenuScreen::ShowMissionEventDlg()
{
    EnsureDialog<UMissionEventDlg>(MsnEventDlgClass, MsnEventDlg);
    if (!MsnEventDlg) return;

    if (MsnEditDlg && MsnEditDlg->GetVisibility() == ESlateVisibility::Visible)
        ShowDialog(MsnEditDlg, false);

    if (MsnEditNavDlg && MsnEditNavDlg->GetVisibility() == ESlateVisibility::Visible)
        ShowDialog(MsnEditNavDlg, false);

    ShowDialog(MsnEventDlg, true);
}

void UMenuScreen::HideMsnEventDlg()
{
    HideDialog(MsnEventDlg);

    if (MsnEditDlg && MsnEditDlg->GetVisibility() == ESlateVisibility::Visible)
        ShowDialog(MsnEditDlg, true);

    if (MsnEditNavDlg && MsnEditNavDlg->GetVisibility() == ESlateVisibility::Visible)
        ShowDialog(MsnEditNavDlg, true);
}

void UMenuScreen::ShowNavDlg()
{
    EnsureDialog<UMissionEditorNavDlg>(MsnEditNavDlgClass, MsnEditNavDlg);
    if (!MsnEditNavDlg) return;

    if (MsnEditNavDlg->GetVisibility() != ESlateVisibility::Visible)
    {
        HideAll();
        ShowDialog(MsnEditNavDlg, true);
    }
}

void UMenuScreen::HideNavDlg()
{
    HideDialog(MsnEditNavDlg);
}

bool UMenuScreen::IsNavShown() const
{
    return (MsnEditNavDlg && MsnEditNavDlg->GetVisibility() == ESlateVisibility::Visible);
}

void UMenuScreen::ShowFirstTimeDlg()
{
    HideAll();
    EnsureDialog<UMenuDlg>(MenuDlgClass, MenuDlg);
    EnsureDialog<UFirstTimeDlg>(FirstTimeDlgClass, FirstTimeDlg);

    if (MenuDlg)
    {
        MenuDlg->Manager = this;
        ShowDialog(MenuDlg, false);
    }

    if (FirstTimeDlg)
        ShowDialog(FirstTimeDlg, true);
}

void UMenuScreen::ShowPlayerDlg()
{
    HideAll();
    EnsureDialog<UPlayerDlg>(PlayerDlgClass, PlayerDlg);
    ShowDialog(PlayerDlg, true);
}

void UMenuScreen::ShowTacRefDlg()
{
    HideAll();
    EnsureDialog<UTacRefDlg>(TacRefDlgClass, TacRefDlg);
    ShowDialog(TacRefDlg, true);
}

void UMenuScreen::ShowAwardDlg()
{
    HideAll();
    EnsureDialog<UAwardShowDlg>(AwardDlgClass, AwardDlg);
    ShowDialog(AwardDlg, true);
}

void UMenuScreen::ShowExitDlg()
{
    HideAll();

    EnsureDialog<UMenuDlg>(MenuDlgClass, MenuDlg);
    EnsureDialog<UExitDlg>(ExitDlgClass, ExitDlg);

    if (MenuDlg)
    {
        MenuDlg->Manager = this;
        ShowDialog(MenuDlg, false);
    }

    if (ExitDlg)
        ShowDialog(ExitDlg, true);
}

void UMenuScreen::ShowConfirmDlg()
{
    EnsureDialog<UConfirmDlg>(ConfirmDlgClass, ConfirmDlg);
    if (!ConfirmDlg) return;

    ShowDialog(ConfirmDlg, true);
}

void UMenuScreen::HideConfirmDlg()
{
    HideDialog(ConfirmDlg);
}

void UMenuScreen::ShowLoadDlg()
{
    EnsureDialog<ULoadDlg>(LoadDlgClass, LoadDlg);
    if (!LoadDlg) return;

    ShowDialog(LoadDlg, true);
}

void UMenuScreen::HideLoadDlg()
{
    HideDialog(LoadDlg);
}

// ------------------------------------------------------------
// Options hub
// ------------------------------------------------------------

void UMenuScreen::ShowOptionsScreen()
{
    HideAll();

    EnsureDialog<UOptionsScreen>(OptionsScreenClass, OptionsScreen);
    if (!OptionsScreen)
        return;

    OptionsScreen->SetMenuManager(this);
    ShowDialog(OptionsScreen, true);

    // Default tab/page:
    OptionsScreen->ShowOptDlg();
}

void UMenuScreen::HideOptionsScreen()
{
    HideDialog(OptionsScreen);
}

void UMenuScreen::ReturnFromOptions()
{
    ShowMenuDlg();
}

// ------------------------------------------------------------
// BaseScreen unified key handling targets
// ------------------------------------------------------------

void UMenuScreen::HandleAccept()
{
    if (CurrentDialog && CurrentDialog != this)
    {
        CurrentDialog->HandleAccept();
        return;
    }

    Super::HandleAccept();
}

void UMenuScreen::HandleCancel()
{
    if (CloseTopmost())
        return;

    Super::HandleCancel();
}


