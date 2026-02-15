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
    - Dialog reads/writes ONLY UStarshatterVideoSettings (config model).
    - Runtime apply is delegated to UStarshatterVideoSubsystem::ApplySettingsToRuntime()
      to avoid signature drift.
    - Subscreen routing goes through UOptionsScreen (NOT MenuDlg).
    - Implements UOptionsPage so OptionsScreen can Apply/Cancel uniformly.
*/

#pragma once

#include "CoreMinimal.h"
#include "BaseScreen.h"
#include "OptionsPage.h"

#include "VideoDlg.generated.h"

// UMG:
class UComboBoxString;
class USlider;
class UButton;

// Host/router:
class UOptionsScreen;

// Model:
class UStarshatterVideoSettings;

// Runtime apply:
class UStarshatterVideoSubsystem;

UCLASS()
class STARSHATTERWARS_API UVideoDlg : public UBaseScreen, public IOptionsPage
{
    GENERATED_BODY()

public:
    UVideoDlg(const FObjectInitializer& ObjectInitializer);

    // Host/router:
    void SetOptionsManager(UOptionsScreen* InManager) { OptionsManager = InManager; }
    UOptionsScreen* GetOptionsManager() const { return OptionsManager.Get(); }

    void Show();

    virtual void ExecFrame(double DeltaTime) override;

    // Legacy callsites:
    void Apply();
    void Cancel();

    // UI -> Settings (Save + optional runtime apply):
    void PushToModel(bool bApplyRuntimeToo);

protected:
    virtual void NativeConstruct() override;
    virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

protected:
    // UBaseScreen overrides:
    virtual void BindFormWidgets() override;
    virtual FString GetLegacyFormText() const override;

    virtual void HandleAccept() override;
    virtual void HandleCancel() override;

public:
    // ------------------------------------------------------------
    // IOptionsPage (OptionsScreen orchestration)
    // ------------------------------------------------------------
    virtual void LoadFromSettings_Implementation() override;
    virtual void ApplySettings_Implementation() override;
    virtual void SaveSettings_Implementation() override;
    virtual void CancelChanges_Implementation() override;

private:
    UStarshatterVideoSettings* GetVideoSettings() const;
    UStarshatterVideoSubsystem* GetVideoSubsystem() const;

    void BuildListsIfNeeded();
    void RefreshFromModel();

    // Helpers:
    static int32 ClampGammaLevel(int32 InGamma) { return FMath::Clamp(InGamma, 32, 224); }
    static float Gamma01FromLevel(int32 GammaLevel);
    static int32 GammaLevelFrom01(float V01);

    // Tex sizes:
    static const TArray<int32>& GetTexSizeOptions();
    static int32 TexSizeIndexFromPow2(int32 Pow2);
    static int32 TexSizePow2FromIndex(int32 Index);

private:
    // Combo handlers:
    UFUNCTION() void OnModeChanged(FString SelectedItem, ESelectInfo::Type SelectionType);
    UFUNCTION() void OnTexSizeChanged(FString SelectedItem, ESelectInfo::Type SelectionType);
    UFUNCTION() void OnDetailChanged(FString SelectedItem, ESelectInfo::Type SelectionType);
    UFUNCTION() void OnTextureChanged(FString SelectedItem, ESelectInfo::Type SelectionType);
    UFUNCTION() void OnGammaChanged(float Value);

    UFUNCTION() void OnLensFlareChanged(FString SelectedItem, ESelectInfo::Type SelectionType);
    UFUNCTION() void OnCoronaChanged(FString SelectedItem, ESelectInfo::Type SelectionType);
    UFUNCTION() void OnNebulaChanged(FString SelectedItem, ESelectInfo::Type SelectionType);
    UFUNCTION() void OnDustChanged(FString SelectedItem, ESelectInfo::Type SelectionType);

    UFUNCTION() void OnShadowsChanged(FString SelectedItem, ESelectInfo::Type SelectionType);
    UFUNCTION() void OnSpecMapsChanged(FString SelectedItem, ESelectInfo::Type SelectionType);
    UFUNCTION() void OnBumpMapsChanged(FString SelectedItem, ESelectInfo::Type SelectionType);

    // Buttons:
    UFUNCTION() void OnApplyClicked();
    UFUNCTION() void OnCancelClicked();

    // Tabs:
    UFUNCTION() void OnAudioClicked();
    UFUNCTION() void OnVideoClicked();
    UFUNCTION() void OnGameClicked();
    UFUNCTION() void OnControlsClicked();
    UFUNCTION() void OnModClicked();

private:

    bool bClosed = true;
    bool bListsBuilt = false;

    // Cached dialog state (mirrors FStarshatterVideoConfig):
    int32 Width = 1920;
    int32 Height = 1080;
    bool  bFullscreen = false;

    int32 MaxTextureSize = 2048;
    int32 GammaLevel = 128;

    bool bShadows = true;
    bool bSpecularMaps = true;
    bool bBumpMaps = true;

    bool bLensFlare = true;
    bool bCorona = true;
    bool bNebula = true;

    int32 DustLevel = 1;
    int32 TerrainDetailIndex = 1;
    bool  bTerrainTextures = true;

protected:
    // Main combos:
    UPROPERTY(meta = (BindWidgetOptional)) UComboBoxString* ModeCombo = nullptr;
    UPROPERTY(meta = (BindWidgetOptional)) UComboBoxString* TexSizeCombo = nullptr;
    UPROPERTY(meta = (BindWidgetOptional)) UComboBoxString* DetailCombo = nullptr;
    UPROPERTY(meta = (BindWidgetOptional)) UComboBoxString* TextureCombo = nullptr;

    // Effects/toggles:
    UPROPERTY(meta = (BindWidgetOptional)) UComboBoxString* LensFlareCombo = nullptr;
    UPROPERTY(meta = (BindWidgetOptional)) UComboBoxString* CoronaCombo = nullptr;
    UPROPERTY(meta = (BindWidgetOptional)) UComboBoxString* NebulaCombo = nullptr;
    UPROPERTY(meta = (BindWidgetOptional)) UComboBoxString* DustCombo = nullptr;

    UPROPERTY(meta = (BindWidgetOptional)) UComboBoxString* ShadowsCombo = nullptr;
    UPROPERTY(meta = (BindWidgetOptional)) UComboBoxString* SpecMapsCombo = nullptr;
    UPROPERTY(meta = (BindWidgetOptional)) UComboBoxString* BumpMapsCombo = nullptr;

    // Slider:
    UPROPERTY(meta = (BindWidgetOptional)) USlider* GammaSlider = nullptr;

    // Apply/Cancel:
    UPROPERTY(meta = (BindWidgetOptional)) UButton* ApplyBtn = nullptr;
    UPROPERTY(meta = (BindWidgetOptional)) UButton* CancelBtn = nullptr;

    // Tabs:
    UPROPERTY(meta = (BindWidgetOptional)) UButton* VidTabButton = nullptr;
    UPROPERTY(meta = (BindWidgetOptional)) UButton* AudTabButton = nullptr;
    UPROPERTY(meta = (BindWidgetOptional)) UButton* CtlTabButton = nullptr;
    UPROPERTY(meta = (BindWidgetOptional)) UButton* OptTabButton = nullptr; // GAME
    UPROPERTY(meta = (BindWidgetOptional)) UButton* ModTabButton = nullptr;
};
