// /*  Project nGenEx	Fractal Dev Games	Copyright (C) 2024. All Rights Reserved.	SUBSYSTEM:    SSW	FILE:         Game.cpp	AUTHOR:       Carlos Bott*/

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "../Game/GameStructs.h" // FS_OOBBattery definition
#include "Blueprint/IUserObjectListEntry.h"
#include "OOBAttackSquadronWidget.generated.h"

class UTextBlock;
class UImage;
class UListView;
struct FS_OOBAttack;
/**
 * 
 */
UCLASS()
class STARSHATTERWARS_API UOOBAttackSquadronWidget : public UUserWidget, public IUserObjectListEntry
{
	GENERATED_BODY()
public:
    // The Carrier data this widget represents
    UPROPERTY()
    FS_OOBAttack Data;

    UPROPERTY(meta = (BindWidgetOptional))
    UTextBlock* NameText;

    UPROPERTY(meta = (BindWidgetOptional))
    UImage* ExpandIcon;

    UPROPERTY(meta = (BindWidgetOptional))
    UListView* ElementListView; // Ships

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    UTexture2D* ExpandedIconTexture;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    UTexture2D* CollapsedIconTexture;

    UPROPERTY()
    bool bIsExpanded = false;

 protected:
    virtual void NativeConstruct() override;
    virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
    virtual void NativeOnListItemObjectSet(UObject* ListItemObject) override;
    virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

    void ToggleExpansion();
    void BuildChildren(const FS_OOBAttack& AttackDataStruct);
    void ShowUnitData();
    void SetHighlight(bool bHighlighted);
};