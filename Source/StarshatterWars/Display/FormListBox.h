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

#pragma once

#include "CoreMinimal.h"
#include "Components/Widget.h"
#include "FormListBox.generated.h"

class SListView;
template<typename ItemType> class STableRow;
class STextBlock;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FFormListSelectionChanged, int32, SelectedIndex);

UCLASS(BlueprintType, Blueprintable)
class STARSHATTERWARS_API UFormListBox : public UWidget
{
    GENERATED_BODY()

public:
    UFormListBox(const FObjectInitializer& ObjectInitializer);

    /** Legacy-like API */
    UFUNCTION(BlueprintCallable, Category = "Form|ListBox")
    void Clear();

    UFUNCTION(BlueprintCallable, Category = "Form|ListBox")
    void AddItem(const FText& InText);

    UFUNCTION(BlueprintCallable, Category = "Form|ListBox")
    int32 GetNumItems() const { return ItemsText.Num(); }

    UFUNCTION(BlueprintCallable, Category = "Form|ListBox")
    int32 GetSelectedIndex() const { return SelectedIndex; }

    UFUNCTION(BlueprintCallable, Category = "Form|ListBox")
    void SetSelectedIndex(int32 InIndex);

public:
    UPROPERTY(BlueprintAssignable, Category = "Form|ListBox")
    FFormListSelectionChanged OnSelectionChanged;

protected:
    // UWidget
    virtual TSharedRef<SWidget> RebuildWidget() override;
    virtual void SynchronizeProperties() override;

private:
    using FItemPtr = TSharedPtr<FString>;

    TSharedRef<ITableRow> GenerateRow(FItemPtr InItem, const TSharedRef<STableViewBase>& OwnerTable);
    void HandleSelectionChanged(FItemPtr InItem, ESelectInfo::Type SelectInfo);

private:
    TArray<FItemPtr> Items;
    TArray<FText>    ItemsText;

    TSharedPtr<SListView<FItemPtr>> ListView;

    int32 SelectedIndex = INDEX_NONE;
};
