// /*  Project nGenEx	Fractal Dev Games	Copyright (C) 2024. All Rights Reserved.	SUBSYSTEM:    SSW	FILE:         Game.cpp	AUTHOR:       Carlos Bott*/

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "../Game/GameStructs.h" // FS_Galaxy struct
#include "../System/SSWGameInstance.h"
#include "GalaxyMap.generated.h"

class UCanvasPanel;
class USystemMarker;
class UTexture2D;

/**
 * 
 */
UCLASS()
class STARSHATTERWARS_API UGalaxyMap : public UUserWidget
{
	GENERATED_BODY()
	
public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Galaxy")
    TSubclassOf<USystemMarker> MarkerClass;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Galaxy")
    TSubclassOf<UUserWidget> LinkWidgetClass; // Optional visual line widget

    UPROPERTY(meta = (BindWidget))
    UCanvasPanel* MapCanvas;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Galaxy")
    TMap<FName, UTexture2D*> StarTextures;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Galaxy")
    float MapScale = 60.0f;

    void BuildMap(const TArray<FS_Galaxy>& Systems);
    
    TMap<FName, UTexture2D*> LoadStarTextures();

private:
    void DrawLinkBetween(const FS_Galaxy& A, const FS_Galaxy& B);
    FVector2D ProjectTo2D(const FVector& Location) const;
    TMap<FString, FS_Galaxy> SystemLookup;
};
	
	
