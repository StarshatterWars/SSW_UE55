// Copyright 2018-2023, Athian Games. All Rights Reserved. 

#include "CustomTreeView.h"

#define LOCTEXT_NAMESPACE "UMG"

UCustomTreeView::UCustomTreeView()

{
	//FClassIconFinder::RegisterIconSource(&FTreeViewStyles::Get());
	bIsVariable = false;
	ExpanderVisibility = true;
	RowDefaultPadding = FMargin(4);
}


#if WITH_EDITOR



const FText UCustomTreeView::GetPaletteCategory()
{
	return LOCTEXT("Views", "Views");
}

#endif

void UCustomTreeView::ReleaseSlateResources(bool bReleaseChildren)
{
	Super::ReleaseSlateResources(bReleaseChildren);

	TreeViewWidget.Reset();
}

TSharedRef<SWidget> UCustomTreeView::RebuildWidget()
 {
	if (TreeViewWidget.IsValid())
	{
		TreeViewWidget.Reset();
	}
	 TreeViewWidget = SNew(SCustomTreeView).TWidget(this).TStyle(&TreeViewStyle)
		 .ExpandedArrowStyle(&ArrowStyle).ExpanderVisibility(ExpanderVisibility);
	 CreateTree();

	
	 return TreeViewWidget.ToSharedRef();
 
 }

int32 UCustomTreeView::GetRootIndex(int32 nodeindex)
{
	if (TreeNodes[nodeindex].ParentID == 0)
	{
		return nodeindex;
	}
	else
	{
		return GetRootIndex(TreeNodes[nodeindex].ParentID-1);
	}
}
void UCustomTreeView::ExpandTreeItem(int32 NodeId)
{
	if (TempStructure.Num() > NodeId)
	{
		TreeViewWidget->ExpandTreeItem(TempStructure[NodeId]);
	}
}

void UCustomTreeView::SelectTreeItem(int32 NodeId)
{
	if (TempStructure.Num() > NodeId)
	{
		TreeViewWidget->SetOnMouseButtonDown(FPointerEventHandler());
		TreeViewWidget->SelectDirectory(TempStructure[NodeId]);
		
	}
}

void UCustomTreeView::CollapseTreeItem(int32 NodeId)
{

	if (TempStructure.Num() > NodeId)
	{
		TreeViewWidget->CollapseTreeItem(TempStructure[NodeId]);
	}
}

void UCustomTreeView::ToggleNodeExpansion(int32 NodeId)
{
	if (TempStructure.Num() > NodeId)
	{
		TreeViewWidget->ToggleNodeExpansion(TempStructure[NodeId]);
	}
}

void UCustomTreeView::CreateTree()
{
	TreeStructure.Empty();
	TempStructure.Empty();
	for (int i = 0; i < TreeNodes.Num(); i++)
	{
		TreeNodes[i].NodeID = i;
		if (TreeNodes[i].ParentID == 0)
		{
			TSharedRef<CustomTreeNode> RootDir = MakeShareable(new CustomTreeNode(nullptr, TreeNodes[i].NodeName, TreeNodes[i].NodeName , TreeNodes[i].NodeID , 0 , TreeNodes[i].NodePadding , TreeNodes[i].ExtraStrings));
			TreeStructure.Add(RootDir);
			TempStructure.Add(RootDir);
		}
		else
		{
			if (TempStructure.Num() >= TreeNodes[i].ParentID)
			{
				TreeNodePtr Parent = TempStructure[TreeNodes[i].ParentID - 1];
				TreeNodePtr Child = MakeShareable(new CustomTreeNode(Parent, TreeNodes[i].NodeName, TreeNodes[i].NodeName, TreeNodes[i].NodeID , TreeNodes[i].ParentID , TreeNodes[i].NodePadding, TreeNodes[i].ExtraStrings));
				Parent->AddSubDirectory(Child);
				TempStructure.Add(Child);
			}
		}
	}

	TreeViewWidget->RefreshTree(TreeStructure);
}


void UCustomTreeView::HandleOnGetChildren(TreeNodePtr Item, TArray<TreeNodePtr> Children)
{
	if (Item.IsValid())
	{

		FTreeNode node;
		node.NodeID = Item->GetNodeID();
		node.NodeName = Item->GetDisplayName();
		node.ParentID = Item->GetParentID();
		node.ExtraStrings = Item->GetExtraStrings();
		TArray<FTreeNode> nodeChildren;

		for (int i = 0; i < Children.Num(); i++)
		{
			FTreeNode childnode;
			childnode.NodeID = Item->GetNodeID();
			childnode.NodeName = Children[i]->GetDisplayName();
			childnode.ParentID = Children[i]->GetParentID();
			childnode.ExtraStrings = Children[i]->GetExtraStrings();
			nodeChildren.Add(childnode);
		}

		OnGetChildren.Broadcast(node, nodeChildren);
	}
}

void UCustomTreeView::HandleOnGenerateRow(TreeNodePtr Item , class UUserWidget* RowWidget , TArray<TreeNodePtr> Children)
 {
	 if (Item.IsValid())
	 {

		 FTreeNode node;
		 node.NodeID = Item->GetNodeID();
		 node.NodeName = Item->GetDisplayName();
		 node.ParentID = Item->GetParentID();
		 node.ExtraStrings = Item->GetExtraStrings();
		 TArray<FTreeNode> nodeChildren;

		 for (int i = 0; i < Children.Num(); i++)
		 {
			 FTreeNode childnode;
			 childnode.NodeID = Item->GetNodeID();
			 childnode.NodeName = Children[i]->GetDisplayName();
			 childnode.ParentID = Children[i]->GetParentID();
			 childnode.ExtraStrings = Children[i]->GetExtraStrings();
			 nodeChildren.Add(childnode);
		 }

		 OnGenerateRow.Broadcast(node , RowWidget , nodeChildren);
	 }
	
 }


void UCustomTreeView::HandleOnSelectionChanged(TreeNodePtr Item, class UUserWidget* RowWidget)
{
	if (Item.IsValid())
	{
		FTreeNode node;
		node.NodeID = Item->GetNodeID();
		node.NodeName = Item->GetDisplayName();
		node.ParentID = Item->GetParentID();
		node.ExtraStrings = Item->GetExtraStrings();
		OnSelectionChanged.Broadcast(node , RowWidget);
	}

}

void UCustomTreeView::HandleOnExpansionChanged(TreeNodePtr Item, class UUserWidget* RowWidget, bool ExpansionState)
{
	if (Item.IsValid())
	{

		FTreeNode node;
		node.NodeID = Item->GetNodeID();
		node.NodeName = Item->GetDisplayName();
		node.ParentID = Item->GetParentID();
		node.ExtraStrings = Item->GetExtraStrings();
		OnExpansionChanged.Broadcast(node, RowWidget , ExpansionState);
	}

}

#undef LOCTEXT_NAMESPACE