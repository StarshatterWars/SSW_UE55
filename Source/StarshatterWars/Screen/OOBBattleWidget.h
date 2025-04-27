// /*  Project nGenEx	Fractal Dev Games	Copyright (C) 2024. All Rights Reserved.	SUBSYSTEM:    SSW	FILE:         Game.cpp	AUTHOR:       Carlos Bott*/

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "../Game/GameStructs.h" // FS_OOBFleet definition
#include "OOBBattleWidget.generated.h"

class UTextBlock;
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

    // UI
    UPROPERTY(meta = (BindWidgetOptional))
    UTextBlock* Label; // Displays the BattleGroup name


    // Sets up this widget with battle data
    void SetData(const FS_OOBBattle& InBattle);

protected:
    virtual void NativeConstruct() override;
};	
	
