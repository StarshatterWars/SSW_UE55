// /*  Project nGenEx	Fractal Dev Games	Copyright (C) 2024. All Rights Reserved.	SUBSYSTEM:    SSW	FILE:         Game.cpp	AUTHOR:       Carlos Bott*/

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "../Game/GameStructs.h" // FS_OOBForce definition
#include "Blueprint/IUserObjectListEntry.h"
#include "OOBForceWidget.generated.h"

class UTextBlock;
class UImage;
class UOOBForceItem; // The Data Object class holding FS_OOBForce
class UListView;
struct FS_OOBForce;
class UOOBForceItem;

/**
 * Force UI Widget - represents a Force at the top of the Order of Battle
 * Expands into a list of Fleets
 */

UCLASS()
class STARSHATTERWARS_API UOOBForceWidget : public UUserWidget, public IUserObjectListEntry
{
	GENERATED_BODY()
	
 public:
    // Bound UI elements
    UPROPERTY(meta = (BindWidgetOptional))
    UTextBlock* NameText;

    UPROPERTY(meta = (BindWidgetOptional))
    UImage* ExpandIcon;

    UPROPERTY(meta = (BindWidgetOptional))
    UListView* FleetListView; // List of Fleets inside Force

    // Expand/Collapse textures
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    UTexture2D* ExpandedIconTexture;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    UTexture2D* CollapsedIconTexture;

    // Expand/collapse state
    UPROPERTY()
    bool bIsExpanded = false;

    // Tree depth level
    UPROPERTY()
    int32 IndentLevel = 0;

public:

    virtual void NativeConstruct() override;

    // ListView binding override
    virtual void NativeOnListItemObjectSet(UObject* ListItemObject) override;
    
    virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

    // Toggle expansion of child ListView
    void ToggleExpansion();

    // Build Fleets under this Force
    void BuildChildren(const FS_OOBForce& ForceDataStruct);
};