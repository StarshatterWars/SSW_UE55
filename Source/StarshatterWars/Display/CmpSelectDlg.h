/*  Project Starshatter Wars
    Fractal Dev Studios
    Copyright © 2025-2026.

    SUBSYSTEM:    Stars.exe
    FILE:         CmpSelectDlg.h
    AUTHOR:       Carlos Bott

    ORIGINAL AUTHOR AND STUDIO
    ==========================
    John DiCamillo / Destroyer Studios LLC

    UNREAL PORT:
    - Converted from FormWindow to UBaseScreen (UUserWidget-derived).
    - Preserves original member names and intent.
*/

#pragma once

#include "CoreMinimal.h"
#include "BaseScreen.h"
#include "CmpSelectDlg.generated.h"

class UButton;
class UListView;
class UTextBlock;
class UImage;

// Forward declarations for your ported gameplay/menu classes:
class MenuScreen;
class Campaign;
class Starshatter;
class Bitmap;
class Text;

/**
 * Mission Select Dialog Active Window class (UE UBaseScreen port)
 */
UCLASS()
class STARSHATTERWARS_API UCmpSelectDlg : public UBaseScreen
{
    GENERATED_BODY()

public:
    UCmpSelectDlg(const FObjectInitializer& ObjectInitializer);

    // Original API surface (ported):
    virtual void      RegisterControls();
    virtual void      Show();
    virtual void      ExecFrame();
    virtual bool      CanClose();

    // Operations (ported from AWEvent style; UE binds directly):
    UFUNCTION()
    virtual void      OnCampaignSelect();

    UFUNCTION()
    virtual void      OnNew();

    UFUNCTION()
    virtual void      OnSaved();

    UFUNCTION()
    virtual void      OnDelete();

    UFUNCTION()
    virtual void      OnConfirmDelete();

    UFUNCTION()
    virtual void      OnAccept();

    UFUNCTION()
    virtual void      OnCancel();

    virtual uint32    LoadProc();

protected:
    virtual void      StartLoadProc();
    virtual void      StopLoadProc();
    virtual void      ShowNewCampaigns();
    virtual void      ShowSavedCampaigns();

protected:
    // UUserWidget lifecycle:
    virtual void      NativeOnInitialized() override;
    virtual void      NativeConstruct() override;
    virtual void      NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

protected:
    // Starshatter: MenuScreen* manager;
    // UE: keep generic until you have a UMenuScreen/UMenuRoot type to cast to.
    UPROPERTY(BlueprintReadWrite, Category = "CmpSelectDlg")
    UObject* manager = nullptr;

    // Buttons:
    UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly)
    UButton* btn_new = nullptr;

    UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly)
    UButton* btn_saved = nullptr;

    UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly)
    UButton* btn_delete = nullptr;

    UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly)
    UButton* btn_accept = nullptr;

    UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly)
    UButton* btn_cancel = nullptr;

    // ListBox* lst_campaigns -> UListView*
    UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly)
    UListView* lst_campaigns = nullptr;

    // ActiveWindow* description -> UTextBlock*
    UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly)
    UTextBlock* description = nullptr;

    // Ported gameplay pointers (non-UObject, managed externally):
    Starshatter* stars = nullptr;
    Campaign* campaign = nullptr;

    // State:
    UPROPERTY(BlueprintReadOnly, Category = "CmpSelectDlg")
    int32 selected_mission = -1;

    // Loading state (ported):
    void* hproc = nullptr;     // HANDLE (Windows). Keep opaque in UE build.
    ThreadSync sync;
    bool       loading = false;
    bool       loaded = false;
    Text       load_file;
    int32      load_index = 0;
    bool       show_saved = false;

    // Classic: List<Bitmap> images;
    // UE: keep legacy container type (if Bitmap is ported core), or replace later.
    List<Bitmap> images;

    Text       select_msg;
};
