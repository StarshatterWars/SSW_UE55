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
    UTextBlock* NameText; // Displays the Starbase name

     // How deep this widget is in the tree (0 = Force, 1 = Fleet, 2 = Battle, etc.)
    UPROPERTY()
    int32 IndentLevel = 2; 

    // Sets this widget's data
    void SetData(const FS_OOBStarbase& InStarbase, int32 InIndentLevel);

protected:
    virtual void NativeConstruct() override;
};
