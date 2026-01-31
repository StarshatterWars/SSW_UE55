/*  Project Starshatter Wars
    Fractal Dev Studios
    Copyright (c) 2025-2026.

    SUBSYSTEM:    Stars.exe
    FILE:         GameScreen.cpp
    AUTHOR:       Carlos Bott

    ORIGINAL AUTHOR AND STUDIO
    ==========================
    John DiCamillo / Destroyer Studios LLC

    OVERVIEW
    ========
    UGameScreen
    - UMG UserWidget replacement for legacy GameScreen.
    - Spawns dialog widgets and manages visibility instead of Screen/Window/FormDef.
    - Keeps legacy game logic/tick flow in ExecFrame/CloseTopmost.
*/
#include "GameScreen.h"

// UMG:
#include "Blueprint/UserWidget.h"

// Dialog widget headers (your actual paths):
#include "NavDlg.h"
#include "EngineeringDlg.h"
#include "FlightOpsDlg.h"
#include "ControlOptionsDlg.h"
#include "KeyDlg.h"
#include "JoyDlg.h"
#include "AudioDlg.h"
#include "VideoDlg.h"
#include "OptDlg.h"
#include "QuitView.h"   // <- make your UQuitView widget header name match

// Legacy:
#include "Sim.h"
#include "Ship.h"
#include "Starshatter.h"
#include "CameraManager.h"
#include "DisplayView.h"
#include "HUDView.h"
#include "WepView.h"
#include "QuantumView.h"
#include "RadioView.h"
#include "TacticalView.h"
#include "CameraView.h"
#include "DataLoader.h"
#include "Bitmap.h"
#include "HUDSounds.h"
#include "Game.h"

UGameScreen* UGameScreen::GameScreenInstance = nullptr;

UGameScreen::UGameScreen(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    GameScreenInstance = this;
}

void UGameScreen::NativeDestruct()
{
    TearDown();
    if (GameScreenInstance == this)
        GameScreenInstance = nullptr;

    Super::NativeDestruct();
}

void UGameScreen::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
    ExecFrame(InDeltaTime);
}

UUserWidget* UGameScreen::MakeDlg(TSubclassOf<UUserWidget> Class, int32 ZOrder)
{
    if (!Class)
        return nullptr;

    UWorld* World = GetWorld();
    if (!World)
        return nullptr;

    UUserWidget* W = CreateWidget<UUserWidget>(World, Class);
    if (W)
    {
        W->AddToViewport(ZOrder);
        W->SetVisibility(ESlateVisibility::Hidden);
    }

    return W;
}

void UGameScreen::SetDlgVisible(UUserWidget* W, bool bVisible) const
{
    if (!W)
        return;

    W->SetVisibility(bVisible ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
}

bool UGameScreen::IsDlgVisible(const UUserWidget* W) const
{
    return W && W->GetVisibility() == ESlateVisibility::Visible;
}

// +--------------------------------------------------------------------+

void UGameScreen::Setup()
{
    // Cache legacy singletons/pointers (raw):
    sim = Sim::GetSim();
    loader = DataLoader::GetLoader();
    cam_dir = CameraManager::GetInstance();
    disp_view = DisplayView::GetInstance();

    // Create dialogs (ZOrder: keep Quit highest/top-most):
    NavDlg = Cast<UNavDlg>(MakeDlg(NavDlgClass, 40));
    EngDlg = Cast<UEngineeringDlg>(MakeDlg(EngDlgClass, 40));
    FltDlg = Cast<UFlightOpsDlg>(MakeDlg(FltDlgClass, 40));
    CtlDlg = Cast<UControlOptionsDlg>(MakeDlg(CtlDlgClass, 50));
    KeyDlg = Cast<UKeyDlg>(MakeDlg(KeyDlgClass, 60));
    JoyDlg = Cast<UJoyDlg>(MakeDlg(JoyDlgClass, 60));
    AudioDlg = Cast<UAudioDlg>(MakeDlg(AudioDlgClass, 70));
    VidDlg = Cast<UVideoDlg>(MakeDlg(VidDlgClass, 70));
    OptDlg = Cast<UOptDlg>(MakeDlg(OptDlgClass, 70));

    // Quit menu should be "last in chain" => highest Z-order:
    QuitView = Cast<UQuitView>(MakeDlg(QuitViewClass, 100));

    HideAll();
    bIsShown = false;
}

void UGameScreen::TearDown()
{
    auto Kill = [](UUserWidget*& W)
        {
            if (W)
            {
                W->RemoveFromParent();
                W = nullptr;
            }
        };

    UUserWidget* W = nullptr;

    W = NavDlg;   Kill(W); NavDlg = nullptr;
    W = EngDlg;   Kill(W); EngDlg = nullptr;
    W = FltDlg;   Kill(W); FltDlg = nullptr;
    W = CtlDlg;   Kill(W); CtlDlg = nullptr;
    W = KeyDlg;   Kill(W); KeyDlg = nullptr;
    W = JoyDlg;   Kill(W); JoyDlg = nullptr;
    W = AudioDlg; Kill(W); AudioDlg = nullptr;
    W = VidDlg;   Kill(W); VidDlg = nullptr;
    W = OptDlg;   Kill(W); OptDlg = nullptr;
    W = QuitView; Kill(W); QuitView = nullptr;

    // legacy pointers are not owned here; just clear:
    sim = nullptr;
    cam_dir = nullptr;
    disp_view = nullptr;
    loader = nullptr;

    bIsShown = false;
}

void UGameScreen::Show()
{
    if (bIsShown)
        return;

    bIsShown = true;

    // In UMG, "show" usually means make root visible:
    SetVisibility(ESlateVisibility::Visible);

    // Your legacy logic that was tied to Screen/Window attach would go here if needed.
}

void UGameScreen::Hide()
{
    if (!bIsShown)
        return;

    HideAll();

    // Stop HUD alert sound (legacy parity):
    HUDSounds::StopSound(HUDSounds::SND_RED_ALERT);

    SetVisibility(ESlateVisibility::Hidden);
    bIsShown = false;
}

void UGameScreen::HideAll()
{
    SetDlgVisible(NavDlg, false);
    SetDlgVisible(EngDlg, false);
    SetDlgVisible(FltDlg, false);
    SetDlgVisible(CtlDlg, false);
    SetDlgVisible(KeyDlg, false);
    SetDlgVisible(JoyDlg, false);
    SetDlgVisible(AudioDlg, false);
    SetDlgVisible(VidDlg, false);
    SetDlgVisible(OptDlg, false);

    // Quit menu should also be hidden unless explicitly shown:
    SetDlgVisible(QuitView, false);
}

bool UGameScreen::IsFormShown() const
{
    return IsDlgVisible(NavDlg) ||
        IsDlgVisible(EngDlg) ||
        IsDlgVisible(FltDlg) ||
        IsDlgVisible(AudioDlg) ||
        IsDlgVisible(VidDlg) ||
        IsDlgVisible(OptDlg) ||
        IsDlgVisible(CtlDlg) ||
        IsDlgVisible(KeyDlg) ||
        IsDlgVisible(JoyDlg);
}

// +--------------------------------------------------------------------+
// Dialog show/hide conversions (no Screen/Window; visibility only)

void UGameScreen::ShowNavDlg()
{
    if (!NavDlg)
        return;

    if (!IsDlgVisible(NavDlg))
    {
        HideAll();

        // Preserve legacy "assign data then show":
        sim = Sim::GetSim();
        if (sim)
        {
            // If your UNavDlg keeps legacy-style setters:
            NavDlg->SetSystem(sim->GetStarSystem());
            NavDlg->SetShip(sim->GetPlayerShip());
        }

        SetDlgVisible(NavDlg, true);
        Starshatter::GetInstance()->Pause(true);
    }
    else
    {
        HideNavDlg();
    }
}

void UGameScreen::HideNavDlg()
{
    if (!NavDlg)
        return;

    if (IsDlgVisible(NavDlg))
    {
        SetDlgVisible(NavDlg, false);
        Starshatter::GetInstance()->Pause(false);
    }
}

bool UGameScreen::IsNavShown() const
{
    return IsDlgVisible(NavDlg);
}

void UGameScreen::ShowEngDlg()
{
    if (!EngDlg)
        return;

    if (!IsDlgVisible(EngDlg))
    {
        HideAll();

        sim = Sim::GetSim();
        if (sim)
            EngDlg->SetShip(sim->GetPlayerShip());

        SetDlgVisible(EngDlg, true);
        Starshatter::GetInstance()->Pause(true);
    }
    else
    {
        HideEngDlg();
    }
}

void UGameScreen::HideEngDlg()
{
    if (EngDlg && IsDlgVisible(EngDlg))
    {
        SetDlgVisible(EngDlg, false);
        Starshatter::GetInstance()->Pause(false);
    }
}

bool UGameScreen::IsEngShown() const
{
    return IsDlgVisible(EngDlg);
}

void UGameScreen::ShowFltDlg()
{
    if (!FltDlg)
        return;

    if (!IsDlgVisible(FltDlg))
    {
        HideAll();

        sim = Sim::GetSim();
        if (sim)
            FltDlg->SetShip(sim->GetPlayerShip());

        SetDlgVisible(FltDlg, true);
        Starshatter::GetInstance()->Pause(true);
    }
    else
    {
        HideFltDlg();
    }
}

void UGameScreen::HideFltDlg()
{
    if (FltDlg && IsDlgVisible(FltDlg))
    {
        SetDlgVisible(FltDlg, false);
        Starshatter::GetInstance()->Pause(false);
    }
}

bool UGameScreen::IsFltShown() const
{
    return IsDlgVisible(FltDlg);
}

void UGameScreen::ShowWepDlg()
{
if (wep_view)
       wep_view->CycleOverlayMode();
}

void
UGameScreen::HideWepDlg()
{
    if (wep_view)
        wep_view->SetOverlayMode(0);
}

void UGameScreen::ShowCtlDlg()
{
    if (!CtlDlg)
        return;

    // Legacy behavior: if Quit is open, close it then pause:
    if (QuitView && QuitView->IsMenuShown())
        QuitView->CloseMenu();

    HideAll();

    SetDlgVisible(CtlDlg, true);
    Starshatter::GetInstance()->Pause(true);
}

void UGameScreen::HideCtlDlg()
{
    if (CtlDlg && IsDlgVisible(CtlDlg))
    {
        SetDlgVisible(CtlDlg, false);
        Starshatter::GetInstance()->Pause(false);

        // Legacy behavior: if quit menu was open, show it again:
        if (QuitView)
            QuitView->ShowMenu();
    }
}

bool UGameScreen::IsCtlShown() const
{
    return IsDlgVisible(CtlDlg);
}

void UGameScreen::ShowKeyDlg()
{
    if (!KeyDlg)
        return;

    if (QuitView && QuitView->IsMenuShown())
        QuitView->CloseMenu();

    HideAll();

    // Legacy: show control dlg under key dlg:
    if (CtlDlg)
        SetDlgVisible(CtlDlg, true);

    SetDlgVisible(KeyDlg, true);
    Starshatter::GetInstance()->Pause(true);
}

bool UGameScreen::IsKeyShown() const
{
    return IsDlgVisible(KeyDlg);
}

void UGameScreen::ShowJoyDlg()
{
    if (!JoyDlg)
        return;

    if (QuitView && QuitView->IsMenuShown())
        QuitView->CloseMenu();

    HideAll();

    // Legacy: show control dlg under joy dlg:
    if (CtlDlg)
        SetDlgVisible(CtlDlg, true);

    SetDlgVisible(JoyDlg, true);
    Starshatter::GetInstance()->Pause(true);
}

bool UGameScreen::IsJoyShown() const
{
    return IsDlgVisible(JoyDlg);
}

void UGameScreen::ShowAudDlg()
{
    if (!AudioDlg)
        return;

    if (QuitView && QuitView->IsMenuShown())
        QuitView->CloseMenu();

    HideAll();
    SetDlgVisible(AudioDlg, true);
    Starshatter::GetInstance()->Pause(true);
}

void UGameScreen::HideAudDlg()
{
    if (AudioDlg && IsDlgVisible(AudioDlg))
    {
        SetDlgVisible(AudioDlg, false);
        Starshatter::GetInstance()->Pause(false);

        if (QuitView)
            QuitView->ShowMenu();
    }
}

bool UGameScreen::IsAudShown() const
{
    return IsDlgVisible(AudioDlg);
}

void UGameScreen::ShowVidDlg()
{
    if (!VidDlg)
        return;

    if (QuitView && QuitView->IsMenuShown())
        QuitView->CloseMenu();

    HideAll();
    SetDlgVisible(VidDlg, true);
    Starshatter::GetInstance()->Pause(true);
}

void UGameScreen::HideVidDlg()
{
    if (VidDlg && IsDlgVisible(VidDlg))
    {
        SetDlgVisible(VidDlg, false);
        Starshatter::GetInstance()->Pause(false);

        if (QuitView)
            QuitView->ShowMenu();
    }
}

bool UGameScreen::IsVidShown() const
{
    return IsDlgVisible(VidDlg);
}

void UGameScreen::ShowOptDlg()
{
    if (!OptDlg)
        return;

    if (QuitView && QuitView->IsMenuShown())
        QuitView->CloseMenu();

    HideAll();
    SetDlgVisible(OptDlg, true);
    Starshatter::GetInstance()->Pause(true);
}

void UGameScreen::HideOptDlg()
{
    if (OptDlg && IsDlgVisible(OptDlg))
    {
        SetDlgVisible(OptDlg, false);
        Starshatter::GetInstance()->Pause(false);

        if (QuitView)
            QuitView->ShowMenu();
    }
}

bool UGameScreen::IsOptShown() const
{
    return IsDlgVisible(OptDlg);
}

// +--------------------------------------------------------------------+
// Options apply/cancel (keep same logic; call into widget methods)

void UGameScreen::ApplyOptions()
{
    if (CtlDlg)   CtlDlg->Apply();
    if (OptDlg)   OptDlg->Apply();
    if (AudioDlg) AudioDlg->Apply();
    if (VidDlg)   VidDlg->ApplySettings();

    HideAll();
    Starshatter::GetInstance()->Pause(false);
}

void UGameScreen::CancelOptions()
{
    if (CtlDlg)   CtlDlg->Cancel();
    if (OptDlg)   OptDlg->Cancel();
    if (AudioDlg) AudioDlg->Cancel();
    if (VidDlg)   VidDlg->CancelSettings();

    HideAll();
    Starshatter::GetInstance()->Pause(false);
}

// +--------------------------------------------------------------------+
// Legacy camera/hud helpers

void UGameScreen::FrameRate(double F)
{
    frame_rate = F;
}

void UGameScreen::SetFieldOfView(double Fov)
{
    if (cam_view)
        cam_view->SetFieldOfView(Fov);
}

double UGameScreen::GetFieldOfView() const
{
    return cam_view ? cam_view->GetFieldOfView() : 0.0;
}

void UGameScreen::CycleMFDMode(int Mfd)
{
    if (hud_view)
        hud_view->CycleMFDMode(Mfd);
}

void UGameScreen::CycleHUDMode()
{
    if (hud_view)
        hud_view->CycleHUDMode();
}

void UGameScreen::CycleHUDColor()
{
    if (hud_view)
        hud_view->CycleHUDColor();
}

void UGameScreen::CycleHUDWarn()
{
    if (hud_view)
        hud_view->CycleHUDWarn();
}

// +--------------------------------------------------------------------+
// ExecFrame: preserve your legacy logic (dialogs tick themselves; views tick when not blocked)

void UGameScreen::ExecFrame(float DeltaTime)
{
    sim = Sim::GetSim();
    if (!sim)
        return;

    Ship* player = sim->GetPlayerShip();
    if (!player)
        return;

    bool bDialogShowing =
        IsFormShown() ||
        (QuitView && QuitView->IsMenuShown());

    // HUD always updates:
    if (hud_view)
    {
        hud_view->UseCameraView(cam_view);
        hud_view->ExecFrame();
    }

    // Quit menu:
    if (QuitView && QuitView->IsMenuShown())
    {
        QuitView->ExecFrame(DeltaTime);
        bDialogShowing = true;
    }

    // Dialog ticks (UMG-style):
    if (NavDlg && IsDlgVisible(NavDlg))
    {
        NavDlg->SetShip(player);
        NavDlg->ExecFrame();
    }

    if (EngDlg && IsDlgVisible(EngDlg))
    {
        EngDlg->SetShip(player);
        EngDlg->ExecFrame(DeltaTime);
    }

    if (FltDlg && IsDlgVisible(FltDlg))
    {
        FltDlg->SetShip(player);
        FltDlg->ExecFrame();
    }

    if (AudioDlg && IsDlgVisible(AudioDlg))
        AudioDlg->ExecFrame(DeltaTime);

    if (VidDlg && IsDlgVisible(VidDlg))
        VidDlg->ExecFrame(DeltaTime);

    if (OptDlg && IsDlgVisible(OptDlg))
        OptDlg->ExecFrame();

    if (CtlDlg && IsDlgVisible(CtlDlg))
        CtlDlg->ExecFrame();

    if (KeyDlg && IsDlgVisible(KeyDlg))
        KeyDlg->ExecFrame();

    if (JoyDlg && IsDlgVisible(JoyDlg))
        JoyDlg->ExecFrame();

    // Only run these when no dialog is blocking:
    if (!bDialogShowing)
    {
        if (quantum_view) quantum_view->ExecFrame();
        if (radio_view)   radio_view->ExecFrame();
        if (wep_view)     wep_view->ExecFrame();
        if (tac_view)     tac_view->ExecFrame();
    }

    if (disp_view)
        disp_view->ExecFrame();
}

// +--------------------------------------------------------------------+

bool UGameScreen::CloseTopmost()
{
    // Keep close priority similar to legacy:
    if (NavDlg && IsDlgVisible(NavDlg)) { HideNavDlg(); return true; }
    if (EngDlg && IsDlgVisible(EngDlg)) { HideEngDlg(); return true; }
    if (FltDlg && IsDlgVisible(FltDlg)) { HideFltDlg(); return true; }

    if (KeyDlg && IsDlgVisible(KeyDlg)) { ShowCtlDlg(); return true; }
    if (JoyDlg && IsDlgVisible(JoyDlg)) { ShowCtlDlg(); return true; }

    if (AudioDlg && IsDlgVisible(AudioDlg)) { CancelOptions(); return true; }
    if (VidDlg && IsDlgVisible(VidDlg)) { CancelOptions(); return true; }
    if (OptDlg && IsDlgVisible(OptDlg)) { CancelOptions(); return true; }
    if (CtlDlg && IsDlgVisible(CtlDlg)) { CancelOptions(); return true; }

    // Menu views:
    if (quantum_view && quantum_view->IsMenuShown()) { quantum_view->CloseMenu(); return true; }
    if (QuitView && QuitView->IsMenuShown()) { QuitView->CloseMenu();     return true; }
    if (radio_view && radio_view->IsMenuShown()) { radio_view->CloseMenu();   return true; }

    return false;
}




