// /*  Project nGenEx	Fractal Dev Games	Copyright (C) 2024. All Rights Reserved.	SUBSYSTEM:    SSW	FILE:         Game.cpp	AUTHOR:       Carlos Bott*/


#include "GalaxyMap.h"
#include "SystemMarker.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/Image.h"
#include "Blueprint/WidgetLayoutLibrary.h"

void UGalaxyMap::BuildMap(const TArray<FS_Galaxy>& Systems)
{
    if (!MapCanvas || !MarkerClass) return;

    MapCanvas->ClearChildren();
    SystemLookup.Empty();

    // Step 1: Register all systems
    for (const FS_Galaxy& System : Systems)
    {
        SystemLookup.Add(System.Name, System);

        auto* Marker = CreateWidget<USystemMarker>(this, MarkerClass);
        if (Marker)
        {
            Marker->Init(System);
            FVector2D MapPos = ProjectTo2D(System.Location);
            auto* MapSlot = MapCanvas->AddChildToCanvas(Marker);
            MapSlot->SetPosition(MapPos);
            MapSlot->SetAutoSize(true);
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
    TMap<FName, UTexture2D*> Textures;

    struct FTexturePath
    {
        FName ClassName;
        const TCHAR* Path;
    };

    const FTexturePath TexturePaths[] = {
        { "G", TEXT("/Game/GameData/Galaxy/StarIcons/StarG_map.StarG_map") },
        { "K", TEXT("/Game/GameData/Galaxy/StarIcons/StarK_map.StarK_map") },
        { "M", TEXT("/Game/GameData/Galaxy/StarIcons/StarM_map.StarM_map") },
        { "A", TEXT("/Game/GameData/Galaxy/StarIcons/StarA_map.StarA_map") },
        { "B", TEXT("/Game/GameData/Galaxy/StarIcons/StarB_map.StarB_map") },
        { "F", TEXT("/Game/GameData/Galaxy/StarIcons/StarF_map.StarF_map") },
        { "O", TEXT("/Game/GameData/Galaxy/StarIcons/StarO_map.StarO_map") },
    };

    for (const FTexturePath& Entry : TexturePaths)
    {
        UTexture2D* Texture = LoadObject<UTexture2D>(nullptr, Entry.Path);
        if (Texture)
        {
            Textures.Add(Entry.ClassName, Texture);
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("Failed to load texture for class %s at path: %s"),
                *Entry.ClassName.ToString(), Entry.Path);
        }
    }

    return Textures;
}

void UGalaxyMap::DrawLinkBetween(const FS_Galaxy& A, const FS_Galaxy& B)
{
    if (!LinkWidgetClass) return;

    FVector2D PosA = ProjectTo2D(A.Location);
    FVector2D PosB = ProjectTo2D(B.Location);
    FVector2D Direction = PosB - PosA;

    float Length = Direction.Size();
    float Angle = FMath::Atan2(Direction.Y, Direction.X) * 180.f / PI;

    UUserWidget* Link = CreateWidget<UUserWidget>(this, LinkWidgetClass);
    if (!Link) return;

    auto* LinkSlot = MapCanvas->AddChildToCanvas(Link);
    if (LinkSlot)
    {
        LinkSlot->SetPosition(PosA);
        LinkSlot->SetSize(FVector2D(Length, 2.f));
        LinkSlot->SetAlignment(FVector2D(0.f, 0.5f));
        Link->SetRenderTransformAngle(Angle);
    }
}

FVector2D UGalaxyMap::ProjectTo2D(const FVector& Location) const
{
    return FVector2D(Location.X, -Location.Y) * MapScale; // Flip Y for 2D top-down
}

