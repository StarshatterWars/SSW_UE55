// /*  Project nGenEx	Fractal Dev Games	Copyright (C) 2024. All Rights Reserved.	SUBSYSTEM:    SSW	FILE:         Game.cpp	AUTHOR:       Carlos Bott*/

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Blueprint/IUserObjectListEntry.h"
#include "../System/SSWGameInstance.h"
#include "../Game/GameStructs.h"
#include "CombatGroupObject.h"
#include "OrderOfBattleRowObject.h"
#include "OrderOfBattleListEntry.generated.h"

class UCombatGroupListItem;
class UOrderOfBattleWidget;
class UTextBlock;
class UButton;

/**
 * 
 */

UCLASS()
class STARSHATTERWARS_API UOrderOfBattleListEntry : public UUserWidget, public IUserObjectListEntry
{
	GENERATED_BODY()
	
public:
	virtual void NativeOnListItemObjectSet(UObject* ListItemObject) override;

protected:
	UPROPERTY(meta = (BindWidget))
	UTextBlock* NameText;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* TypeText;
};