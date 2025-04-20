// /*  Project nGenEx	Fractal Dev Games	Copyright (C) 2024. All Rights Reserved.	SUBSYSTEM:    SSW	FILE:         Game.cpp	AUTHOR:       Carlos Bott*/

#pragma once

#include "CoreMinimal.h"
#include "Components/ListView.h"
#include "OrderOfBattleRowObject.h"
#include "OrderOfBattleListView.generated.h"

/**
 * 
 */
UCLASS()
class STARSHATTERWARS_API UOrderOfBattleListView : public UListView
{
	GENERATED_BODY()

public:
	// Override the OnGenerateRow method to create a custom row widget
	//virtual UUserWidget* OnGenerateRow(UObject* Item, UUserWidget* OwnerWidget) override;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	TSubclassOf<UUserWidget> RowWidgetClass;
};
