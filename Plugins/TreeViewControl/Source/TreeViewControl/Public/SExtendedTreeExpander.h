// Copyright 2018-2023, Athian Games. All Rights Reserved. 

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/Views/STableRow.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Input/SButton.h"


/**
 * Expander arrow and indentation component that can be placed in a TableRow
 * of a TreeView. Intended for use by TMultiColumnRow in TreeViews.
 */
class SExtendedTreeExpander : public SCompoundWidget
{
public:

	SLATE_BEGIN_ARGS( SExtendedTreeExpander )
		:_IndentAmount(10)
		, _BaseIndentLevel(0)
	{ }
		SLATE_ARGUMENT(const struct FExpandedArrowStyle*, StyleSet)
		SLATE_ATTRIBUTE(float, IndentAmount)
		SLATE_ATTRIBUTE(EVisibility, ExpanderVisibility)
		SLATE_ATTRIBUTE(int32, BaseIndentLevel)
	SLATE_END_ARGS()

	SExtendedTreeExpander(): StyleSet(nullptr)
	{
	}
	

	SExtendedTreeExpander(const TWeakPtr<ITableRow>& owner_row_ptr, const TSharedPtr<SButton>& expander_arrow,
	                      const FExpandedArrowStyle* style_set, const TAttribute<float>& indent_amount,
	                      const TAttribute<int32>& base_indent_level)
		: OwnerRowPtr(owner_row_ptr),
		  ExpanderArrow(expander_arrow),
		  StyleSet(style_set),
		  IndentAmount(indent_amount),
		  BaseIndentLevel(base_indent_level)
	{
	}

	void Construct( const FArguments& InArgs, const TSharedPtr<class ITableRow>& TableRow );

protected:

	/** Invoked when the expanded button is clicked (toggle item expansion) */
	FReply OnArrowClicked();

	/** @return Visible when has children; invisible otherwise */
	EVisibility GetExpanderVisibility() const;

	/** @return the margin corresponding to how far this item is indented */
	FMargin GetExpanderPadding() const;

	/** @return the name of an image that should be shown as the expander arrow */
	const FSlateBrush* GetExpanderImage() const;

	TWeakPtr<class ITableRow> OwnerRowPtr;

	/** A reference to the expander button */
	TSharedPtr<SButton> ExpanderArrow;

	/** The slate style to use */
	const struct FExpandedArrowStyle* StyleSet;

	/** The amount of space to indent at each level */
	TAttribute<float> IndentAmount;

	/** The level in the tree that begins the indention amount */
	TAttribute<int32> BaseIndentLevel;

};
