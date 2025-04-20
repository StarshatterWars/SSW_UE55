// /*  Project nGenEx	Fractal Dev Games	Copyright (C) 2024. All Rights Reserved.	SUBSYSTEM:    SSW	FILE:         Game.cpp	AUTHOR:       Carlos Bott*/


#include "OrderOfBattleListView.h"
#include "OrderOfBattleRowObject.h"
#include "Components/TextBlock.h"
#include "UObject/ConstructorHelpers.h"

/*UUserWidget* UOrderOfBattleListView::OnGenerateRow(UObject* Item, UUserWidget* OwnerWidget)
{
	UOrderOfBattleRowObject* RowData = Cast<UOrderOfBattleRowObject>(Item);
	if (!RowData)
	{
		return nullptr;
	}

	// Create the row widget (You can create your own custom widget for rows)
	/*UUserWidget* RowWidget = CreateWidget<UUserWidget>(this, RowWidgetClass);
	if (RowWidget)
	{
		// Assuming you have a UTextBlock in your custom row widget to display the row's name
		UTextBlock* TextBlock = Cast<UTextBlock>(RowWidget->GetWidgetFromName(TEXT("RowTextBlock")));
		if (TextBlock)
		{
			TextBlock->SetText(FText::FromString(RowData->Name));
		}
	}

	//return RowWidget;
}*/
