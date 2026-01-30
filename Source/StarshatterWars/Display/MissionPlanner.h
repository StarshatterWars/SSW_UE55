/*  Project Starshatter Wars
    Fractal Dev Studios
    Copyright (c) 2025-2026.

    SUBSYSTEM:    Stars.exe
    FILE:         MissionPlanner.h
    AUTHOR:       Carlos Bott

    OVERVIEW
    ========
    MissionPlanner (Unreal)
    - Mission planning screen controller (legacy PlanScreen equivalent).
    - Owns mission planning dialogs (Objective/Package/Nav/Weapon) + Award/Debrief.
    - Provides Show/Hide and "tab" switching, matching legacy PlanScreen behavior.
    - Uses UBaseScreen (FORM-driven) dialogs.
*/

#pragma once

#include "CoreMinimal.h"
#include "BaseScreen.h"
#include "MissionPlanner.generated.h"

class UMissionObjectiveDlg;     // (Renamed from UMsnObjDlg / MissionObjectiveDlg)
class UMissionPackageDlg;
class UMissionWeaponDlg;
class UMissionNavDlg;
class UMissionDebriefDlg;
class UMissionAwardDlg;

UCLASS()
class STARSHATTERWARS_API UMissionPlanner : public UBaseScreen
{
    GENERATED_BODY()

public:
    UMissionPlanner(const FObjectInitializer& ObjectInitializer);

    // ----------------------------------------------------------------
    // Legacy-like lifecycle
    // ----------------------------------------------------------------
    virtual void Setup();      // Create child dialogs/widgets
    virtual void TearDown();   // Destroy/remove dialogs/widgets
    virtual bool CloseTopmost();

    virtual bool IsShown() const { return bIsShown; }
    virtual void Show();
    virtual void Hide();

    // ----------------------------------------------------------------
    // Dialog switching (legacy API preserved, names kept for familiarity)
    // ----------------------------------------------------------------
    virtual void ShowMsnDlg();
    virtual void HideMsnDlg();
    virtual bool IsMsnShown() const;

    virtual void ShowMsnObjDlg();
    virtual void HideMsnObjDlg();
    virtual bool IsMsnObjShown() const;
    UMissionObjectiveDlg* GetMsnObjDlg() { return ObjectiveDlg; }

    virtual void ShowMsnPkgDlg();
    virtual void HideMsnPkgDlg();
    virtual bool IsMsnPkgShown() const;
    UMissionPackageDlg* GetMsnPkgDlg() { return PackageDlg; }

    virtual void ShowMsnWepDlg();
    virtual void HideMsnWepDlg();
    virtual bool IsMsnWepShown() const;
    UMissionWeaponDlg* GetMsnWepDlg() { return WeaponDlg; }

    virtual void ShowNavDlg();
    virtual void HideNavDlg();
    virtual bool IsNavShown() const;
    UMissionNavDlg* GetNavDlg() { return NavDlg; }

    virtual void ShowDebriefDlg();
    virtual void HideDebriefDlg();
    virtual bool IsDebriefShown() const;
    UMissionDebriefDlg* GetDebriefDlg() { return DebriefDlg; }

    virtual void ShowAwardDlg();
    virtual void HideAwardDlg();
    virtual bool IsAwardShown() const;
    UMissionAwardDlg* GetAwardDlg() { return AwardDlg; }

    virtual void HideAll();

    // ----------------------------------------------------------------
    // Frame/tick style API (if you still call it externally)
    // ----------------------------------------------------------------
    virtual void ExecFrame(float DeltaSeconds);

protected:
    // ----------------------------------------------------------------
    // UUserWidget lifecycle
    // ----------------------------------------------------------------
    virtual void NativeConstruct() override;
    virtual void NativeDestruct() override;
    virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

private:
    // ----------------------------------------------------------------
    // Helpers
    // ----------------------------------------------------------------
    template<typename TWidget>
    TWidget* CreateDialogWidget(TSubclassOf<UUserWidget> Class, const TCHAR* DebugName);

    static bool IsDialogActuallyShown(const UUserWidget* W);

    void KeepAlive(UObject* Obj);
    void ReleaseAlive(UObject* Obj);

    void ShowDialog(UUserWidget* Dlg);
    void HideDialog(UUserWidget* Dlg);

private:
    // ----------------------------------------------------------------
    // Class refs (set these from BP or defaults)
    // ----------------------------------------------------------------
    UPROPERTY(EditAnywhere, Category = "MissionPlanner|Classes")
    TSubclassOf<UUserWidget> MissionObjectiveDlgClass;

    UPROPERTY(EditAnywhere, Category = "MissionPlanner|Classes")
    TSubclassOf<UUserWidget> MissionPackageDlgClass;

    UPROPERTY(EditAnywhere, Category = "MissionPlanner|Classes")
    TSubclassOf<UUserWidget> MissionWeaponDlgClass;

    UPROPERTY(EditAnywhere, Category = "MissionPlanner|Classes")
    TSubclassOf<UUserWidget> MissionNavDlgClass;

    UPROPERTY(EditAnywhere, Category = "MissionPlanner|Classes")
    TSubclassOf<UUserWidget> DebriefDlgClass;

    UPROPERTY(EditAnywhere, Category = "MissionPlanner|Classes")
    TSubclassOf<UUserWidget> AwardDlgClass;

private:
    // ----------------------------------------------------------------
    // Owned dialogs (raw pointers by request; NO UPROPERTY(Transient))
    // NOTE: We keep them alive via AddToRoot/RemoveFromRoot in cpp.
    // ----------------------------------------------------------------
    UMissionObjectiveDlg* ObjectiveDlg = nullptr;
    UMissionPackageDlg* PackageDlg = nullptr;
    UMissionWeaponDlg* WeaponDlg = nullptr;
    UMissionNavDlg* NavDlg = nullptr;
    UMissionDebriefDlg* DebriefDlg = nullptr;
    UMissionAwardDlg* AwardDlg = nullptr;

    bool bIsShown = false;
};
