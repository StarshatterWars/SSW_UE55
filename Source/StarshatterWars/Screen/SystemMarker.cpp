// /*  Project nGenEx	Fractal Dev Games	Copyright (C) 2024. All Rights Reserved.	SUBSYSTEM:    SSW	FILE:         Game.cpp	AUTHOR:       Carlos Bott*/


#include "SystemMarker.h"

FString EnumToString(ESPECTRAL_CLASS Class) {
    const UEnum* EnumPtr = StaticEnum<ESPECTRAL_CLASS>();
    if (!EnumPtr) return "Invalid";

    return EnumPtr->GetNameStringByValue((int64)Class);
}

void USystemMarker::Init(const FS_Galaxy& System, const TMap<FString, UTexture2D*>& StarTextures)
{
    if (!StarImage) return;

    SetToolTipText(FText::FromString(System.Name));

    if (SystemNameText) {
        SystemNameText->SetText(FText::FromString(System.Name));
    }

    // Get the star class as string key
    FString ClassKey = EnumToString(System.Class);
   
   // Star texture by class
    if (StarTextures.Contains(ClassKey)) {
        UTexture2D* Texture = StarTextures[ClassKey];
        if (Texture) {
            StarImage->SetBrushFromTexture(Texture);
        }
    }

    // Optional: Add border color by faction
    FLinearColor Tint;

    switch (System.Iff) {
    case 1: Tint = FLinearColor::Blue; break;
    case 2: Tint = FLinearColor::Red; break;
    default: Tint = FLinearColor::Gray; break;
    }

    if (IffImage) {
        IffImage->SetColorAndOpacity(Tint);
    }
}
