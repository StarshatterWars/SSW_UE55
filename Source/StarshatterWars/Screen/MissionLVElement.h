// /*  Project nGenEx	Fractal Dev Games	Copyright (C) 2024. All Rights Reserved.	SUBSYSTEM:    SSW	FILE:         Game.cpp	AUTHOR:       Carlos Bott*/

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "MissionListObject.h"
#include "Blueprint/IUserObjectListEntry.h"

#include "../System/SSWGameInstance.h"

#include "MissionLVElement.generated.h"

class UOperationsScreen;
/**
 * 
 */
UCLASS()
class STARSHATTERWARS_API UMissionLVElement : public UUserWidget, public IUserObjectListEntry
{
	GENERATED_BODY()
	
	UPROPERTY(meta = (BindWidgetOptional))
	class UTextBlock* MissionName;
	UPROPERTY(meta = (BindWidgetOptional))
	class UTextBlock* MissionType;
	UPROPERTY(meta = (BindWidgetOptional))
	class UTextBlock* MissionStatus;
	UPROPERTY(meta = (BindWidgetOptional))
	class UTextBlock* MissionTime;
	UPROPERTY(meta = (BindWidgetOptional))
	class UButton* MissionButton;
	
protected:
	void NativeConstruct() override;
	virtual void NativeOnListItemObjectSet(UObject* ListItemObject) override;

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission LV Variables")
	FString Name;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission LV Variables")
	FString Status;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission LV Variables")
	FString Time;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission LV Variables")
	FString Type;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission LV Variables")
	int32 MissionId;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission LV Variables")
	int32 selectedMission;
	
	UOperationsScreen* OpsScreen;

	UFUNCTION(BlueprintCallable)
	void SetMissionStatus();
	UFUNCTION()
	void OnMissionButtonClicked();

	UPROPERTY()
	UMissionListObject* MissionList;
};
