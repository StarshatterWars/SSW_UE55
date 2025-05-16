// /*  Project nGenEx	Fractal Dev Games	Copyright (C) 2024. All Rights Reserved.	SUBSYSTEM:    SSW	FILE:         Game.cpp	AUTHOR:       Carlos Bott*/

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "../Game/GameStructs.h"

#include "Engine/Texture2D.h"
#include "Components/Image.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "Components/CanvasPanel.h"
#include "Components/Border.h"
#include "Components/ComboBoxString.h"
#include "Components/ScrollBox.h"
#include "Components/EditableTextBox.h"
#include "Components/WidgetSwitcher.h"
  
#include "Components/ListView.h"

#include "Kismet/GameplayStatics.h"
#include "../System/SSWGameInstance.h"
#include "OperationsScreen.generated.h"

// Forward declarations
class UListView;

class UOOBForceItem;
class UOOBFleetItem;
class UOOBCarrierGroupItem;
class UOOBBattleItem;
class UOOBDestroyerItem;
class UOOBWingItem;
class UOOBUnitItem;
class UOOBSquadronItem;
class UOOBFighterSquadronItem;
class UOOBFighterUnit;
class UOOBBattalion;
class UOOBBatteryItem;
class UOOBCivilianItem;
class UOOBForceWidget;

class UPanelWidget;
class USelectableButtonGroup;
class UMenuButton;
class UVerticalBox;
class UScrollBox;

class USystemMarker;
class UTexture2D;

struct FSubGroupArray
{
	ECOMBATGROUP_TYPE Type;
	TArray<int32> Ids;

	FSubGroupArray(ECOMBATGROUP_TYPE InType)
		: Type(InType)
	{
	}
};

USTRUCT()
struct FMatchedGroupKey
{
    GENERATED_BODY()

    UPROPERTY()
    EEMPIRE_NAME Empire;

    UPROPERTY()
    ECOMBATGROUP_TYPE Type;

    UPROPERTY()
    int32 Id;

    FMatchedGroupKey()
        : Empire(EEMPIRE_NAME::NONE)
        , Type(ECOMBATGROUP_TYPE::NONE)
        , Id(INDEX_NONE)
    {}

    FMatchedGroupKey(EEMPIRE_NAME InEmpire, ECOMBATGROUP_TYPE InType, int32 InId)
        : Empire(InEmpire)
        , Type(InType)
        , Id(InId)
    {}

    bool operator==(const FMatchedGroupKey& Other) const
    {
        return Empire == Other.Empire && Type == Other.Type && Id == Other.Id;
    }
};

// Required to use in TSet/TMap
FORCEINLINE uint32 GetTypeHash(const FMatchedGroupKey& Key)
{
    uint32 Hash = HashCombine(::GetTypeHash((uint8)Key.Empire), ::GetTypeHash((uint8)Key.Type));
    return HashCombine(Hash, ::GetTypeHash(Key.Id));
}

/**
 * 
 */
UCLASS()
class STARSHATTERWARS_API UOperationsScreen : public UUserWidget
{
	GENERATED_BODY()
	
	UPROPERTY(meta = (BindWidgetOptional))
	class UTextBlock* TitleText;

	UPROPERTY(meta = (BindWidgetOptional))
	class UTextBlock* PlayerNameText;

	UPROPERTY(meta = (BindWidgetOptional))
	class UTextBlock* GameTimeText;

	UPROPERTY(meta = (BindWidgetOptional))
	class UTextBlock* CampaignNameText;
	UPROPERTY(meta = (BindWidgetOptional))
	class UTextBlock* DescriptionText;
	UPROPERTY(meta = (BindWidgetOptional))
	class UTextBlock* SituationText;
	UPROPERTY(meta = (BindWidgetOptional))
	class UTextBlock* Orders1Text;
	UPROPERTY(meta = (BindWidgetOptional))
	class UTextBlock* Orders2Text;
	UPROPERTY(meta = (BindWidgetOptional))
	class UTextBlock* Orders3Text;
	UPROPERTY(meta = (BindWidgetOptional))
	class UTextBlock* Orders4Text;
	UPROPERTY(meta = (BindWidgetOptional))
	class UTextBlock* LocationSystemText;

	UPROPERTY(meta = (BindWidgetOptional))
	class UTextBlock* MissionNameText;
	UPROPERTY(meta = (BindWidgetOptional))
	class UTextBlock* MissionDescriptionText;

	UPROPERTY(meta = (BindWidgetOptional))
	class UTextBlock* MissionSitrepText;
	UPROPERTY(meta = (BindWidgetOptional))
	class UTextBlock* MissionStartText;
	UPROPERTY(meta = (BindWidgetOptional))
	class UTextBlock* MissionStatusText;
	UPROPERTY(meta = (BindWidgetOptional))
	class UTextBlock* MissionTypeText;
	UPROPERTY(meta = (BindWidgetOptional))
	class UTextBlock* MissionSystemText;
	UPROPERTY(meta = (BindWidgetOptional))
	class UTextBlock* MissionRegionText;
	UPROPERTY(meta = (BindWidgetOptional))
	class UTextBlock* MissionObjectiveText;

	UPROPERTY(meta = (BindWidgetOptional))
	class UTextBlock* IntelNameText;
	UPROPERTY(meta = (BindWidgetOptional))
	class UTextBlock* IntelSourceText;
	UPROPERTY(meta = (BindWidgetOptional))
	class UTextBlock* IntelLocationText;
	UPROPERTY(meta = (BindWidgetOptional))
	class UTextBlock* IntelMessageText;
	UPROPERTY(meta = (BindWidgetOptional))
	class UTextBlock* IntelDateText;

	UPROPERTY(meta = (BindWidgetOptional))
	class UTextBlock* GroupInfoText;
	UPROPERTY(meta = (BindWidgetOptional))
	class UTextBlock* GroupLocationText;
	UPROPERTY(meta = (BindWidgetOptional))
	class UTextBlock* GroupTypeText;
	UPROPERTY(meta = (BindWidgetOptional))
	class UTextBlock* GroupEmpireText;
	UPROPERTY(meta = (BindWidgetOptional))
	class UTextBlock* GroupInformationText;
	
	UPROPERTY(meta = (BindWidgetOptional))
	class UTextBlock* OperationsModeText;
	
	UPROPERTY(meta = (BindWidgetOptional))
	class UTextBlock* PlayButtonText;
	UPROPERTY(meta = (BindWidgetOptional))
	class UTextBlock* CancelButtonText;
	UPROPERTY(meta = (BindWidgetOptional))
	class UTextBlock* GroupInfomationText;
	UPROPERTY(meta = (BindWidgetOptional))
	class UTextBlock* InformationLabel;
	UPROPERTY(meta = (BindWidgetOptional))
	class UTextBlock* CurrentUnitLabel;
	UPROPERTY(meta = (BindWidgetOptional))
	class UTextBlock* LocationLabel;
	UPROPERTY(meta = (BindWidgetOptional))
	class UButton* SelectButton;
	UPROPERTY(meta = (BindWidgetOptional))
	class UButton* CancelButton;

	UPROPERTY(meta = (BindWidgetOptional))
	class UImage* MissionImage;
	UPROPERTY(meta = (BindWidgetOptional))
	class UImage* IntelImage;
	UPROPERTY(meta = (BindWidgetOptional))
	class UImage* GroupImage;

	UPROPERTY(meta = (BindWidgetOptional))
	class UButton* OrdersButton;
	UPROPERTY(meta = (BindWidgetOptional))
	class UButton* TheaterButton;
	UPROPERTY(meta = (BindWidgetOptional))
	class UButton* ForcesButton;
	UPROPERTY(meta = (BindWidgetOptional))
	class UButton* IntelButton;
	UPROPERTY(meta = (BindWidgetOptional))
	class UButton* MissionsButton;

	UPROPERTY(meta = (BindWidgetOptional))
	class UButton* AudioButton;

	UPROPERTY(meta = (BindWidgetOptional))
	class UComboBoxString* EmpireSelectionDD;

	UPROPERTY(meta = (BindWidgetOptional))
	class UBorder* InformationBorder;
	UPROPERTY(meta = (BindWidgetOptional))
	class UCanvasPanel* InfoPanel;
	UPROPERTY(meta = (BindWidgetOptional))
	class UCanvasPanel* InfoBoxPanel;

	 // Canvas panel for placing icons
    UPROPERTY(meta = (BindWidgetOptional))
    class UCanvasPanel* MapCanvas;

	// Parent container to hold Forces
    UPROPERTY(meta = (BindWidgetOptional))
    UScrollBox* ForcesScrollBox;
		
	UPROPERTY(meta = (BindWidgetOptional))
	class UWidgetSwitcher* OperationalSwitcher;

	UPROPERTY(meta = (BindWidgetOptional))
	UListView* MissionList;

	UPROPERTY(meta = (BindWidgetOptional))
	UListView* IntelList;
	
	UPROPERTY(meta = (BindWidgetOptional))
	int SelectedMission;

	UPROPERTY()
	USoundBase* AudioAsset;

	UPROPERTY(meta = (BindWidgetOptional))
	USelectableButtonGroup* MenuToggleGroup;

	UPROPERTY(meta = (BindWidgetOptional))
	UPanelWidget* MenuButtonContainer;

public:
	UFUNCTION()
	void SetSelectedMissionData(int Selected);
	UFUNCTION()
	void SetSelectedIntelData(int Selected);
	UFUNCTION()
	void LoadForces(EEMPIRE_NAME Empire);
	UFUNCTION()
	void HandleUnitClicked();
	UFUNCTION()
	void HandleElementClicked();
	TArray<FSubGroupArray> GetSubGroupArrays(const FS_OOBFleet& Fleet);
	TArray<FSubGroupArray> GetBattalionSubGroups(const FS_OOBBattalion& Battalion);
	void FilterOutput(TArray<FS_OOBForce>& Forces, EEMPIRE_NAME Empire);

	void MatchCombatantGroups(EEMPIRE_NAME Empire, int32 SubId, ECOMBATGROUP_TYPE SubType, const TArray<FS_Combatant>& Combatants, TSet<FMatchedGroupKey>& MatchedIds); // now using FMatchedGroupKey
	UFUNCTION()
	void OnSetEmpireSelected(FString dropDownInt, ESelectInfo::Type type);
	UFUNCTION()
	void OnMenuToggleHovered(UMenuButton* SelectedButton);
	UFUNCTION()
	void OnMenuToggleSelected(UMenuButton* SelectedButton);
	UFUNCTION()
	void OnMenuButtonSelected(UMenuButton* SelectedButton);
	void PopulateEmpireDDList();

	void SetupMapIcons();

	UPROPERTY(meta = (BindWidgetOptional))
	UListView* ForceListView;

	UPROPERTY(EditAnywhere, Category = "UI")
	TSubclassOf<UMenuButton> MenuButtonClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FS_OOBForce> LoadedForces;

	// Star class icons (G, K, M, etc.)
    UPROPERTY()
    TMap<FString, UTexture2D*> StarTextures;
	
	// Scaling factor (optional)
    UPROPERTY()
    float MapScale = 60.0f;

protected:
	void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	UTexture2D* LoadTextureFromFile();
	FSlateBrush CreateBrushFromTexture(UTexture2D* Texture, FVector2D ImageSize);
	UFUNCTION()
	FString GetOrdinal(int id);
	UFUNCTION()
	void ClearForces();
	UFUNCTION()
	void GetIntelImageFile(FString IntelImageName);
	UFUNCTION()
	void GetIntelAudioFile(FString IntelAudioName);
	UFUNCTION()
	void GetMissionImageFile(int selected);
	UFUNCTION()
	void OnSelectButtonClicked();
	UFUNCTION()
	void OnSelectButtonHovered();

	UFUNCTION()
	void OnCancelButtonClicked();
	UFUNCTION()
	void OnCancelButtonHovered();
	
	UFUNCTION()
	void LoadForcesInfo();
	UFUNCTION()
	void LoadOrdersInfo();
	UFUNCTION()
	void LoadMissionsInfo();
	UFUNCTION()
	void LoadIntelInfo();
	UFUNCTION()
	void LoadTheaterInfo();
	
	// Store all buttons created
	UPROPERTY()
	TArray<UMenuButton*> AllMenuButtons;

	UFUNCTION()
	void OnAudioButtonClicked();
	UFUNCTION()
	void SetCampaignOrders();
	UFUNCTION()
	void SetCampaignMissions();
	UFUNCTION()
	void PopulateMissionList();
	UFUNCTION()
	void PopulateIntelList();

	UFUNCTION()
	FDateTime GetCampaignTime();

	   // Called to render the map
    UFUNCTION()
    void BuildGalaxyMap(const TArray<FS_Galaxy>& Systems);

    // TSubclassOf must be set in UMG (or via C++)
    UPROPERTY()
    TSubclassOf<USystemMarker> MarkerClass;

    

private:
	FS_Campaign ActiveCampaign;

	UPROPERTY()
	FString ImagePath;

	UPROPERTY()
	FString AudioPath;

	UPROPERTY()
	TArray<FS_CampaignAction> ActionList;

	UPROPERTY()
	TArray<FS_Combatant> CombatantList;
	
	UPROPERTY()
	TArray<FString> MenuItems = {
		TEXT("ORDERS"),
		TEXT("THEATER"),
		TEXT("FORCES"),
		TEXT("INTEL"),
		TEXT("MISSIONS")
	};

	TArray<FString> EmpireDDItems;
	EEMPIRE_NAME SelectedEmpire; 
	int IndentLevel = 0;
};
