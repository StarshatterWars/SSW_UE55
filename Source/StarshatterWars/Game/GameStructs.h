// /*  Project nGenEx	Fractal Dev Games	Copyright (C) 2024. All Rights Reserved.	SUBSYSTEM:    SSW	FILE:         Game.cpp	AUTHOR:       Carlos Bott*/

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "GameStructs.generated.h"

/**
 * STRUCTS
 */

USTRUCT(BlueprintType)
struct FS_AwardInfo : public FTableRowBase {
	GENERATED_BODY()
	
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	int      AwardId;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FString     AwardType;	
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FString     AwardName;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FString     AwardAbrv;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FString     AwardDesc;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FString     AwardText;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FString     DescSound;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FString     GrantSound;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FString		LargeImage;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FString		SmallImage;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	int		AwardGrant;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	int		RequiredAwards;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	int		Lottery;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	int		MinRank;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	int		MaxRank;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	int     MinShipClass;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	int     MaxShipClass;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	int		GrantedShipClasses;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	int		TotalPoints;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	int		MissionPoints;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	int		TotalMissions;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	int		Kills;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	int		Lost;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	int		Collision;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	int		CampaignId;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	bool	CampaignComplete;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	bool	DynamicCampaign;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	bool	Ceremony;

	FS_AwardInfo() {
		AwardId = 0;
		AwardType = "";
		AwardName = "";
		AwardAbrv = "";
		AwardDesc = "";
		AwardText = "";

		DescSound = "";
		GrantSound = "";
		LargeImage = "";
		SmallImage = "";

		AwardGrant = 0;
		RequiredAwards = 0;
		Lottery = 0;
		MinRank = 0;
		MaxRank = 0;
		MinShipClass = 0;
		MaxShipClass = 0;
		GrantedShipClasses = 0;
	
		TotalPoints = 0;
		MissionPoints = 0;
		TotalMissions = 0;
	
		Kills = 0;
		Lost = 0;
		Collision = 0;
		CampaignId = 0;

		CampaignComplete = false;
		DynamicCampaign = false;
		Ceremony = false;
	}
};

USTRUCT(BlueprintType)
struct FS_ShipPower : public FTableRowBase {
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FString  DesignName;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FString PName;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FString PAbrv;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	int   SType;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	int   EType;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	int   Emcon1;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	int   Emcon2;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	int   Emcon3;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	float Output;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	float Fuel;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	float Size;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	float Hull;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FVector Loc;

	FS_ShipPower() {
		DesignName = "";
		PName = "";
		PAbrv = "";

		SType = 0;
		EType = 0;
		Emcon1 = -1;
		Emcon2 = -1;
		Emcon3 = -1;

		Output = 1000.0f;
		Fuel = 0.0f;
		Size = 0.0f;
		Hull = 0.5f;
		Loc = FVector::ZeroVector;

	}
};

USTRUCT(BlueprintType)
struct FS_ShipDesign : public FTableRowBase {
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FString ShipName;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FString DisplayName;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FString Abrv;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FString Description;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FString	DetailName;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FString	ShipClass;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FString	CockpitName;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FString	BeautyName;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FString HudIconName;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FString DetailName0;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FString DetailName1;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FString DetailName2;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FString DetailName3;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	int PCS;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	int ACS;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	int Detet;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	int RepairTeams;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	int ShipType;
	
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	float Scale;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	float ExplosionScale;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	double Mass;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	float Vlimit;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	float Agility;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	float AirFactor;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	float RollRate;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	float PitchRate;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	float YawRate;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	float TurnBank;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	float CockpitScale;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	float AutoRoll;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	float CL;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	float CD;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	float Stall;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	float Drag;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	float ArcadeDrag;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	float RollDrag;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	float PitchDrag;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	float YawDrag;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	float PrepTime;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	float AvoidTime;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	float AvoidFighter;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	float AvoidStrike;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	float AvoidTarget;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	float CommitRange;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	float SplashRadius;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	float Scuttle;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	float RepairSpeed;
	
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	bool Secret;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	bool RepairAuto;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	bool RepairScreen;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	bool WepScreen;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	bool Degrees;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	TArray<float> FeatureSize;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	TArray<float> EFactor;
	
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FVector Trans;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	TArray<FVector> Offset;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FVector Spin;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FVector BeautyCam;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FVector	ChaseVec;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FVector	BridgeVec;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	TArray<FS_ShipPower> Power;

	FS_ShipDesign() {
		ShipName = "";
		DisplayName = "";
		Description = "";
		Abrv = "";

		DetailName0 = "";
		DetailName1 = "";
		DetailName2 = "";
		DetailName3 = "";

		ShipClass = "";
		CockpitName = "";
		BeautyName = "";
		HudIconName = "";

		PCS = 3.0f;
		ACS = 1.0f;
		Detet = 250.0e3f;
		RepairTeams = 2;
		ShipType = 0;


		Scale = 1.0f;
		ExplosionScale = 0.0f;
		Mass = 0;
		
		Vlimit = 8e3f;
		Agility = 2e2f;
		AirFactor = 0.1f;
		RollRate = 0.0f;
		PitchRate = 0.0f;
		YawRate = 0.0f;
	

		TurnBank = (float)(PI / 8);

		CockpitScale = 1.0f;
		AutoRoll = 0;

		CL = 0.0f;
		CD = 0.0f;
		Stall = 0.0f;
		Drag = 2.5e-5f;

		ArcadeDrag = 1.0f;
		RollDrag = 5.0f;
		PitchDrag = 5.0f;
		YawDrag = 5.0f;

		PrepTime = 30.0f;
		AvoidTime = 0.0f;
		AvoidFighter = 0.0f;
		AvoidStrike = 0.0f;
		AvoidTarget = 0.0f;
		CommitRange = 0.0f;

		SplashRadius = -1.0f;
		Scuttle = 5e3f;
		RepairSpeed = 1.0f;

		FeatureSize.SetNum(4);
		EFactor.SetNum(3);
		Offset.SetNum(4); 

		EFactor[0] = 0.1f;
		EFactor[1] = 0.3f;
		EFactor[2] = 1.0f;

		Secret = false;
		RepairAuto = true;
		RepairScreen = true;
		WepScreen = true;
		Degrees = false;

		Trans = FVector::ZeroVector;

		Spin = FVector::ZeroVector;
		BeautyCam = FVector::ZeroVector;
		ChaseVec = FVector(0, -100, 20);
		BridgeVec = FVector::ZeroVector;
	}
};

USTRUCT(BlueprintType)
struct FS_ComponentDesign : public FTableRowBase {
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FString Name;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FString Abrv;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	int RepairTime;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	int ReplaceTime;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	int Spares;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	int Affects;

	FS_ComponentDesign() {
		Name = "";
		Abrv = "";
		RepairTime = 0;
		ReplaceTime = 0,
			Spares = 1;
		Affects = 0;
	}
};

USTRUCT(BlueprintType)
struct FS_SystemDesign : public FTableRowBase {
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FString Name;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	TArray<FS_ComponentDesign> Component;

	FS_SystemDesign() {
		Name = "";
	}
};

USTRUCT(BlueprintType)
struct FS_FormDesign : public FTableRowBase {
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FString Name;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FString Caption;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	int Id;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	int PId;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FVector4 Rect;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FVector4 Cells;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FVector4 Insets;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FVector4 TextInsets;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FVector4 CellInsets;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FString Font;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FColor BackColor;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FColor BaseColor;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FColor ForeColor;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FString Texture;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	bool Transparent;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	int Style;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	int Align;

	FS_FormDesign() {
		Name = "";
		Caption = "";
		Id = 0;
		PId = 0;
		Rect = FVector4::Zero();
		Cells = FVector4::Zero();
		Insets = FVector4::Zero();
		TextInsets = FVector4::Zero();
		CellInsets = FVector4::Zero();
		Font = "";
		BackColor = FColor::Black;
		BaseColor = FColor::Black;
		ForeColor = FColor::Black;
		Texture = "";
		Transparent = false;
		Style = 0;
		Align = 0;
	}
};
