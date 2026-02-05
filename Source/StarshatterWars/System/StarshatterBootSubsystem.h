#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "StarshatterBootSubsystem.generated.h"

UCLASS()
class STARSHATTERWARS_API UStarshatterBootSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

private:
    void BootSettings();
    void BootFonts();
    void BootAudio();
    void BootVideo();
    void BootControls();
    void BootKeyboard();
};
