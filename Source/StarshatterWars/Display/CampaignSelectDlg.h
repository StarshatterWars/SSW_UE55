/*  Project Starshatter Wars
    Fractal Dev Studios
    Copyright (c) 2025-2026.

    SUBSYSTEM:    Stars.exe
    FILE:         CampaignSelectDlg.h
    AUTHOR:       Carlos Bott

    ORIGINAL AUTHOR AND STUDIO
    ==========================
    John DiCamillo / Destroyer Studios LLC

    UNREAL PORT:
    - Converted from FormWindow/AWEvent mapping to UBaseScreen (UUserWidget-derived).
    - Preserves original member names and intent where applicable.
*/

#pragma once

#include "CoreMinimal.h"
#include "BaseScreen.h"
#include "Text.h"
#include "List.h"

#include "CampaignSelectDlg.generated.h"



// UMG fwd:
class UButton;
class UListView;
class UTextBlock;
class URichTextBlock;
class UImage;

// Starshatter fwd:
class MenuScreen;
class Campaign;
class Starshatter;

// Legacy containers/types (ported in your codebase):
template<typename T> class List;
class Bitmap;
class Text;
class ThreadSync;



UCLASS()
class STARSHATTERWARS_API UCampaignSelectDlg : public UBaseScreen
{
    GENERATED_BODY()

public:
    UCampaignSelectDlg(const FObjectInitializer& ObjectInitializer);

    // ----------------------------------------------------------------
    // UUserWidget lifecycle
    // ----------------------------------------------------------------
    virtual void NativeOnInitialized() override;
    virtual void NativeConstruct() override;
    virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

    // ----------------------------------------------------------------
    // Legacy dialog surface (ported)
    // ----------------------------------------------------------------
    virtual void      RegisterControls();
    virtual void      Show();
    virtual void      ExecFrame();
    virtual bool      CanClose();

    // Operations (AWEvent -> UFUNCTION handlers):
    UFUNCTION() virtual void OnCampaignSelect();
    UFUNCTION() virtual void OnNew();
    UFUNCTION() virtual void OnSaved();
    UFUNCTION() virtual void OnDelete();
    UFUNCTION() virtual void OnConfirmDelete();
    UFUNCTION() virtual void OnAccept();
    UFUNCTION() virtual void OnCancel();

    virtual uint32    LoadProc();

protected:
    virtual void      StartLoadProc();
    virtual void      StopLoadProc();
    virtual void      ShowNewCampaigns();
    virtual void      ShowSavedCampaigns();

protected:
    // ----------------------------------------------------------------
    // UBaseScreen overrides
    // ----------------------------------------------------------------
    virtual void BindFormWidgets() override;
    virtual FString GetLegacyFormText() const override;

public:
    // Screen manager (set by owning menu screen)
    MenuScreen* manager = nullptr;

protected:
    // ----------------------------------------------------------------
    // Bound UMG controls (match FORM ids)
    // ----------------------------------------------------------------

    // Buttons:
    UPROPERTY(meta = (BindWidgetOptional)) UButton* btn_new = nullptr;       // id 100
    UPROPERTY(meta = (BindWidgetOptional)) UButton* btn_saved = nullptr;     // id 101
    UPROPERTY(meta = (BindWidgetOptional)) UButton* btn_delete = nullptr;    // id 102
    UPROPERTY(meta = (BindWidgetOptional)) UButton* btn_accept = nullptr;    // id 1
    UPROPERTY(meta = (BindWidgetOptional)) UButton* btn_cancel = nullptr;    // id 2

    // List:
    UPROPERTY(meta = (BindWidgetOptional)) UListView* lst_campaigns = nullptr; // id 201

    // Text description (FORM type "text" -> URichTextBlock in BaseScreen):
    UPROPERTY(meta = (BindWidgetOptional)) URichTextBlock* description = nullptr; // id 200

    // Optional labels (if your UMG includes them):
    UPROPERTY(meta = (BindWidgetOptional)) UTextBlock* lbl_title = nullptr;         // id 10
    UPROPERTY(meta = (BindWidgetOptional)) UTextBlock* lbl_hdr_campaign = nullptr;  // id 901
    UPROPERTY(meta = (BindWidgetOptional)) UTextBlock* lbl_hdr_desc = nullptr;      // id 902

    // Optional background images (if you mapped them in UMG):
    UPROPERTY(meta = (BindWidgetOptional)) UImage* bg_9991 = nullptr; // id 9991
    UPROPERTY(meta = (BindWidgetOptional)) UImage* bg_9992 = nullptr; // id 9992

protected:
    // ----------------------------------------------------------------
    // Legacy state (ported)
    // ----------------------------------------------------------------
    Starshatter* stars = nullptr;
    Campaign* campaign = nullptr;
    int          selected_mission = 0;

    // Threading placeholders (keep names; avoid Win32 HANDLE in UE headers):
    void* hproc = nullptr;
    ThreadSync   sync;

    bool         loading = false;
    bool         loaded = false;

    Text         load_file;
    int          load_index = -1;
    bool         show_saved = false;

    List<Bitmap> images;
    Text         select_msg;
};
