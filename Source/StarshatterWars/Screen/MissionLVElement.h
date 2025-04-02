// /*  Project nGenEx	Fractal Dev Games	Copyright (C) 2024. All Rights Reserved.	SUBSYSTEM:    SSW	FILE:         Game.cpp	AUTHOR:       Carlos Bott*/

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "MissionListObject.h"
#include "../System/SSWGameInstance.h"

#include "MissionLVElement.generated.h"

/**
 * 
 */
UCLASS()
class STARSHATTERWARS_API UMissionLVElement : public UUserWidget
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
	//void NativeOnListItemObjectSet(UObject* ListItemObject);

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
	
	
	UFUNCTION(BlueprintCallable)
	void SetMissionStatus();
	UFUNCTION()
	void OnMissionButtonClicked();
};
