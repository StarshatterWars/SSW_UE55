/*  Project Starshatter 4.5
    Destroyer Studios LLC
    Copyright © 1997-2004. All Rights Reserved.

    SUBSYSTEM:    Stars.exe
    FILE:         LoadDlg.h
    AUTHOR:       John DiCamillo

    UNREAL PORT:
    - Converted from FormWindow to UBaseScreen (UUserWidget).
    - Preserves original member names and intent.
    - UE-only: UMG widgets are bound via BindWidget (Widget Blueprint).
*/

#pragma once

#include "CoreMinimal.h"
#include "BaseScreen.h"

// UMG components:
#include "Components/TextBlock.h"
#include "Components/ProgressBar.h"

#include "LoadDlg.generated.h"

/**
 * Loading progress dialog box (UE UUserWidget port)
 */
UCLASS()
class STARSHATTERWARS_API ULoadDlg : public UBaseScreen
{
    GENERATED_BODY()

public:
    ULoadDlg(const FObjectInitializer& ObjectInitializer);

    // Original API surface (ported):
    virtual void RegisterControls();   // cache pointers / bind events if needed
    virtual void ExecFrame();          // optional per-frame logic

protected:
    // UUserWidget lifecycle:
    virtual void NativeOnInitialized() override;
    virtual void NativeConstruct() override;
    virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

protected:
    // Starshatter: ActiveWindow* title/activity; Slider* progress;
    // UE: map to UTextBlock and UProgressBar.

    UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly)
    UTextBlock* activity = nullptr;

    UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly)
    UProgressBar* progress = nullptr;

protected:
    float ProgressValue = 0.0f;
};
