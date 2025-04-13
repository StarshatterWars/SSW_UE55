// /*  Project nGenEx	Fractal Dev Games	Copyright (C) 2024. All Rights Reserved.	SUBSYSTEM:    SSW	FILE:         Game.cpp	AUTHOR:       Carlos Bott*/

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "MissionListObject.h"
#include "Blueprint/IUserObjectListEntry.h"

#include "../System/SSWGameInstance.h"
#include "IntelLVElement.generated.h"

/**
 * 
 */
UCLASS()
class STARSHATTERWARS_API UIntelLVElement : public UUserWidget, public IUserObjectListEntry
{
	GENERATED_BODY()
	
protected:
	void NativeConstruct() override;
	virtual void NativeOnListItemObjectSet(UObject* ListItemObject) override;
	
	
};
