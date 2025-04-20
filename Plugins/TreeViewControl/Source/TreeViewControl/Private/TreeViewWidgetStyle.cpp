// Copyright 2018-2023, Athian Games. All Rights Reserved. 

#include "TreeViewWidgetStyle.h"



void FTreeViewStyle::GetResources(TArray<const FSlateBrush*>& OutBrushes) const
{
}

const FName FTreeViewStyle::TypeName = TEXT("FTreeViewStyle");

const FTableRowStyle& FTreeViewStyle::GetNoHoverTableRowStyle()
{
	return FAppStyle::Get().GetWidgetStyle<FTableRowStyle>("TableView.NoHoverTableRow");
}


const FTreeViewStyle& FTreeViewStyle::GetDefault()
{
	static FTreeViewStyle Default;
	return Default;
}

UTreeViewWidgetStyle::UTreeViewWidgetStyle()
{
	
}


