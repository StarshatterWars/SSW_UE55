// /*  Project nGenEx	Fractal Dev Games	Copyright (C) 2024. All Rights Reserved.	SUBSYSTEM:    SSW	FILE:         Game.cpp	AUTHOR:       Carlos Bott*/

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "../Game/GameStructs.h" // FS_OOBFleet definition
#include "OOBBattleWidget.generated.h"

class UTextBlock;
class UImage;
struct FS_OOBBattle;

/**
 * BattleGroup UI Widget - represents one BattleGroup inside a Fleet
 */

UCLASS()
class STARSHATTERWARS_API UOOBBattleWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
    // The BattleGroup data this widget represents
    UPROPERTY()
    FS_OOBBattle Data;

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
    // Sets up this widget with battle data
    void SetData(const FS_OOBBattle& InBattle, int32 InIndentLevel);

    void BuildChildren(); // Build units under Battle

protected:
    virtual void NativeConstruct() override;
};	
	
