// /*  Project nGenEx	Fractal Dev Games	Copyright (C) 2024. All Rights Reserved.	SUBSYSTEM:    SSW	FILE:         Game.cpp	AUTHOR:       Carlos Bott*/

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "../Game/GameStructs.h" // FS_OOBBattery definition
#include "OOBBatteryWidget.generated.h"

class UTextBlock;
struct FS_OOBBattery;

/**
 * Battery UI Widget - represents one Battery under a Battalion
 */

UCLASS()
class STARSHATTERWARS_API UOOBBatteryWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
    // The Battery data this widget represents
    UPROPERTY()
    FS_OOBBattery Data;

    // UI
    UPROPERTY(meta = (BindWidgetOptional))
    UTextBlock* Label; // Displays the Battery name

    // Sets up this widget with battery data
    void SetData(const FS_OOBBattery& InBattery);

protected:
    virtual void NativeConstruct() override;  
};
	
	

