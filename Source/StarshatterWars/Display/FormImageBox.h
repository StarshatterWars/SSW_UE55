/*  Project Starshatter Wars
    Fractal Dev Studios
    Copyright (C) 2025-2026. All Rights Reserved.

    SUBSYSTEM:    Stars.exe (UE)
    FILE:         FormImageBox.h
    AUTHOR:       Carlos Bott

    ORIGINAL AUTHOR AND STUDIO
    ==========================
    John DiCamillo / Destroyer Studios LLC

    OVERVIEW
    ========
    UE port of legacy FormImageBox control.
*/

#pragma once

#include "CoreMinimal.h"
#include "Components/Widget.h"
#include "Styling/SlateBrush.h"
#include "FormImageBox.generated.h"

class SImage;

UCLASS(BlueprintType, Blueprintable)
class STARSHATTERWARS_API UFormImageBox : public UWidget
{
    GENERATED_BODY()

public:
    UFormImageBox(const FObjectInitializer& ObjectInitializer);

    /** Legacy-like API */
    UFUNCTION(BlueprintCallable, Category = "Form|ImageBox")
    void SetTexture(UTexture2D* InTexture);

    UFUNCTION(BlueprintCallable, Category = "Form|ImageBox")
    void SetBrush(const FSlateBrush& InBrush);

    UFUNCTION(BlueprintCallable, Category = "Form|ImageBox")
    UTexture2D* GetTexture() const { return Texture; }

public:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Form|ImageBox")
    UTexture2D* Texture = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Form|ImageBox")
    FSlateBrush Brush;

protected:
    // UWidget
    virtual TSharedRef<SWidget> RebuildWidget() override;
    virtual void SynchronizeProperties() override;

private:
    const FSlateBrush* GetSlateBrush() const;

private:
    TSharedPtr<SImage> Image;
};

