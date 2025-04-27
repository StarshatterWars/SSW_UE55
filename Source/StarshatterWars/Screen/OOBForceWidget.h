// /*  Project nGenEx	Fractal Dev Games	Copyright (C) 2024. All Rights Reserved.	SUBSYSTEM:    SSW	FILE:         Game.cpp	AUTHOR:       Carlos Bott*/

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "../Game/GameStructs.h" // FS_OOBForce definition
#include "OOBForceWidget.generated.h"

class UTextBlock;
class UImage;
class UOOBForceItem; // The Data Object class holding FS_OOBForce
class UListView;
struct FS_OOBForce;

/**
 * Force UI Widget - represents a Force at the top of the Order of Battle
 * Expands into a list of Fleets
 */

UCLASS()
class STARSHATTERWARS_API UOOBForceWidget : public UUserWidget
{
	GENERATED_BODY()
	
 public:

    // The Force data this widget represents
    UPROPERTY()
    FS_OOBForce ForceData;

    // Whether this Force is expanded (showing Fleets)
    UPROPERTY()
    bool bIsExpanded = false;

    // Tree indent level (usually 0 for Forces)
    UPROPERTY()
    int32 IndentLevel = 0;

    // List of child fleet widgets (optional caching)
    UPROPERTY()
    TArray<UUserWidget*> Children;

    // UI: Text block showing the Force name
    UPROPERTY(meta = (BindWidget))
    UTextBlock* NameText;

    // UI: Expand/collapse icon (plus/minus)
    UPROPERTY(meta = (BindWidget))
    UImage* ExpandIcon;

    // UI: ListView holding child Fleets
    UPROPERTY(meta = (BindWidget))
    UListView* FleetListView;

    // Icons for expand/collapse states
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    UTexture2D* ExpandedIconTexture;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    UTexture2D* CollapsedIconTexture;

    // Set the Force data and indent level
    void SetForceData(const FS_OOBForce& InForce, int32 InIndentLevel);

    // Dynamically create child fleet widgets
    void BuildChildren();

    // Toggle expand/collapse state
    void ToggleExpansion();

protected:
    virtual void NativeConstruct() override;

};
	
