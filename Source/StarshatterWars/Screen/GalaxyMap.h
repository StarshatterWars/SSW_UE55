// /*  Project nGenEx	Fractal Dev Games	Copyright (C) 2024. All Rights Reserved.	SUBSYSTEM:    SSW	FILE:         Game.cpp	AUTHOR:       Carlos Bott*/

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "../Game/GameStructs.h" // FS_Galaxy struct
#include "../System/SSWGameInstance.h"
#include "GalaxyMap.generated.h"

class UCanvasPanel;
class USystemMarker;
class UGalaxyLink;
class UMapGridLine;
class UTexture2D;

/**
 * 
 */
UCLASS()
class STARSHATTERWARS_API UGalaxyMap : public UUserWidget
{
	GENERATED_BODY()
	
protected:
    void NativeConstruct() override;
    virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
 
public:
    // TSubclassOf must be set in UMG (or via C++)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Galaxy")
    TSubclassOf<USystemMarker> MarkerClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Galaxy")
    TSubclassOf<UGalaxyLink> GalaxyLink; // Optional visual line widget

    UPROPERTY(EditAnywhere, Category = "Galaxy")
    TSubclassOf<UMapGridLine> MapGridLines;

    UPROPERTY(meta = (BindWidget))
    UCanvasPanel* MapCanvas;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Galaxy")
    TMap<FName, UTexture2D*> StarTextures;

    UPROPERTY()
    TArray<FS_Galaxy> GalaxySystems; // Store this to access system bounds

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Galaxy")
    float MapScale = 60.0f;


    void BuildGalaxyMap(const TArray<FS_Galaxy>& Systems);
    
    TMap<FName, UTexture2D*> LoadStarTextures();

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    FVector2D ScreenOffset;
	
	USystemMarker* SelectedMarker = nullptr;

private:
    
    UFUNCTION()
    void DrawLinkBetween(const FS_Galaxy& A, const FS_Galaxy& B);
    UFUNCTION()
    FVector2D ProjectTo2D(const FVector& Location) const;
    UFUNCTION()
    FVector2D LineProjectTo2D(const FVector& Location) const;
    TMap<FString, FS_Galaxy> SystemLookup;
    TMap<FString, USystemMarker*> MarkerMap;
};
	
	
