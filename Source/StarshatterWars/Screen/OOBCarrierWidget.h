// /*  Project nGenEx	Fractal Dev Games	Copyright (C) 2024. All Rights Reserved.	SUBSYSTEM:    SSW	FILE:         Game.cpp	AUTHOR:       Carlos Bott*/

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "../Game/GameStructs.h" // FS_OOBCarrier definition
#include "Blueprint/IUserObjectListEntry.h"
#include "OOBCarrierWidget.generated.h"

class UTextBlock;
class UImage;
class UListView;
struct FS_OOBCarrier;

/**
 * Carrier Group UI Widget - represents one Carrier Group inside a Fleet
 */

UCLASS()
class STARSHATTERWARS_API UOOBCarrierWidget : public UUserWidget, public IUserObjectListEntry
{
	GENERATED_BODY()

public:
    // The Carrier data this widget represents
    UPROPERTY()
    FS_OOBCarrier Data;

    UPROPERTY(meta = (BindWidgetOptional))
    UTextBlock* NameText;

    UPROPERTY(meta = (BindWidgetOptional))
    UImage* ExpandIcon;

    UPROPERTY(meta = (BindWidgetOptional))
    UListView* ElementListView; // Ships

    UPROPERTY(meta = (BindWidgetOptional))
    UListView* WingListView; // Wings

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    UTexture2D* ExpandedIconTexture;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    UTexture2D* CollapsedIconTexture;

    UPROPERTY()
    bool bIsExpanded = false;

 protected:
    virtual void NativeConstruct() override;
    virtual void NativeOnListItemObjectSet(UObject* ListItemObject) override;
    virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

    void ToggleExpansion();
    void BuildChildren(const FS_OOBCarrier& CarrierDataStruct);
};
	

