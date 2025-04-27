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
    UTextBlock* NameText; // Displays the Battery name

    // How deep this widget is in the tree (0 = Force, 1 = Fleet, 2 = Battle, etc.)
    UPROPERTY()
    int32 IndentLevel = 2; 

    // Sets up this widget with battery data
    void SetData(const FS_OOBBattery& InBattery, int32 InIndentLevel);

protected:
    virtual void NativeConstruct() override;  
};
	
	

