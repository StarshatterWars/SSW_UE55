// /*  Project nGenEx	Fractal Dev Games	Copyright (C) 2024. All Rights Reserved.	SUBSYSTEM:    SSW	FILE:         Game.cpp	AUTHOR:       Carlos Bott*/

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "../Game/GameStructs.h" // FS_OOBStation definition
#include "OOBStationWidget.generated.h"

class UTextBlock;
struct FS_OOBStation;

/**
 * Station UI Widget - represents one Station under a Battalion
 */

UCLASS()
class STARSHATTERWARS_API UOOBStationWidget : public UUserWidget
{
	GENERATED_BODY()
	
	public:

    // The Station data this widget represents
    UPROPERTY()
    FS_OOBStation 
    Data;

    // UI
    UPROPERTY(meta = (BindWidgetOptional))
    UTextBlock* NameText; // Displays the Station name
    
    // How deep this widget is in the tree (0 = Force, 1 = Fleet, 2 = Battle, etc.)
    UPROPERTY()
    int32 IndentLevel = 2; 

    // Sets this widget's data
    void SetData(const FS_OOBStation& InStation, int32 InIndentLevel);

protected:
    virtual void NativeConstruct() override;

};

