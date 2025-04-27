// /*  Project nGenEx	Fractal Dev Games	Copyright (C) 2024. All Rights Reserved.	SUBSYSTEM:    SSW	FILE:         Game.cpp	AUTHOR:       Carlos Bott*/

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "../Game/GameStructs.h" // FS_OOBFleet definition
#include "Components/ListView.h"
#include "Blueprint/IUserObjectListEntry.h"
#include "OOBFleetWidget.generated.h"

class UTextBlock;
class UImage;
class UOOBBattleWidget;
class UOOBCarrierWidget;
class UOOBDesronWidget;
struct FS_OOBFleet;

/**
 * Fleet UI Widget - represents one Fleet in the Force Tree
 */

UCLASS()
class STARSHATTERWARS_API UOOBFleetWidget : public UUserWidget, public IUserObjectListEntry
{
	GENERATED_BODY()
	
public:

    // The Fleet data this widget represents
    UPROPERTY()
    FS_OOBFleet Data;

    // Whether this Fleet is expanded (showing Battles, Carriers, DesRons)
    UPROPERTY()
    bool bIsExpanded = false;

    // Tree depth level (0 = Force, 1 = Fleet, etc.)
    UPROPERTY()
    int32 IndentLevel = 0;

    // Children widgets (Battles, Carriers, DesRons)
    UPROPERTY()
    TArray<UUserWidget*> Children;

    // UI: Text block showing the Fleet name
    UPROPERTY(meta = (BindWidget))
    UTextBlock* NameText;

    // UI: Expand/collapse icon (plus/minus)
    UPROPERTY(meta = (BindWidget))
    UImage* ExpandIcon;

    // UI: ListView to hold child Battles, Carriers, DesRons
    UPROPERTY(meta = (BindWidget))
    UListView* ChildListView;

    // Optional textures for expanded/collapsed
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    UTexture2D* ExpandedIconTexture;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    UTexture2D* CollapsedIconTexture;

    // Set the Fleet data and tree indentation level
    void SetFleetData(const FS_OOBFleet& InFleet, int32 InIndentLevel);

    // Build children dynamically: Battles, Carriers, DesRons
    void BuildChildren();

    // Toggle expand/collapse state
    void ToggleExpansion();

public:
    virtual void NativeConstruct() override;
    virtual void NativeOnListItemObjectSet(UObject* ListItemObject) override;

};