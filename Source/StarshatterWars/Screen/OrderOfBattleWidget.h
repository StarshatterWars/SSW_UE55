// /*  Project nGenEx	Fractal Dev Games	Copyright (C) 2024. All Rights Reserved.	SUBSYSTEM:    SSW	FILE:         Game.cpp	AUTHOR:       Carlos Bott*/

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "../Game/GameStructs.h"
#include "../Game/OrderOfBattleManager.h"
#include "../System/SSWGameInstance.h"
#include "OrderOfBattleRowObject.h"
#include "Components/ListView.h"
#include "OrderOfBattleWidget.generated.h"

/**
 * 
 */
UCLASS()
class STARSHATTERWARS_API UOrderOfBattleWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	// Native function called when the widget is constructed
	virtual void NativeConstruct() override;

	// Populate the ListView with the order of battle data
	UFUNCTION(BlueprintCallable)
	void PopulateListView();

protected:
	// The ListView widget to display entries
	UPROPERTY(meta = (BindWidget))
	UListView* OrderListView;

	// The RowWidget class to be used for each entry in the ListView
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	TSubclassOf<UUserWidget> RowWidgetClass;

	// The OrderOfBattleManager to handle the data
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data")
	UOrderOfBattleManager* OrderOfBattleManager;

	// Function to generate list items
	UFUNCTION()
	UUserWidget* OnGenerateRow(UObject* Item);
};