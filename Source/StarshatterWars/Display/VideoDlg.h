/*  Project Starshatter Wars
    Fractal Dev Studios
    Copyright (c) 2025-2026.

    SUBSYSTEM:    Stars.exe (Unreal Port)
    FILE:         VideoDlg.h
    AUTHOR:       Carlos Bott

    OVERVIEW
    ========
    UVideoDlg
    - Video options dialog (UMG + legacy-form bridge via UBaseScreen).
    - Refactored so the dialog only touches UStarshatterVideoSettings (config model).
    - Runtime apply is delegated to UStarshatterVideoSettings::ApplyToRuntimeVideo(...).
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

// Host/router:
class UOptionsScreen;

// Model:
class UStarshatterVideoSettings;

UCLASS()
class STARSHATTERWARS_API UVideoDlg : public UBaseScreen
{
    GENERATED_BODY()

public:
    UVideoDlg(const FObjectInitializer& ObjectInitializer);

    // Host/router:
    void SetManager(UOptionsScreen* InManager) { Manager = InManager; }
    UOptionsScreen* GetManager() const { return Manager; }

    // Surface:
    void Show();

    // IMPORTANT: match UBaseScreen signature (same as AudioDlg)
    virtual void ExecFrame(double DeltaTime) override;

    // Legacy callsites:
    void Apply();
    void Cancel();

    // Preferred names:
    void ApplySettings();
    void CancelSettings();

    // UI -> Settings (Save + optional runtime apply):
    void PushToModel(bool bApplyRuntimeToo);

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
    UStarshatterVideoSettings* GetVideoSettings() const;

    void BuildModeList();
    void RefreshFromModel();

    static int32 TexSizeIndexFromPow2(int32 Pow2);
    static int32 TexSizePow2FromIndex(int32 Index);

    static float Gamma01FromLevel(int32 GammaLevel);
    static int32 GammaLevelFrom01(float V01);

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

    bool bClosed = true;

    // Cached dialog state (mirrors settings):
    int32 Width = 1920;
    int32 Height = 1080;
    bool  bFullscreen = true;

    int32 SelectedTexSize = 5;   // index (maps to pow2)
    int32 SelectedMode = 0;
    int32 SelectedDetail = 0;
    int32 SelectedTexture = 1;

    bool bLensFlare = true;
    bool bCorona = true;
    bool bNebula = true;
    int32 Dust = 0;

    bool bShadows = true;
    bool bSpecMaps = true;
    bool bBumpMaps = true;

    int32 GammaLevel = 128;
    float DepthBias = 0.0f;

protected:
    // Main combos:
    UPROPERTY(meta = (BindWidgetOptional)) UComboBoxString* ModeCombo = nullptr;      // frm id 203
    UPROPERTY(meta = (BindWidgetOptional)) UComboBoxString* TexSizeCombo = nullptr;  // frm id 204
    UPROPERTY(meta = (BindWidgetOptional)) UComboBoxString* DetailCombo = nullptr;   // frm id 205
    UPROPERTY(meta = (BindWidgetOptional)) UComboBoxString* TextureCombo = nullptr;  // frm id 206

    // Effects/toggles:
    UPROPERTY(meta = (BindWidgetOptional)) UComboBoxString* LensFlareCombo = nullptr; // 211
    UPROPERTY(meta = (BindWidgetOptional)) UComboBoxString* CoronaCombo = nullptr;    // 212
    UPROPERTY(meta = (BindWidgetOptional)) UComboBoxString* NebulaCombo = nullptr;    // 213
    UPROPERTY(meta = (BindWidgetOptional)) UComboBoxString* DustCombo = nullptr;      // 214

    UPROPERTY(meta = (BindWidgetOptional)) UComboBoxString* ShadowsCombo = nullptr;   // 222
    UPROPERTY(meta = (BindWidgetOptional)) UComboBoxString* SpecMapsCombo = nullptr;  // 223
    UPROPERTY(meta = (BindWidgetOptional)) UComboBoxString* BumpMapsCombo = nullptr;  // 224

    // Slider:
    UPROPERTY(meta = (BindWidgetOptional)) USlider* GammaSlider = nullptr;            // 215

    // Apply/Cancel:
    UPROPERTY(meta = (BindWidgetOptional)) UButton* ApplyBtn = nullptr;               // id 1
    UPROPERTY(meta = (BindWidgetOptional)) UButton* CancelBtn = nullptr;              // id 2

    // Tabs:
    UPROPERTY(meta = (BindWidgetOptional)) UButton* VidTabButton = nullptr;           // 901
    UPROPERTY(meta = (BindWidgetOptional)) UButton* AudTabButton = nullptr;           // 902
    UPROPERTY(meta = (BindWidgetOptional)) UButton* CtlTabButton = nullptr;           // 903
    UPROPERTY(meta = (BindWidgetOptional)) UButton* OptTabButton = nullptr;           // 904
    UPROPERTY(meta = (BindWidgetOptional)) UButton* ModTabButton = nullptr;           // 905
};
