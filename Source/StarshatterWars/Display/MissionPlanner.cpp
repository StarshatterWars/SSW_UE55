/*  Project Starshatter Wars
    Fractal Dev Studios
    Copyright (c) 2025-2026.

    SUBSYSTEM:    Stars.exe
    FILE:         MissionPlanner.cpp
    AUTHOR:       Carlos Bott

    OVERVIEW
    ========
    MissionPlanner (Unreal)
    - Legacy PlanScreen port.
    - Creates and owns mission planning dialogs (FORM-driven UBaseScreen widgets).
    - Centralizes tab switching and topmost dialog closing.
*/

#include "MissionPlanner.h"

#include "Blueprint/UserWidget.h"
#include "Engine/World.h"

#include "MissionObjectiveDlg.h"
#include "MissionPackageDlg.h"
#include "MissionWeaponDlg.h"
#include "MissionNavDlg.h"
#include "MissionDebriefDlg.h"
#include "MissionAwardDlg.h"

// +--------------------------------------------------------------------+
// CONSTRUCTION / LIFECYCLE
// +--------------------------------------------------------------------+

UMissionPlanner::UMissionPlanner(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
}

void UMissionPlanner::NativeConstruct()
{
    Super::NativeConstruct();

    // If you want auto-setup on construct, uncomment:
    // Setup();
}

void UMissionPlanner::NativeDestruct()
{
    TearDown();
    Super::NativeDestruct();
}

void UMissionPlanner::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
    Super::NativeTick(MyGeometry, InDeltaTime);
    ExecFrame(InDeltaTime);
}

// +--------------------------------------------------------------------+
// SETUP / TEARDOWN
// +--------------------------------------------------------------------+

void UMissionPlanner::Setup()
{
    // Prevent double-setup:
    if (ObjectiveDlg || PackageDlg || WeaponDlg || NavDlg || DebriefDlg || AwardDlg)
    {
        return;
    }

    ObjectiveDlg = CreateDialogWidget<UMissionObjectiveDlg>(MissionObjectiveDlgClass, TEXT("MissionObjectiveDlg"));
    PackageDlg = CreateDialogWidget<UMissionPackageDlg>(MissionPackageDlgClass, TEXT("MissionPackageDlg"));
    NavDlg = CreateDialogWidget<UMissionNavDlg>(MissionNavDlgClass, TEXT("MissionNavigationDlg"));
    WeaponDlg = CreateDialogWidget<UMissionWeaponDlg>(MissionWeaponDlgClass, TEXT("MissionWeaponDlg"));
    AwardDlg = CreateDialogWidget<UMissionAwardDlg>(AwardDlgClass, TEXT("MissionAwardDlg"));
    DebriefDlg = CreateDialogWidget<UMissionDebriefDlg>(DebriefDlgClass, TEXT("MissionDebriefDlg"));

    // Start hidden; legacy PlanScreen shows Obj dialog when shown:
    HideAll();
    bIsShown = false;
}

void UMissionPlanner::TearDown()
{
    // Hide first (avoids “zombie” input focus issues in some setups)
    HideAll();

    // Remove + release in a safe order:
    auto DestroyDlg = [this](UUserWidget*& W)
        {
            if (!W) return;

            ReleaseAlive(W);

            if (W->IsInViewport())
            {
                W->RemoveFromParent();
            }

            W = nullptr;
        };

    DestroyDlg(reinterpret_cast<UUserWidget*&>(ObjectiveDlg));
    DestroyDlg(reinterpret_cast<UUserWidget*&>(PackageDlg));
    DestroyDlg(reinterpret_cast<UUserWidget*&>(WeaponDlg));
    DestroyDlg(reinterpret_cast<UUserWidget*&>(NavDlg));
    DestroyDlg(reinterpret_cast<UUserWidget*&>(DebriefDlg));
    DestroyDlg(reinterpret_cast<UUserWidget*&>(AwardDlg));

    bIsShown = false;
}

// +--------------------------------------------------------------------+
// FRAME / CLOSE TOPMOST (LEGACY BEHAVIOR)
// +--------------------------------------------------------------------+

void UMissionPlanner::ExecFrame(float DeltaSeconds)
{
    // Mirror legacy behavior: only tick visible dialogs.
    if (!bIsShown)
        return;

    // If your dialogs implement ExecFrame(DeltaSeconds), call that instead.
    // Here we do nothing because UUserWidget tick already runs; we just keep
    // the legacy PlanScreen API intact.
    (void)DeltaSeconds;
}

bool UMissionPlanner::CloseTopmost()
{
    // Legacy behavior:
    // - If debrief is shown: close it (but function can still return false)
    // - If award is shown: return true
    // - Else: return false

    if (DebriefDlg && IsDialogActuallyShown(DebriefDlg))
    {
        HideDialog(DebriefDlg);
        // Do NOT early-return; legacy continues.
    }

    if (AwardDlg && IsDialogActuallyShown(AwardDlg))
    {
        return true;
    }

    return false;
}

// +--------------------------------------------------------------------+
// SHOW / HIDE (SCREEN LEVEL)
// +--------------------------------------------------------------------+

void UMissionPlanner::Show()
{
    if (!bIsShown)
    {
        ShowMsnDlg();
        bIsShown = true;
    }
}

void UMissionPlanner::Hide()
{
    HideAll();
    bIsShown = false;
}

// +--------------------------------------------------------------------+
// DIALOG SWITCHING (LEGACY API)
// +--------------------------------------------------------------------+

void UMissionPlanner::ShowMsnDlg()
{
    HideAll();
    ShowDialog(ObjectiveDlg);
}

void UMissionPlanner::HideMsnDlg()
{
    HideAll();
}

bool UMissionPlanner::IsMsnShown() const
{
    return IsMsnObjShown() || IsMsnPkgShown() || IsMsnWepShown();
}

// -----------------------------
// Mission Objective
// -----------------------------

void UMissionPlanner::ShowMsnObjDlg()
{
    HideAll();
    ShowDialog(ObjectiveDlg);
}

void UMissionPlanner::HideMsnObjDlg()
{
    HideAll();
}

bool UMissionPlanner::IsMsnObjShown() const
{
    return ObjectiveDlg && IsDialogActuallyShown(ObjectiveDlg);
}

// -----------------------------
// Mission Package
// -----------------------------

void UMissionPlanner::ShowMsnPkgDlg()
{
    HideAll();
    ShowDialog(PackageDlg);
}

void UMissionPlanner::HideMsnPkgDlg()
{
    HideAll();
}

bool UMissionPlanner::IsMsnPkgShown() const
{
    return PackageDlg && IsDialogActuallyShown(PackageDlg);
}

// -----------------------------
// Mission Weapon
// -----------------------------

void UMissionPlanner::ShowMsnWepDlg()
{
    HideAll();
    ShowDialog(WeaponDlg);
}

void UMissionPlanner::HideMsnWepDlg()
{
    HideAll();
}

bool UMissionPlanner::IsMsnWepShown() const
{
    return WeaponDlg && IsDialogActuallyShown(WeaponDlg);
}

// -----------------------------
// Navigation
// -----------------------------

void UMissionPlanner::ShowNavDlg()
{
    // Legacy: only show if it exists and isn't already shown:
    if (NavDlg && !IsDialogActuallyShown(NavDlg))
    {
        HideAll();
        ShowDialog(NavDlg);
    }
}

void UMissionPlanner::HideNavDlg()
{
    if (NavDlg && IsDialogActuallyShown(NavDlg))
    {
        HideAll();
    }
}

bool UMissionPlanner::IsNavShown() const
{
    return NavDlg && IsDialogActuallyShown(NavDlg);
}

// -----------------------------
// Debrief
// -----------------------------

void UMissionPlanner::ShowDebriefDlg()
{
    HideAll();
    ShowDialog(DebriefDlg);
}

void UMissionPlanner::HideDebriefDlg()
{
    HideAll();
}

bool UMissionPlanner::IsDebriefShown() const
{
    return DebriefDlg && IsDialogActuallyShown(DebriefDlg);
}

// -----------------------------
// Award
// -----------------------------

void UMissionPlanner::ShowAwardDlg()
{
    HideAll();
    ShowDialog(AwardDlg);
}

void UMissionPlanner::HideAwardDlg()
{
    HideAll();
}

bool UMissionPlanner::IsAwardShown() const
{
    return AwardDlg && IsDialogActuallyShown(AwardDlg);
}

// +--------------------------------------------------------------------+
// HIDE ALL
// +--------------------------------------------------------------------+

void UMissionPlanner::HideAll()
{
    HideDialog(ObjectiveDlg);
    HideDialog(PackageDlg);
    HideDialog(WeaponDlg);
    HideDialog(NavDlg);
    HideDialog(AwardDlg);
    HideDialog(DebriefDlg);
}

// +--------------------------------------------------------------------+
// INTERNAL HELPERS
// +--------------------------------------------------------------------+

template<typename TWidget>
TWidget* UMissionPlanner::CreateDialogWidget(TSubclassOf<UUserWidget> Class, const TCHAR* DebugName)
{
    if (!Class)
    {
        UE_LOG(LogTemp, Warning, TEXT("MissionPlanner: Missing widget class for %s"), DebugName);
        return nullptr;
    }

    UWorld* World = GetWorld();
    if (!World)
    {
        UE_LOG(LogTemp, Warning, TEXT("MissionPlanner: No world for %s"), DebugName);
        return nullptr;
    }

    UUserWidget* Created = CreateWidget<UUserWidget>(World, Class);
    if (!Created)
    {
        UE_LOG(LogTemp, Warning, TEXT("MissionPlanner: Failed to create %s"), DebugName);
        return nullptr;
    }

    // Add to viewport once; toggle visibility for tab switching.
    Created->AddToViewport();
    Created->SetVisibility(ESlateVisibility::Collapsed);

    // Keep alive per your “raw pointer” requirement:
    KeepAlive(Created);

    return Cast<TWidget>(Created);
}

bool UMissionPlanner::IsDialogActuallyShown(const UUserWidget* W)
{
    if (!W) return false;
    if (!W->IsInViewport()) return false;

    const ESlateVisibility V = W->GetVisibility();
    return (V != ESlateVisibility::Collapsed && V != ESlateVisibility::Hidden);
}

void UMissionPlanner::KeepAlive(UObject* Obj)
{
    if (Obj && !Obj->IsRooted())
    {
        Obj->AddToRoot();
    }
}

void UMissionPlanner::ReleaseAlive(UObject* Obj)
{
    if (Obj && Obj->IsRooted())
    {
        Obj->RemoveFromRoot();
    }
}

void UMissionPlanner::ShowDialog(UUserWidget* Dlg)
{
    if (!Dlg) return;

    if (!Dlg->IsInViewport())
    {
        Dlg->AddToViewport();
    }

    Dlg->SetVisibility(ESlateVisibility::Visible);
    // Optional: set focus or input mode here if your UI framework needs it.
}

void UMissionPlanner::HideDialog(UUserWidget* Dlg)
{
    if (!Dlg) return;
    Dlg->SetVisibility(ESlateVisibility::Collapsed);
}
