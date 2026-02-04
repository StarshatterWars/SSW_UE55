#pragma once

#include "CoreMinimal.h"
#include "BaseScreen.h"
#include "OptionsScreen.generated.h"

// Forward declares:
class UButton;
class UComboBoxString;
class UTextBlock;

class UAudioDlg;
class UVideoDlg;
class UControlOptionsDlg;
class UMenuScreen;

UCLASS()
class STARSHATTERWARS_API UOptionsScreen : public UBaseScreen
{
    GENERATED_BODY()

public:
    UOptionsScreen(const FObjectInitializer& ObjectInitializer);

    virtual void NativeConstruct() override;
    virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

protected:
    virtual bool NativeSupportsKeyboardFocus() const override { return true; }
    virtual FReply NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;

public:
    // MenuScreen hook:
    void SetMenuManager(UMenuScreen* InManager) { MenuManager = InManager; }
    UMenuScreen* GetMenuManager() const { return MenuManager; }

    // Page routing (used by tabs + subdialogs if you want):
    void ShowOptDlg();   // this page
    void ShowAudDlg();
    void ShowVidDlg();
    void ShowCtlDlg();
    void ShowModDlg();   // stub

    void ApplyOptions();
    void CancelOptions();

    // Legacy apply/cancel for THIS page’s values:
    UFUNCTION(BlueprintCallable, Category = "StarshatterWars|UI")
    void Apply();

    UFUNCTION(BlueprintCallable, Category = "StarshatterWars|UI")
    void Cancel();

protected:
    // Helpers you were calling but didn’t declare:
    void EnsureSubDialogs();
    void HideAllPages();

protected:
    // Buttons
    UFUNCTION() void OnApplyClicked();
    UFUNCTION() void OnCancelClicked();

    // Combo handlers
    UFUNCTION() void OnFlightModelChanged(FString SelectedItem, ESelectInfo::Type SelectionType);
    UFUNCTION() void OnFlyingStartChanged(FString SelectedItem, ESelectInfo::Type SelectionType);
    UFUNCTION() void OnLandingsChanged(FString SelectedItem, ESelectInfo::Type SelectionType);
    UFUNCTION() void OnAIDifficultyChanged(FString SelectedItem, ESelectInfo::Type SelectionType);
    UFUNCTION() void OnHudModeChanged(FString SelectedItem, ESelectInfo::Type SelectionType);
    UFUNCTION() void OnHudColorChanged(FString SelectedItem, ESelectInfo::Type SelectionType);
    UFUNCTION() void OnFfModeChanged(FString SelectedItem, ESelectInfo::Type SelectionType);
    UFUNCTION() void OnGridModeChanged(FString SelectedItem, ESelectInfo::Type SelectionType);
    UFUNCTION() void OnGunsightChanged(FString SelectedItem, ESelectInfo::Type SelectionType);

    // Tabs
    UFUNCTION() void OnAudioClicked();
    UFUNCTION() void OnVideoClicked();
    UFUNCTION() void OnOptionsClicked();
    UFUNCTION() void OnControlsClicked();
    UFUNCTION() void OnModClicked();

protected:
    // ------------------------------------------------------------
    // Subdialog classes (you referenced these in .cpp)
    // ------------------------------------------------------------
    UPROPERTY(EditDefaultsOnly, Category = "Options|Classes")
    TSubclassOf<UAudioDlg> AudioDlgClass;

    UPROPERTY(EditDefaultsOnly, Category = "Options|Classes")
    TSubclassOf<UVideoDlg> VideoDlgClass;

    UPROPERTY(EditDefaultsOnly, Category = "Options|Classes")
    TSubclassOf<UControlOptionsDlg> ControlDlgClass;

protected:
    // ------------------------------------------------------------
    // Subdialogs (you referenced these in .cpp)
    // ------------------------------------------------------------
    UPROPERTY(Transient)
    TObjectPtr<UAudioDlg> AudDlg = nullptr;

    UPROPERTY(Transient)
    TObjectPtr<UVideoDlg> VidDlg = nullptr;

    UPROPERTY(Transient)
    TObjectPtr<UControlOptionsDlg> CtlDlg = nullptr;

protected:
    // MenuScreen owner (tiny setter requested)
    UPROPERTY(Transient)
    TObjectPtr<UMenuScreen> MenuManager = nullptr;

protected:
    // Main options widgets (BindWidgetOptional)
    UPROPERTY(meta = (BindWidgetOptional)) UComboBoxString* flight_model = nullptr;
    UPROPERTY(meta = (BindWidgetOptional)) UComboBoxString* flying_start = nullptr;
    UPROPERTY(meta = (BindWidgetOptional)) UComboBoxString* landings = nullptr;
    UPROPERTY(meta = (BindWidgetOptional)) UComboBoxString* ai_difficulty = nullptr;
    UPROPERTY(meta = (BindWidgetOptional)) UComboBoxString* hud_mode = nullptr;
    UPROPERTY(meta = (BindWidgetOptional)) UComboBoxString* hud_color = nullptr;
    UPROPERTY(meta = (BindWidgetOptional)) UComboBoxString* ff_mode = nullptr;
    UPROPERTY(meta = (BindWidgetOptional)) UComboBoxString* grid_mode = nullptr;
    UPROPERTY(meta = (BindWidgetOptional)) UComboBoxString* gunsight = nullptr;

    UPROPERTY(meta = (BindWidgetOptional)) UTextBlock* description = nullptr;

    // Nav buttons
    UPROPERTY(meta = (BindWidgetOptional)) UButton* aud_btn = nullptr;
    UPROPERTY(meta = (BindWidgetOptional)) UButton* vid_btn = nullptr;
    UPROPERTY(meta = (BindWidgetOptional)) UButton* opt_btn = nullptr;
    UPROPERTY(meta = (BindWidgetOptional)) UButton* ctl_btn = nullptr;
    UPROPERTY(meta = (BindWidgetOptional)) UButton* mod_btn = nullptr;

    // Apply/Cancel buttons
    UPROPERTY(meta = (BindWidgetOptional)) UButton* ApplyBtn = nullptr;
    UPROPERTY(meta = (BindWidgetOptional)) UButton* CancelBtn = nullptr;

protected:
    bool bClosed = true;
};
