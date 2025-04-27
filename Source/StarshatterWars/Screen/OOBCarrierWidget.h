// /*  Project nGenEx	Fractal Dev Games	Copyright (C) 2024. All Rights Reserved.	SUBSYSTEM:    SSW	FILE:         Game.cpp	AUTHOR:       Carlos Bott*/

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "../Game/GameStructs.h" // FS_OOBCarrier definition
#include "OOBCarrierWidget.generated.h"

class UTextBlock;
class UImage;
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

    UPROPERTY()
    bool bIsExpanded = false;

    UPROPERTY()
    int32 IndentLevel = 0;

    UPROPERTY()
    TArray<UUserWidget*> Children;

    UPROPERTY(meta = (BindWidget))
    UTextBlock* NameText;

    UPROPERTY(meta = (BindWidget))
    UImage* ExpandIcon;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    UTexture2D* ExpandedIconTexture;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    UTexture2D* CollapsedIconTexture;

    // Sets this widget's data
    void SetData(const FS_OOBCarrier& InCarrier, int32 InIndentLevel);

    void BuildChildren(); // Build units under Carrier

protected:
    virtual void NativeConstruct() override;
};
	

