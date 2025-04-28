// /*  Project nGenEx	Fractal Dev Games	Copyright (C) 2024. All Rights Reserved.	SUBSYSTEM:    SSW	FILE:         Game.cpp	AUTHOR:       Carlos Bott*/

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "../Game/GameStructs.h" // FS_OOBFleet definition
#include "Blueprint/IUserObjectListEntry.h"
#include "OOBFleetWidget.generated.h"

class UTextBlock;
class UImage;
class UOOBBattleWidget;
class UOOBCarrierWidget;
class UOOBDesronWidget;
class UListView;
struct FS_OOBFleet;

/**
 * Fleet UI Widget - represents one Fleet in the Force Tree
 */

UCLASS()
class STARSHATTERWARS_API UOOBFleetWidget : public UUserWidget, public IUserObjectListEntry
{
	GENERATED_BODY()
	
public:

    UPROPERTY(meta = (BindWidgetOptional))
    UTextBlock* NameText;

    UPROPERTY(meta = (BindWidgetOptional))
    UImage* ExpandIcon;

    UPROPERTY(meta = (BindWidgetOptional))
    UListView* CarrierListView; // Carriers

    UPROPERTY(meta = (BindWidgetOptional))
    UListView* BattleListView; // Battle Groups

    UPROPERTY(meta = (BindWidgetOptional))
    UListView* DestroyerListView; // DESRONs

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
    void BuildChildren(const FS_OOBFleet& FleetDataStruct);
};