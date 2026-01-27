/*  Project Starshatter Wars
    Fractal Dev Studios
    Copyright (C) 2025-2026.

    SUBSYSTEM:    nGenEx.lib (ported to Unreal)
    FILE:         View.h
    AUTHOR:       Carlos Bott

    ORIGINAL AUTHOR AND STUDIO
    ==========================
    John DiCamillo / Destroyer Studios LLC

    OVERVIEW
    ========
    Abstract View class (Unreal UUserWidget version)

    NOTES
    =====
    - Legacy View converted to UUserWidget (UView)
    - Legacy API preserved as wrappers
    - Rendering is Slate-native via NativePaint (Option A: build + flush every paint)
*/

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"

// Unreal:
#include "Math/Color.h"
#include "Engine/Font.h"

#include "View.generated.h"

// +--------------------------------------------------------------------+
// Forward declarations

class Window;

// +--------------------------------------------------------------------+

UCLASS(Abstract)
class STARSHATTERWARS_API UView : public UUserWidget
{
    GENERATED_BODY()

public:
    static const char* TYPENAME() { return "View"; } // Preserve Starshatter RTTI string

    UView(const FObjectInitializer& ObjectInitializer);
    virtual ~UView() override;

    // ----------------------------------------------------------------
    // Legacy View API (preserved)
    // ----------------------------------------------------------------
    virtual void Refresh() {}
    virtual void OnWindowMove() {}
    virtual void OnShow() {}
    virtual void OnHide() {}

    virtual void SetWindow(Window* InWindow);
    virtual Window* GetWindow() const;

    // ----------------------------------------------------------------
    // Legacy global state (now class-level statics)
    // ----------------------------------------------------------------
    static FColor HudColor;
    static FColor TxtColor;
    static bool   bShowMenu;

    static TObjectPtr<UFont*> HudFont;
    static TObjectPtr<UFont*> BigFont;

    static bool   bMouseIn;
    static int32  MouseLatch;
    static int32  MouseIndex;

    static int32  ShipStatus;
    static int32  TgtStatus;

protected:
    // ----------------------------------------------------------------
    // UUserWidget lifecycle
    // ----------------------------------------------------------------
    virtual void NativeConstruct() override;
    virtual void NativeDestruct() override;

    // ----------------------------------------------------------------
    // Slate render bridge (Option A: build draw list here each paint)
    // ----------------------------------------------------------------
    virtual int32 NativePaint(
        const FPaintArgs& Args,
        const FGeometry& AllottedGeometry,
        const FSlateRect& MyCullingRect,
        FSlateWindowElementList& OutDrawElements,
        int32 LayerId,
        const FWidgetStyle& InWidgetStyle,
        bool bParentEnabled
    ) const override;

protected:
    // Legacy window pointer (intentionally non-UObject)
    Window* WindowPtr = nullptr;
};
