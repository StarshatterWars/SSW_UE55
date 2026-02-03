/*  Project Starshatter Wars
    Fractal Dev Studios
    Copyright (c) 2025-2026.

    SUBSYSTEM:    Stars.exe
    FILE:         MenuScreen.cpp
    AUTHOR:       Carlos Bott
*/

#include "MenuScreen.h"

#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Kismet/GameplayStatics.h"

#include "MenuDlg.h"
#include "ExitDlg.h"
#include "AudioDlg.h"
#include "VideoDlg.h"
#include "OptDlg.h"
#include "ControlOptionsDlg.h"
#include "JoyDlg.h"
#include "KeyDlg.h"
#include "ConfirmDlg.h"

#include "PlayerDlg.h"
#include "MissionSelectDlg.h"
#include "MissionEditorDlg.h"
#include "MissionElementDlg.h"
#include "MissionEventDlg.h"
#include "MissionEditorNavDlg.h"
#include "FirstTimeDlg.h"
#include "AwardShowDlg.h"
#include "LoadDlg.h"
#include "TacRefDlg.h"

#include "CampaignSelectDlg.h"

UMenuScreen::UMenuScreen(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
}

void UMenuScreen::NativeConstruct()
{
    Super::NativeConstruct();
    // Setup(); // optional
}

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
        Storage->AddToViewport(0);
        Storage->SetVisibility(ESlateVisibility::Hidden);
        Storage->SetDialogInputEnabled(false);
    }

    return Storage;
}

void UMenuScreen::ShowDialog(UBaseScreen* Dialog, bool bTopMost)
{
    if (!Dialog)
        return;

    if (Dialog->IsInViewport())
    {
        Dialog->RemoveFromParent();
    }

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

void UMenuScreen::Setup()
{
    // Create what you need:
    EnsureDialog<UMenuDlg>(MenuDlgClass, MenuDlg);
    EnsureDialog<UExitDlg>(ExitDlgClass, ExitDlg);
    EnsureDialog<UAudioDlg>(AudDlgClass, AudDlg);
    EnsureDialog<UVideoDlg>(VidDlgClass, VidDlg);
    EnsureDialog<UOptDlg>(OptDlgClass, OptDlg);
    EnsureDialog<UControlOptionsDlg>(CtlDlgClass, CtlDlg);
    EnsureDialog<UJoyDlg>(JoyDlgClass, JoyDlg);
    EnsureDialog<UKeyDlg>(KeyDlgClass, KeyDlg);
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

    ShowMenuDlg();
}

void UMenuScreen::TearDown()
{
    // IMPORTANT:
    // These widgets are still UObjects. If you do not hold them in UPROPERTY,
    // GC can collect them. To make this safe without UPROPERTY, we explicitly:
    // - AddToRoot() after creation (prevent GC)
    // - RemoveFromRoot() here
    //
    // If you do NOT want AddToRoot(), then you MUST store pointers in UPROPERTY.

    HideAll();

    auto Unroot = [](UBaseScreen*& W)
        {
            if (W)
            {
                W->RemoveFromParent();
                W->RemoveFromRoot();
                W = nullptr;
            }
        };

    Unroot(reinterpret_cast<UBaseScreen*&>(MenuDlg));
    Unroot(reinterpret_cast<UBaseScreen*&>(ExitDlg));
    Unroot(reinterpret_cast<UBaseScreen*&>(AudDlg));
    Unroot(reinterpret_cast<UBaseScreen*&>(VidDlg));
    Unroot(reinterpret_cast<UBaseScreen*&>(OptDlg));
    Unroot(reinterpret_cast<UBaseScreen*&>(CtlDlg));
    Unroot(reinterpret_cast<UBaseScreen*&>(JoyDlg));
    Unroot(reinterpret_cast<UBaseScreen*&>(KeyDlg));
    Unroot(reinterpret_cast<UBaseScreen*&>(ConfirmDlg));

    Unroot(reinterpret_cast<UBaseScreen*&>(FirstTimeDlg));
    Unroot(reinterpret_cast<UBaseScreen*&>(PlayerDlg));
    Unroot(reinterpret_cast<UBaseScreen*&>(AwardDlg));

    Unroot(reinterpret_cast<UBaseScreen*&>(MissionSelectDlg));
    Unroot(reinterpret_cast<UBaseScreen*&>(CmpSelectDlg));
;
    Unroot(reinterpret_cast<UBaseScreen*&>(MsnEditDlg));
    Unroot(reinterpret_cast<UBaseScreen*&>(MsnElemDlg));
    Unroot(reinterpret_cast<UBaseScreen*&>(MsnEventDlg));
    Unroot(reinterpret_cast<UBaseScreen*&>(MsnEditNavDlg));

    Unroot(reinterpret_cast<UBaseScreen*&>(LoadDlg));
    Unroot(reinterpret_cast<UBaseScreen*&>(TacRefDlg));

    CurrentDialog = nullptr;
    bIsShown = false;
}

void UMenuScreen::HideAll()
{
    HideDialog(MenuDlg);
    HideDialog(ExitDlg);
    HideDialog(AudDlg);
    HideDialog(VidDlg);
    HideDialog(OptDlg);
    HideDialog(CtlDlg);
    HideDialog(JoyDlg);
    HideDialog(KeyDlg);
    HideDialog(ConfirmDlg);

    HideDialog(PlayerDlg);
    HideDialog(AwardDlg);
    HideDialog(FirstTimeDlg);;

    HideDialog(MissionSelectDlg);
    HideDialog(MsnEditDlg);
    HideDialog(MsnElemDlg);
    HideDialog(MsnEventDlg);
    //HideDialog(MissionEditorNavDlg);

    HideDialog(CmpSelectDlg);

    HideDialog(LoadDlg);
    HideDialog(TacRefDlg);

    CurrentDialog = nullptr;
}

bool UMenuScreen::CloseTopmost()
{
    if (JoyDlg && JoyDlg->IsVisible()) { ShowCtlDlg(); return true; }
    if (KeyDlg && KeyDlg->IsVisible()) { ShowCtlDlg(); return true; }

    if (MsnElemDlg && MsnElemDlg->IsVisible()) { HideMsnElemDlg(); return true; }
    if (MsnEventDlg && MsnEventDlg->IsVisible()) { HideMsnEventDlg(); return true; }

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

void UMenuScreen::Show()
{
    if (bIsShown)
        return;

    ShowMenuDlg();
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
// Show* routing (shortened here; keep same as prior version)
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
    //EnsureDialog<UCampaignSelectDlg>(CampaignDlgClass, CtlDlg);
    //ShowDialog(CtlDlg, true);
}

void UMenuScreen::ShowCtlDlg()
{
    HideAll();
    EnsureDialog<UControlOptionsDlg>(CtlDlgClass, CtlDlg);
    ShowDialog(CtlDlg, true);
}

void UMenuScreen::ShowJoyDlg()
{
    EnsureDialog<UControlOptionsDlg>(CtlDlgClass, CtlDlg);
    EnsureDialog<UJoyDlg>(JoyDlgClass, JoyDlg);

    HideAll();
    if (CtlDlg) ShowDialog(CtlDlg, false);
    if (JoyDlg) ShowDialog(JoyDlg, true);
}

void UMenuScreen::ShowKeyDlg()
{
    EnsureDialog<UControlOptionsDlg>(CtlDlgClass, CtlDlg);
    EnsureDialog<UKeyDlg>(KeyDlgClass, KeyDlg);

    HideAll();
    if (CtlDlg) ShowDialog(CtlDlg, false);
    if (KeyDlg) ShowDialog(KeyDlg, true);
}

// (Implement remaining Show* methods exactly as in previous message.)

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
