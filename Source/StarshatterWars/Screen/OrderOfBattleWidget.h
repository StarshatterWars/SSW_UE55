// /*  Project nGenEx	Fractal Dev Games	Copyright (C) 2024. All Rights Reserved.	SUBSYSTEM:    SSW	FILE:         Game.cpp	AUTHOR:       Carlos Bott*/

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "../Game/GameStructs.h"
#include "../Game/OrderOfBattleManager.h"
#include "../System/SSWGameInstance.h"
#include "CombatGroupListItem.h"
#include "Templates/SharedPointer.h"
#include "OrderOfBattleWidget.generated.h"


class UListView;
/**
 * 
 */
UCLASS()
class STARSHATTERWARS_API UOrderOfBattleWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
    UOrderOfBattleWidget(const FObjectInitializer& ObjectInitializer);

    /** Initialize the widget from a data table */
    void InitializeFromTable(UDataTable* CGroupTable);

    /** Populate the ListView with data */
    void BuildFlatTree();

protected:
    virtual void NativeConstruct() override;

    /** Handle row generation for ListView */
    UFUNCTION()
    UWidget* OnGenerateRow(UObject* Item, UListView* OwningList);

    /** The ListView that will display our combat groups */
    UPROPERTY(meta = (BindWidget))
    UListView* CombatGroupListView;

private:
   
    /** The widget class for each row */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI", meta = (AllowPrivateAccess = "true"))
    TSubclassOf<class UOrderOfBattleListEntry> ListEntryWidgetClass;

    /** Manager for the order of battle data */
    class UOrderOfBattleManager* OrderOfBattleManager;

    /** Array to store the flat list of combat groups (flattened for ListView) */
    TArray<UCombatGroupListItem*> FlatTree;

    /** Recursively add visible items to the list */
    void AddVisibleItemsRecursive(int32 GroupId, int32 Indent);

     /** Combat Group Data Table */
    UPROPERTY()
    UDataTable* CombatGroupTable;
};