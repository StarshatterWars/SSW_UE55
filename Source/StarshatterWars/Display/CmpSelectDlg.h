/*  Project Starshatter Wars
    Fractal Dev Studios
    Copyright (c) 2025-2026.

    ORIGINAL AUTHOR AND STUDIO
    ==========================
    John DiCamillo / Destroyer Studios LLC

    SUBSYSTEM:    Stars.exe
    FILE:         CmpSelectDlg.h
    AUTHOR:       Carlos Bott

    OVERVIEW
    ========
    Campaign/Mission Select dialog Unreal UUserWidget implementation.

    UNREAL PORT NOTES
    ================
    - Ported from FormWindow/AWEvent mapping to UBaseScreen (UUserWidget-derived).
    - Manager is MenuScreen (non-UObject). Stored as a raw pointer; set via SetManager().
    - Uses UMG widgets (UButton/UListView/UTextBlock) and optional FORM-ID binding helpers.
    - Provides a UObject list item type (UCmpSelectItem) for UListView population/selection.
*/

#pragma once

#include "CoreMinimal.h"
#include "BaseScreen.h"

// Starshatter container/types (your ported Types/Text/List/ThreadSync):
#include "Types.h"
#include "Text.h"
#include "Bitmap.h"
#include "List.h"   

// UMG:
#include "Components/Button.h"
#include "Components/ListView.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"

#include "CmpSelectDlg.generated.h"

// Starshatter core forward declarations (non-UObject):
class MenuScreen;
class Campaign;
class Starshatter;


// +--------------------------------------------------------------------+
// ListView Item Object
// +--------------------------------------------------------------------+

UCLASS()
class STARSHATTERWARS_API UCmpSelectItem : public UObject
{
    GENERATED_BODY()

public:
    // Display:
    UPROPERTY(BlueprintReadOnly) FString Title;

    // If this is a "new campaign" entry, LoadIndex >= 0 and SaveFile is empty.
    // If this is a "saved campaign" entry, LoadIndex == -1 and SaveFile is set.
    UPROPERTY(BlueprintReadOnly) int32 LoadIndex = -1;
    UPROPERTY(BlueprintReadOnly) FString SaveFile;

    // Optional thumbnail you can show in your entry widget:
    UPROPERTY(BlueprintReadOnly) TObjectPtr<UTexture2D> Thumbnail = nullptr;

    bool IsSaved() const { return LoadIndex < 0 && !SaveFile.IsEmpty(); }
};

// +--------------------------------------------------------------------+
// Dialog Widget
// +--------------------------------------------------------------------+

UCLASS()
class STARSHATTERWARS_API UCmpSelectDlg : public UBaseScreen
{
    GENERATED_BODY()

public:
    UCmpSelectDlg(const FObjectInitializer& ObjectInitializer);

    /** Non-UObject manager setter */
    void SetManager(MenuScreen* InManager) { manager = InManager; }

    // Legacy-like surface:
    void RegisterControls();
    void Show();
    void ExecFrame();
    bool CanClose();

    // Operations:
    UFUNCTION() void OnCampaignSelect(UObject* SelectedItem);
    UFUNCTION() void OnNew();
    UFUNCTION() void OnSaved();
    UFUNCTION() void OnDelete();
    UFUNCTION() void OnConfirmDelete();
    UFUNCTION() void OnAccept();
    UFUNCTION() void OnCancel();

    uint32 LoadProc();

protected:
    void StartLoadProc();
    void StopLoadProc();
    void ShowNewCampaigns();
    void ShowSavedCampaigns();

    void SetDescriptionFromCampaign(Campaign* InCampaign);
    void SetDescriptionSelectMsg();

    void RebuildListFromNewCampaigns();
    void RebuildListFromSavedCampaigns();

    // ---- UBaseScreen overrides --------------------------------------
    virtual void BindFormWidgets() override;
    virtual FString GetLegacyFormText() const override { return FString(); }

    // ---- UUserWidget -------------------------------------------------
    virtual void NativeOnInitialized() override;
    virtual void NativeConstruct() override;
    virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
    virtual FReply NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;

protected:
    // -----------------------------------------------------------------
    // OPTIONAL: FORM IDs (adjust to match your .frm when you wire it)
    // -----------------------------------------------------------------
    static constexpr int32 ID_BTN_NEW = 101;
    static constexpr int32 ID_BTN_SAVED = 102;
    static constexpr int32 ID_BTN_DELETE = 103;
    static constexpr int32 ID_BTN_ACCEPT = 1;
    static constexpr int32 ID_BTN_CANCEL = 2;

    static constexpr int32 ID_LST_CAMPAIGNS = 200;
    static constexpr int32 ID_TXT_DESC = 201;

protected:
    // -----------------------------------------------------------------
    // Manager (non-UObject)
    // -----------------------------------------------------------------
    MenuScreen* manager = nullptr;

protected:
    // -----------------------------------------------------------------
    // Widgets (BindWidgetOptional)
    // -----------------------------------------------------------------
    UPROPERTY(meta = (BindWidgetOptional)) UButton* btn_new = nullptr;
    UPROPERTY(meta = (BindWidgetOptional)) UButton* btn_saved = nullptr;
    UPROPERTY(meta = (BindWidgetOptional)) UButton* btn_delete = nullptr;
    UPROPERTY(meta = (BindWidgetOptional)) UButton* btn_accept = nullptr;
    UPROPERTY(meta = (BindWidgetOptional)) UButton* btn_cancel = nullptr;

    UPROPERTY(meta = (BindWidgetOptional)) UListView* lst_campaigns = nullptr;
    UPROPERTY(meta = (BindWidgetOptional)) UTextBlock* description = nullptr;

protected:
    // -----------------------------------------------------------------
    // Starshatter core state (ported from classic)
    // -----------------------------------------------------------------
    Starshatter* stars = nullptr;
    Campaign* campaign = nullptr;

    int32 selected_mission = 0;

    // Keep your legacy ThreadSync; avoid Win32 HANDLE in UE headers:
    void* hproc = nullptr;
    ThreadSync sync;

    bool loading = false;
    bool loaded = false;

    Text  load_file;
    int32 load_index = -1;

    bool show_saved = false;

    // Legacy thumbnails (Bitmap list):
    List<Bitmap> images;

    Text select_msg;

protected:
    // Cached items for list:
    UPROPERTY(Transient) TArray<TObjectPtr<UCmpSelectItem>> ListItems;
};
