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

/**
 * 
 */
UCLASS()
class STARSHATTERWARS_API URosterTVElement : public UUserWidget, public IUserObjectListEntry
{
	GENERATED_BODY()
	
	UPROPERTY(meta = (BindWidgetOptional))
	class UTextBlock* RosterNameText;
	UPROPERTY(meta = (BindWidgetOptional))
	class UTextBlock* RosterTypeText;
	UPROPERTY(meta = (BindWidgetOptional))
	class UTextBlock* RosterLocationText;

	protected:
    UPROPERTY(meta = (BindWidgetOptional))
    class UTextBlock* NameText;  // TextBlock for displaying the name of the item

	UOperationsScreen* OpsScreen;
	
protected:
	virtual void NativeOnListItemObjectSet(UObject* ListItemObject) override;

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Roster View Variables")
	FString Name;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Roster View Variables")
	FString Type;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Roster View Variables")
	FString Location;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Roster View Variables")
	int32 RosterId;

	UPROPERTY()
	URosterViewObject* RosterView;
	
};
