#include "MenuScreen.h"

// UE:
#include "Engine/World.h"

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
    // Setup(); // optional
}

// ------------------------------------------------------------
// Dialog creation (raw pointers + AddToRoot to prevent GC)
// ------------------------------------------------------------

template<typename TDialog>
TDialog* UMenuScreen::EnsureDialog(TSubclassOf<UBaseScreen> ClassToSpawn, TDialog*& Storage)
{
    if (Storage)
        return Storage;

    if (!ClassToSpawn)
        return nullptr;

    UWorld* World = GetWorld();
    if (!World)
        return nullptr;

    UBaseScreen* Created = CreateWidget<UBaseScreen>(World, ClassToSpawn);
    Storage = Cast<TDialog>(Created);

    if (Storage)
    {
        Storage->AddToRoot();
        Storage->AddToViewport(0);
        Storage->SetVisibility(ESlateVisibility::Hidden);
        Storage->SetDialogInputEnabled(false);
    }

    return Storage;
}

// ------------------------------------------------------------
// Show/Hide helpers
// ------------------------------------------------------------

void UMenuScreen::ShowDialog(UBaseScreen* Dialog, bool bTopMost)
{
    if (!Dialog)
        return;

    if (Dialog->IsInViewport())
        Dialog->RemoveFromParent();

    int32 Z = 0;
    if (bTopMost)
    {
        ++ZCounter;
        Z = ZCounter;
    }

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

    // NEW: hide options hub
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

    // NEW: options hub
    EnsureDialog<UOptionsScreen>(OptionsScreenClass, OptionsScreen);
    if (OptionsScreen)
    {
        // OptionsScreen needs to return to menu and/or quit confirm sometimes
        OptionsScreen->SetMenuManager(this); // implement this on OptionsScreen (tiny method)
    }

    ShowMenuDlg();
}

void UMenuScreen::TearDown()
{
    HideAll();

    auto Unroot = [](UBaseScreen*& W)
        {
            if (W)
            {
                if (W->IsInViewport())
                    W->RemoveFromParent();

                if (W->IsRooted())
                    W->RemoveFromRoot();

                W = nullptr;
            }
        };

    Unroot(reinterpret_cast<UBaseScreen*&>(MenuDlg));
    Unroot(reinterpret_cast<UBaseScreen*&>(ExitDlg));
    Unroot(reinterpret_cast<UBaseScreen*&>(ConfirmDlg));

    Unroot(reinterpret_cast<UBaseScreen*&>(FirstTimeDlg));
    Unroot(reinterpret_cast<UBaseScreen*&>(PlayerDlg));
    Unroot(reinterpret_cast<UBaseScreen*&>(AwardDlg));

    Unroot(reinterpret_cast<UBaseScreen*&>(MissionSelectDlg));
    Unroot(reinterpret_cast<UBaseScreen*&>(CmpSelectDlg));

    Unroot(reinterpret_cast<UBaseScreen*&>(MsnEditDlg));
    Unroot(reinterpret_cast<UBaseScreen*&>(MsnElemDlg));
    Unroot(reinterpret_cast<UBaseScreen*&>(MsnEventDlg));
    Unroot(reinterpret_cast<UBaseScreen*&>(MsnEditNavDlg));

    Unroot(reinterpret_cast<UBaseScreen*&>(LoadDlg));
    Unroot(reinterpret_cast<UBaseScreen*&>(TacRefDlg));

    // NEW:
    Unroot(reinterpret_cast<UBaseScreen*&>(OptionsScreen));

    CurrentDialog = nullptr;
    bIsShown = false;
}

// ------------------------------------------------------------

void UMenuScreen::ExecFrame(float DeltaTime)
{
    (void)DeltaTime;
}

// ------------------------------------------------------------
// CloseTopmost
// ------------------------------------------------------------

bool UMenuScreen::CloseTopmost()
{
    // Mission editor overlays
    if (MsnElemDlg && MsnElemDlg->IsVisible()) { HideMsnElemDlg(); return true; }
    if (MsnEventDlg && MsnEventDlg->IsVisible()) { HideMsnEventDlg(); return true; }

    // Options hub
    if (OptionsScreen && OptionsScreen->IsVisible())
    {
        ReturnFromOptions();
        return true;
    }

    if (CmpSelectDlg && CmpSelectDlg->IsVisible())
    {
        ShowMenuDlg();
        return true;
    }

    if (MenuDlg && !MenuDlg->IsVisible())
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
    HideAll();
    EnsureDialog<UMenuDlg>(MenuDlgClass, MenuDlg);
    ShowDialog(MenuDlg, true);
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

    if (MsnEditDlg && MsnEditDlg->IsVisible())
        ShowDialog(MsnEditDlg, false);

    if (MsnEditNavDlg && MsnEditNavDlg->IsVisible())
        ShowDialog(MsnEditNavDlg, false);

    ShowDialog(MsnElemDlg, true);
}

void UMenuScreen::HideMsnElemDlg()
{
    HideDialog(MsnElemDlg);

    if (MsnEditDlg && MsnEditDlg->IsVisible())
        ShowDialog(MsnEditDlg, true);

    if (MsnEditNavDlg && MsnEditNavDlg->IsVisible())
        ShowDialog(MsnEditNavDlg, true);
}

void UMenuScreen::ShowMissionEventDlg()
{
    EnsureDialog<UMissionEventDlg>(MsnEventDlgClass, MsnEventDlg);
    if (!MsnEventDlg) return;

    if (MsnEditDlg && MsnEditDlg->IsVisible())
        ShowDialog(MsnEditDlg, false);

    if (MsnEditNavDlg && MsnEditNavDlg->IsVisible())
        ShowDialog(MsnEditNavDlg, false);

    ShowDialog(MsnEventDlg, true);
}

void UMenuScreen::HideMsnEventDlg()
{
    HideDialog(MsnEventDlg);

    if (MsnEditDlg && MsnEditDlg->IsVisible())
        ShowDialog(MsnEditDlg, true);

    if (MsnEditNavDlg && MsnEditNavDlg->IsVisible())
        ShowDialog(MsnEditNavDlg, true);
}

void UMenuScreen::ShowNavDlg()
{
    EnsureDialog<UMissionEditorNavDlg>(MsnEditNavDlgClass, MsnEditNavDlg);
    if (!MsnEditNavDlg) return;

    if (!MsnEditNavDlg->IsVisible())
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
    return (MsnEditNavDlg && MsnEditNavDlg->IsVisible());
}

void UMenuScreen::ShowFirstTimeDlg()
{
    HideAll();
    EnsureDialog<UMenuDlg>(MenuDlgClass, MenuDlg);
    EnsureDialog<UFirstTimeDlg>(FirstTimeDlgClass, FirstTimeDlg);

    if (MenuDlg)      ShowDialog(MenuDlg, false);
    if (FirstTimeDlg) ShowDialog(FirstTimeDlg, true);
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

    if (MenuDlg) ShowDialog(MenuDlg, false);
    if (ExitDlg) ShowDialog(ExitDlg, true);
}

void UMenuScreen::ShowConfirmDlg()
{
    EnsureDialog<UConfirmDlg>(ConfirmDlgClass, ConfirmDlg);
    if (!ConfirmDlg) return;

    // Overlay confirm on top of whatever is visible
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
// NEW: Options hub
// ------------------------------------------------------------

void UMenuScreen::ShowOptionsScreen()
{
    HideAll();

    EnsureDialog<UOptionsScreen>(OptionsScreenClass, OptionsScreen);
    if (!OptionsScreen)
        return;

    OptionsScreen->SetMenuManager(this);
    ShowDialog(OptionsScreen, true);

    // Default tab/page inside OptionsScreen:
    OptionsScreen->ShowOptDlg(); // or ShowVidDlg(), your choice
}

void UMenuScreen::HideOptionsScreen()
{
    HideDialog(OptionsScreen);
}

void UMenuScreen::ReturnFromOptions()
{
    // OptionsScreen should have already applied/canceled internally.
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
