// Copyright 2018-2023, Athian Games. All Rights Reserved. 

#pragma once

#include "SCustomTreeView.h"
#include "UMGStyle.h"
#include "TreeViewWidgetStyle.h"
#include "Blueprint/UserWidget.h"

#include "CustomTreeView.generated.h"

USTRUCT(BlueprintType)
struct FIconStyle
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Style")
		UTexture2D* IconTexture;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Style")
		FVector2D IconSize;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Style")
		FLinearColor IconColor;

	FIconStyle()
	{
		IconSize = FVector2D(10.0f, 10.0f);
		IconColor = FLinearColor(0.72f, 0.72f, 0.72f, 1);
	}
};

USTRUCT(BlueprintType)
struct FExpandedArrowStyle
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Style")
		FSlateBrush CollapsedStyle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Style")
		FSlateBrush CollapsedHoveredStyle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Style")
		FSlateBrush ExpandedStyle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Style")
		FSlateBrush ExpandedHoveredStyle;
};

USTRUCT(BlueprintType)
struct FTreeNode
{
	GENERATED_BODY()

		UPROPERTY(BlueprintReadOnly, Category = "CustomTreeNode")
     		int32 NodeID;

		UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CustomTreeNode")
		    FString NodeName;

	    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CustomTreeNode")
		    int32 ParentID;

		UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CustomTreeNode")
			FMargin NodePadding;

		UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CustomTreeNode")
			TArray<FString> ExtraStrings;
		
};

USTRUCT(BlueprintType)
struct FRowContentTypeByParent
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Style")
		int32 ParentID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Style")
		TSubclassOf<class UUserWidget> RowContent;
};

USTRUCT(BlueprintType)
struct FRowContentTypeById
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Style")
		int32 NodeId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Style")
		TSubclassOf<class UUserWidget> RowContent;
};

UCLASS()
class  UCustomTreeView : public UWidget
{
	GENERATED_BODY()


public:
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FGenerateRowEvent, const FTreeNode&, Row , class UUserWidget* ,  RowWidget, const TArray<FTreeNode>&, Children);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnSelectionChangedEvent, const FTreeNode&, Item, class UUserWidget*, RowWidget);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnExpansionChangedEvent, const FTreeNode&, Item, class UUserWidget*, RowWidget, const bool, ExpansionState);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FGetChildrenEvent, const FTreeNode&, Row, const TArray<FTreeNode>&, Children);

	UCustomTreeView();
	virtual void ReleaseSlateResources(bool bReleaseChildren) override;

#if WITH_EDITOR
	virtual const FText GetPaletteCategory() override;
#endif

	UPROPERTY(BlueprintAssignable, Category = "Widget Event")
		FGenerateRowEvent OnGenerateRow;

	UPROPERTY(BlueprintAssignable, Category = "Widget Event")
		FOnSelectionChangedEvent OnSelectionChanged;

	UPROPERTY(BlueprintAssignable, Category = "Widget Event")
		FOnExpansionChangedEvent OnExpansionChanged;

	UPROPERTY(BlueprintAssignable, Category = "Widget Event")
		FGetChildrenEvent OnGetChildren;


	UPROPERTY(EditAnyWhere , BlueprintReadWrite, Category = "TreeView")
		TArray<FTreeNode> TreeNodes;

	UPROPERTY(EditAnyWhere, BlueprintReadWrite, Category = "Style")
		FTreeViewStyle TreeViewStyle;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Style")
		FExpandedArrowStyle ArrowStyle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Style")
		FMargin RowDefaultPadding;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
		bool ExpanderVisibility;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Content")
		TArray<FRowContentTypeByParent> RowContentsByParent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Content")
		TArray<FRowContentTypeById> RowContentsById;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Content")
		TSubclassOf<class UUserWidget> DefaultRowContent;



	UFUNCTION(BlueprintCallable, Category = "TreeView")
		void CreateTree();

	UFUNCTION(BlueprintCallable, Category = "TreeView")
		void ExpandTreeItem(int32 NodeId);

	UFUNCTION(BlueprintCallable, Category = "TreeView")
		void CollapseTreeItem(int32 NodeId);

	UFUNCTION(BlueprintCallable, Category = "TreeView")
		void ToggleNodeExpansion(int32 NodeId);

	UFUNCTION(BlueprintCallable, Category = "TreeView")
		void SelectTreeItem(int32 NodeId);


	int32 GetRootIndex(int32 nodeindex);

	void HandleOnGenerateRow(TreeNodePtr Item , class UUserWidget* RowWidget , TArray<TreeNodePtr> Children);
	void HandleOnSelectionChanged(TreeNodePtr Item, class UUserWidget* RowWidget);
	void HandleOnExpansionChanged(TreeNodePtr Item ,  class UUserWidget* RowWidget, bool ExpansionState);
	void HandleOnGetChildren(TreeNodePtr Item, TArray<TreeNodePtr> Children);

protected:
	TSharedPtr< SCustomTreeView > TreeViewWidget;
	TArray< TreeNodePtr > TreeStructure;
	TArray<TreeNodePtr> TempStructure;

	virtual TSharedRef<SWidget> RebuildWidget() override;

};
