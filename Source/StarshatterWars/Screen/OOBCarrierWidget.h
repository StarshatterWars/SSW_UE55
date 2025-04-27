// /*  Project nGenEx	Fractal Dev Games	Copyright (C) 2024. All Rights Reserved.	SUBSYSTEM:    SSW	FILE:         Game.cpp	AUTHOR:       Carlos Bott*/

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "../Game/GameStructs.h" // FS_OOBCarrier definition
#include "OOBCarrierWidget.generated.h"

class UTextBlock;
struct FS_OOBCarrier;

/**
 * Carrier Group UI Widget - represents one Carrier Group inside a Fleet
 */

UCLASS()
class STARSHATTERWARS_API UOOBCarrierWidget : public UUserWidget
{
	GENERATED_BODY()
public:

    // The Carrier data this widget represents
    UPROPERTY()
    FS_OOBCarrier Data;

    // UI
    UPROPERTY(meta = (BindWidgetOptional))
    UTextBlock* Label; // Displays the Carrier Group name

    // Sets this widget's data
    void SetData(const FS_OOBCarrier& InCarrier);

protected:
    virtual void NativeConstruct() override;
};
	

