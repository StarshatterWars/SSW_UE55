/*  Project Starshatter Wars
    Fractal Dev Studios
    Copyright (c) 2025-2026.

    ORIGINAL AUTHOR AND STUDIO
    ==========================
    John DiCamillo / Destroyer Studios LLC

    SUBSYSTEM:    Stars.exe
    FILE:         ExitDlg.h
    AUTHOR:       Carlos Bott

    OVERVIEW
    ========
    ExitDlg (Unreal)
    - Unreal UMG replacement for legacy ExitDlg FormWindow
    - Maintains original method names and semantics
*/

#pragma once

#include "CoreMinimal.h"
#include "BaseScreen.h"

#include "Components/ScrollBox.h"
#include "Components/RichTextBlock.h"

#include "ExitDlg.generated.h"

class UMenuScreen;

UCLASS()
class STARSHATTERWARS_API UExitDlg : public UBaseScreen
{
    GENERATED_BODY()

public:
    // Unreal-style constructor
    UExitDlg(const FObjectInitializer& ObjectInitializer);

    // ------------------------------------------------------------
    // Legacy API (preserved)
    // ------------------------------------------------------------
    virtual void RegisterControls();
    virtual void Show();
    virtual void ExecFrame(float DeltaTime);

    virtual void OnApply();
    virtual void OnCancel();

    void SetManager(UMenuScreen* InManager);

protected:
    // ------------------------------------------------------------
    // UBaseScreen overrides
    // ------------------------------------------------------------
    virtual void BindFormWidgets() override;
    virtual FString GetLegacyFormText() const override;

    virtual void NativeConstruct() override;
    virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

    virtual void HandleAccept() override;
    virtual void HandleCancel() override;

protected:
    // ------------------------------------------------------------
    // Button handlers (NO lambdas)
    // ------------------------------------------------------------
    UFUNCTION()
    void HandleApplyClicked();

    UFUNCTION()
    void HandleCancelClicked();

protected:
    // ------------------------------------------------------------
    // Optional UMG bindings
    // ------------------------------------------------------------
    UPROPERTY(meta = (BindWidgetOptional))
    UScrollBox* CreditsScroll = nullptr;

    UPROPERTY(meta = (BindWidgetOptional))
    URichTextBlock* CreditsText = nullptr;

protected:
    // ------------------------------------------------------------
    // State
    // ------------------------------------------------------------
    UPROPERTY(Transient)
    TObjectPtr<UMenuScreen> Manager = nullptr;

    bool  bExitLatch = false;
    float ScrollOffset = 0.0f;

    UPROPERTY(EditAnywhere, Category = "ExitDlg")
    float ScrollPixelsPerSecond = 22.0f;

protected:
    // ------------------------------------------------------------
    // Helpers
    // ------------------------------------------------------------
    bool LoadCreditsFile(FString& OutText) const;
    void ApplyCredits(const FString& Text);
};
