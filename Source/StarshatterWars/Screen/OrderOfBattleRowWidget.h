// /*  Project nGenEx	Fractal Dev Games	Copyright (C) 2024. All Rights Reserved.	SUBSYSTEM:    SSW	FILE:         Game.cpp	AUTHOR:       Carlos Bott*/

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Blueprint/IUserObjectListEntry.h"
#include "../Game/GameStructs.h"
#include "OrderOfBattleRowWidget.generated.h"

/**
 * 
 */
UCLASS()
class STARSHATTERWARS_API UOrderOfBattleRowWidget : public UUserWidget, public IUserObjectListEntry
{
	GENERATED_BODY()
	
	public:
	// Called when this widget is assigned a list item object
	virtual void NativeOnListItemObjectSet(UObject* ListItemObject) override;

protected:
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* NameText;

	// You can bind more widgets (like class, design, etc.) here
};
	
	
	

