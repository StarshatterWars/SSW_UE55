/*  Project Starshatter Wars
    Fractal Dev Studios
    Copyright (c) 2025-2026. All Rights Reserved.

    ORIGINAL AUTHOR AND STUDIO
    ==========================
    John DiCamillo / Destroyer Studios LLC

    SUBSYSTEM:    Stars.exe
    FILE:         AudioDlg.cpp
    AUTHOR:       Carlos Bott

    OVERVIEW
    ========
    Main Menu Audio Dialog Unreal User Widget implementation.
    Port of Starshatter 4.5 AudDlg (FormWindow) to Unreal UUserWidget.
*/

#include "AudioDlg.h"

#include "GameStructs.h"

// Input:
#include "Input/Reply.h"
#include "InputCoreTypes.h"

// Starshatter core:
#include "AudioConfig.h"

// INCLUDE THE REAL MANAGER HEADER HERE:
// Replace this include with whatever file actually declares your legacy manager.
// Example: #include "Display/MenuScreenMgr.h"
#include "GameScreen.h"

UAudioDlg::UAudioDlg(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
}

void UAudioDlg::BindFormWidgets()
{
    // Tabs:
    BindButton(901, vid_btn);
    BindButton(902, aud_btn);
    BindButton(903, ctl_btn);
    BindButton(904, opt_btn);
    BindButton(905, mod_btn);

    // Sliders:
    BindSlider(201, efx_volume_slider);
    BindSlider(202, gui_volume_slider);
    BindSlider(203, wrn_volume_slider);
    BindSlider(204, vox_volume_slider);
    BindSlider(205, menu_music_slider);
    BindSlider(206, game_music_slider);

    // Apply / Cancel:
    BindButton(1, ApplyBtn);
    BindButton(2, CancelBtn);
}

FString UAudioDlg::GetLegacyFormText() const
{
    return TEXT(R"FORM(
// +--------------------------------------------------------------------+
//  Project:   Starshatter 4.5
//  File:      AudDlg.frm
//
//  Destroyer Studios LLC
//  Copyright © 1997-2004. All Rights Reserved.
// +--------------------------------------------------------------------+

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

   ctrl: { id: 9991, type: background, texture: Frame2a, cells: (1,3,2,3), cell_insets: (0,0,0,10), margins: (2,32,40,32) hide_partial: false }
   ctrl: { id: 9992, type: background, texture: Frame2b, cells: (3,3,2,3), cell_insets: (0,0,0,10), margins: (0,40,40,32) hide_partial: false }

   ctrl: { id: 10, type: label, text: "Options", align: left, font: Limerick18, fore_color: (255,255,255), transparent: true, cells: (1,1,3,1) cell_insets: (0,0,0,0) hide_partial: false }

   ctrl: { id: 900 type: panel transparent: true cells: (1,3,4,1) hide_partial: false layout: { x_mins: (100, 100, 100, 100, 100, 0), x_weights: (0.2, 0.2, 0.2, 0.2, 0.2, 1), y_mins: (24), y_weights: (1) } }

   defctrl: { align: left, font: Limerick12, fore_color: (255, 255, 255), standard_image: BlueTab_0, activated_image: BlueTab_1, sticky: true, bevel_width: 6, margins: (8,8,0,0), cell_insets: (0,4,0,0) },

   ctrl: { id: 901 pid: 900 type: button text: Video      cells: (0,0,1,1) }
   ctrl: { id: 902 pid: 900 type: button text: Audio      cells: (1,0,1,1) }
   ctrl: { id: 903 pid: 900 type: button text: Controls   cells: (2,0,1,1) }
   ctrl: { id: 904 pid: 900 type: button text: Gameplay   cells: (3,0,1,1) }
   ctrl: { id: 905 pid: 900 type: button text: "Mod Config" cells: (4,0,1,1) }

   ctrl: {
      id:               300
      type:             panel
      transparent:      false
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

   ctrl: { id: 201 pid: 300 type: slider cells: (2,1,1,1) }
   ctrl: { id: 202 pid: 300 type: slider cells: (2,2,1,1) }
   ctrl: { id: 203 pid: 300 type: slider cells: (2,3,1,1) }
   ctrl: { id: 204 pid: 300 type: slider cells: (2,4,1,1) }
   ctrl: { id: 205 pid: 300 type: slider cells: (2,6,1,1) }
   ctrl: { id: 206 pid: 300 type: slider cells: (2,7,1,1) }

   ctrl: { id: 1 type: button text: "Apply"  cells: (3,5,1,1) }
   ctrl: { id: 2 type: button text: "Cancel" cells: (4,5,1,1) }
}
)FORM");
}

void UAudioDlg::NativeConstruct()
{
    Super::NativeConstruct();

    if (ApplyBtn)
        ApplyBtn->OnClicked.AddDynamic(this, &UAudioDlg::OnApplyClicked);

    if (CancelBtn)
        CancelBtn->OnClicked.AddDynamic(this, &UAudioDlg::OnCancelClicked);

    if (vid_btn)
        vid_btn->OnClicked.AddDynamic(this, &UAudioDlg::OnVideoClicked);

    if (aud_btn)
        aud_btn->OnClicked.AddDynamic(this, &UAudioDlg::OnAudioClicked);

    if (ctl_btn)
        ctl_btn->OnClicked.AddDynamic(this, &UAudioDlg::OnControlsClicked);

    if (opt_btn)
        opt_btn->OnClicked.AddDynamic(this, &UAudioDlg::OnOptionsClicked);

    if (mod_btn)
        mod_btn->OnClicked.AddDynamic(this, &UAudioDlg::OnModClicked);

    if (closed && AudioConfig::GetInstance())
    {
        AudioConfig* audio = AudioConfig::GetInstance();

        if (efx_volume_slider)  efx_volume_slider->SetValue(audio->GetEfxVolume());
        if (gui_volume_slider)  gui_volume_slider->SetValue(audio->GetGuiVolume());
        if (wrn_volume_slider)  wrn_volume_slider->SetValue(audio->GetWrnVolume());
        if (vox_volume_slider)  vox_volume_slider->SetValue(audio->GetVoxVolume());

        if (menu_music_slider)  menu_music_slider->SetValue(audio->GetMenuMusic());
        if (game_music_slider)  game_music_slider->SetValue(audio->GetGameMusic());
    }

    closed = false;
}

void UAudioDlg::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
    Super::NativeTick(MyGeometry, InDeltaTime);
}

FReply UAudioDlg::NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
    const FKey Key = InKeyEvent.GetKey();

    if (Key == EKeys::Enter || Key == EKeys::Virtual_Accept)
    {
        OnApplyClicked();
        return FReply::Handled();
    }

    if (Key == EKeys::Escape || Key == EKeys::Virtual_Back)
    {
        OnCancelClicked();
        return FReply::Handled();
    }

    return Super::NativeOnKeyDown(InGeometry, InKeyEvent);
}

// ---------------------------------------------------------------------
// Navigation (calls into legacy manager):

void UAudioDlg::OnAudioClicked()
{
    if (manager) manager->ShowAudDlg();
    else UE_LOG(LogTemp, Warning, TEXT("AudioDlg: manager is null (OnAudioClicked)."));
}

void UAudioDlg::OnVideoClicked()
{
    if (manager) manager->ShowVidDlg();
    else UE_LOG(LogTemp, Warning, TEXT("AudioDlg: manager is null (OnVideoClicked)."));
}

void UAudioDlg::OnOptionsClicked()
{
    if (manager) manager->ShowOptDlg();
    else UE_LOG(LogTemp, Warning, TEXT("AudioDlg: manager is null (OnOptionsClicked)."));
}

void UAudioDlg::OnControlsClicked()
{
    if (manager) manager->ShowCtlDlg();
    else UE_LOG(LogTemp, Warning, TEXT("AudioDlg: manager is null (OnControlsClicked)."));
}

void UAudioDlg::OnModClicked()
{
    if (manager) manager->ShowModDlg();
    else UE_LOG(LogTemp, Warning, TEXT("AudioDlg: manager is null (OnModClicked)."));
}

// ---------------------------------------------------------------------
// Actions:

void UAudioDlg::OnApplyClicked()
{
    if (manager)
        manager->ApplyOptions();
    else
        UE_LOG(LogTemp, Warning, TEXT("AudioDlg: manager is null (OnApplyClicked)."));
}

void UAudioDlg::OnCancelClicked()
{
    if (manager)
        manager->CancelOptions();
    else
        UE_LOG(LogTemp, Warning, TEXT("AudioDlg: manager is null (OnCancelClicked)."));
}

void UAudioDlg::Apply()
{
    if (!closed && AudioConfig::GetInstance())
    {
        AudioConfig* audio = AudioConfig::GetInstance();

        if (efx_volume_slider)  audio->SetEfxVolume(efx_volume_slider->GetValue());
        if (gui_volume_slider)  audio->SetGuiVolume(gui_volume_slider->GetValue());
        if (wrn_volume_slider)  audio->SetWrnVolume(wrn_volume_slider->GetValue());
        if (vox_volume_slider)  audio->SetVoxVolume(vox_volume_slider->GetValue());

        if (menu_music_slider)  audio->SetMenuMusic(menu_music_slider->GetValue());
        if (game_music_slider)  audio->SetGameMusic(game_music_slider->GetValue());

        audio->Save();
    }

    closed = true;
}

void UAudioDlg::Cancel()
{
    closed = true;
}
