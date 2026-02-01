/*  Project Starshatter Wars
    Fractal Dev Studios
    Copyright (c) 2025-2026.

    SUBSYSTEM:    Stars.exe
    FILE:         GameScreen.h
    AUTHOR:       Carlos Bott

    ORIGINAL AUTHOR AND STUDIO
    ==========================
    John DiCamillo / Destroyer Studios LLC

    OVERVIEW
    ========
    UGameScreen
    - Unreal port of legacy GameScreen.
    - Implemented as a UMG UserWidget (inherits from UBaseScreen).
    - Owns and spawns all in-game dialog widgets (Nav/Eng/Flt/Ctl/Key/Joy/Audio/Vid/Opt/Quit).
    - Preserves legacy game logic flow (ExecFrame, CloseTopmost, Show/Hide, option apply/cancel).
    - Legacy "views" (HUD/Wep/Quantum/Radio/Tactical) remain pure C++ systems and are ticked from ExecFrame.
*/
#pragma once

#include "CoreMinimal.h"
#include "BaseScreen.h"   
#include "CameraView.h"
#include "GameScreen.generated.h"

// Forward declares (UMG dialogs):
class UNavDlg;
class UEngineeringDlg;
class UFlightOpsDlg;
class UControlOptionsDlg;
class UJoyDlg;
class UKeyDlg;
class UWepDlg;
class UAudioDlg;
class UVideoDlg;
class UOptDlg;
class QuitView;

// Legacy systems (raw classes):
class Sim;
class Ship;
class Starshatter;
class CameraManager;
class DisplayView;
class HUDView;
class WepView;
class QuantumView;
class CameraView;
class RadioView;
class TacticalView;
class Bitmap;
class DataLoader;
class SystemFont;

UCLASS()
class STARSHATTERWARS_API UGameScreen : public UBaseScreen
{
    GENERATED_BODY()

public:
    UGameScreen(const FObjectInitializer& ObjectInitializer);

    // Lifecycle (UE-style):
    UFUNCTION() void Setup();
    UFUNCTION() void TearDown();

    UFUNCTION() bool CloseTopmost();

    UFUNCTION() void Show();
    UFUNCTION() void Hide();

    UFUNCTION() bool IsShown() const { return bIsShown; }
    UFUNCTION() bool IsFormShown() const;

    // Dialog show/hide:
    UFUNCTION() void ShowNavDlg();
    UFUNCTION() void HideNavDlg();
    UFUNCTION() bool IsNavShown() const;

    UFUNCTION() void ShowEngDlg();
    UFUNCTION() void HideEngDlg();
    UFUNCTION() bool IsEngShown() const;

    UFUNCTION() void ShowFltDlg();
    UFUNCTION() void HideFltDlg();
    UFUNCTION() bool IsFltShown() const;

    UFUNCTION() void ShowWepDlg();
    UFUNCTION() void HideWepDlg();
    UFUNCTION() bool IsWepShown() const;

    UFUNCTION() void ShowCtlDlg();
    UFUNCTION() void HideCtlDlg();
    UFUNCTION() bool IsCtlShown() const;

    UFUNCTION() void ShowKeyDlg();
    UFUNCTION() bool IsKeyShown() const;

    UFUNCTION() void ShowJoyDlg();
    UFUNCTION() bool IsJoyShown() const;

    UFUNCTION() void ShowAudDlg();
    UFUNCTION() void HideAudDlg();
    UFUNCTION() bool IsAudShown() const;

    UFUNCTION() void ShowVidDlg();
    UFUNCTION() void HideVidDlg();
    UFUNCTION() bool IsVidShown() const;

    UFUNCTION() void ShowOptDlg();
    UFUNCTION() void HideOptDlg();
    UFUNCTION() bool IsOptShown() const;

    UFUNCTION() void ApplyOptions();
    UFUNCTION() void CancelOptions();

    // Legacy ops:
    
    void SetFieldOfView(double Fov);
    double GetFieldOfView() const;

    void CycleMFDMode(int Mfd);
    void CycleHUDMode();
    void CycleHUDColor();
    void CycleHUDWarn();

    void FrameRate(double F);
    void ExecFrame(float DeltaTime);

    // Accessors (keep raw pointers):
    UNavDlg* GetNavDlg()   const { return NavDlg; }
    UEngineeringDlg*    GetEngDlg()   const { return EngDlg; }
    UFlightOpsDlg*      GetFltDlg()   const { return FltDlg; }
    UControlOptionsDlg* GetCtlDlg()   const { return CtlDlg; }
    UKeyDlg* GetKeyDlg()   const { return KeyDlg; }
    UJoyDlg* GetJoyDlg()   const { return JoyDlg; }
    UAudioDlg* GetAudioDlg() const { return AudioDlg; }
    UVideoDlg* GetVidDlg()   const { return VidDlg; }
    UOptDlg* GetOptDlg()   const { return OptDlg; }
    QuitView* GetQuitView() const { return QuitViewClass; }

    static UGameScreen* GetInstance() { return GameScreenInstance; }

protected:
    virtual void NativeDestruct() override;
    virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

private:
    UUserWidget* MakeDlg(TSubclassOf<UUserWidget> Class, int32 ZOrder);

    void HideAll();
    void SetDlgVisible(UUserWidget* W, bool bVisible) const;
    bool IsDlgVisible(const UUserWidget* W) const;

private:
    // Spawnable widget classes (assign in defaults or BP):
    UPROPERTY(EditDefaultsOnly, Category = "UI|Classes") TSubclassOf<UNavDlg>   NavDlgClass;
    UPROPERTY(EditDefaultsOnly, Category = "UI|Classes") TSubclassOf<UEngineeringDlg>   EngDlgClass;
    UPROPERTY(EditDefaultsOnly, Category = "UI|Classes") TSubclassOf<UFlightOpsDlg>   FltDlgClass;
    UPROPERTY(EditDefaultsOnly, Category = "UI|Classes") TSubclassOf<UControlOptionsDlg>   CtlDlgClass;
    UPROPERTY(EditDefaultsOnly, Category = "UI|Classes") TSubclassOf<UKeyDlg>   KeyDlgClass;
    UPROPERTY(EditDefaultsOnly, Category = "UI|Classes") TSubclassOf<UJoyDlg>   JoyDlgClass;
    UPROPERTY(EditDefaultsOnly, Category = "UI|Classes") TSubclassOf<UAudioDlg> AudioDlgClass;
    UPROPERTY(EditDefaultsOnly, Category = "UI|Classes") TSubclassOf<UVideoDlg>   VidDlgClass;
    UPROPERTY(EditDefaultsOnly, Category = "UI|Classes") TSubclassOf<UOptDlg>   OptDlgClass;
    QuitView*   QuitViewClass;

    // Spawned dialog instances:
    UNavDlg* NavDlg = nullptr;
    UEngineeringDlg*    EngDlg = nullptr;
    UFlightOpsDlg*      FltDlg = nullptr;
    UControlOptionsDlg* CtlDlg = nullptr;
    UKeyDlg* KeyDlg = nullptr;
    UJoyDlg* JoyDlg = nullptr;
    UAudioDlg* AudioDlg = nullptr;
    UVideoDlg* VidDlg = nullptr;
    UWepDlg* WepDlg = nullptr;
    UOptDlg* OptDlg = nullptr;

    // Legacy raw systems (keep logic; no UPROPERTY):
    Sim* sim = nullptr;
    CameraManager* cam_dir = nullptr;
    DisplayView* disp_view = nullptr;
    HUDView* hud_view = nullptr;
    WepView* wep_view = nullptr;
    QuantumView* quantum_view = nullptr;
    RadioView* radio_view = nullptr;
    TacticalView* tac_view = nullptr;
    CameraView* cam_view = nullptr;
    QuitView* quit_View = nullptr;

    DataLoader* loader = nullptr;
    Bitmap* flare1 = nullptr;
    Bitmap* flare2 = nullptr;
    Bitmap* flare3 = nullptr;
    Bitmap* flare4 = nullptr;

    double frame_rate = 0.0;
    bool   bIsShown = false;

    static UGameScreen* GameScreenInstance;
};
