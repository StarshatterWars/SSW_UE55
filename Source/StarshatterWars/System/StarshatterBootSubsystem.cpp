#include "StarshatterBootSubsystem.h"

#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "EngineUtils.h"

// Subsystems
#include "FontManagerSubsystem.h"
#include "StarshatterAudioSubsystem.h"
#include "StarshatterVideoSubsystem.h"
#include "StarshatterControlsSubsystem.h"
#include "StarshatterKeyboardSubsystem.h"
#include "StarshatterSettingsSaveSubsystem.h"

// SaveGame
#include "StarshatterSettingsSaveGame.h"

// World service actor
#include "GameDataLoader.h"

// --------------------------------------------------
// UGameInstanceSubsystem
// --------------------------------------------------

void UStarshatterBootSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    FBootContext Ctx;
    if (BuildContext(Ctx))
    {
        BootFonts(Ctx);
        BootAudio(Ctx);
        BootVideo(Ctx);
        BootControls(Ctx);
        BootKeyboard(Ctx);
    }

    // World-dependent boot (actors, UWorld-only systems)
    HookWorldBoot();
}

void UStarshatterBootSubsystem::Deinitialize()
{
    if (PostWorldInitHandle.IsValid())
    {
        FWorldDelegates::OnPostWorldInitialization.Remove(PostWorldInitHandle);
        PostWorldInitHandle.Reset();
    }

    Super::Deinitialize();
}

// --------------------------------------------------
// GameInstance boot
// --------------------------------------------------

bool UStarshatterBootSubsystem::BuildContext(FBootContext& OutCtx)
{
    OutCtx.GI = GetGameInstance();
    if (!OutCtx.GI)
        return false;

    OutCtx.SaveSS = OutCtx.GI->GetSubsystem<UStarshatterSettingsSaveSubsystem>();
    if (OutCtx.SaveSS)
    {
        OutCtx.SaveSS->LoadOrCreate();
        OutCtx.SG = OutCtx.SaveSS->GetSettings();
    }

    OutCtx.FontSS = OutCtx.GI->GetSubsystem<UFontManagerSubsystem>();
    OutCtx.AudioSS = OutCtx.GI->GetSubsystem<UStarshatterAudioSubsystem>();
    OutCtx.VideoSS = OutCtx.GI->GetSubsystem<UStarshatterVideoSubsystem>();
    OutCtx.ControlsSS = OutCtx.GI->GetSubsystem<UStarshatterControlsSubsystem>();
    OutCtx.KeyboardSS = OutCtx.GI->GetSubsystem<UStarshatterKeyboardSubsystem>();

    return true;
}

void UStarshatterBootSubsystem::BootFonts(const FBootContext& Ctx)
{
    if (!Ctx.FontSS)
        return;

    // Reserved for future font config hooks
}

void UStarshatterBootSubsystem::BootAudio(const FBootContext& Ctx)
{
    if (!Ctx.AudioSS)
        return;

    if (Ctx.SG)
        Ctx.AudioSS->LoadFromSaveGame(Ctx.SG);

    Ctx.AudioSS->ApplySettingsToRuntime();
}

void UStarshatterBootSubsystem::BootVideo(const FBootContext& Ctx)
{
    if (!Ctx.VideoSS)
        return;

    if (Ctx.SG)
        Ctx.VideoSS->LoadFromSaveGame(Ctx.SG);
    else
        Ctx.VideoSS->LoadVideoConfig(TEXT("video.cfg"), true);

    Ctx.VideoSS->ApplySettingsToRuntime();
}

void UStarshatterBootSubsystem::BootControls(const FBootContext& Ctx)
{
    if (!Ctx.ControlsSS)
        return;

    if (Ctx.SG)
        Ctx.ControlsSS->LoadFromSaveGame(Ctx.SG);

    Ctx.ControlsSS->ApplySettingsToRuntime(this);
}

void UStarshatterBootSubsystem::BootKeyboard(const FBootContext& Ctx)
{
    if (!Ctx.KeyboardSS)
        return;

    if (Ctx.SG)
        Ctx.KeyboardSS->LoadFromSaveGame(Ctx.SG);

    Ctx.KeyboardSS->ApplySettingsToRuntime(this);
}

// --------------------------------------------------
// World boot
// --------------------------------------------------

void UStarshatterBootSubsystem::HookWorldBoot()
{
    if (PostWorldInitHandle.IsValid())
        return;

    PostWorldInitHandle =
        FWorldDelegates::OnPostWorldInitialization.AddUObject(
            this, &UStarshatterBootSubsystem::OnPostWorldInit);
}

void UStarshatterBootSubsystem::OnPostWorldInit(
    UWorld* World,
    const UWorld::InitializationValues)
{
    if (bWorldBootDone || !World)
        return;

    if (World->WorldType != EWorldType::Game &&
        World->WorldType != EWorldType::PIE)
        return;

    BootGameData(World);
    bWorldBootDone = true;
}

void UStarshatterBootSubsystem::BootGameData(UWorld* World)
{
    if (!World)
        return;

    // Reuse if already present
    for (TActorIterator<AGameDataLoader> It(World); It; ++It)
    {
        return;
    }

    FActorSpawnParameters Params;
    Params.Name = TEXT("Game Data");
    Params.SpawnCollisionHandlingOverride =
        ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

    World->SpawnActor<AGameDataLoader>(
        AGameDataLoader::StaticClass(),
        FVector::ZeroVector,
        FRotator::ZeroRotator,
        Params);
}
