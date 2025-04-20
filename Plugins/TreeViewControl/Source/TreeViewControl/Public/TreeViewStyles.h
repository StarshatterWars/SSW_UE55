// Copyright 2018-2023, Athian Games. All Rights Reserved. 
#pragma once


class FTreeViewStyles
{
public:

	static void Initialize();

	static void Shutdown();

	static const class ISlateStyle& Get();

	static FName GetStyleSetName();
	static void SetExpandedArrowStyle(const struct FExpandedArrowStyle* style);

private:
	// Creates the Style Set.
	static TSharedRef<class FSlateStyleSet> Create();

	// Singleton instance used for our Style Set.
	static TSharedPtr<class FSlateStyleSet> TreeViewStyleInstance;
	
	static FString InContent(const FString& RelativePath, const ANSICHAR* Extension);
};