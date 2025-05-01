// /*  Project nGenEx	Fractal Dev Games	Copyright (C) 2024. All Rights Reserved.	SUBSYSTEM:    SSW	FILE:         Game.cpp	AUTHOR:       Carlos Bott*/

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "../Game/GameStructs.h" // FS_OOBFleet definition
#include "Blueprint/IUserObjectListEntry.h"
#include "OOBBattalionWidget.generated.h"

class UTextBlock;
class UImage;
class UListView;
struct FS_OOBBattalion;

/**
 * Battalion UI Widget - represents one Battalion in the Force Tree
 */

UCLASS()
class STARSHATTERWARS_API UOOBBattalionWidget : public UUserWidget, public IUserObjectListEntry
{
	GENERATED_BODY()
	
public:
    // The Carrier data this widget represents
    UPROPERTY()
    FS_OOBBattalion Data;

    UPROPERTY(meta = (BindWidgetOptional))
    UTextBlock* NameText;

    UPROPERTY(meta = (BindWidgetOptional))
    UImage* ExpandIcon;

    UPROPERTY(meta = (BindWidgetOptional))
    UListView* StarbaseListView; // Carriers

    UPROPERTY(meta = (BindWidgetOptional))
    UListView* StationListView; // Battle Groups

    UPROPERTY(meta = (BindWidgetOptional))
    UListView* BatteryListView; // DESRONs

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    UTexture2D* ExpandedIconTexture;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    UTexture2D* CollapsedIconTexture;

    UPROPERTY()
    bool bIsExpanded = false;

public:

    virtual void NativeConstruct() override;
    virtual void NativeOnListItemObjectSet(UObject* ListItemObject) override;
    virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

    void ToggleExpansion();
    void BuildChildren(const FS_OOBBattalion& BattalionDataStruct);
};