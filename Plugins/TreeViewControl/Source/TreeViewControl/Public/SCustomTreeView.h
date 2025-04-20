// Copyright 2018-2023, Athian Games. All Rights Reserved. 

#pragma once

#include "CustomTreeNode.h"
#include "SlateCore.h"
#include "Engine.h"
#include "TreeViewStyles.h"
#include "SAdvancedTableRow.h"

typedef STreeView< TreeNodePtr > STView;

struct FRow
{
	int32 NodeId;
	class UUserWidget* RowWidget;
};

class SCustomTreeView : public SCompoundWidget

{
public:


	SLATE_BEGIN_ARGS(SCustomTreeView)
	{}
	SLATE_ARGUMENT(TWeakObjectPtr<class UCustomTreeView>, TWidget)
	SLATE_ARGUMENT(const struct FTreeViewStyle*, TStyle)
	SLATE_ARGUMENT(bool , ExpanderVisibility)

	SLATE_ARGUMENT(const struct FExpandedArrowStyle*, ExpandedArrowStyle)
	//SLATE_ARGUMENT(TArray<const struct FRowContentType>*, RowContents)
	SLATE_END_ARGS()

		/** Widget constructor */
	void Construct(const FArguments& InArgs);

	/** @return Returns the currently selected category item */
	TreeNodePtr GetSelectedDirectory() const;

	/** Selects the specified category */
	void SelectDirectory(const TreeNodePtr& CategoryToSelect);

	/** @return Returns true if the specified item is currently expanded in the tree */
	bool IsItemExpanded(const TreeNodePtr Item) const;

	void RefreshTree(TArray< TreeNodePtr > structure);
	void ExpandTreeItem(TreeNodePtr Item);
	void CollapseTreeItem(TreeNodePtr Item);
	void ToggleNodeExpansion(TreeNodePtr Item);
	

	TWeakObjectPtr<class UCustomTreeView> TWidget;
	const struct FTreeViewStyle* TStyle;
	const struct FExpandedArrowStyle* ExpandedArrowStyle;
	bool ExpanderVisibility;
	//TArray<const struct FRowContentType>* RowContents;
	TArray<FRow> Rows;

protected:

	TSharedRef<ITableRow> OnGenerateRow(TreeNodePtr Item, const TSharedRef<STableViewBase>& OwnerTable);

	TSharedPtr<SScrollBar> ExternalScrollbar();

	void OnGetChildren(TreeNodePtr Item, TArray< TreeNodePtr >& OutChildren);

	void OnSelectionChanged(TreeNodePtr Item, ESelectInfo::Type SelectInfo);

	void OnExpansionChanged(TreeNodePtr Item, bool ExpansionState);

	/** SWidget overrides */
	virtual void Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime) override;


private:

	TArray< TreeNodePtr > TreeStructure;
	/** The tree view widget*/
	TSharedPtr< STView > TView;
	//TSharedPtr< SScrollBar > verticalbar;
	TSharedPtr< SScrollBox > verticalscrollbox;
	//TSharedPtr< SScrollBar > horizontalbar;
	FGeometry CachedGeometry;
	float currentscrolldisremaining;

};
