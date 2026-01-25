/*  Project Starshatter 4.5
    Destroyer Studios LLC
    Copyright © 1997-2004.

    SUBSYSTEM:    Stars.exe
    FILE:         AwardShowDlg.h
    AUTHOR:       John DiCamillo

    UNREAL PORT:
    - Converted from FormWindow to UBaseScreen (UUserWidget-derived).
    - Preserves original member names and intent.
*/

#pragma once

#include "CoreMinimal.h"
#include "BaseScreen.h"
#include "AwardShowDlg.generated.h"

class UButton;
class UTextBlock;
class UImage;

/**
 * Main Menu Dialog Active Window class (UE UBaseScreen port)
 */
UCLASS()
class STARSHATTERWARS_API UAwardShowDlg : public UBaseScreen
{
    GENERATED_BODY()

public:
    UAwardShowDlg(const FObjectInitializer& ObjectInitializer);

    // Original API surface (ported):
    virtual void      RegisterControls();
    virtual void      Show();
    virtual void      ExecFrame();

    // Operations:
    UFUNCTION()
    virtual void      OnClose();

    virtual void      ShowAward();
    virtual void      SetRank(int r);
    virtual void      SetMedal(int r);

protected:
    // UUserWidget lifecycle:
    virtual void      NativeOnInitialized() override;
    virtual void      NativeConstruct() override;
    virtual void      NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

protected:
    // Starshatter: MenuScreen* manager;
    UPROPERTY(BlueprintReadWrite, Category = "AwardShowDlg")
    UObject* manager = nullptr;

    // ActiveWindow* -> UTextBlock*
    UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly)
    UTextBlock* lbl_name = nullptr;

    UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly)
    UTextBlock* lbl_info = nullptr;

    // ImageBox* -> UImage*
    UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly)
    UImage* img_rank = nullptr;

    // Button* -> UButton*
    UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly)
    UButton* btn_close = nullptr;

    UPROPERTY(BlueprintReadOnly, Category = "AwardShowDlg")
    bool              exit_latch = false;

    UPROPERTY(BlueprintReadOnly, Category = "AwardShowDlg")
    int32             rank = 0;

    UPROPERTY(BlueprintReadOnly, Category = "AwardShowDlg")
    int32             medal = 0;
};
