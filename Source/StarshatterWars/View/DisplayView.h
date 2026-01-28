/*  Project Starshatter Wars
    Fractal Dev Studios
    Copyright (c) 2025-2026.

    SUBSYSTEM:    Stars.exe
    FILE:         DisplayView.h
    AUTHOR:       Carlos Bott

    ORIGINAL AUTHOR: John DiCamillo
    ORIGINAL STUDIO: Destroyer Studios LLC

    OVERVIEW
    ========
    View class for Quantum Destination HUD Overlay
    Unreal conversion:
    - UDisplayView is a UUserWidget (UMG)
    - Inherits from UView
    - SystemFont replaced by UFont
*/

#pragma once

#include "CoreMinimal.h"
#include "View.h"

// Legacy types:
#include "Types.h"
#include "Text.h"
#include "Bitmap.h"

#include "DisplayView.generated.h"

class Window;
class UFont;

// +--------------------------------------------------------------------+

USTRUCT()
struct FDisplayElement
{
    GENERATED_BODY()

    // Legacy fields:
    Text    TextValue;
    Bitmap* Image = nullptr;

    // SystemFont -> UE font:
    UPROPERTY(Transient)
    TObjectPtr<UFont> Font = nullptr;

    FColor  Color = FColor::White;
    Rect    RectValue;

    int     Blend = 0;
    double  Hold = 0.0;
    double  FadeIn = 0.0;
    double  FadeOut = 0.0;

    FORCEINLINE bool HasText() const
    {
        return TextValue.length() > 0;
    }

    FORCEINLINE bool HasImage() const
    {
        return Image != nullptr;
    }
};

// +--------------------------------------------------------------------+

UCLASS()
class STARSHATTERWARS_API UDisplayView : public UUserWidget
{
    GENERATED_BODY()

public:
    UDisplayView(const FObjectInitializer& ObjectInitializer);

    static const char* TYPENAME() { return "DisplayView"; }

    // UMG-safe init (replaces legacy DisplayView(Window*)):
    void InitializeView(Window* InWindow);

    // Operations (legacy-style API kept):
    virtual void Refresh() override;
    virtual void OnWindowMove() override;

    // In legacy this was ExecFrame() with internal Game::GUITime().
    // In UMG you can call this from wherever you tick your HUD stack.
    virtual void ExecFrame(float DeltaTime);

    virtual void ClearDisplay();

    virtual void AddText(
        const char* txt,
        SystemFont* font,
        FColor color,
        const Rect& rect,
        double hold = 1e9,
        double fade_in = 0.0,
        double fade_out = 0.0);

    virtual void AddImage(
        Bitmap* texture,
        FColor color,
        int blend,
        const Rect& rect,
        double hold = 1e9,
        double fade_in = 0.0,
        double fade_out = 0.0);

    // Widget singleton (world-aware; widgets cannot be created with new):
    static UDisplayView* GetInstance(UWorld* World, TSubclassOf<UDisplayView> WidgetClass);

protected:
    // Cached window geometry:
    int     width = 0;
    int     height = 0;
    double  xcenter = 0.0;
    double  ycenter = 0.0;

    // Elements to render:
    UPROPERTY(Transient)
    TArray<FDisplayElement> Elements;

private:
    // World-safe singleton storage:
    static TWeakObjectPtr<UDisplayView> DisplayViewInstance;

    // Convenience:
    void UpdateCachedWindowMetrics();
};
