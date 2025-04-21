// /*  Project nGenEx	Fractal Dev Games	Copyright (C) 2024. All Rights Reserved.	SUBSYSTEM:    SSW	FILE:         Game.cpp	AUTHOR:       Carlos Bott*/

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "RosterViewObject.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Components/HorizontalBoxSlot.h"
#include "Blueprint/IUserObjectListEntry.h"
#include "../System/SSWGameInstance.h"

#include "CombatGroupObject.h"

#include "RosterTVElement.generated.h"

class UOperationsScreen;
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
	UPROPERTY(meta = (BindWidgetOptional))
	class UButton* RosterButton;

	UPROPERTY(meta = (BindWidgetOptional))
	class UHorizontalBoxSlot* IndentSlot;
	
	UOperationsScreen* OpsScreen;
	
protected:
	void NativeConstruct() override;
	virtual void NativeOnListItemObjectSet(UObject* ListItemObject) override;

private:
	UFUNCTION()
	void OnRosterButtonClicked();
	UFUNCTION()
	void SetRosterInfo();

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
