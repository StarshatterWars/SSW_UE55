// PlayerDlg.h

#pragma once

#include "CoreMinimal.h"
#include "BaseScreen.h"
#include "PlayerDlg.generated.h"

class UButton;
class UEditableTextBox;
class UImage;
class UListView;
class UTextBlock;
class UScrollBox;
class UHorizontalBox;
class UVerticalBox;
class UUniformGridPanel;
class UBorder;

class UPlayerRosterItem;
class PlayerCharacter;
class UMenuScreen;

UCLASS()
class STARSHATTERWARS_API UPlayerDlg : public UBaseScreen
{
    GENERATED_BODY()

public:
    UPlayerDlg(const FObjectInitializer& ObjectInitializer);

    void InitializeDlg(UMenuScreen* InManager);

    virtual void NativeOnInitialized() override;
    virtual void NativePreConstruct() override;
    virtual void NativeConstruct() override;
    virtual void NativeDestruct() override;

    virtual void HandleAccept() override;
    virtual void HandleCancel() override;

    void ShowDlg();
    void HideDlg();

protected:
    // -------- Layout (runtime) --------
    void EnsureLayout();
    void EnsureScrollHostsStats();

    // -------- Build UI rows (Options-style) --------
    void BuildRosterPanel();
    void BuildStatsRows();     // Name/Password/Squad/Signature/Created/Flight/Missions/Kills/Losses/Points/Rank + insignia + medals + macros

    // -------- Model <-> UI --------
    void BuildRoster();
    void RefreshRoster();

    void UpdatePlayerFromUI();   // commit edits to model (best-effort, gated by concept checks)
    void RefreshUIFromPlayer();  // model -> UI

    PlayerCharacter* GetSelectedPlayer() const;

    // -------- Events --------
    UFUNCTION()
    void OnRosterSelectionChanged(UObject* SelectedItem);

    UFUNCTION()
    void OnAdd();

    UFUNCTION()
    void OnDel();

    UFUNCTION()
    void OnApply();

    UFUNCTION()
    void OnCancel();

    // -------- Helpers --------
    static FString FormatTimeHMS(double Seconds);
    static FString FormatDateFromUnixSeconds(int64 UnixSeconds);

    // Options-style row helpers (local to PlayerDlg)
    UHorizontalBox* AddStatRow(const FText& Label, UWidget* RightWidget, float RightWidth = 420.f, float RowPadY = 6.f);
    UTextBlock* MakeValueText();
    UEditableTextBox* MakeEditBox(bool bPassword = false);

    void BuildMedalsGrid(int32 Columns = 5, int32 Rows = 3);
    void BuildChatMacrosRows();

protected:
    // -------- Manager --------
    UPROPERTY()
    TObjectPtr<UMenuScreen> manager = nullptr;

    // -------- Root containers (optional BP binds) --------
    UPROPERTY(meta = (BindWidgetOptional))
    UCanvasPanel* RootCanvasPlayer = nullptr; // if your BP uses a different canvas name than BaseScreen::RootCanvas

    // -------- Runtime-built layout --------
    UPROPERTY()
    TObjectPtr<UHorizontalBox> MainRow = nullptr;

    UPROPERTY()
    TObjectPtr<UVerticalBox> LeftPanel = nullptr;

    UPROPERTY()
    TObjectPtr<UScrollBox> StatsScroll = nullptr;

    UPROPERTY()
    TObjectPtr<UVerticalBox> StatsVBox = nullptr; // will be used as "AutoVBox" for AddLabeledRow-like behavior

    // -------- Left / roster --------
    UPROPERTY()
    TObjectPtr<UListView> lst_roster = nullptr;

    // -------- Stats widgets (right side) --------
    UPROPERTY()
    TObjectPtr<UEditableTextBox> edt_name = nullptr;

    UPROPERTY()
    TObjectPtr<UEditableTextBox> edt_password = nullptr;

    UPROPERTY()
    TObjectPtr<UEditableTextBox> edt_squadron = nullptr;

    UPROPERTY()
    TObjectPtr<UEditableTextBox> edt_signature = nullptr;

    UPROPERTY()
    TObjectPtr<UTextBlock> txt_created = nullptr;

    UPROPERTY()
    TObjectPtr<UTextBlock> txt_flighttime = nullptr;

    UPROPERTY()
    TObjectPtr<UTextBlock> txt_missions = nullptr;

    UPROPERTY()
    TObjectPtr<UTextBlock> txt_kills = nullptr;

    UPROPERTY()
    TObjectPtr<UTextBlock> txt_losses = nullptr;

    UPROPERTY()
    TObjectPtr<UTextBlock> txt_points = nullptr;

    UPROPERTY()
    TObjectPtr<UTextBlock> txt_rank = nullptr;

    UPROPERTY()
    TObjectPtr<UImage> img_rank = nullptr;

    UPROPERTY()
    TObjectPtr<UUniformGridPanel> medals_grid = nullptr;

    UPROPERTY()
    TArray<TObjectPtr<UImage>> MedalImages;

    UPROPERTY()
    TArray<TObjectPtr<UEditableTextBox>> MacroEdits; // 10

    // -------- Selection --------
    UPROPERTY()
    TObjectPtr<UPlayerRosterItem> selected_item = nullptr;

    PlayerCharacter* SelectedPlayer = nullptr;
};
