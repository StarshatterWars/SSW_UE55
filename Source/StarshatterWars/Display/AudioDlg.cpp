/*  Project Starshatter Wars
    Fractal Dev Studios
    Copyright (c) 2025-2026.

    ORIGINAL AUTHOR AND STUDIO
    ==========================
    John DiCamillo / Destroyer Studios LLC

    SUBSYSTEM:    Stars.exe
    FILE:         AudioDlg.cpp
    AUTHOR:       Carlos Bott

    OVERVIEW
    ========
    UAudioDlg implementation (Unreal)
*/

#include "AudioDlg.h"

// UMG:
#include "Components/Button.h"
#include "Components/Slider.h"
#include "Components/TextBlock.h"

// Starshatter core:
#include "AudioConfig.h"

UAudioDlg::UAudioDlg(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    // Use UBaseScreen unified dialog input:
    SetDialogInputEnabled(true);
}

void UAudioDlg::SetManager(UGameScreen* InManager)
{
    Manager = InManager;
}

void UAudioDlg::BindFormWidgets()
{
    // ---- Labels ----
    if (TitleLabel) BindLabel(10, TitleLabel);

    // ---- Tabs (buttons) ----
    if (vid_btn) BindButton(901, vid_btn);
    if (aud_btn) BindButton(902, aud_btn);
    if (ctl_btn) BindButton(903, ctl_btn);
    if (opt_btn) BindButton(904, opt_btn);
    if (mod_btn) BindButton(905, mod_btn);

    // ---- Sliders ----
    if (efx_volume_slider) BindSlider(201, efx_volume_slider);
    if (gui_volume_slider) BindSlider(202, gui_volume_slider);
    if (wrn_volume_slider) BindSlider(203, wrn_volume_slider);
    if (vox_volume_slider) BindSlider(204, vox_volume_slider);
    if (menu_music_slider) BindSlider(205, menu_music_slider);
    if (game_music_slider) BindSlider(206, game_music_slider);

    // ---- Apply/Cancel ----
    if (ApplyBtn)  BindButton(1, ApplyBtn);
    if (CancelBtn) BindButton(2, CancelBtn);
}

FString UAudioDlg::GetLegacyFormText() const
{
    // Embedded legacy FORM text (AudDlg.frm)
    return TEXT(R"FORM(
form: {
   back_color: (  0,   0,   0),
   fore_color: (255, 255, 255),

   texture:    "Frame1.pcx",
   margins:    (1,1,64,8),

   layout: {
      x_mins:     (10, 100,  20, 100, 100, 10),
      x_weights:  ( 0, 0.2, 0.4, 0.2, 0.2,  0),

      y_mins:     (28, 30,  20,  24, 60, 45),
      y_weights:  ( 0,  0,   0,   0,  1,  0)
   },

   // background images:

   ctrl: {
      id:            9991,
      type:          background,
      texture:       Frame2a,
      cells:         (1,3,2,3),
      cell_insets:   (0,0,0,10),
      margins:       (2,32,40,32)
      hide_partial:  false
   }

   ctrl: {
      id:            9992,
      type:          background,
      texture:       Frame2b,
      cells:         (3,3,2,3),
      cell_insets:   (0,0,0,10),
      margins:       (0,40,40,32)
      hide_partial:  false
   }

   // title:

   ctrl: {
      id:            10,
      type:          label,
      text:          "Options",
      align:         left,
      font:          Limerick18,
      fore_color:    (255,255,255),
      transparent:   true,
      cells:         (1,1,3,1)
      cell_insets:   (0,0,0,0)
      hide_partial:  false
   }

   // tabs:

   ctrl: {
      id:            900
      type:          panel
      transparent:   true
      cells:         (1,3,4,1)
      hide_partial:  false

      layout: {
         x_mins:     (100, 100, 100, 100, 100, 0),
         x_weights:  (0.2, 0.2, 0.2, 0.2, 0.2, 1),

         y_mins:     (24),
         y_weights:  ( 1)
      }
   }

   defctrl: {
      align:            left,
      font:             Limerick12,
      fore_color:       (255, 255, 255),
      standard_image:   BlueTab_0,
      activated_image:  BlueTab_1,
      sticky:           true,
      bevel_width:      6,
      margins:          (8,8,0,0),
      cell_insets:      (0,4,0,0)
   },

   ctrl: { id: 901 pid: 900 type: button text: Video      cells: (0,0,1,1) },
   ctrl: { id: 902 pid: 900 type: button text: Audio      cells: (1,0,1,1) },
   ctrl: { id: 903 pid: 900 type: button text: Controls   cells: (2,0,1,1) },
   ctrl: { id: 904 pid: 900 type: button text: Gameplay   cells: (3,0,1,1) },
   ctrl: { id: 905 pid: 900 type: button text: "Mod Config" cells: (4,0,1,1) },

   // main panel:

   ctrl: {
      id:               300
      type:             panel
      transparent:      false //true

      texture:          Panel
      margins:          (12,12,12,0),

      cells:            (1,4,4,2)
      cell_insets:      (10,10,12,54)

      layout: {
         x_mins:     ( 20, 100, 100,  20, 100, 100,  20)
         x_weights:  (0.2, 0.3, 0.3, 0.2, 0.3, 0.3, 0.2)

         y_mins:     ( 20,  25,  25,  25,  25,  25,  25,  25,  20)
         y_weights:  (0.3,   0,   0,   0,   0,   0,   0,   0, 0.7)
      }
   }

   defctrl: {
      fore_color:       (255,255,255)
      font:             Verdana,
      standard_image:   ""
      activated_image:  ""
      align:            left
      sticky:           false
      transparent:      true
   },

   ctrl: { id: 101 pid: 300 type: label text: "Effects Volume:"  cells: (1,1,1,1) },
   ctrl: { id: 102 pid: 300 type: label text: "GUI Volume:"      cells: (1,2,1,1) },
   ctrl: { id: 103 pid: 300 type: label text: "Warning Volume:"  cells: (1,3,1,1) },
   ctrl: { id: 104 pid: 300 type: label text: "Vox Volume:"      cells: (1,4,1,1) },
   ctrl: { id: 105 pid: 300 type: label text: "Menu Music:"      cells: (1,6,1,1) },
   ctrl: { id: 106 pid: 300 type: label text: "In Game Music:"   cells: (1,7,1,1) },

   defctrl: {
      cell_insets:      (0,0,0,16)
      simple:           true
      bevel_width:      3
      text_align:       left
      transparent:      false

      active_color:     (250, 250, 100)
      back_color:       ( 41,  41,  41)
      border:           false
      active:           true
   },

   ctrl: { id: 201 pid: 300 type: slider cells: (2,1,1,1) },
   ctrl: { id: 202 pid: 300 type: slider cells: (2,2,1,1) },
   ctrl: { id: 203 pid: 300 type: slider cells: (2,3,1,1) },
   ctrl: { id: 204 pid: 300 type: slider cells: (2,4,1,1) },
   ctrl: { id: 205 pid: 300 type: slider cells: (2,6,1,1) },
   ctrl: { id: 206 pid: 300 type: slider cells: (2,7,1,1) },

   // buttons:

   defctrl: {
      align:            left,
      font:             Limerick12,
      fore_color:       (0,0,0),
      standard_image:   Button17_0,
      activated_image:  Button17_1,
      transition_image: Button17_2,
      bevel_width:      6,
      margins:          (3,18,0,0),
      cell_insets:      (0,10,0,26)
   },

   ctrl: { id: 1 type: button text: "Apply"  cells: (3,5,1,1) },
   ctrl: { id: 2 type: button text: "Cancel" cells: (4,5,1,1) },

}
)FORM");
}

void UAudioDlg::NativeConstruct()
{
    Super::NativeConstruct();

    RegisterControls();

    if (bClosed)
        LoadFromConfig();

    bClosed = false;
}

void UAudioDlg::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
    Super::NativeTick(MyGeometry, InDeltaTime);
    ExecFrame(InDeltaTime);
}

void UAudioDlg::RegisterControls()
{
    // Prevent double-binding on rebuild:
    if (ApplyBtn)
    {
        ApplyBtn->OnClicked.RemoveAll(this);
        ApplyBtn->OnClicked.AddDynamic(this, &UAudioDlg::HandleApplyClicked);
    }

    if (CancelBtn)
    {
        CancelBtn->OnClicked.RemoveAll(this);
        CancelBtn->OnClicked.AddDynamic(this, &UAudioDlg::HandleCancelClicked);
    }

    if (vid_btn)
    {
        vid_btn->OnClicked.RemoveAll(this);
        vid_btn->OnClicked.AddDynamic(this, &UAudioDlg::HandleVideoClicked);
    }

    if (aud_btn)
    {
        aud_btn->OnClicked.RemoveAll(this);
        aud_btn->OnClicked.AddDynamic(this, &UAudioDlg::HandleAudioClicked);
    }

    if (ctl_btn)
    {
        ctl_btn->OnClicked.RemoveAll(this);
        ctl_btn->OnClicked.AddDynamic(this, &UAudioDlg::HandleControlsClicked);
    }

    if (opt_btn)
    {
        opt_btn->OnClicked.RemoveAll(this);
        opt_btn->OnClicked.AddDynamic(this, &UAudioDlg::HandleOptionsClicked);
    }
}

void UAudioDlg::Show()
{
    LoadFromConfig();
    bClosed = false;
    SetVisibility(ESlateVisibility::Visible);
}

void UAudioDlg::ExecFrame(float DeltaTime)
{
    (void)DeltaTime;
    // Legacy AudDlg didn't require per-frame logic; keep empty unless needed.
}

// ---------------------------------------------------------------------
// Centralized Enter/Escape via UBaseScreen

void UAudioDlg::HandleAccept()
{
    OnApply();
}

void UAudioDlg::HandleCancel()
{
    OnCancel();
}

// ---------------------------------------------------------------------
// Click handlers (NO lambdas)

void UAudioDlg::HandleApplyClicked() { OnApply(); }
void UAudioDlg::HandleCancelClicked() { OnCancel(); }
void UAudioDlg::HandleAudioClicked() { OnAudio(); }
void UAudioDlg::HandleVideoClicked() { OnVideo(); }
void UAudioDlg::HandleOptionsClicked() { OnOptions(); }
void UAudioDlg::HandleControlsClicked() { OnControls(); }

// ---------------------------------------------------------------------
// Legacy semantic methods

void UAudioDlg::OnAudio()
{
    if (Manager) Manager->ShowAudDlg();
    else UE_LOG(LogTemp, Warning, TEXT("AudioDlg: Manager is null (OnAudio)."));
}

void UAudioDlg::OnVideo()
{
    if (Manager) Manager->ShowVidDlg();
    else UE_LOG(LogTemp, Warning, TEXT("AudioDlg: Manager is null (OnVideo)."));
}

void UAudioDlg::OnOptions()
{
    if (Manager) Manager->ShowOptDlg();
    else UE_LOG(LogTemp, Warning, TEXT("AudioDlg: Manager is null (OnOptions)."));
}

void UAudioDlg::OnControls()
{
    if (Manager) Manager->ShowCtlDlg();
    else UE_LOG(LogTemp, Warning, TEXT("AudioDlg: Manager is null (OnControls)."));
}

void UAudioDlg::OnApply()
{
    Apply();

    if (Manager) Manager->ApplyOptions();
    else UE_LOG(LogTemp, Warning, TEXT("AudioDlg: Manager is null (OnApply)."));
}

void UAudioDlg::OnCancel()
{
    Cancel();

    if (Manager) Manager->CancelOptions();
    else UE_LOG(LogTemp, Warning, TEXT("AudioDlg: Manager is null (OnCancel)."));
}

// ---------------------------------------------------------------------
// Operations

void UAudioDlg::Apply()
{
    if (!bClosed)
        SaveToConfig();

    bClosed = true;
}

void UAudioDlg::Cancel()
{
    bClosed = true;
}

// ---------------------------------------------------------------------
// Config IO

void UAudioDlg::LoadFromConfig()
{
    AudioConfig* Audio = AudioConfig::GetInstance();
    if (!Audio) return;

    if (efx_volume_slider)  efx_volume_slider->SetValue(Audio->GetEfxVolume());
    if (gui_volume_slider)  gui_volume_slider->SetValue(Audio->GetGuiVolume());
    if (wrn_volume_slider)  wrn_volume_slider->SetValue(Audio->GetWrnVolume());
    if (vox_volume_slider)  vox_volume_slider->SetValue(Audio->GetVoxVolume());

    if (menu_music_slider)  menu_music_slider->SetValue(Audio->GetMenuMusic());
    if (game_music_slider)  game_music_slider->SetValue(Audio->GetGameMusic());
}

void UAudioDlg::SaveToConfig()
{
    AudioConfig* Audio = AudioConfig::GetInstance();
    if (!Audio) return;

    if (efx_volume_slider)  Audio->SetEfxVolume(efx_volume_slider->GetValue());
    if (gui_volume_slider)  Audio->SetGuiVolume(gui_volume_slider->GetValue());
    if (wrn_volume_slider)  Audio->SetWrnVolume(wrn_volume_slider->GetValue());
    if (vox_volume_slider)  Audio->SetVoxVolume(vox_volume_slider->GetValue());

    if (menu_music_slider)  Audio->SetMenuMusic(menu_music_slider->GetValue());
    if (game_music_slider)  Audio->SetGameMusic(game_music_slider->GetValue());

    Audio->Save();
}
