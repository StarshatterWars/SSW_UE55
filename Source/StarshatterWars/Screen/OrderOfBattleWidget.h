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
	virtual void NativeConstruct() override;

protected:
	UPROPERTY(meta = (BindWidget))
	UListView* OrderListView;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "OrderOfBattle")
	UOrderOfBattleManager* OrderOfBattleManager;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "OrderOfBattle")
	UDataTable* OrderOfBattleData;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "OrderOfBattle")
	TSubclassOf<UUserWidget> RowWidgetClass;
};