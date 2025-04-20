// Copyright 2018-2023, Athian Games. All Rights Reserved. 

#include "SExtendedTreeExpander.h"
#include "CustomTreeView.h"

void SExtendedTreeExpander::Construct( const FArguments& InArgs, const TSharedPtr<class ITableRow>& TableRow  )
{
	OwnerRowPtr = TableRow;
	StyleSet = InArgs._StyleSet;
	IndentAmount = InArgs._IndentAmount;
	BaseIndentLevel = InArgs._BaseIndentLevel;

	this->ChildSlot
	.Padding( TAttribute<FMargin>( this, &SExtendedTreeExpander::GetExpanderPadding ) )
	[
		SAssignNew(ExpanderArrow, SButton)
		.ButtonStyle( FCoreStyle::Get(), "NoBorder" )
		.VAlign(VAlign_Center)
		.HAlign(HAlign_Center)
		.Visibility( this, &SExtendedTreeExpander::GetExpanderVisibility )
		.ClickMethod( EButtonClickMethod::MouseDown )
		.OnClicked( this, &SExtendedTreeExpander::OnArrowClicked )
		.ContentPadding(0.f)
		.ForegroundColor( FSlateColor::UseForeground() )
		.IsFocusable( false )
		[
			SNew(SImage)
			.Image( this, &SExtendedTreeExpander::GetExpanderImage )
			.ColorAndOpacity( FSlateColor::UseForeground() )
		]
	];
}

/** Invoked when the expanded button is clicked (toggle item expansion) */
FReply SExtendedTreeExpander::OnArrowClicked()
{
	// Recurse the expansion if "shift" is being pressed
	const FModifierKeysState ModKeyState = FSlateApplication::Get().GetModifierKeys();
	if(ModKeyState.IsShiftDown())
	{
		OwnerRowPtr.Pin()->Private_OnExpanderArrowShiftClicked();
	}
	else
	{
		OwnerRowPtr.Pin()->ToggleExpansion();
	}

	return FReply::Handled();
}

/** @return Visible when has children; invisible otherwise */
EVisibility SExtendedTreeExpander::GetExpanderVisibility() const
{
	return OwnerRowPtr.Pin()->DoesItemHaveChildren()  ? EVisibility::Visible : EVisibility::Hidden;
}

/** @return the margin corresponding to how far this item is indented */
FMargin SExtendedTreeExpander::GetExpanderPadding() const
{
	const int32 NestingDepth = FMath::Max(0, OwnerRowPtr.Pin()->GetIndentLevel() - BaseIndentLevel.Get());
	const float Indent = IndentAmount.Get(10.f);
	return FMargin( NestingDepth * Indent, 0,0,0 );
}

/** @return the name of an image that should be shown as the expander arrow */
const FSlateBrush* SExtendedTreeExpander::GetExpanderImage() const
{
	if (const bool bIsItemExpanded = OwnerRowPtr.Pin()->IsItemExpanded())
	{
		if ( ExpanderArrow->IsHovered() )
		{

			return &StyleSet->ExpandedHoveredStyle;
		}
		else
		{

			return &StyleSet->ExpandedStyle;

		}
	}
	else
	{
		if ( ExpanderArrow->IsHovered() )
		{
			return &StyleSet->CollapsedHoveredStyle;

		}
		else
		{
			return &StyleSet->CollapsedStyle;

		}
	}

}

