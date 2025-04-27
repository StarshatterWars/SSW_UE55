// /*  Project nGenEx	Fractal Dev Games	Copyright (C) 2024. All Rights Reserved.	SUBSYSTEM:    SSW	FILE:         Game.cpp	AUTHOR:       Carlos Bott*/

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/TextBlock.h"
#include "../Game/GameStructs.h" // FS_OOBFleet definition
#include "OOBFleetWidget.generated.h"

class UTextBlock;
struct FS_OOBFleet;

/**
 * Fleet UI Widget - represents one Fleet in the Force Tree
 */

UCLASS()
class STARSHATTERWARS_API UOOBFleetWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
    // The Fleet data this widget represents
    UPROPERTY()
    FS_OOBFleet Data;

    // Whether the Fleet is currently expanded to show children
    UPROPERTY()
    bool bIsExpanded = false;

    // Child widgets: BattleGroups, Carriers, DesRons
    UPROPERTY()
    TArray<UUserWidget*> Children;

    // UI Elements
    UPROPERTY(meta = (BindWidgetOptional))
    UTextBlock* Label; // Displays the Fleet name

    // Sets up the Fleet widget with fleet data
    void SetData(const FS_OOBFleet& InFleet);

    // Builds child BattleGroups, Carriers, DesRons if expanded
    void BuildChildren();

protected:
    virtual void NativeConstruct() override;
};

