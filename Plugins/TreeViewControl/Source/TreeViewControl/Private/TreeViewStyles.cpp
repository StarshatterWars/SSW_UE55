// Copyright 2018-2023, Athian Games. All Rights Reserved. 

#include "TreeViewStyles.h"
#include "Interfaces/IPluginManager.h"
#include "CustomTreeView.h"

#define IMAGE_BRUSH( RelativePath, ... ) FSlateImageBrush( FTreeViewStyles::InContent( RelativePath, ".png" ), __VA_ARGS__ )


FString FTreeViewStyles::InContent(const FString& RelativePath, const ANSICHAR* Extension)
{
	FString Dir = IPluginManager::Get().FindPlugin(TEXT("TreeViewControl"))->GetBaseDir();
	FString Path = (Dir / RelativePath) + Extension;
	return Path;
}

TSharedPtr<FSlateStyleSet> FTreeViewStyles::TreeViewStyleInstance = nullptr;


void FTreeViewStyles::Initialize()
{
	const FVector2D Icon40x40(40.0f, 40.0f);
	const FVector2D Icon10x10(10.0f, 10.0f);

	if (!TreeViewStyleInstance.IsValid())
		TreeViewStyleInstance = MakeShareable(new FSlateStyleSet(GetStyleSetName()));
	else
		FSlateStyleRegistry::UnRegisterSlateStyle(*TreeViewStyleInstance);
		//TreeViewStyleInstance->Set("TreeArrow_Collapsed", new FSlateBrush(ExpandedArrowStyle->CollapsedStyle));
		//TreeViewStyleInstance->Set("TreeArrow_Collapsed_Hovered", new FSlateBrush(ExpandedArrowStyle->CollapsedHoveredStyle));
		//TreeViewStyleInstance->Set("TreeArrow_Expanded", new FSlateBrush(ExpandedArrowStyle->ExpandedStyle));
		//TreeViewStyleInstance->Set("TreeArrow_Expanded_Hovered", new FSlateBrush(ExpandedArrowStyle->ExpandedHoveredStyle));

		TreeViewStyleInstance->Set("ClassIcon.TreeView", new IMAGE_BRUSH("Resources/TreeViewWidgetIcon", Icon10x10));

		//TreeViewStyleInstance->Set("TreeArrow_Collapsed_Hovered", new IMAGE_BRUSH("Icons/" + (ExpandedArrowStyle->CollapsedHoveredStyle.IconTexture == NULL ? "TreeArrow_Collapsed_Hovered" : ExpandedArrowStyle->CollapsedHoveredStyle.IconTexture->GetName()), ExpandedArrowStyle->CollapsedHoveredStyle.IconSize, ExpandedArrowStyle->CollapsedHoveredStyle.IconColor));
		//TreeViewStyleInstance->Set("TreeArrow_Expanded", new IMAGE_BRUSH("Icons/" + (ExpandedArrowStyle->ExpandedStyle.IconTexture == NULL ? "TreeArrow_Expanded" : ExpandedArrowStyle->ExpandedStyle.IconTexture->GetName()), ExpandedArrowStyle->ExpandedStyle.IconSize, ExpandedArrowStyle->ExpandedStyle.IconColor));
		//TreeViewStyleInstance->Set("TreeArrow_Expanded_Hovered", new IMAGE_BRUSH("Icons/" + (ExpandedArrowStyle->ExpandedHoveredStyle.IconTexture == NULL ? "TreeArrow_Expanded_Hovered" : ExpandedArrowStyle->ExpandedHoveredStyle.IconTexture->GetName()), ExpandedArrowStyle->ExpandedHoveredStyle.IconSize, ExpandedArrowStyle->ExpandedHoveredStyle.IconColor));

		FSlateStyleRegistry::RegisterSlateStyle(*TreeViewStyleInstance);
		FTableRowStyle NormalTableRowStyle;

		TreeViewStyleInstance->Set("TableView.DisabledView", FTableRowStyle(NormalTableRowStyle)
			//.SetEvenRowBackgroundHoveredBrush(FSlateNoResource())
			//.SetOddRowBackgroundHoveredBrush(FSlateNoResource())
			//.SetActiveHoveredBrush(FSlateNoResource())
			//.SetInactiveHoveredBrush(FSlateNoResource())
		);

}

void FTreeViewStyles::Shutdown()
{
	if (TreeViewStyleInstance.IsValid())
	{
		FSlateStyleRegistry::UnRegisterSlateStyle(*TreeViewStyleInstance);
		ensure(TreeViewStyleInstance.IsUnique());
		TreeViewStyleInstance.Reset();
	}
}

FName FTreeViewStyles::GetStyleSetName()
{
	static FName StyleSetName(TEXT("TreeViewStyles"));
	return StyleSetName;
}



const ISlateStyle& FTreeViewStyles::Get()
{
	Initialize();
	return *TreeViewStyleInstance;
}