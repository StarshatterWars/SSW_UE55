// /*  Project nGenEx	Fractal Dev Games	Copyright (C) 2024. All Rights Reserved.	SUBSYSTEM:    SSW	FILE:         Game.cpp	AUTHOR:       Carlos Bott*/

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "OOBForceWidget.generated.h"

class UTextBlock;
class UImage;
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
    UTextBlock* NameText;// Shows Force name

    // UI: Expand/collapse icon (optional)
    UPROPERTY(meta = (BindWidget))
    UImage* ExpandIcon;

    // Optional icons to switch (plus/minus)
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    UTexture2D* ExpandedIconTexture;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    UTexture2D* CollapsedIconTexture;

    // How deep this widget is in the tree (0 = Force, 1 = Fleet, 2 = Battle, etc.)
    UPROPERTY()
    int32 IndentLevel = 0;  

    void SetData(UOOBForceItem* InForceObject);

    void BuildChildren();

protected:
    // Functions
    virtual void NativeConstruct() override;
};
	
