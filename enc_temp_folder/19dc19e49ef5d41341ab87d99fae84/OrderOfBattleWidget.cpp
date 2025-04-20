// /*  Project nGenEx	Fractal Dev Games	Copyright (C) 2024. All Rights Reserved.	SUBSYSTEM:    SSW	FILE:         Game.cpp	AUTHOR:       Carlos Bott*/


#include "OrderOfBattleWidget.h"
#include "Components/ListView.h"
#include "Blueprint/UserWidget.h"
#include "OrderOfBattleRowWidget.h"
#include "OrderOfBattleRowObject.h"
#include "../Game/OrderOfBattleManager.h"

void UOrderOfBattleWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (!OrderListView || !OrderOfBattleManager || !OrderOfBattleData)
	{
		UE_LOG(LogTemp, Error, TEXT("OrderOfBattleWidget: Missing reference(s)."));
		return;
	}

	// Load data into the manager
	OrderOfBattleManager->InitializeFromDataTable(OrderOfBattleData);

	// Clear and populate the list
	OrderListView->ClearListItems();

	const TArray<UCombatGroupObject*>& CombatGroups = OrderOfBattleManager->GetCombatGroups();
	for (UCombatGroupObject* Group : CombatGroups)
	{
		OrderListView->AddItem(Group);
	}
}