// /*  Project nGenEx	Fractal Dev Games	Copyright (C) 2024. All Rights Reserved.	SUBSYSTEM:    SSW	FILE:         Game.cpp	AUTHOR:       Carlos Bott*/


#include "GalaxyMap.h"
#include "SystemMarker.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/Image.h"
#include "SystemMarker.h"
#include "MapGridLine.h"
#include "GalaxyLink.h"
#include "JumpLinksWidget.h"
#include "SelectionLinesWidget.h"
#include "Blueprint/WidgetLayoutLibrary.h"


void UGalaxyMap::NativeConstruct()
{
	Super::NativeConstruct();

	USSWGameInstance* SSWInstance = (USSWGameInstance*)GetGameInstance(); 
	
	ScreenOffset.X = 600;
	ScreenOffset.Y = 300;

	BuildGalaxyMap(SSWInstance->GalaxyData);
}


void UGalaxyMap::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{

}

void UGalaxyMap::BuildGalaxyMap(const TArray<FS_Galaxy>& Systems)
{
	if (!MapCanvas)
	{
		UE_LOG(LogTemp, Warning, TEXT("UGalaxyMap::BuildGalaxyMap(): Missing MapCanvas"));
		return;
	}

	if (!MarkerClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("UGalaxyMap::BuildGalaxyMap(): Missing MarkerClass"));
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("UGalaxyMap::BuildGalaxyMap(): Creating"));

	// Get canvas center offset
	FVector2D CanvasSize = MapCanvas->GetCachedGeometry().GetLocalSize();
	FVector2D CenterOffset = CanvasSize * 0.5f;
	
	MapCanvas->ClearChildren();
	SystemLookup.Empty();
	MarkerMap.Empty();

	// Add Selection Lines Layer (on top of markers)
	if (SelectionLinesWidgetClass)
	{
		SelectionLinesWidget = CreateWidget<USelectionLinesWidget>(this, SelectionLinesWidgetClass);
		MapCanvas->AddChildToCanvas(SelectionLinesWidget);
		SelectionLinesWidget->SetVisibility(ESlateVisibility::Hidden);

		if (UCanvasPanelSlot* SelectionSlot = MapCanvas->AddChildToCanvas(SelectionLinesWidget))
		{
			SelectionSlot->SetAnchors(FAnchors(0.f, 0.f, 1.f, 1.f));
			SelectionSlot->SetOffsets(FMargin(0));
			SelectionSlot->SetAlignment(FVector2D(0.f, 0.f));
			SelectionSlot->SetZOrder(7); // above markers
		}
	}

	// Add grid layer first
	UMapGridLine* GridLayer = CreateWidget<UMapGridLine>(this, MapGridLines);
	GridLayer->SetGalaxyData(Systems, MapScale);
	MapCanvas->AddChildToCanvas(GridLayer); // No need for high Z-order
	if (UCanvasPanelSlot* GridSlot = MapCanvas->AddChildToCanvas(GridLayer))
	{
		GridSlot->SetAnchors(FAnchors(0.f, 0.f, 1.f, 1.f));
		GridSlot->SetOffsets(FMargin(0.f));
		GridSlot->SetAlignment(FVector2D(0.f, 0.f));
		GridSlot->SetZOrder(5); // ensure it's behind markers
	}

	if (JumpLinksWidgetClass)
	{
		JumpLinksWidget = CreateWidget<UJumpLinksWidget>(this, JumpLinksWidgetClass);
		JumpLinksWidget->SetJumpLinks(JumpLinks);

		if (UCanvasPanelSlot* LinksSlot = MapCanvas->AddChildToCanvas(JumpLinksWidget))
		{
			LinksSlot->SetAnchors(FAnchors(0, 0, 1, 1));
			LinksSlot->SetOffsets(FMargin(0));
			LinksSlot->SetAlignment(FVector2D(0, 0));
			LinksSlot->SetZOrder(8); // beneath markers, above grid
		}
	}

	for (const FS_Galaxy& System : Systems)
	{
		SystemLookup.Add(System.Name, System);
		// Step 1: Register all systems
		USystemMarker* Marker = CreateWidget<USystemMarker>(this, MarkerClass);
		if (!Marker) continue;
		MapCanvas->AddChildToCanvas(Marker);
		
		Marker->Init(System);

		/*Marker->OnClicked.BindLambda([this](const FString& SystemName)
			{
				if (SelectedMarker)
				{
					SelectedMarker->SetSelected(false);
				}

				USystemMarker* NewSelection = MarkerMap.FindRef(SystemName);
				if (NewSelection)
				{
					NewSelection->SetSelected(true);
					SelectedMarker = NewSelection;
				}
			});
		*/
		USystemMarker* LocalMarker = Marker; // capture marker per loop

		LocalMarker->OnClicked.BindLambda([this, LocalMarker](const FString& SystemName)
			{
				// Deselect previous marker
				if (SelectedMarker)
				{
					SelectedMarker->SetSelected(false);
				}

				// Select this one
				LocalMarker->SetSelected(true);
				SelectedMarker = LocalMarker;

				if (!SelectionLinesWidget) return;

				// Delay layout-dependent update to next frame
				FTimerHandle TimerHandle;
				GetWorld()->GetTimerManager().SetTimer(TimerHandle, FTimerDelegate::CreateWeakLambda(this,
					[this, LocalMarker]()
					{
						if (!IsValid(LocalMarker) || !IsValid(SelectionLinesWidget)) return;

						auto GetCenter = [](UWidget* Widget) -> FVector2D
							{
								if (UCanvasPanelSlot* Slot = Cast<UCanvasPanelSlot>(Widget->Slot))
								{
									return Slot->GetPosition() + Slot->GetSize() * Slot->GetAlignment();
								}
								return FVector2D::ZeroVector;
							};

						FVector2D MarkerCenter = GetCenter(LocalMarker);
						SelectionLinesWidget->SetMarkerCenter(MarkerCenter);
						SelectionLinesWidget->SetVisibility(ESlateVisibility::Visible);

						UE_LOG(LogTemp, Log, TEXT("Selection lines set to %s"), *MarkerCenter.ToString());

					}), 0.01f, false);
			});
		MarkerMap.Add(System.Name, Marker);

		FVector2D MapPosition = ProjectTo2D(System.Location) + CenterOffset + ScreenOffset;

		UCanvasPanelSlot* PanelSlot = MapCanvas->AddChildToCanvas(Marker);
		if (PanelSlot)
		{
			PanelSlot->SetPosition(MapPosition);
			PanelSlot->SetAlignment(FVector2D(0.5f, 0.5f)); // Align start of line
			PanelSlot->SetAutoSize(true);
			PanelSlot->SetZOrder(10);
		}
	}
	// Step 2: Draw links between systems
	for (const FS_Galaxy& System : Systems)
	{
		for (const FString& LinkedName : System.Link)
		{
			if (const FS_Galaxy* LinkedSystem = SystemLookup.Find(LinkedName))
			{
				DrawLinkBetween(System, *LinkedSystem);
			}
		}
	}
}


TMap<FName, UTexture2D*> UGalaxyMap::LoadStarTextures()
{
	TMap<FName, UTexture2D*> StellarTextures;

	struct FTextureEntry
	{
		FName ClassName;
		const TCHAR* Path;
	};

	const FTextureEntry TexturePaths[] = {
		{ "G", TEXT("/Game/GameData/Galaxy/StarIcons/StarG_map.StarG_map") },
		{ "K", TEXT("/Game/GameData/Galaxy/StarIcons/StarK_map.StarK_map") },
		{ "M", TEXT("/Game/GameData/Galaxy/StarIcons/StarM_map.StarM_map") },
		{ "A", TEXT("/Game/GameData/Galaxy/StarIcons/StarA_map.StarA_map") },
		{ "B", TEXT("/Game/GameData/Galaxy/StarIcons/StarB_map.StarB_map") },
		{ "F", TEXT("/Game/GameData/Galaxy/StarIcons/StarF_map.StarF_map") },
		{ "O", TEXT("/Game/GameData/Galaxy/StarIcons/StarO_map.StarO_map") },
	};

	for (const FTextureEntry& Entry : TexturePaths)
	{
		UTexture2D* Texture = LoadObject<UTexture2D>(nullptr, Entry.Path);
		if (Texture)
		{
			StellarTextures.Add(Entry.ClassName, Texture);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Failed to load texture for star class %s at path: %s"),
				*Entry.ClassName.ToString(), Entry.Path);
		}
	}

	return StellarTextures;
}
FVector2D UGalaxyMap::ProjectTo2D(const FVector& Location) const
{
	return FVector2D(Location.X, -Location.Y) * MapScale; // Flip Y for 2D top-down
}

FVector2D UGalaxyMap::LineProjectTo2D(const FVector& Location) const
{
	return FVector2D(Location.X, -Location.Y) * MapScale;
}

void UGalaxyMap::DrawLinkBetween(const FS_Galaxy& A, const FS_Galaxy& B)
{
	if (!GalaxyLink) return;

	FGalaxyLinkKey LinkKey(A.Name, B.Name);
	if (DrawnLinks.Contains(LinkKey)) return;
	DrawnLinks.Add(LinkKey); // Mark as drawn

	USystemMarker* MarkerA = MarkerMap.FindRef(A.Name);
	USystemMarker* MarkerB = MarkerMap.FindRef(B.Name);
	if (!MarkerA || !MarkerB) return;

	auto GetCenter = [](UWidget* Widget) -> FVector2D
		{
			if (UCanvasPanelSlot* Slot = Cast<UCanvasPanelSlot>(Widget->Slot))
			{
				return Slot->GetPosition() + Slot->GetSize() * Slot->GetAlignment();
			}
			return FVector2D::ZeroVector;
		};

	FVector2D MarkerOffset(-48.f, -12.f);
	FVector2D StartPos = GetCenter(MarkerMap.FindRef(A.Name)) + MarkerOffset;
	FVector2D EndPos = GetCenter(MarkerMap.FindRef(B.Name)) + MarkerOffset;

	FJumpLink Link;
	Link.Start = StartPos;
	Link.End = EndPos;

	JumpLinks.Add(Link);

	// Update the paint-based widget
	if (JumpLinksWidget)
	{
		JumpLinksWidget->SetJumpLinks(JumpLinks);
	}
}

void UGalaxyMap::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	if (!MapCanvas) return;

	if (MapGridLines)
	{
		GalaxyGridWidget = CreateWidget<UMapGridLine>(this, MapGridLines);
		GalaxyGridWidget->SetVisibility(ESlateVisibility::Visible);
		MapCanvas->AddChildToCanvas(GalaxyGridWidget);

		if (UCanvasPanelSlot* GridSlot = MapCanvas->AddChildToCanvas(GalaxyGridWidget))
		{
			GridSlot->SetAnchors(FAnchors(0.f, 0.f, 1.f, 1.f));
			GridSlot->SetOffsets(FMargin(0.f));
			GridSlot->SetAlignment(FVector2D(0.f, 0.f));
			GridSlot->SetZOrder(5); // ensure it's behind markers
		}
	}

	// Add Selection Lines Layer (on top of markers)
	if (SelectionLinesWidgetClass)
	{
		SelectionLinesWidget = CreateWidget<USelectionLinesWidget>(this, SelectionLinesWidgetClass);
		MapCanvas->AddChildToCanvas(SelectionLinesWidget);
		SelectionLinesWidget->SetVisibility(ESlateVisibility::Hidden);

		if (UCanvasPanelSlot* SelectionSlot = MapCanvas->AddChildToCanvas(SelectionLinesWidget))
		{
			SelectionSlot->SetAnchors(FAnchors(0.f, 0.f, 1.f, 1.f));
			SelectionSlot->SetOffsets(FMargin(0));
			SelectionSlot->SetAlignment(FVector2D(0.f, 0.f));
			SelectionSlot->SetZOrder(20); // above markers
		}
	}
}


