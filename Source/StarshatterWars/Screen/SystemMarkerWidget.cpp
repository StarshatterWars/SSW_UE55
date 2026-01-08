// /*  Project nGenEx	Fractal Dev Games	Copyright (C) 2024. All Rights Reserved.	SUBSYSTEM:    SSW	FILE:         Game.cpp	AUTHOR:       Carlos Bott*/


#include "SystemMarkerWidget.h"

void USystemMarkerWidget::SetSelected(bool bSelected)
{
	if (ObjectBorder)
	{
		ObjectBorder->SetVisibility(bSelected ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
	}
}

void USystemMarkerWidget::InitCommon(const FString& DisplayName, float Radius)
{
	SetVisibility(ESlateVisibility::Visible);

	CachedName = DisplayName;
	CachedRadius = Radius;

	SetToolTipText(FText::FromString(CachedName));

	if (ObjectNameText)
	{
		ObjectNameText->SetText(FText::FromString(CachedName));
		ObjectNameText->SetColorAndOpacity(FLinearColor::White);
	}
}






