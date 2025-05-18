// /*  Project nGenEx	Fractal Dev Games	Copyright (C) 2024. All Rights Reserved.	SUBSYSTEM:    SSW	FILE:         Game.cpp	AUTHOR:       Carlos Bott*/


#include "GalaxyMap.h"
#include "SystemMarker.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/Image.h"
#include "SystemMarker.h"
#include "MapGridLine.h"
#include "GalaxyLink.h"
#include "Blueprint/WidgetLayoutLibrary.h"


void UGalaxyMap::NativeConstruct()
{
	Super::NativeConstruct();

	USSWGameInstance* SSWInstance = (USSWGameInstance*)GetGameInstance(); 
	
	ScreenOffset.X = 600;
	ScreenOffset.Y = 300;

	BuildGalaxyMap(SSWInstance->GalaxyData);
	//DrawGrid(SSWInstance->GalaxyData);
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

	for (const FS_Galaxy& System : Systems)
	{
		SystemLookup.Add(System.Name, System);
		// Step 1: Register all systems
		USystemMarker* Marker = CreateWidget<USystemMarker>(this, MarkerClass);
		if (!Marker) continue;
		MapCanvas->AddChildToCanvas(Marker);
		
		Marker->Init(System);

		Marker->OnClicked.BindLambda([this](const FString& SystemName)
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

	FVector2D MarkerOffset = { -48, 0 };
	FVector2D StartPos = GetCenter(MarkerA) + MarkerOffset;
	FVector2D EndPos = GetCenter(MarkerB) + MarkerOffset;
	FVector2D Direction = EndPos - StartPos;

	float Length = Direction.Size();
	float Angle = FMath::Atan2(Direction.Y, Direction.X) * 180.f / PI;

	UGalaxyLink* Link = CreateWidget<UGalaxyLink>(this, GalaxyLink);
	if (!Link) return;

	Link->ConfigureLine(Length, Angle);

	if (UCanvasPanelSlot* LinkSlot = MapCanvas->AddChildToCanvas(Link))
	{
		LinkSlot->SetPosition(StartPos); // aligns the left edge of the line to marker center
		LinkSlot->SetAlignment(FVector2D(0.f, 0.5f)); // rotates from left-center of the line
		LinkSlot->SetZOrder(9);
	}
}


