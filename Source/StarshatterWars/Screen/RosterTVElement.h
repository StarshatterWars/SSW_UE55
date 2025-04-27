// /*  Project nGenEx	Fractal Dev Games	Copyright (C) 2024. All Rights Reserved.	SUBSYSTEM:    SSW	FILE:         Game.cpp	AUTHOR:       Carlos Bott*/

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "RosterViewObject.h"

#include "Blueprint/IUserObjectListEntry.h"
#include "../System/SSWGameInstance.h"

#include "CombatGroupObject.h"

#include "RosterTVElement.generated.h"

class UOperationsScreen;
class UTextBlock;
class UButton;

class UObject;

class UOOBForceItem;
class UOOBFleetItem;
class UOOBCarrierGroupItem;
class UOOBBattleItem;
class UOOBDestroyerItem;
class UOOBWingItem;
class UOOBUnitItem;
class UOOBSquadronItem;
class UOOBFighterSquadronItem;
class UOOBFighterUnit;
class UOOBBattalion;
class UOOBCivilianItem;
class UOOBBatteryItem;

/**
 * 
 */
UCLASS()
class STARSHATTERWARS_API URosterTVElement : public UUserWidget, public IUserObjectListEntry
{
	GENERATED_BODY()
	
protected:
    UPROPERTY(meta = (BindWidgetOptional))
    class UTextBlock* NameText;  // TextBlock for displaying the name of the item

	UOperationsScreen* OpsScreen;
	
protected:
	virtual void NativeOnListItemObjectSet(UObject* ListItemObject) override;

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Roster View Variables")
	FString Name;

};
