/*  Project Starshatter Wars
	Fractal Dev Games
	Copyright (C) 2024. All Rights Reserved.

	SUBSYSTEM:    System
	FILE:         BaseScreen.h
	AUTHOR:       Carlos Bott

	Base Screen Widget
*/

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Engine/Texture2D.h"
#include "Components/Image.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/ComboBoxString.h"
#include "../System/SSWGameInstance.h"
#include "BaseScreen.generated.h"

/**
 * 
 */
UCLASS()
class STARSHATTERWARS_API UBaseScreen : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(meta = (BindWidgetOptional))
	class UTextBlock* Title;

	UPROPERTY(meta = (BindWidgetOptional))
	class UTextBlock* CancelButtonText;
	UPROPERTY(meta = (BindWidgetOptional))
	class UTextBlock* ApplyButtonText;

	UPROPERTY(meta = (BindWidgetOptional))
	class UButton* ApplyButton;
	UPROPERTY(meta = (BindWidgetOptional))
	class UButton* CancelButton;
	
protected:
	virtual void NativeConstruct() override;
	virtual void NativePreConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
	
	
};
