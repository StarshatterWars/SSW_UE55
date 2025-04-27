// /*  Project nGenEx	Fractal Dev Games	Copyright (C) 2024. All Rights Reserved.	SUBSYSTEM:    SSW	FILE:         Game.cpp	AUTHOR:       Carlos Bott*/

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "OOBForceWidget.generated.h"

class UTextBlock;
class UOOBForceItem; // The Data Object class holding FS_OOBForce

/**
 * 
 */
UCLASS()
class STARSHATTERWARS_API UOOBForceWidget : public UUserWidget
{
	GENERATED_BODY()
	
 public:
    // Pointer to Data Object
    UPROPERTY()
    UOOBForceItem* Object;

    // Is this Force expanded in the ListView?
    UPROPERTY()
    bool bIsExpanded = false;

    // Children widgets (Fleets + Battalions)
    UPROPERTY()
    TArray<UUserWidget*> Children;

    // UI Elements
    UPROPERTY(meta = (BindWidgetOptional))
    UTextBlock* Label;// Shows Force name


    void SetData(UOOBForceItem* InForceObject);

    void BuildChildren();

protected:
    // Functions
    virtual void NativeConstruct() override;
};
	
