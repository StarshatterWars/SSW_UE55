/*  Project Starshatter Wars
    Fractal Dev Studios
    Copyright (c) 2025-2026.

    SUBSYSTEM:    Stars.exe (Unreal Port)
    FILE:         VideoDlg.h
    AUTHOR:       Carlos Bott

    OVERVIEW
    ========
    UVideoDlg
    - Unreal UUserWidget replacement for legacy VidDlg (video options dialog).
    - Inherits from UBaseScreen to use legacy FORM parsing / control-id binding.
    - Uses standard UMG widget bindings (BindWidgetOptional).
*/

#pragma once

#include "CoreMinimal.h"
#include "BaseScreen.h"
#include "VideoDlg.generated.h"

class UComboBoxString;
class USlider;
class UButton;

class Starshatter;

UCLASS()
class STARSHATTERWARS_API UVideoDlg : public UBaseScreen
{
    GENERATED_BODY()

public:
    UVideoDlg();
    void InitializeVideoDlg(UBaseScreen* InManager);
    void ShowVideoDlg();

    void ApplySettings();
    void CancelSettings();

    virtual void        ExecFrame(float DeltaTime);

protected:
    virtual void        NativeConstruct() override;
   
protected:
    // UBaseScreen overrides
    virtual void BindFormWidgets() override;
    virtual FString GetLegacyFormText() const override;

private:
    void BuildModeList();
    void RefreshSelectionsFromRuntime();

private:
    UFUNCTION() void OnModeChanged(FString SelectedItem, ESelectInfo::Type SelectionType);
    UFUNCTION() void OnTexSizeChanged(FString SelectedItem, ESelectInfo::Type SelectionType);
    UFUNCTION() void OnDetailChanged(FString SelectedItem, ESelectInfo::Type SelectionType);
    UFUNCTION() void OnTextureChanged(FString SelectedItem, ESelectInfo::Type SelectionType);
    UFUNCTION() void OnGammaChanged(float Value);

    UFUNCTION() void OnApplyClicked();
    UFUNCTION() void OnCancelClicked();

    UFUNCTION() void OnAudioClicked();
    UFUNCTION() void OnVideoClicked();
    UFUNCTION() void OnOptionsClicked();
    UFUNCTION() void OnControlsClicked();
    UFUNCTION() void OnModClicked();

private:
    // Legacy FORM text
    UPROPERTY(EditDefaultsOnly, Category = "FORM")
    FString LegacyFormText;

private:
    UPROPERTY(Transient)
    UBaseScreen* Manager = nullptr;

    Starshatter* StarsInstance = nullptr;
    bool bClosed = true;

private:
    int32 SelectedRender = 0;
    int32 SelectedCard = 0;
    int32 SelectedTexSize = 0;
    int32 SelectedMode = 0;
    int32 SelectedDetail = 0;
    int32 SelectedTexture = 0;
    int32 OrigGamma = 128;

protected:
    // Standard UMG widget bindings (OPTIONAL):
    // These names must match the widget names in your UMG Designer.

    // main combos
    UPROPERTY(meta = (BindWidgetOptional)) UComboBoxString* ModeCombo = nullptr; // frm id 203
    UPROPERTY(meta = (BindWidgetOptional)) UComboBoxString* TexSizeCombo = nullptr; // frm id 204
    UPROPERTY(meta = (BindWidgetOptional)) UComboBoxString* DetailCombo = nullptr; // frm id 205
    UPROPERTY(meta = (BindWidgetOptional)) UComboBoxString* TextureCombo = nullptr; // frm id 206

    // effects/toggles
    UPROPERTY(meta = (BindWidgetOptional)) UComboBoxString* LensFlareCombo = nullptr; // frm id 211
    UPROPERTY(meta = (BindWidgetOptional)) UComboBoxString* CoronaCombo = nullptr; // frm id 212
    UPROPERTY(meta = (BindWidgetOptional)) UComboBoxString* NebulaCombo = nullptr; // frm id 213
    UPROPERTY(meta = (BindWidgetOptional)) UComboBoxString* DustCombo = nullptr; // frm id 214

    UPROPERTY(meta = (BindWidgetOptional)) UComboBoxString* ShadowsCombo = nullptr; // frm id 222
    UPROPERTY(meta = (BindWidgetOptional)) UComboBoxString* SpecMapsCombo = nullptr; // frm id 223
    UPROPERTY(meta = (BindWidgetOptional)) UComboBoxString* BumpMapsCombo = nullptr; // frm id 224

    // slider
    UPROPERTY(meta = (BindWidgetOptional)) USlider* GammaSlider = nullptr; // frm id 215

    // tabs
    UPROPERTY(meta = (BindWidgetOptional)) UButton* VidTabButton = nullptr; // frm id 901
    UPROPERTY(meta = (BindWidgetOptional)) UButton* AudTabButton = nullptr; // frm id 902
    UPROPERTY(meta = (BindWidgetOptional)) UButton* CtlTabButton = nullptr; // frm id 903
    UPROPERTY(meta = (BindWidgetOptional)) UButton* OptTabButton = nullptr; // frm id 904
    UPROPERTY(meta = (BindWidgetOptional)) UButton* ModTabButton = nullptr; // frm id 905

};
