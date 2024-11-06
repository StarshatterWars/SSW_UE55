// /*  Project nGenEx	Fractal Dev Games	Copyright (C) 2024. All Rights Reserved.	SUBSYSTEM:    SSW	FILE:         Game.cpp	AUTHOR:       Carlos Bott*/

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Campaign.generated.h"

UCLASS()
class STARSHATTERWARS_API ACampaign : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ACampaign();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	
	
};
