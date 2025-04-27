// /*  Project nGenEx	Fractal Dev Games	Copyright (C) 2024. All Rights Reserved.	SUBSYSTEM:    SSW	FILE:         Game.cpp	AUTHOR:       Carlos Bott*/

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "../Game/GameStructs.h" // FS_OOBFleet definition
#include "OOBBattalionWidget.generated.h"

class UTextBlock;
class UImage;
struct FS_OOBBattalion;

/**
 * Battalion UI Widget - represents one Battalion in the Force Tree
 */

UCLASS()
class STARSHATTERWARS_API UOOBBattalionWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
    // The Battalion data this widget represents
    UPROPERTY()
    FS_OOBBattalion Data;

    // Whether the Fleet is currently expanded to show children
    UPROPERTY()
    bool bIsExpanded = false;

    // Child widgets: BattleGroups, Carriers, DesRons
    UPROPERTY()
    TArray<UUserWidget*> Children;

    // UI Elements
    UPROPERTY(meta = (BindWidgetOptional))
    UTextBlock* NameText; // Displays the Fleet name

    // UI: Expand/Collapse Icon
    UPROPERTY(meta = (BindWidget))
    UImage* ExpandIcon;

    // Optional icons for expanded/collapsed
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    UTexture2D* ExpandedIconTexture;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    UTexture2D* CollapsedIconTexture;

    // How deep this widget is in the tree (0 = Force, 1 = Fleet, 2 = Battle, etc.)
    UPROPERTY()
    int32 IndentLevel = 1; 

    // Sets up the Battalion widget with battalion data
    void SetData(const FS_OOBBattalion& InBattalion);

    // Builds child Batteries, Starbases, Stations if expanded
    void BuildChildren();

protected:
    virtual void NativeConstruct() override;
};

