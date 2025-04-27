// /*  Project nGenEx	Fractal Dev Games	Copyright (C) 2024. All Rights Reserved.	SUBSYSTEM:    SSW	FILE:         Game.cpp	AUTHOR:       Carlos Bott*/

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "../Game/GameStructs.h" // FS_OOBStation definition
#include "OOBStationWidget.generated.h"

class UTextBlock;
struct FS_OOBStation;

/**
 * Station UI Widget - represents one Station under a Battalion
 */

UCLASS()
class STARSHATTERWARS_API UOOBStationWidget : public UUserWidget
{
	GENERATED_BODY()
	
	public:

    // The Station data this widget represents
    UPROPERTY()
    FS_OOBStation 
    Data;

    // UI
    UPROPERTY(meta = (BindWidgetOptional))
    UTextBlock* Label; // Displays the Station name

    // Sets this widget's data
    void SetData(const FS_OOBStation& InStation);

protected:
    virtual void NativeConstruct() override;

};

