// /*  Project nGenEx	Fractal Dev Games	Copyright (C) 2024. All Rights Reserved.	SUBSYSTEM:    SSW	FILE:         Game.cpp	AUTHOR:       Carlos Bott*/


#include "PlanetUtils.h"
#include "IImageWrapperModule.h"
#include "IImageWrapper.h"
#include "Modules/ModuleManager.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Misc/FileHelper.h"
#include "Engine/Texture2D.h"

TMap<FString, UTexture2D*> PlanetUtils::LoadedTextureCache;

UTexture2D* PlanetUtils::LoadPlanetTexture(const FString& TextureName)
{
	// Check cache first
	if (LoadedTextureCache.Contains(TextureName))
	{
		return LoadedTextureCache[TextureName];
	}

	// Build full path
	const FString FilePath = FPaths::ProjectContentDir() / "GameData/Galaxy/PlanetMaterials/" + TextureName + ".png";

	TArray<uint8> RawData;
	if (!FFileHelper::LoadFileToArray(RawData, *FilePath))
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to load planet texture file: %s"), *FilePath);
		return nullptr;
	}

	IImageWrapperModule& ImageWrapperModule = FModuleManager::LoadModuleChecked<IImageWrapperModule>("ImageWrapper");
	EImageFormat Format = ImageWrapperModule.DetectImageFormat(RawData.GetData(), RawData.Num());
	if (Format == EImageFormat::Invalid)
	{
		UE_LOG(LogTemp, Error, TEXT("Unsupported image format for: %s"), *FilePath);
		return nullptr;
	}

	TSharedPtr<IImageWrapper> Wrapper = ImageWrapperModule.CreateImageWrapper(Format);
	if (!Wrapper.IsValid() || !Wrapper->SetCompressed(RawData.GetData(), RawData.Num()))
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to decompress image: %s"), *FilePath);
		return nullptr;
	}

	TArray<uint8> Uncompressed;
	if (!Wrapper->GetRaw(ERGBFormat::BGRA, 8, Uncompressed))
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to extract RGBA pixels from: %s"), *FilePath);
		return nullptr;
	}

	int32 Width = Wrapper->GetWidth();
	int32 Height = Wrapper->GetHeight();

	UTexture2D* Texture = UTexture2D::CreateTransient(Width, Height, PF_B8G8R8A8);
	if (!Texture) return nullptr;

	void* Data = Texture->GetPlatformData()->Mips[0].BulkData.Lock(LOCK_READ_WRITE);
	FMemory::Memcpy(Data, Uncompressed.GetData(), Uncompressed.Num());
	if (Texture)
	{
		#define UpdateResource UpdateResource
		Texture->GetPlatformData()->Mips[0].BulkData.Unlock();
		Texture->UpdateResource();
	}
	// Safe unique name
	int32 Suffix = FMath::RandRange(1000, 9999);
	FString UniqueName = FString::Printf(TEXT("Tex_%s_%d"), *TextureName, Suffix);
	Texture->Rename(*UniqueName, GetTransientPackage());

	// Cache and return
	Texture->AddToRoot(); // Prevent GC
	Texture->SetFlags(RF_Public | RF_Standalone);
	LoadedTextureCache.Add(TextureName, Texture);

	UE_LOG(LogTemp, Log, TEXT("Planet texture loaded and cached: % s(% dx % d)"), *UniqueName, Width, Height);
	return Texture;
}

float PlanetUtils::GetUISizeFromRadius(float Radius, float MinSize, float MaxSize)
{
	constexpr float MinRadiusKm = 3.15e6;
	constexpr float MaxRadiusKm = 38.2e6;

	float LogRadius = FMath::LogX(10.f, FMath::Max(Radius, 1.f));
	float MinLog = FMath::LogX(10.f, MinRadiusKm);
	float MaxLog = FMath::LogX(10.f, MaxRadiusKm);

	float Normalized = FMath::Clamp((LogRadius - MinLog) / (MaxLog - MinLog), 0.f, 1.f);
	//return FMath::Lerp(MinSize, MaxSize, Normalized);

	// UI scaling range — safe for mesh scale and texture logic
	constexpr float MinUIScale = 32.0f;
	constexpr float MaxUIScale = 96.0f;

	// Final safe UI scale
	float UIScale = FMath::Lerp(MinUIScale, MaxUIScale, static_cast<float>(Normalized));

	// Clamp final value to hard max
	return FMath::Clamp(UIScale, MinUIScale, MaxUIScale);
}

UTextureRenderTarget2D* PlanetUtils::CreatePlanetRenderTarget(const FString& BaseName, UObject* Outer, int32 Resolution)
{
	// Ensure Outer is not null (avoid creating in CDO space)
	if (!Outer)
	{
		Outer = GetTransientPackage();
	}

	// Generate unique name
	FString UniqueName = FString::Printf(TEXT("RT_%s_%d"), *BaseName, FMath::RandRange(1000, 9999));

	// Create transient render target
	UTextureRenderTarget2D* RenderTarget = NewObject<UTextureRenderTarget2D>(Outer, *UniqueName, RF_Transient);
	if (!RenderTarget)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to create RenderTarget: %s"), *UniqueName);
		return nullptr;
	}

	RenderTarget->RenderTargetFormat = RTF_RGBA8;
	RenderTarget->ClearColor = FLinearColor::Transparent;
	RenderTarget->bAutoGenerateMips = false;
	RenderTarget->InitAutoFormat(Resolution, Resolution);
	RenderTarget->UpdateResourceImmediate(true);

	UE_LOG(LogTemp, Log, TEXT("Created Planet RenderTarget: %s [%p]"), *UniqueName, RenderTarget);

	return RenderTarget;
}


UTexture2D* PlanetUtils::LoadPlanetAssetTexture(const FString& TextureName)
{
	FString AssetPath = FString::Printf(TEXT("/Game/GameData/Galaxy/PlanetMaterials/%s.%s"), *TextureName, *TextureName);

	UTexture2D* Texture = Cast<UTexture2D>(StaticLoadObject(UTexture2D::StaticClass(), nullptr, *AssetPath));
	if (!Texture)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to load planet texture asset: %s"), *AssetPath);
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("Loaded planet texture asset: %s"), *Texture->GetName());
	}

	return Texture;
}

int32 PlanetUtils::GetRenderTargetResolutionForRadius(double RadiusKm)
{
	const double MinRadius = 1000.0;      // Small asteroid
	const double MaxRadius = 150000.0;    // Gas giant

	// Normalize with optional logarithmic scaling
	RadiusKm = FMath::Clamp(RadiusKm, MinRadius, MaxRadius);

	const double LogMin = FMath::LogX(10.0, MinRadius);
	const double LogMax = FMath::LogX(10.0, MaxRadius);
	const double LogVal = FMath::LogX(10.0, RadiusKm);
	double T = (LogVal - LogMin) / (LogMax - LogMin); // 0.0 - 1.0

	// Map T to a range of powers of 2: [128, 256, 512, 1024]
	T = FMath::Clamp(T, 0.0, 1.0);
	const TArray<int32> ResOptions = { 128, 256, 512, 1024 };

	int32 Index = FMath::FloorToInt(T * (ResOptions.Num() - 1));
	return ResOptions[Index];
}

float PlanetUtils::GetPlanetUIScale(double RadiusKm)
{
	// Min and max expected radius (from Galaxy.def or design)
	constexpr double MinRadius = 2000.0;     // Small moons
	constexpr double MaxRadius = 150000.0;   // Large gas giants

	// Get normalized [0.0, 1.0] range
	double Normalized = FMath::Clamp((RadiusKm - MinRadius) / (MaxRadius - MinRadius), 0.0, 1.0);

	// UI scaling range — safe for mesh scale and texture logic
	constexpr float MinUIScale = 0.5f;
	constexpr float MaxUIScale = 4.0f;

	// Final safe UI scale
	float UIScale = FMath::Lerp(MinUIScale, MaxUIScale, static_cast<float>(Normalized));

	// Clamp final value to hard max
	return FMath::Clamp(UIScale, MinUIScale, MaxUIScale);
}

FRotator PlanetUtils::GetPlanetAxisTilt(float TiltDegrees)
{
	// Clamp tilt to reasonable real-world range (-90 to +90)
	float ClampedTilt = FMath::Clamp(TiltDegrees, -90.0f, 90.0f);

	// Apply tilt to pitch (or roll if rotating sideways)
	// This assumes planet spins around Y-axis (roll)
	return FRotator(0.0f, 0.0f, ClampedTilt);
}

FRotator PlanetUtils::GetPlanetRotation(float TimeSeconds, float RotationSpeedDegreesPerSec, float TiltDegrees)
{
	// Clamp tilt for safety
	float ClampedTilt = FMath::Clamp(TiltDegrees, -90.0f, 90.0f);

	// Compute current Yaw based on speed and time
	float CurrentYaw = FMath::Fmod(TimeSeconds * RotationSpeedDegreesPerSec, 360.0f);

	// Rotation = spin (Yaw) combined with axis tilt (Roll or Pitch)
	return FRotator(0.0f, CurrentYaw, ClampedTilt); // (Pitch, Yaw, Roll)
}

float PlanetUtils::GetNormalizedPlanetUIScale(double RadiusKm)
{
	// Based on actual Galaxy.def data
	constexpr double MinRadius = 3.15e6;    // Relay
	constexpr double MaxRadius = 38.2e6;    // Tal Amin

	// Clamp input radius to valid bounds
	double Clamped = FMath::Clamp(RadiusKm, MinRadius, MaxRadius);

	// Normalize to 0.0 - 1.0 range
	float Normalized = static_cast<float>((Clamped - MinRadius) / (MaxRadius - MinRadius));

	// Optional: Map to a display-friendly scale range
	constexpr float MinScale = 1.0f;
	constexpr float MaxScale = 4.0f;
	return FMath::Lerp(MinScale, MaxScale, Normalized);
}