/*  Project Starshatter Wars
    Fractal Dev Studios
    Copyright (c) 2025-2026. All Rights Reserved.

    ORIGINAL AUTHOR AND STUDIO
    ==========================
    John DiCamillo / Destroyer Studios LLC

    SUBSYSTEM:    Stars.exe
    FILE:         AudioDlg.h
    AUTHOR:       Carlos Bott

    OVERVIEW
    ========
    Main Menu Audio Dialog Unreal User Widget declaration.
    Port of Starshatter 4.5 AudDlg (FormWindow) to Unreal UUserWidget.

    FIX NOTES
    =========
    C2027: use of undefined type 'BaseScreen'
    - Caused by a name collision between:
        1) your Unreal widget base class UBaseScreen (BaseScreen.h)
        2) the legacy manager/controller class also named BaseScreen
    - Solution: rename the legacy manager pointer type to a non-colliding name,
      e.g. "FMenuScreenMgr" (or "SBaseScreenMgr"), and forward declare that.
    - This file implements that rename so AudioDlg compiles without requiring the
      legacy BaseScreen definition here.
*/

#pragma once

#include "CoreMinimal.h"
#include "BaseScreen.h"


// UMG:
#include "Components/Button.h"
#include "Components/Slider.h"

#include "AudioDlg.generated.h"

/*
    IMPORTANT:
    "BaseScreen" is already used by UBaseScreen (your Unreal class) via BaseScreen.h.
    Do NOT forward declare another type named BaseScreen here.

    Instead, forward declare your legacy manager/controller using a distinct name.
    Change the real legacy manager class name (recommended), or typedef it elsewhere.
*/

// Forward declare legacy manager/controller (rename this to match your real type):
class GameScreen;

UCLASS()
class STARSHATTERWARS_API UAudioDlg : public UBaseScreen
{
    GENERATED_BODY()

public:
    UAudioDlg(const FObjectInitializer& ObjectInitializer);

    void SetManager(GameScreen* InManager) { manager = InManager; }
    GameScreen* GetManager() const { return manager; }

    // Operations:
    void Apply();
    void Cancel();

protected:
    virtual void NativeConstruct() override;
    virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
    virtual FReply NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;

    virtual void BindFormWidgets() override;
    virtual FString GetLegacyFormText() const override;

protected:
    UFUNCTION() void OnApplyClicked();
    UFUNCTION() void OnCancelClicked();

    UFUNCTION() void OnVideoClicked();
    UFUNCTION() void OnAudioClicked();
    UFUNCTION() void OnControlsClicked();
    UFUNCTION() void OnOptionsClicked();
    UFUNCTION() void OnModClicked();

protected:
    // Legacy manager/controller (renamed to avoid collision):
    GameScreen* manager = nullptr;

    // Tabs (pid=900):
    UPROPERTY(meta = (BindWidgetOptional)) UButton* vid_btn = nullptr; // id=901
    UPROPERTY(meta = (BindWidgetOptional)) UButton* aud_btn = nullptr; // id=902
    UPROPERTY(meta = (BindWidgetOptional)) UButton* ctl_btn = nullptr; // id=903
    UPROPERTY(meta = (BindWidgetOptional)) UButton* opt_btn = nullptr; // id=904
    UPROPERTY(meta = (BindWidgetOptional)) UButton* mod_btn = nullptr; // id=905

    // Sliders (pid=300):
    UPROPERTY(meta = (BindWidgetOptional)) USlider* efx_volume_slider = nullptr; // id=201
    UPROPERTY(meta = (BindWidgetOptional)) USlider* gui_volume_slider = nullptr; // id=202
    UPROPERTY(meta = (BindWidgetOptional)) USlider* wrn_volume_slider = nullptr; // id=203
    UPROPERTY(meta = (BindWidgetOptional)) USlider* vox_volume_slider = nullptr; // id=204
    UPROPERTY(meta = (BindWidgetOptional)) USlider* menu_music_slider = nullptr; // id=205
    UPROPERTY(meta = (BindWidgetOptional)) USlider* game_music_slider = nullptr; // id=206

    // Apply/Cancel:
    UPROPERTY(meta = (BindWidgetOptional)) UButton* ApplyBtn = nullptr; // id=1
    UPROPERTY(meta = (BindWidgetOptional)) UButton* CancelBtn = nullptr; // id=2

    bool closed = true;
};
