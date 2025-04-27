// /*  Project nGenEx	Fractal Dev Games	Copyright (C) 2024. All Rights Reserved.	SUBSYSTEM:    SSW	FILE:         Game.cpp	AUTHOR:       Carlos Bott*/

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "../Game/GameStructs.h" // FS_OOBStarbase definition
#include "OOBStarbaseWidget.generated.h"

class UTextBlock;
struct FS_OOBStarbase;

/**
 * Starbase UI Widget - represents one Starbase under a Battalion
 */

UCLASS()
class STARSHATTERWARS_API UOOBStarbaseWidget : public UUserWidget
{
	GENERATED_BODY()

public:

    // The Starbase data this widget represents
    UPROPERTY()
    FS_OOBStarbase Data;

    // UI
    UPROPERTY(meta = (BindWidgetOptional))
    UTextBlock* Label; // Displays the Starbase name

    // Sets this widget's data
    void SetData(const FS_OOBStarbase& InStarbase);

protected:
    virtual void NativeConstruct() override;
};
