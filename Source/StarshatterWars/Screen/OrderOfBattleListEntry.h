// /*  Project nGenEx	Fractal Dev Games	Copyright (C) 2024. All Rights Reserved.	SUBSYSTEM:    SSW	FILE:         Game.cpp	AUTHOR:       Carlos Bott*/

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Blueprint/IUserObjectListEntry.h"
#include "../System/SSWGameInstance.h"
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
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "UI")
	UTextBlock* DisplayNameText;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Order of Battle")
	FString DisplayName;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Order of Battle")
	bool bIsUnit;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Order of Battle")
	int32 EntryId;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "UI")
	UOrderOfBattleRowObject* RowData;

	// Set the data for the entry widget
	void Setup(UOrderOfBattleRowObject* Data);

protected:
	virtual void NativeOnInitialized() override;
};