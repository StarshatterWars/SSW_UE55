/*  Project Starshatter Wars
    Fractal Dev Studios
    Copyright (c) 2025-2026.

    SUBSYSTEM:    Stars.exe (Unreal Port)
    FILE:         VideoDlg.h
    AUTHOR:       Carlos Bott

    OVERVIEW
    ========
    UVideoDlg
    - Unreal replacement for legacy VidDlg (video options dialog).
    - Inherits from UBaseScreen to use legacy FORM parsing / control-id binding.
    - Subscreen routing goes through UOptionsScreen (NOT GameScreen).
*/

#pragma once

#include "CoreMinimal.h"
#include "BaseScreen.h"
#include "VideoDlg.generated.h"

// UMG:
class UComboBoxString;
class USlider;
class UButton;

// Starshatter:
class Starshatter;

// Host/router:
class UOptionsScreen;

UCLASS()
class STARSHATTERWARS_API UVideoDlg : public UBaseScreen
{
    GENERATED_BODY()

public:
    UVideoDlg(const FObjectInitializer& ObjectInitializer);

    // Host/router:
    void SetManager(UOptionsScreen* InManager) { Manager = InManager; }
    UOptionsScreen* GetManager() const { return Manager; }

    // Legacy-ish surface:
    virtual void Show();
    virtual void ExecFrame(float DeltaTime);

    // These exist because older callsites expect them:
    void Apply();
    void Cancel();

    // If you still want explicit names:
    void ApplySettings();
    void CancelSettings();

protected:
    virtual void NativeConstruct() override;
    virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

protected:
    // UBaseScreen overrides:
    virtual void BindFormWidgets() override;
    virtual FString GetLegacyFormText() const override;

    // Centralized Enter/Escape (UBaseScreen):
    virtual void HandleAccept() override;
    virtual void HandleCancel() override;

private:
    void BuildModeList();
    void RefreshSelectionsFromRuntime();

private:
    // Combo handlers:
    UFUNCTION() void OnModeChanged(FString SelectedItem, ESelectInfo::Type SelectionType);
    UFUNCTION() void OnTexSizeChanged(FString SelectedItem, ESelectInfo::Type SelectionType);
    UFUNCTION() void OnDetailChanged(FString SelectedItem, ESelectInfo::Type SelectionType);
    UFUNCTION() void OnTextureChanged(FString SelectedItem, ESelectInfo::Type SelectionType);
    UFUNCTION() void OnGammaChanged(float Value);

    // Buttons:
    UFUNCTION() void OnApplyClicked();
    UFUNCTION() void OnCancelClicked();

    // Tabs:
    UFUNCTION() void OnAudioClicked();
    UFUNCTION() void OnVideoClicked();
    UFUNCTION() void OnOptionsClicked();
    UFUNCTION() void OnControlsClicked();
    UFUNCTION() void OnModClicked();

private:
    // Host/router:
    UPROPERTY(Transient)
    UOptionsScreen* Manager = nullptr;

    // Starshatter singleton:
    Starshatter* StarsInstance = nullptr;

    bool  bClosed = true;
    int32 OrigGamma = 128;

    // Selections/state:
    int32 SelectedTexSize = 0;   // 0..N (maps to pow2 sizes)
    int32 SelectedMode = 0;   // index within ModeCombo
    int32 SelectedDetail = 0;
    int32 SelectedTexture = 0;

protected:
    // ------------------------------------------------------------
    // UMG widget bindings (OPTIONAL) — must match UMG widget names
    // ------------------------------------------------------------

    // Main combos:
    UPROPERTY(meta = (BindWidgetOptional)) UComboBoxString* ModeCombo = nullptr; // frm id 203
    UPROPERTY(meta = (BindWidgetOptional)) UComboBoxString* TexSizeCombo = nullptr; // frm id 204
    UPROPERTY(meta = (BindWidgetOptional)) UComboBoxString* DetailCombo = nullptr; // frm id 205
    UPROPERTY(meta = (BindWidgetOptional)) UComboBoxString* TextureCombo = nullptr; // frm id 206

    // Effects/toggles:
    UPROPERTY(meta = (BindWidgetOptional)) UComboBoxString* LensFlareCombo = nullptr; // 211
    UPROPERTY(meta = (BindWidgetOptional)) UComboBoxString* CoronaCombo = nullptr; // 212
    UPROPERTY(meta = (BindWidgetOptional)) UComboBoxString* NebulaCombo = nullptr; // 213
    UPROPERTY(meta = (BindWidgetOptional)) UComboBoxString* DustCombo = nullptr; // 214

    UPROPERTY(meta = (BindWidgetOptional)) UComboBoxString* ShadowsCombo = nullptr; // 222
    UPROPERTY(meta = (BindWidgetOptional)) UComboBoxString* SpecMapsCombo = nullptr; // 223
    UPROPERTY(meta = (BindWidgetOptional)) UComboBoxString* BumpMapsCombo = nullptr; // 224

    // Slider:
    UPROPERTY(meta = (BindWidgetOptional)) USlider* GammaSlider = nullptr; // 215

    // Apply/Cancel:
    UPROPERTY(meta = (BindWidgetOptional)) UButton* ApplyBtn = nullptr; // id 1
    UPROPERTY(meta = (BindWidgetOptional)) UButton* CancelBtn = nullptr; // id 2

    // Tabs:
    UPROPERTY(meta = (BindWidgetOptional)) UButton* VidTabButton = nullptr; // 901
    UPROPERTY(meta = (BindWidgetOptional)) UButton* AudTabButton = nullptr; // 902
    UPROPERTY(meta = (BindWidgetOptional)) UButton* CtlTabButton = nullptr; // 903
    UPROPERTY(meta = (BindWidgetOptional)) UButton* OptTabButton = nullptr; // 904
    UPROPERTY(meta = (BindWidgetOptional)) UButton* ModTabButton = nullptr; // 905
};
