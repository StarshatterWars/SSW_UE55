// /*  Project nGenEx	Fractal Dev Games	Copyright (C) 2024. All Rights Reserved.	SUBSYSTEM:    SSW	FILE:         Game.cpp	AUTHOR:       Carlos Bott*/

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/Button.h"
#include "MenuButton.generated.h"

/**
 * 
 */
UCLASS()
class STARSHATTERWARS_API UMenuButton : public UUserWidget
{
	GENERATED_BODY()
public:
	virtual void NativeConstruct() override;

	// Expose button to be set in UMG Designer
	UPROPERTY(meta = (BindWidget))
	UButton* Button;

	// Current selection state
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Selection")
	bool bIsSelected = false;

	// Optional: color states
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
	FLinearColor SelectedColor = FLinearColor::Gray;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
	FLinearColor DefaultColor = FLinearColor::White;

	void SetSelectedState(bool bSelected);
	bool GetSelectedState();

protected:
	UFUNCTION()
	void OnButtonClicked();

	void UpdateStyle();
};