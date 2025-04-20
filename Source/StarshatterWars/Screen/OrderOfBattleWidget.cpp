// /*  Project nGenEx	Fractal Dev Games	Copyright (C) 2024. All Rights Reserved.	SUBSYSTEM:    SSW	FILE:         Game.cpp	AUTHOR:       Carlos Bott*/


#include "OrderOfBattleWidget.h"
#include "Components/ListView.h"
#include "OrderOfBattleListEntry.h"
#include "CombatGroupListItem.h"
#include "../Game/OrderOfBattleManager.h"

UOrderOfBattleWidget::UOrderOfBattleWidget(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    // Ensure ListEntryWidgetClass is assigned from the UMG Blueprint
    static ConstructorHelpers::FClassFinder<UOrderOfBattleListEntry> ListEntryClass(TEXT("/Game/UI/Widgets/OrderOfBattleListEntry"));
    if (ListEntryClass.Succeeded())
    {
        ListEntryWidgetClass = ListEntryClass.Class;
    }
}

void UOrderOfBattleWidget::NativeConstruct()
{
    Super::NativeConstruct();

    // Bind OnGenerateRow delegate
    if (CombatGroupListView)
    {
        //CombatGroupListView->GenerateRow().BindUFunction(this, TEXT("OnGenerateRow"));
    }

    // Initialize from the data table
    if (CombatGroupTable)
    {
        InitializeFromTable(CombatGroupTable);
    }
}

void UOrderOfBattleWidget::InitializeFromTable(UDataTable* CGroupTable)
{
    if (!CGroupTable || !CombatGroupListView || !ListEntryWidgetClass) return;

    // Initialize the manager
    OrderOfBattleManager = NewObject<UOrderOfBattleManager>(this);
    OrderOfBattleManager->Initialize(CGroupTable);

    // Build the flat tree for ListView
    BuildFlatTree();
}

UWidget* UOrderOfBattleWidget::OnGenerateRow(UObject* Item, UListView* OwningList)
{
    if (!Item || !ListEntryWidgetClass) return nullptr;

    // Create the Entry widget for the row using CreateWidget
    UOrderOfBattleListEntry* EntryWidget = CreateWidget<UOrderOfBattleListEntry>(this, ListEntryWidgetClass);
    if (EntryWidget)
    {
        UCombatGroupListItem* ListItem = Cast<UCombatGroupListItem>(Item);
        EntryWidget->Setup(ListItem, this);
    }
    return EntryWidget;
}

void UOrderOfBattleWidget::BuildFlatTree()
{
    FlatTree.Empty();
    CombatGroupListView->ClearListItems();

    // Get the root group IDs and add them recursively
    TArray<int32> RootIds = OrderOfBattleManager->GetRootGroupIds();
    for (int32 RootId : RootIds)
    {
        AddVisibleItemsRecursive(RootId, 0);
    }

    // Add all the groups (items) to the ListView
    for (UCombatGroupListItem* Item : FlatTree)
    {
        CombatGroupListView->AddItem(Item);
    }
}

void UOrderOfBattleWidget::AddVisibleItemsRecursive(int32 GroupId, int32 Indent)
{
    const FS_CombatGroup* Group = OrderOfBattleManager->GetGroupById(GroupId);
    if (!Group) return;

    // Create a new ListItem to wrap the group
    UCombatGroupListItem* ListItem = NewObject<UCombatGroupListItem>(this);
    ListItem->Group = *Group;
    ListItem->IndentLevel = Indent;

    // Check if this group has children
    TArray<int32> Children = OrderOfBattleManager->GetChildrenOfGroup(GroupId);
    ListItem->bHasChildren = Children.Num() > 0;

    // Add the current group to the FlatTree array
    FlatTree.Add(ListItem);

    // Recursively add child groups if expanded
    if (ListItem->bIsExpanded)
    {
        for (int32 ChildId : Children)
        {
            AddVisibleItemsRecursive(ChildId, Indent + 1);
        }
    }
}