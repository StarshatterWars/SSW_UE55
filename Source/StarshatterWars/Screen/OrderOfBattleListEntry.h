// /*  Project nGenEx	Fractal Dev Games	Copyright (C) 2024. All Rights Reserved.	SUBSYSTEM:    SSW	FILE:         Game.cpp	AUTHOR:       Carlos Bott*/

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Blueprint/IUserObjectListEntry.h"
#include "../System/SSWGameInstance.h"
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
	// Setup function to initialize this entry with data
	UFUNCTION(BlueprintCallable)
	void Setup(UCombatGroupListItem* InItem, UOrderOfBattleWidget* InParentWidget);

	UOrderOfBattleWidget* ParentWidget;

protected:
	UPROPERTY(meta = (BindWidget))
	UTextBlock* GroupNameText;

	UPROPERTY(meta = (BindWidget))
	UButton* EntryButton;

	UPROPERTY()
	UCombatGroupListItem* BoundItem;

	UFUNCTION()
	void OnEntryClicked();
};
	

