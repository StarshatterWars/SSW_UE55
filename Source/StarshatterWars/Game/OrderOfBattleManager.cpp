// /*  Project nGenEx	Fractal Dev Games	Copyright (C) 2024. All Rights Reserved.	SUBSYSTEM:    SSW	FILE:         Game.cpp	AUTHOR:       Carlos Bott*/


#include "OrderOfBattleManager.h"
#include "../Screen/OrderOfBattleRowObject.h"
#include "Engine/Engine.h"

// Initializes the order of battle data
void UOrderOfBattleManager::InitializeOrderOfBattleData()
{
	// Optionally load data here, or initialize with mock data
	LoadOrderOfBattleData();
}

// Load Order of Battle data
void UOrderOfBattleManager::LoadOrderOfBattleData()
{
	// Example of hardcoding data for testing purposes
	UOrderOfBattleListEntry SampleEntry;
	SampleEntry.DisplayName = "Battalion Alpha";
	SampleEntry.bIsUnit = false;
	SampleEntry.EntryId = 1;

	AddEntry(SampleEntry);

	// Add more sample entries as needed
}

// Adds an entry to the list
void UOrderOfBattleManager::AddEntry(const UOrderOfBattleListEntry& Entry)
{
	UOrderOfBattleRowObject* NewEntry = NewObject<UOrderOfBattleRowObject>(this);
	NewEntry->DisplayName = Entry.DisplayName;
	NewEntry->bIsUnit = Entry.bIsUnit;

	OrderOfBattleItems.Add(NewEntry);
}

// Returns all order of battle items
const TArray<UOrderOfBattleRowObject*>& UOrderOfBattleManager::GetOrderOfBattleItems() const
{
	return OrderOfBattleItems;
}

// Find a specific entry by ID
UOrderOfBattleRowObject* UOrderOfBattleManager::GetOrderOfBattleEntryById(int32 Id)
{
	for (UOrderOfBattleRowObject* Item : OrderOfBattleItems)
	{
		if (Item->EntryId == Id)
		{
			return Item;
		}
	}
	return nullptr; // Return nullptr if not found
}