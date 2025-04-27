// /*  Project nGenEx	Fractal Dev Games	Copyright (C) 2024. All Rights Reserved.	SUBSYSTEM:    SSW	FILE:         Game.cpp	AUTHOR:       Carlos Bott*/

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "../Game/GameStructs.h" // FS_OOBFleet definition
#include "OOBBattalionWidget.generated.h"

class UTextBlock;
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

    // Whether the Battalion is currently expanded to show children
    UPROPERTY()
    bool bIsExpanded = false;

    // Child widgets: Batteries, Starbases, Stations
    UPROPERTY()
    TArray<UUserWidget*> Children;

    // UI Elements
    UPROPERTY(meta = (BindWidgetOptional))
    UTextBlock* Label; // Displays the Battalion name

    // Sets up the Battalion widget with battalion data
    void SetData(const FS_OOBBattalion& InBattalion);

    // Builds child Batteries, Starbases, Stations if expanded
    void BuildChildren();

protected:
    virtual void NativeConstruct() override;
};

