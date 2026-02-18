#pragma once

#include "CoreMinimal.h"
#include "BaseScreen.h"
#include "Engine/DataTable.h"
#include "TacRefDlg.generated.h"

class UButton;
class UTextBlock;
class URichTextBlock;
class UComboBoxString;
class UMenuScreen;

struct FShipDesign;
struct FShipWeapon;

UCLASS()
class STARSHATTERWARS_API UTacRefDlg : public UBaseScreen
{
    GENERATED_BODY()

public:
    UTacRefDlg(const FObjectInitializer& ObjectInitializer);

    virtual void SetMenuManager(UMenuScreen* InManager);
    virtual void InitializeDlg(UMenuScreen* InManager);

    void Show();
    void ExecFrame();

protected:
    virtual void NativeOnInitialized() override;
    virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

protected:
    // ------------------------------------------------------------
    // Data
    // ------------------------------------------------------------
    UPROPERTY(EditDefaultsOnly, Category = "TacRef|Data")
    TObjectPtr<UDataTable> ShipDesignTable = nullptr;

    // Dropdown option index -> RowName mapping
    UPROPERTY(Transient)
    TArray<FName> ShipRowNames;

    UPROPERTY(Transient)
    int32 SelectedShipIndex = INDEX_NONE;

protected:
    // ------------------------------------------------------------
    // Widgets (these names MUST match your WBP_* widget names)
    // ------------------------------------------------------------

    // Title at top ("TACTICAL REFERENCE") - optional
    UPROPERTY(meta = (BindWidgetOptional)) UTextBlock* TitleText = nullptr;

    // Dropdown list (you already have this)
    UPROPERTY(meta = (BindWidgetOptional)) UComboBoxString* ShipCombo = nullptr;

    UPROPERTY(meta = (BindWidgetOptional)) UButton* StationButton = nullptr;
    UPROPERTY(meta = (BindWidgetOptional)) UButton* ShipButton = nullptr;
    UPROPERTY(meta = (BindWidgetOptional)) UButton* FighterButton = nullptr;
    UPROPERTY(meta = (BindWidgetOptional)) UButton* WeaponButton = nullptr;

    // These are NOT shown in your snippet; make sure they exist in the WBP
    // and are named exactly like this, OR rename these fields to match your WBP.
    UPROPERTY(meta = (BindWidgetOptional)) UTextBlock* TxtCaption = nullptr;
    UPROPERTY(meta = (BindWidgetOptional)) URichTextBlock* TxtStats = nullptr;
    UPROPERTY(meta = (BindWidgetOptional)) URichTextBlock* TxtDescription = nullptr;

protected:
    // ------------------------------------------------------------
    // Events
    // ------------------------------------------------------------
    UFUNCTION() void HandleCloseClicked();
    UFUNCTION() void HandleShipModeClicked();
    UFUNCTION() void HandleWeaponModeClicked();
    UFUNCTION() void HandleShipComboChanged(FString SelectedItem, ESelectInfo::Type SelectionType);

protected:
    // ------------------------------------------------------------
    // Internals
    // ------------------------------------------------------------
    void PopulateShipDropdown();
    void SelectShipByIndex(int32 Index);
    void BuildShipTexts(const FShipDesign& Dsn, FString& OutCaption, FString& OutStats, FString& OutDesc) const;

protected:
    UPROPERTY(Transient)
    TObjectPtr<UMenuScreen> manager = nullptr;

    int32 Mode = 0; // 0=ship, 1=weapon
};
