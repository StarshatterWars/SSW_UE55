// /*  Project nGenEx	Fractal Dev Games	Copyright (C) 2024. All Rights Reserved.	SUBSYSTEM:    SSW	FILE:         Game.cpp	AUTHOR:       Carlos Bott*/


#include "OrderOfBattleListEntry.h"
#include "OrderOfBattleWidget.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "CombatGroupListItem.h"

void UOrderOfBattleListEntry::Setup(UCombatGroupListItem* InItem, UOrderOfBattleWidget* InParent)
{
	BoundItem = InItem;
	ParentWidget = InParent;

	if (GroupNameText)
	{
		FString Prefix = "";
		if (InItem->bHasChildren)
			Prefix = InItem->bIsExpanded ? "- " : "+ ";

		const FString Indent = FString::ChrN(InItem->IndentLevel * 4, ' ');
		GroupNameText->SetText(FText::FromString(Indent + Prefix + InItem->Group.Name));
	}

	if (EntryButton)
	{
		EntryButton->OnClicked.Clear();
		EntryButton->OnClicked.AddDynamic(this, &UOrderOfBattleListEntry::OnEntryClicked);
	}
}

void UOrderOfBattleListEntry::OnEntryClicked()
{
	if (BoundItem && ParentWidget)
	{
		if (BoundItem->bHasChildren)
		{
			BoundItem->bIsExpanded = !BoundItem->bIsExpanded;
			ParentWidget->BuildFlatTree(); // Refresh display
		}
		else
		{
			UE_LOG(LogTemp, Log, TEXT("Selected unit: %s"), *BoundItem->Group.Name);
		}
	}
}