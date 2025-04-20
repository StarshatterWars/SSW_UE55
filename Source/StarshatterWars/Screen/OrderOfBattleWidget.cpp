// /*  Project nGenEx	Fractal Dev Games	Copyright (C) 2024. All Rights Reserved.	SUBSYSTEM:    SSW	FILE:         Game.cpp	AUTHOR:       Carlos Bott*/


#include "OrderOfBattleWidget.h"
#include "Components/ListView.h"
#include "Blueprint/UserWidget.h"
#include "OrderOfBattleRowWidget.h"
#include "OrderOfBattleRowObject.h"
#include "../Game/OrderOfBattleManager.h"

// Called when the widget is constructed
void UOrderOfBattleWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// Ensure the OrderOfBattleManager is available
	if (OrderOfBattleManager)
	{
		// Populate the ListView with order of battle entries
		PopulateListView();
	}
}

// Populate the ListView with data from the OrderOfBattleManager
void UOrderOfBattleWidget::PopulateListView()
{
	if (!OrderListView) return;

	// Get the list of order of battle items from the manager
	TArray<UOrderOfBattleRowObject*> Entries = OrderOfBattleManager->GetOrderOfBattleItems();

	// Clear any previous entries in the ListView
	OrderListView->ClearListItems();

	// Add each entry to the ListView
	for (UOrderOfBattleRowObject* Entry : Entries)
	{
		OrderListView->AddItem(Entry);
	}
}

// Generate row widget for each item in the ListView
UUserWidget* UOrderOfBattleWidget::OnGenerateRow(UObject* Item)
{
	// Cast the passed item to a UOrderOfBattleRowObject
	UOrderOfBattleRowObject* RowData = Cast<UOrderOfBattleRowObject>(Item);
	if (!RowData) return nullptr;

	// Create a widget for this row using the RowWidgetClass
	UUserWidget* RowWidget = CreateWidget<UUserWidget>(this, RowWidgetClass);
	if (RowWidget)
	{
		// Optionally, you can set up the widget with data from RowData
		// For example, you could set the text of a TextBlock inside the RowWidget
	}

	return RowWidget;
}