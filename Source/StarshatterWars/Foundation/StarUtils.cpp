// /*  Project nGenEx	Fractal Dev Games	Copyright (C) 2024. All Rights Reserved.	SUBSYSTEM:    SSW	FILE:         Game.cpp	AUTHOR:       Carlos Bott*/


#include "StarUtils.h"
#include "Engine/TextureRenderTarget2D.h"
#include "UObject/EnumProperty.h" // often not required, but safe in some setups

FLinearColor StarUtils::GetColor(ESPECTRAL_CLASS SpectralClass)
{
	
	FLinearColor StarColor; 

	switch (SpectralClass)
	{
	case ESPECTRAL_CLASS::O: StarColor = FLinearColor(0.5f, 0.6f, 1.0f); break;
	case ESPECTRAL_CLASS::B: StarColor = FLinearColor(0.7f, 0.7f, 1.0f); break;
	case ESPECTRAL_CLASS::A: StarColor = FLinearColor(0.9f, 0.9f, 1.0f); break;
	case ESPECTRAL_CLASS::F: StarColor = FLinearColor(1.0f, 1.0f, 0.9f); break;
	case ESPECTRAL_CLASS::G: StarColor = FLinearColor(1.0f, 0.95f, 0.6f); break;
	case ESPECTRAL_CLASS::K: StarColor = FLinearColor(1.0f, 0.6f, 0.3f); break;
	case ESPECTRAL_CLASS::M: StarColor = FLinearColor(1.0f, 0.2f, 0.1f); break;
	case ESPECTRAL_CLASS::R: StarColor = FLinearColor(0.9f, 0.1f, 0.1f); break;
	case ESPECTRAL_CLASS::N: StarColor = FLinearColor(0.75f, 0.05f, 0.05f); break;
	case ESPECTRAL_CLASS::S: StarColor = FLinearColor(0.95f, 0.4f, 0.2f); break;
	case ESPECTRAL_CLASS::BLACK_HOLE: StarColor = FLinearColor::Black; break;
	case ESPECTRAL_CLASS::WHITE_DWARF: StarColor = FLinearColor::White; break;
	case ESPECTRAL_CLASS::RED_GIANT: StarColor = FLinearColor(1.0f, 0.5f, 0.3f); break;
	default: StarColor = FLinearColor::Gray; break;
	}

	UE_LOG(LogTemp, Warning, TEXT("SpectralClass = %s, StarColor = R=%.2f G=%.2f B=%.2f"),
		*UEnum::GetValueAsString(SpectralClass),
		StarColor.R, StarColor.G, StarColor.B);

	return StarColor;
}

float StarUtils::GetGlowStrength(ESPECTRAL_CLASS Class)
{
	switch (Class)
	{
	case ESPECTRAL_CLASS::O: return 60.f;
	case ESPECTRAL_CLASS::B: return 48.f;
	case ESPECTRAL_CLASS::A: return 36.f;
	case ESPECTRAL_CLASS::F: return 28.f;
	case ESPECTRAL_CLASS::G: return 20.f;
	case ESPECTRAL_CLASS::K: return 12.f;
	case ESPECTRAL_CLASS::M: return 8.f;
	case ESPECTRAL_CLASS::WHITE_DWARF: return 15.f;
	case ESPECTRAL_CLASS::RED_GIANT:   return 25.f;
	default: return 10.f;
	}
}

float StarUtils::GetSunspotStrength(ESPECTRAL_CLASS Class)
{
	switch (Class)
	{
	case ESPECTRAL_CLASS::O: return 0.0f;
	case ESPECTRAL_CLASS::G: return 0.3f;
	case ESPECTRAL_CLASS::M: return 0.6f;
	case ESPECTRAL_CLASS::RED_GIANT: return 0.5f;
	default: return 0.2f;
	}
}

FString StarUtils::GetSystemClassName(ESPECTRAL_CLASS Class)
{
	const UEnum* EnumPtr = StaticEnum<ESPECTRAL_CLASS>();
	if (!EnumPtr)
	{
		return TEXT("Unknown");
	}

	const int64 Value = static_cast<int64>(Class);

	// Prefer DisplayName if present
	const FText DisplayText = EnumPtr->GetDisplayNameTextByValue(Value);
	if (!DisplayText.IsEmpty())
	{
		return DisplayText.ToString();
	}

	// Fallback to raw enum name (e.g. "G2V")
	return EnumPtr->GetNameStringByValue(Value);
}

float StarUtils::GetRotationSpeed(float Rotation)
{
	return 60.f / FMath::Max(Rotation, 0.1f);
}

float StarUtils::GetUISizeFromRadius(float Radius, float MinSize, float MaxSize)
{
	constexpr float MinRadiusKm = 1.2e9f;
	constexpr float MaxRadiusKm = 2.2e9f;

	float LogRadius = FMath::LogX(10.f, FMath::Max(Radius, 1.f));
	float MinLog = FMath::LogX(10.f, MinRadiusKm);
	float MaxLog = FMath::LogX(10.f, MaxRadiusKm);

	float Normalized = FMath::Clamp((LogRadius - MinLog) / (MaxLog - MinLog), 0.f, 1.f);
	return FMath::Lerp(MinSize, MaxSize, Normalized);
}

UTextureRenderTarget2D* StarUtils::CreateStarRenderTarget(const FString& Name, UObject* Outer, int32 Resolution)
{
	if (!Outer)
	{
		Outer = GetTransientPackage(); // fallback
	}

	UTextureRenderTarget2D* RenderTarget = NewObject<UTextureRenderTarget2D>(Outer, *Name);
	if (!RenderTarget)
	{
		return nullptr;
	}

	RenderTarget->RenderTargetFormat = RTF_RGBA16f; // good for lit previews
	RenderTarget->ClearColor = FLinearColor::Black;
	RenderTarget->bAutoGenerateMips = false;
	RenderTarget->InitAutoFormat(Resolution, Resolution);
	RenderTarget->UpdateResourceImmediate(true);

	return RenderTarget;
}

int32 StarUtils::GetRenderTargetResolutionForRadius(double RadiusKm)
{
	constexpr float MinRadiusKm = 1.2e9f;
	constexpr float MaxRadiusKm = 2.2e9f;

	// Normalize with optional logarithmic scaling
	RadiusKm = FMath::Clamp(RadiusKm, MinRadiusKm, MaxRadiusKm);

	const double LogMin = FMath::LogX(10.0, MinRadiusKm);
	const double LogMax = FMath::LogX(10.0, MaxRadiusKm);
	const double LogVal = FMath::LogX(10.0, RadiusKm);
	double T = (LogVal - LogMin) / (LogMax - LogMin); // 0.0 - 1.0

	// Map T to a range of powers of 2: [128, 256, 512, 1024]
	T = FMath::Clamp(T, 0.0, 1.0);
	const TArray<int32> ResOptions = { 128, 256, 512, 1024 };

	int32 Index = FMath::FloorToInt(T * (ResOptions.Num() - 1));
	return ResOptions[Index];
}

UTexture2D* StarUtils::LoadStarAssetTexture(const FString& TextureName)
{
	FString AssetPath = FString::Printf(TEXT("/Game/GameData/Galaxy/StarMaterials/%s.%s"), *TextureName, *TextureName);

	UTexture2D* Texture = Cast<UTexture2D>(StaticLoadObject(UTexture2D::StaticClass(), nullptr, *AssetPath));
	if (!Texture)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to load star texture asset: %s"), *AssetPath);
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("Loaded star texture asset: %s"), *Texture->GetName());
	}

	return Texture;
}