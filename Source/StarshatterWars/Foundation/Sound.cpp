#include "Sound.h"

#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundAttenuation.h"
#include "UObject/SoftObjectPath.h"

// Unreal logging:
#include "Logging/LogMacros.h"

DEFINE_LOG_CATEGORY_STATIC(LogStarshatterSound, Log, All);

SoundCard* USound::creator = nullptr;
FVector    USound::ListenerVelocity = FVector::ZeroVector;

USound::USound()
{
    status = UNINITIALIZED;
    volume = 0;
    flags = 0;
    looped = 0;
    location = FVector::ZeroVector;
    velocity = FVector::ZeroVector;
    sound_check = nullptr;

    // legacy default:
    FCStringAnsi::Strcpy(filename_ansi, "Sound()");
}

void USound::SetListener(const Camera& /*cam*/, const FVector& InVel)
{
    // UE listener is owned by PlayerController/AudioDevice.
    // We keep this for legacy compatibility and Doppler bookkeeping if you later extend.
    ListenerVelocity = InVel;
}

void USound::SetFilename(const char* s)
{
    if (!s) return;

    int n = (int)FCStringAnsi::Strlen(s);
    if (n >= 60)
    {
        FMemory::Memzero(filename_ansi, sizeof(filename_ansi));
        FCStringAnsi::Strcpy(filename_ansi, "...");
        FCStringAnsi::Strcat(filename_ansi, s + n - 59);
        filename_ansi[63] = 0;
    }
    else
    {
        FCStringAnsi::Strcpy(filename_ansi, s);
    }
}

float USound::CentibelsToVolumeMultiplier(long Centibels)
{
    // legacy: 0 .. -10000 centibels
    // UE: linear multiplier
    // dB = centibels / 100
    // linear = 10^(dB/20)
    const float dB = (float)Centibels / 100.0f;
    const float linear = FMath::Pow(10.0f, dB / 20.0f);
    return FMath::Clamp(linear, 0.0f, 10.0f);
}

void USound::SetVolume(long v)
{
    volume = v;
    if (AudioComp)
    {
        AudioComp->SetVolumeMultiplier(CentibelsToVolumeMultiplier(volume));
    }
}

void USound::SetLocation(const FVector& l)
{
    location = l;
    if (AudioComp)
    {
        AudioComp->SetWorldLocation(location);
    }
}

UObject* USound::ResolveWorldContext()
{
    // Best-effort:
    // - If you later implement SoundCard as a UObject provider, use it here.
    // - Otherwise fall back to first game world.
    if (GEngine)
    {
        for (const FWorldContext& Ctx : GEngine->GetWorldContexts())
        {
            if (Ctx.World() && (Ctx.WorldType == EWorldType::Game || Ctx.WorldType == EWorldType::PIE))
            {
                return Ctx.World();
            }
        }
    }
    return nullptr;
}

USoundBase* USound::LoadSoundAssetFromString(const FString& InRef)
{
    // Accept either:
    // - Soft object path: "/Game/Audio/UI_Click.UI_Click"
    // - Or a package path that StaticLoadObject can resolve
    if (InRef.IsEmpty())
        return nullptr;

    // Prefer soft load:
    {
        FSoftObjectPath SoftPath(InRef);
        if (SoftPath.IsValid())
        {
            UObject* Obj = SoftPath.TryLoad();
            return Cast<USoundBase>(Obj);
        }
    }

    // Fallback: StaticLoadObject
    return Cast<USoundBase>(StaticLoadObject(USoundBase::StaticClass(), nullptr, *InRef));
}

void USound::EnsureAssetLoaded()
{
    if (SoundAsset)
        return;

    const FString Ref = FString(ANSI_TO_TCHAR(filename_ansi));
    SoundAsset = LoadSoundAssetFromString(Ref);

    if (!SoundAsset)
    {
        UE_LOG(LogStarshatterSound, Warning,
            TEXT("USound: Failed to load sound asset from ref '%s' (filename='%s')."),
            *Ref, ANSI_TO_TCHAR(filename_ansi));
        status = DONE;
        return;
    }

    // Looping: UE typically encodes looping in SoundCue/SoundWave settings.
    // We can force looping on the component when we create it.
    status = READY;
}

void USound::ApplyAttenuation()
{
    if (!(flags & LOCALIZED) && !(flags & LOC_3D))
        return;

    if (!AttenuationAsset)
    {
        AttenuationAsset = NewObject<USoundAttenuation>(this);
    }

    if (AttenuationAsset)
    {
        FSoundAttenuationSettings& S = AttenuationAsset->Attenuation;
        S.bAttenuate = true;
        S.bSpatialize = true;

        // Use simple distance model:
        S.FalloffDistance = FMath::Max(0.0f, max_distance - min_distance);
        S.AttenuationShape = EAttenuationShape::Sphere;
        S.AttenuationShapeExtents = FVector(FMath::Max(1.0f, max_distance), 0, 0);

        // Min distance approximation:
        S.dBAttenuationAtMax = -60.0f;

        if (AudioComp)
        {
            AudioComp->AttenuationSettings = AttenuationAsset;
        }
    }
}

void USound::EnsureAudioComponent()
{
    if (AudioComp)
        return;

    UObject* WorldCtxObj = ResolveWorldContext();
    UWorld* World = WorldCtxObj ? WorldCtxObj->GetWorld() : nullptr;

    if (!World)
    {
        UE_LOG(LogStarshatterSound, Warning, TEXT("USound: No valid UWorld context to spawn audio."));
        status = DONE;
        return;
    }

    // Decide 2D vs 3D:
    const bool bIs3D = (flags & LOCALIZED) || (flags & LOC_3D);
    const bool bIsUI = (flags & INTERFACE);

    // Spawn:
    if (bIs3D)
    {
        AudioComp = UGameplayStatics::SpawnSoundAtLocation(
            World,
            SoundAsset,
            location,
            FRotator::ZeroRotator,
            CentibelsToVolumeMultiplier(volume)
        );
    }
    else
    {
        AudioComp = UGameplayStatics::SpawnSound2D(
            World,
            SoundAsset,
            CentibelsToVolumeMultiplier(volume)
        );
    }

    if (!AudioComp)
    {
        UE_LOG(LogStarshatterSound, Warning, TEXT("USound: Failed to spawn AudioComponent."));
        status = DONE;
        return;
    }

    // UI sounds: you can route to a UI sound class/submix later.
    (void)bIsUI;

    // Looping:
    const bool bLoop = (flags & LOOP) != 0;
    AudioComp->bIsUISound = bIsUI;
    AudioComp->bAutoDestroy = !bLoop; // looped sounds shouldn't auto-destroy
    AudioComp->SetVolumeMultiplier(CentibelsToVolumeMultiplier(volume));

    if (bLoop)
    {
        // This only works reliably if the underlying asset supports looping.
        // For SoundCue, add a Looping node; for SoundWave, enable looping in asset.
        AudioComp->Play();
        looped = 1;
    }

    ApplyAttenuation();
}

void USound::AddToSoundCard()
{
    // In UE, we don’t have a "sound card" list. This is a compatibility hook.
    // We ensure asset is loaded so the sound is READY.
    EnsureAssetLoaded();
}

USound* USound::CreateStream(const char* InFilename)
{
    if (!InFilename || !InFilename[0])
        return nullptr;

    USound* S = NewObject<USound>(GetTransientPackage());
    S->SetFilename(InFilename);

    // Decide ogg by extension (legacy behavior); both paths load assets in UE:
    const int32 Len = FCStringAnsi::Strlen(InFilename);
    if (Len >= 4)
    {
        const char c1 = InFilename[Len - 3];
        const char c2 = InFilename[Len - 2];
        const char c3 = InFilename[Len - 1];

        const bool bIsOgg =
            (c1 == 'o' || c1 == 'O') &&
            (c2 == 'g' || c2 == 'G') &&
            (c3 == 'g' || c3 == 'G');

        if (bIsOgg)
        {
            return CreateOggStream(InFilename);
        }
    }

    S->status = INITIALIZING;
    S->EnsureAssetLoaded();
    return S->IsReady() ? S : nullptr;
}

USound* USound::CreateOggStream(const char* InFilename)
{
    if (!InFilename || !InFilename[0])
        return nullptr;

    // UE-native approach:
    // - Import .ogg into Content -> becomes USoundWave
    // - Pass soft object path to that asset here.
    USound* S = NewObject<USound>(GetTransientPackage());
    S->SetFilename(InFilename);

    S->flags |= OGGVORBIS;
    S->status = INITIALIZING;
    S->EnsureAssetLoaded();
    return S->IsReady() ? S : nullptr;
}

void USound::Update()
{
    if (sound_check)
    {
        sound_check->Update(this);
    }

    if (AudioComp)
    {
        // Track state:
        if (AudioComp->IsPlaying())
            status = PLAYING;
        else if (status == PLAYING)
            status = DONE;

        // Keep 3D position updated:
        if ((flags & LOCALIZED) || (flags & LOC_3D))
        {
            AudioComp->SetWorldLocation(location);
        }
    }
}

void USound::Release()
{
    flags &= ~LOCKED;

    // In UE, if not locked, you can stop and allow GC:
    if (!(flags & LOCKED))
    {
        Stop();
    }
}

HRESULT USound::Play()
{
    EnsureAssetLoaded();
    if (!SoundAsset)
        return E_FAIL;

    EnsureAudioComponent();
    if (!AudioComp)
        return E_FAIL;

    const bool bLoop = (flags & LOOP) != 0;
    AudioComp->bAutoDestroy = !bLoop;

    AudioComp->Play();
    status = PLAYING;
    return S_OK;
}

HRESULT USound::Pause()
{
    if (!AudioComp)
        return E_NOINTERFACE;

    AudioComp->SetPaused(true);
    status = READY;
    return S_OK;
}

HRESULT USound::Stop()
{
    if (AudioComp)
    {
        AudioComp->Stop();
        AudioComp = nullptr;
    }

    status = DONE;
    return S_OK;
}

HRESULT USound::Rewind()
{
    if (!AudioComp)
        return E_NOINTERFACE;

    AudioComp->Stop();
    AudioComp->Play(0.0f);
    status = READY;
    return S_OK;
}
