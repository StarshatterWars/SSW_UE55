/*  Project Starshatter Wars
    Fractal Dev Studios
    Copyright (c) 2025-2026. All Rights Reserved.

    ORIGINAL AUTHOR AND STUDIO
    ==========================
    John DiCamillo / Destroyer Studios LLC

    SUBSYSTEM:    Stars.exe (ported to Unreal)
    FILE:         MFDView.h
    AUTHOR:       Carlos Bott

    OVERVIEW
    ========
    UMFDView
    - UView-based UUserWidget replacement for legacy MFD(Window*) rendering
    - Draws using Slate (NativePaint) so it uses native UE rendering internally
*/

#pragma once

#include "CoreMinimal.h"
#include "View.h"              // your UView base (must derive from UUserWidget)
#include "Math/Color.h"
#include "Math/Vector2D.h"
#include "GameStructs.h"
#include "MFDView.generated.h"

class Ship;
class Sensor;
class Sprite;
class Bitmap;
class CameraView;
class SystemFont;

UCLASS(BlueprintType)
class STARSHATTERWARS_API UMFDView : public UUserWidget
{
    GENERATED_BODY()

public:

    UMFDView(const FObjectInitializer& ObjectInitializer);

    // ----- Setup -----
    void SetIndex(int32 InIndex) { Index = InIndex; }

    int32 GetIndex() const { return Index; }

    void SetShip(Ship* InShip) { ShipPtr = InShip; }
    Ship* GetShip() const { return ShipPtr; }

    void SetMode(EMFDMode InMode);

    EMFDMode GetMode() const { return Mode; }

    void ShowMFD() { bHidden = false; }
    void HideMFD() { bHidden = true; }

    // Legacy-equivalent rect (widget-local)
    void SetMFDRect(const FIntRect& InRect);

    // Colors (HUD/Text)
    void SetHUDColor(const FColor& InHUD, const FColor& InText);

    // ----- UView interface -----
    virtual void Refresh() override;

    // ----- UUserWidget -----
    virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
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
    // ---------- Drawing primitives (Slate) ----------
    void DrawLine(const FVector2D& A, const FVector2D& B, const FLinearColor& C, float Thickness) const;
    void DrawRectOutline(const FSlateRect& R, const FLinearColor& C, float Thickness) const;
    void FillRect(const FSlateRect& R, const FLinearColor& C) const;
    void DrawEllipseOutline(const FVector2D& Center, float Rx, float Ry, const FLinearColor& C, float Thickness, int32 Segments) const;
    void DrawTextAt(const FVector2D& Pos, const FString& Text, const FSlateFontInfo& FontInfo, const FLinearColor& C) const;

    // ---------- Text helpers ----------
    FSlateFontInfo GetSlateFont() const;
    FVector2D MeasureText(const FString& S, const FSlateFontInfo& FontInfo) const;

    // ---------- MFD draw passes ----------
    void Draw() const;
    void DrawGameMFD() const;
    void DrawStatusMFD() const;
    void DrawSensorMFD() const;
    void DrawHSD() const;
    void Draw3D() const;

    void DrawSensorLabels(const FString& ModeLabel) const;
    void DrawGauge(int32 X, int32 Y, int32 Percent) const;

    // MFD text slots:
    void DrawMFDText(int32 Slot, const FString& Txt, const FSlateRect& Rect, int32 AlignFlags, int32 Status = -1) const;
    void HideMFDText(int32 Slot) const;

    // ---------- Internal state ----------
    static FLinearColor Dim(const FLinearColor& C, float Factor);

protected:
    // NOTE: NativePaint is const; we store the current paint targets as mutable pointers.
    mutable FSlateWindowElementList* PaintElements = nullptr;
    mutable const FGeometry* PaintGeo = nullptr;
    mutable int32 PaintLayer = 0;

    // Layout (widget local pixels):
    FIntRect MFDRect = FIntRect(0, 0, 128, 128);

    UPROPERTY()
    int32 Index = 0;

    UPROPERTY()
    EMFDMode Mode = EMFDMode::OFF;

    UPROPERTY()
    bool bHidden = true;

    // External pointers (legacy core):
    Ship* ShipPtr = nullptr;
    CameraView* CamViewPtr = nullptr;

    // Appearance:
    FColor HUDColor = FColor(0, 255, 255, 255);
    FColor TextColor = FColor::White;

    // Timing / click latch:
    mutable int32 MouseLatch = 0;
    mutable bool bMouseIn = false;

    // Font (legacy SystemFont bridge):
    SystemFont* HudFont = nullptr;

    // Text slot struct:
    struct FMFDTextSlot
    {
        SystemFont* Font = nullptr;
        FColor Color = FColor::White;
        FSlateRect Rect = FSlateRect(0, 0, 0, 0);
        bool bHidden = true;
    };

    enum { TXT_LAST = 20 };
    mutable FMFDTextSlot Slots[TXT_LAST] = {};
};
