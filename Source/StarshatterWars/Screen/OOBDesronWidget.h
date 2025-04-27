// /*  Project nGenEx	Fractal Dev Games	Copyright (C) 2024. All Rights Reserved.	SUBSYSTEM:    SSW	FILE:         Game.cpp	AUTHOR:       Carlos Bott*/

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "../Game/GameStructs.h" // FS_OOBDesron definition
#include "OOBDesronWidget.generated.h"

class UTextBlock;
struct FS_OOBDestroyer;

/**
 * DESRON (Destroyer Squadron) UI Widget - represents one DesRon inside a Fleet
 */

UCLASS()
class STARSHATTERWARS_API UOOBDesronWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
    // The DesRon data this widget represents
    UPROPERTY()
    FS_OOBDestroyer Data;

    // UI
    UPROPERTY(meta = (BindWidgetOptional))
    UTextBlock* Label; // Displays the DesRon name

    // Sets up this widget with DesRon data
    void SetData(const FS_OOBDestroyer& InDesron);

protected:
    virtual void NativeConstruct() override;
};
	

