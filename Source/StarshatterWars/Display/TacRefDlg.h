/*  Project Starshatter Wars
    Fractal Dev Studios
    Copyright (c) 2025-2026. All Rights Reserved.

    SUBSYSTEM:    Stars.exe
    FILE:         TacRefDlg.h
    AUTHOR:       Carlos Bott

    ORIGINAL AUTHOR AND STUDIO
    ==========================
    John DiCamillo / Destroyer Studios LLC

    OVERVIEW
    ========
    Tactical Reference / Mission Briefing screen (UMG UserWidget) — Unreal port of the legacy
    tactical reference dialog.
*/

#pragma once

#include "CoreMinimal.h"
#include "BaseScreen.h"
#include "MenuScreen.h"

// Minimal Unreal includes requested for headers:
#include "Math/Vector.h"              // FVector
#include "Math/Color.h"               // FColor
#include "Math/UnrealMathUtility.h"   // Math

#include "TacRefDlg.generated.h"

// Forward declarations (UMG):
class UButton;
class UListView;
class UListViewBase;
class UTextBlock;
class URichTextBlock;
class UImage;

class ShipDesign;
class WeaponDesign;

// NOTE: legacy types renamed per your rules:
// Scene* -> SimScene*
// CameraDirector* -> CameraManager*
// Font* -> SystemFont*
// (this header intentionally does NOT expose non-UObject types to reflection)

/**
 * Tactical reference screen.
 * Replaces FormWindow-based TacRefDlg with a UBaseScreen-derived UUserWidget.
 */
UCLASS()
class STARSHATTERWARS_API UTacRefDlg : public UBaseScreen
{
    GENERATED_BODY()

public:
    enum MODES { MODE_NONE, MODE_SHIPS, MODE_WEAPONS };

public:
    UTacRefDlg(const FObjectInitializer& ObjectInitializer);

public:
    // ---- Legacy-style API (native-only; NOT BlueprintCallable) --------

    void Show();
    void ExecFrame();

    // Operations (native-only):
    void OnClose(void* Event);
    void OnMode(void* Event);
    void OnSelect(void* Event);
    void OnCamRButtonDown(void* Event);
    void OnCamRButtonUp(void* Event);
    void OnCamMove(void* Event);
    void OnCamZoom(void* Event);

protected:
    // ---- UBaseScreen --------------------------------------------------

    virtual void BindFormWidgets() override;

protected:
    // ---- UUserWidget lifecycle ---------------------------------------

    virtual void NativeOnInitialized() override;
    virtual void NativeConstruct() override;
    virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

protected:
    // ---- Internal operations -----------------------------------------

    void SelectShip(const ShipDesign* dsn);
    void SelectWeapon(const WeaponDesign* dsn);

    void UpdateZoom(double r);
    void UpdateAzimuth(double a);
    void UpdateElevation(double e);
    void UpdateCamera();
    bool SetCaptureBeauty();
    bool ReleaseCaptureBeauty();

protected:
    // ---- Widget event handlers ---------------------------------------

    UFUNCTION() void HandleCloseClicked();
    UFUNCTION() void HandleShipsClicked();
    UFUNCTION() void HandleWeaponsClicked();

protected:
    // ---- External/legacy manager -------------------------------------

    // Legacy MenuScreen manager (non-UObject):
    UMenuScreen* manager = nullptr;

protected:
    // ---- UMG widgets --------------------------------------------------

    // These are UMG equivalents of the legacy ActiveWindow/Form controls.
    // You can bind them in the widget blueprint.

    UPROPERTY(meta = (BindWidgetOptional)) UImage* Beauty = nullptr;
    UPROPERTY(meta = (BindWidgetOptional)) UListView* LstDesigns = nullptr;

    UPROPERTY(meta = (BindWidgetOptional)) UTextBlock* TxtCaption = nullptr;
    UPROPERTY(meta = (BindWidgetOptional)) URichTextBlock* TxtStats = nullptr;
    UPROPERTY(meta = (BindWidgetOptional)) URichTextBlock* TxtDescription = nullptr;

    UPROPERTY(meta = (BindWidgetOptional)) UButton* BtnShips = nullptr;
    UPROPERTY(meta = (BindWidgetOptional)) UButton* BtnWeaps = nullptr;
    UPROPERTY(meta = (BindWidgetOptional)) UButton* BtnClose = nullptr;

protected:
    // ---- Legacy camera/scene state (kept as plain C++ members) --------

    // NOTE: These are *not* UObjects. They should remain Starshatter core types.
    // Implement rendering/preview either via:
    // - a 3D preview actor + SceneCaptureComponent2D feeding Beauty image, or
    // - your existing panel render pipeline.

    // Per your mapping rules:
    // Scene -> SimScene, Camera -> your camera struct/class (kept core-side).
    // Forward declare / include in .cpp only, to keep header light.

    int    mode = MODE_NONE;
    double radius = 0.0;
    double cam_zoom = 0.0;
    double cam_az = 0.0;
    double cam_el = 0.0;
    int    mouse_x = 0;
    int    mouse_y = 0;
    bool   update_scene = false;
    bool   captured = false;

    int ship_index = -1;
    int weap_index = -1;
};

