// /*  Project nGenEx	Fractal Dev Games	Copyright (C) 2024. All Rights Reserved.	SUBSYSTEM:    SSW	FILE:         Game.cpp	AUTHOR:       Carlos Bott*/

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "../Game/GameStructs.h" // FS_OOBWing definition
#include "Blueprint/IUserObjectListEntry.h"
#include "OOBWingWidget.generated.h"

class UTextBlock;
class UImage;
struct FS_OOBWing;

/**
 * Wing UI Widget - represents a Wing under a Carrier, expandable into Squadrons
 */

UCLASS()
class STARSHATTERWARS_API UOOBWingWidget : public UUserWidget, public IUserObjectListEntry
{
	GENERATED_BODY()

public:

    UPROPERTY()
    FS_OOBWing Data;

    UPROPERTY()
    bool bIsExpanded = false;

    UPROPERTY()
    int32 IndentLevel = 0;

    UPROPERTY()
    TArray<UUserWidget*> Children;

    UPROPERTY(meta = (BindWidgetOptional))
    UTextBlock* NameText;

    UPROPERTY(meta = (BindWidgetOptional))
    UImage* ExpandIcon;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    UTexture2D* ExpandedIconTexture;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    UTexture2D* CollapsedIconTexture;

    void SetData(const FS_OOBWing& InWing, int32 InIndentLevel);

    void BuildChildren(); // Expand into Fighter, Attack, Landing Squadrons

protected:
    virtual void NativeConstruct() override;
    virtual void NativeOnListItemObjectSet(UObject* ListItemObject) override;
};	
	
	
	

