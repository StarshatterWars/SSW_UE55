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
#include "StarshatterPlayerSubsystem.h"
#include "Blueprint/UserWidget.h"
#include "Kismet/GameplayStatics.h"
#include "GameStructs.h"
// NEW:
#include "OptionsScreen.h"
#include "StarshatterAssetRegistrySubsystem.h"

// ------------------------------------------------------------

void UMenuScreen::Initialize(UGameInstance* InGI)
{
    if (!InGI) return;

    UStarshatterAssetRegistrySubsystem* Assets =
        InGI->GetSubsystem<UStarshatterAssetRegistrySubsystem>();
    if (!ensure(Assets)) return;

    // ? Put your three lines here:
    //MenuScreenWidgetClass = ResolveWidgetOrLog(Assets, TEXT("UI.MenuScreenClass"));
    MenuDlgClass = ResolveWidgetOrLog<UMenuDlg>(Assets, TEXT("UI.MenuScreenClass"));

    FirstTimeDlgClass = ResolveWidgetOrLog<UFirstTimeDlg>(Assets, TEXT("UI.FirstRunDlgClass"));
    ExitDlgClass = ResolveWidgetOrLog<UExitDlg>(Assets, TEXT("UI.ExitDlgClass"));

    PlayerDlgClass = ResolveWidgetOrLog<UPlayerDlg>(Assets, TEXT("UI.PlayerDlgClass"));
    AwardDlgClass = ResolveWidgetOrLog<UAwardShowDlg>(Assets, TEXT("UI.AwardDlgClass"));

    MsnSelectDlgClass = ResolveWidgetOrLog<UMissionSelectDlg>(Assets, TEXT("UI.MissionSelectDlgClass"));
    CmpSelectDlgClass = ResolveWidgetOrLog<UCampaignSelectDlg>(Assets, TEXT("UI.CampaignSelectDlgClass"));

    MsnEditDlgClass = ResolveWidgetOrLog<UMissionEditorDlg>(Assets, TEXT("UI.MissionEditorDlgClass"));
    MsnElemDlgClass = ResolveWidgetOrLog<UMissionElementDlg>(Assets, TEXT("UI.MissionElementDlgClass"));
    MsnEventDlgClass = ResolveWidgetOrLog<UMissionEventDlg>(Assets, TEXT("UI.MissionEventDlgClass"));
    MsnEditNavDlgClass = ResolveWidgetOrLog<UMissionEditorNavDlg>(Assets, TEXT("UI.MissionEditorNavDlgClass"));

    LoadDlgClass = ResolveWidgetOrLog<ULoadDlg>(Assets, TEXT("UI.LoadDlgClass"));
    TacRefDlgClass = ResolveWidgetOrLog<UTacRefDlg>(Assets, TEXT("UI.TacRefDlgClass"));

    // THE ONE YOU ASKED FOR:
    OptionsScreenClass = ResolveWidgetOrLog<UOptionsScreen>(Assets, TEXT("UI.OptionsScreenClass"));
}

static void ApplyUIFocus(APlayerController* PC, UUserWidget* FocusWidget)
{
    if (!PC || !FocusWidget)
        return;

    PC->bShowMouseCursor = true;
    PC->bEnableClickEvents = true;
    PC->bEnableMouseOverEvents = true;

    // Use GameAndUI for reliability (especially with Enhanced Input / legacy code)
    FInputModeGameAndUI Mode;
    Mode.SetWidgetToFocus(FocusWidget->TakeWidget());
    Mode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
    Mode.SetHideCursorDuringCapture(false);

    PC->SetInputMode(Mode);
}

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
        return Storage.Get();

    if (!ClassToSpawn)
        return nullptr;

    APlayerController* PC = GetOwningPlayer();
    if (!PC)
    {
        UE_LOG(LogTemp, Warning, TEXT("EnsureDialog: OwningPlayer is null (%s)"), *GetName());
        return nullptr;
    }

    TDialog* Created = CreateWidget<TDialog>(PC, ClassToSpawn);
    if (!Created)
        return nullptr;

    // Router assignment (BaseScreen owns MenuManager)
    Created->SetMenuManager(this);

    // Park hidden; real Z-order is applied by ShowDialog(...)
    Created->AddToViewport(10);
    Created->SetVisibility(ESlateVisibility::Hidden);
    Created->SetIsEnabled(false);

    Storage = Created;
    return Created;
}

// ------------------------------------------------------------
// Show/Hide helpers
// ------------------------------------------------------------

void UMenuScreen::ShowDialog(UBaseScreen* Dialog, int32 ZOrder)
{
    if (!Dialog) {
        return;
    }

    if (Dialog->IsInViewport())
        Dialog->RemoveFromParent();

    Dialog->AddToViewport(ZOrder);
    Dialog->SetVisibility(ESlateVisibility::Visible);
    Dialog->SetDialogInputEnabled(true);

    CurrentDialog = Dialog;

    // Always restore UI input to the visible dialog
    if (APlayerController* PC = GetOwningPlayer())
    {
        PC->bShowMouseCursor = true;
        PC->bEnableClickEvents = true;
        PC->bEnableMouseOverEvents = true;

        FInputModeGameAndUI Mode;
        Mode.SetWidgetToFocus(Dialog->TakeWidget());
        Mode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
        Mode.SetHideCursorDuringCapture(false);
        PC->SetInputMode(Mode);
    }
}

void UMenuScreen::HideDialog(UBaseScreen* Dialog)
{
    if (!Dialog)
        return;

    Dialog->SetDialogInputEnabled(false);
    Dialog->SetVisibility(ESlateVisibility::Collapsed);


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

/* --------------------------------------------------------------------
   Show
   -------------------------------------------------------------------- */

void UMenuScreen::Show()
{
   UE_LOG(LogTemp, Warning, TEXT("[MenuScreen] UMenuScreen::Show()"));
   if (bIsShown)
        return;

    bool bHasSaveNow = false;

    if (UGameInstance* GI = GetGameInstance())
    {
        if (UStarshatterPlayerSubsystem* PlayerSS =
            GI->GetSubsystem<UStarshatterPlayerSubsystem>())
        {
            UE_LOG(LogTemp, Warning, TEXT("[MenuScreen] UStarshatterPlayerSubsystem"));
           
           if (!PlayerSS->HasLoaded())
            {
                PlayerSS->LoadFromBoot();
            }

            bHasSaveNow = PlayerSS->DoesSaveExistNow();

            UE_LOG(LogTemp, Warning,
                TEXT("[MenuScreen] SaveExistsNow=%d"),
                bHasSaveNow ? 1 : 0);
        }
    }

    if (bHasSaveNow)
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
        MenuDlg->AddToViewport((int32) EMenuZOrder::Z_MENU_BASE);                    // added once; Z handled below
        MenuDlg->SetDialogInputEnabled(false);
        MenuDlg->SetVisibility(ESlateVisibility::Hidden);
    }

    // Always re-assert manager in case of re-instancing
    MenuDlg->Manager = this;

    // Bring to front (no SetZOrderInViewport dependency)
    ++ZCounter;
    if (MenuDlg->IsInViewport())
        MenuDlg->RemoveFromParent();
    MenuDlg->AddToViewport((int32)EMenuZOrder::Z_MENU_BASE);

    MenuDlg->SetIsEnabled(true);
    MenuDlg->SetIsFocusable(true);
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
    // Ensure Menu exists
    if (!MenuDlg)
        ShowMenuDlg();

    // Ensure FirstTimeDlg exists
    if (!FirstTimeDlg && FirstTimeDlgClass)
    {
        FirstTimeDlg = CreateWidget<UFirstTimeDlg>(GetOwningPlayer(), FirstTimeDlgClass);
        if (!FirstTimeDlg)
            return;

        FirstTimeDlg->SetMenuManager(this);
        FirstTimeDlg->AddToViewport((int32)EMenuZOrder::Z_MODAL);
    }

    // ----------------------------------------------------
    // Z-ORDER + INPUT CONTROL
    // ----------------------------------------------------

    // Menu stays visible but cannot receive input
    ShowDialog(MenuDlg, (int32)EMenuZOrder::Z_MENU_BASE);
    MenuDlg->SetDialogInputEnabled(false);

    // FirstTimeDlg is modal and interactive
    ShowDialog(FirstTimeDlg, (int32)EMenuZOrder::Z_MODAL);
    FirstTimeDlg->SetDialogInputEnabled(true);

    CurrentDialog = FirstTimeDlg;
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
    EnsureDialog<UMenuDlg>(MenuDlgClass, MenuDlg);
    EnsureDialog<UExitDlg>(ExitDlgClass, ExitDlg);

    if (!MenuDlg || !ExitDlg) return;

    MenuDlg->SetMenuManager(this);
    ExitDlg->SetMenuManager(this);

    // Menu behind, inert:
    MenuDlg->SetDialogInputEnabled(false);
    ShowDialog(MenuDlg, 100);

    // ExitDlg on top, interactive:
    ExitDlg->SetDialogInputEnabled(true);
    ShowDialog(ExitDlg, 200);
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



