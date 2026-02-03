/*  Project Starshatter Wars
    Fractal Dev Studios
    Copyright (C) 2025-2026. All Rights Reserved.

    SUBSYSTEM:    Stars.exe (UE)
    FILE:         FormListBox.h
    AUTHOR:       Carlos Bott

    ORIGINAL AUTHOR AND STUDIO
    ==========================
    John DiCamillo / Destroyer Studios LLC

    OVERVIEW
    ========
    UE port of legacy FormListBox control.
*/

#include "FormListBox.h"

#include "Widgets/Views/SListView.h"
#include "Widgets/Text/STextBlock.h"

UFormListBox::UFormListBox(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
}

TSharedRef<SWidget> UFormListBox::RebuildWidget()
{
    ListView =
        SNew(SListView<FItemPtr>)
        .ListItemsSource(&Items)
        .OnGenerateRow(this, &UFormListBox::GenerateRow)
        .OnSelectionChanged(this, &UFormListBox::HandleSelectionChanged);

    return ListView.ToSharedRef();
}

void UFormListBox::SynchronizeProperties()
{
    Super::SynchronizeProperties();

    if (ListView.IsValid())
    {
        ListView->RequestListRefresh();

        if (SelectedIndex >= 0 && SelectedIndex < Items.Num())
            ListView->SetSelection(Items[SelectedIndex]);
    }
}

TSharedRef<ITableRow> UFormListBox::GenerateRow(FItemPtr InItem, const TSharedRef<STableViewBase>& OwnerTable)
{
    return SNew(STableRow<FItemPtr>, OwnerTable)
        [
            SNew(STextBlock).Text(FText::FromString(InItem.IsValid() ? *InItem : TEXT("")))
        ];
}

void UFormListBox::HandleSelectionChanged(FItemPtr InItem, ESelectInfo::Type)
{
    SelectedIndex = INDEX_NONE;

    if (InItem.IsValid())
        SelectedIndex = Items.IndexOfByKey(InItem);

    OnSelectionChanged.Broadcast(SelectedIndex);
}

void UFormListBox::Clear()
{
    Items.Reset();
    ItemsText.Reset();
    SelectedIndex = INDEX_NONE;

    if (ListView.IsValid())
        ListView->RequestListRefresh();
}

void UFormListBox::AddItem(const FText& InText)
{
    ItemsText.Add(InText);
    Items.Add(MakeShared<FString>(InText.ToString()));

    if (ListView.IsValid())
        ListView->RequestListRefresh();
}

void UFormListBox::SetSelectedIndex(int32 InIndex)
{
    if (InIndex < 0 || InIndex >= Items.Num())
    {
        SelectedIndex = INDEX_NONE;
        if (ListView.IsValid())
            ListView->ClearSelection();
        OnSelectionChanged.Broadcast(SelectedIndex);
        return;
    }

    SelectedIndex = InIndex;

    if (ListView.IsValid())
        ListView->SetSelection(Items[SelectedIndex]);

    OnSelectionChanged.Broadcast(SelectedIndex);
}
