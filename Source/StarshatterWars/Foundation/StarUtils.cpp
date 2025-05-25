// /*  Project nGenEx	Fractal Dev Games	Copyright (C) 2024. All Rights Reserved.	SUBSYSTEM:    SSW	FILE:         Game.cpp	AUTHOR:       Carlos Bott*/


#include "StarUtils.h"
#include "Engine/TextureRenderTarget2D.h"

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

FString StarUtils::GetClassName(ESPECTRAL_CLASS Class)
{
	const UEnum* EnumPtr = StaticEnum<ESPECTRAL_CLASS>();
	if (!EnumPtr) return FString("Unknown");

	return EnumPtr->GetDisplayNameTextByValue(static_cast<int64>(Class)).ToString();
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

UTextureRenderTarget2D* StarUtils::CreateRenderTarget(const FString& Name, UObject* Outer)
{
	if (!Outer) return nullptr;

	UTextureRenderTarget2D* RT = NewObject<UTextureRenderTarget2D>(Outer, *Name);
	if (RT)
	{
		RT->RenderTargetFormat = RTF_RGBA8;
		RT->ClearColor = FLinearColor::Black;
		RT->InitAutoFormat(512, 512);
		RT->UpdateResourceImmediate(true);

		UE_LOG(LogTemp, Log, TEXT("Stellar RenderTarget created: %s"), *Name);
	}
	return RT;
}