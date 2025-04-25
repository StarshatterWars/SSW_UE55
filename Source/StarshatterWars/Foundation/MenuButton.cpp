// /*  Project nGenEx	Fractal Dev Games	Copyright (C) 2024. All Rights Reserved.	SUBSYSTEM:    SSW	FILE:         Game.cpp	AUTHOR:       Carlos Bott*/


#include "MenuButton.h"
#include "Components/Button.h"
#include "Components/ButtonSlot.h"
#include "Components/TextBlock.h"
#include "Components/Border.h"


void UMenuButton::NativeConstruct()
{
	Super::NativeConstruct();

	if (Button)
	{
		Button->OnClicked.AddDynamic(this, &UMenuButton::OnButtonClicked);
		UpdateStyle();
	}
}


void UMenuButton::SetSelectedState(bool bSelected)
{
	bIsSelected = bSelected;
}


bool UMenuButton::GetSelectedState()
{
	return bIsSelected;
}

void UMenuButton::OnButtonClicked()
{
	bIsSelected = !bIsSelected;
	UpdateStyle();

	UE_LOG(LogTemp, Log, TEXT("Button is now %s"), bIsSelected ? TEXT("Selected") : TEXT("Unselected"));
}

void UMenuButton::UpdateStyle()
{
	if (Button)
	{
		FButtonStyle NewStyle = Button->WidgetStyle;

		const FSlateColor ColorToUse = bIsSelected ? SelectedColor : DefaultColor;
		NewStyle.Normal.TintColor = FSlateColor(ColorToUse);
		NewStyle.Hovered.TintColor = FSlateColor(ColorToUse);
		NewStyle.Pressed.TintColor = FSlateColor(ColorToUse);

		Button->SetStyle(NewStyle);
	}
}
