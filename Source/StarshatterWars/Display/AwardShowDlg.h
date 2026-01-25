/*  Project Starshatter Wars
    Fractal Dev Studios
    Copyright (c) 2025-2026. All Rights Reserved.

    ORIGINAL AUTHOR AND STUDIO
    ==========================
    John DiCamillo / Destroyer Studios LLC

    SUBSYSTEM:    Stars.exe
    FILE:         AwardShowDlg.h
    AUTHOR:       Carlos Bott

    OVERVIEW
    ========
    Award / Rank display dialog Unreal UUserWidget implementation.
    Port of Starshatter 4.5 AwardShowDlg (FormWindow) to UMG + UBaseScreen.

    NOTES
    =====
    - Manager is MenuScreen (non-UObject). Forward-declared in header, fully included in .cpp.
    - Uses UBaseScreen FORM-ID bindings for controls:
        201 = lbl_info
        202 = img_rank
        203 = lbl_name
        1   = btn_close
*/

#pragma once

#include "CoreMinimal.h"
#include "BaseScreen.h"

// UMG:
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"

#include "AwardShowDlg.generated.h"

// Forward declaration (MenuScreen is NOT a UObject here):
class GameScreen;

UCLASS()
class STARSHATTERWARS_API UAwardShowDlg : public UBaseScreen
{
    GENERATED_BODY()

public:
    UAwardShowDlg(const FObjectInitializer& ObjectInitializer);

    /** Non-UObject manager setter */
    void SetManager(GameScreen* InManager) { manager = InManager; }

    // ---- UBaseScreen overrides --------------------------------------
    virtual void BindFormWidgets() override;

    // Optional: provide raw legacy .frm text for parser (override if desired)
    virtual FString GetLegacyFormText() const override { return FString(); }

    // ---- Operations --------------------------------------------------
    void ShowAward();
    void SetRank(int32 InRank);
    void SetMedal(int32 InMedal);

protected:
    virtual void NativeConstruct() override;
    virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
    virtual FReply NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;

protected:
    // -----------------------------------------------------------------
    // Manager (non-UObject)
    // -----------------------------------------------------------------
    GameScreen* manager = nullptr;

    // -----------------------------------------------------------------
    // Widgets (BindWidgetOptional) – align these with your UMG names
    // -----------------------------------------------------------------
    UPROPERTY(meta = (BindWidgetOptional)) UTextBlock* lbl_info = nullptr;  // 201
    UPROPERTY(meta = (BindWidgetOptional)) UImage* img_rank = nullptr;  // 202
    UPROPERTY(meta = (BindWidgetOptional)) UTextBlock* lbl_name = nullptr;  // 203
    UPROPERTY(meta = (BindWidgetOptional)) UButton* btn_close = nullptr; // 1

protected:
    // Legacy latches/state:
    bool  exit_latch = true;
    int32 rank = -1;
    int32 medal = -1;

protected:
    UFUNCTION() void OnCloseClicked();
};
