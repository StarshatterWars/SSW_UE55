/*  Project Starshatter Wars
    Fractal Dev Studios
    Copyright © 2025-2026.

    SUBSYSTEM:    StarshatterWars (UE)
    FILE:         Sound.h
    AUTHOR:       Carlos Bott

    OVERVIEW
    ========
    Unreal Sound handler that preserves legacy Sound API.
*/

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "UObject/SoftObjectPtr.h"
#include "Sound/SoundBase.h"
#include "Components/AudioComponent.h"
#include "Sound.generated.h"

class SoundCard;   // legacy placeholder (kept for API compatibility)
class Camera;      // legacy placeholder (kept for API compatibility)

UCLASS(BlueprintType)
class STARSHATTERWARS_API USound : public UObject
{
    GENERATED_BODY()

public:
    static const char* TYPENAME() { return "Sound"; }

    // Legacy-like factories (now load USoundBase assets):
    static USound* CreateStream(const char* InFilename);
    static USound* CreateOggStream(const char* InFilename);

    // Legacy-ish listener hook (UE manages listener via PlayerController/CameraManager).
    // Kept for compatibility; stored but not forced into AudioDevice.
    static void SetListener(const Camera& /*cam*/, const FVector& InVel);

    // Legacy "sound card" hook; in UE this is just a world/context provider.
    static void UseSoundCard(SoundCard* s) { creator = s; }

public:
    USound();

    // once per frame:
    virtual void Update();

    // mark for collection:
    virtual void Release();

    // data loading (legacy stubs; use assets in UE):
    virtual HRESULT StreamFile(const char* /*name*/, DWORD /*offset*/) { return E_NOINTERFACE; }
    virtual HRESULT Load(DWORD /*bytes*/, BYTE* /*data*/) { return E_NOINTERFACE; }
    virtual USound* Duplicate() { return nullptr; }

    // transport operations:
    virtual HRESULT Play();
    virtual HRESULT Rewind();
    virtual HRESULT Pause();
    virtual HRESULT Stop();

public:
    int  IsReady()   const { return status == READY; }
    int  IsPlaying() const { return status == PLAYING; }
    int  IsDone()    const { return status == DONE; }
    int  LoopCount() const { return looped; }

    virtual DWORD GetFlags()  const { return flags; }
    virtual void  SetFlags(DWORD f) { flags = f; }
    virtual DWORD GetStatus() const { return status; }

    // Legacy volume is "centibels" (0 .. -10000). We convert to UE volume multiplier.
    virtual long GetVolume() const { return volume; }
    virtual void SetVolume(long v);

    // Pan is not directly supported on AudioComponent without submix/SourceEffect.
    // Keeping stubs for compatibility:
    virtual long GetPan() const { return 0; }
    virtual void SetPan(long /*p*/) {}

    // Stream time accessors not implemented for asset-based playback:
    virtual double GetTotalTime()     const { return 0; }
    virtual double GetTimeRemaining() const { return 0; }
    virtual double GetTimeElapsed()   const { return 0; }

    // Localized 3D controls:
    virtual const FVector& GetLocation() const { return location; }
    virtual void SetLocation(const FVector& l);

    virtual const FVector& GetVelocity() const { return velocity; }
    virtual void SetVelocity(const FVector& v) { velocity = v; }

    virtual float GetMinDistance() const { return min_distance; }
    virtual void  SetMinDistance(float f) { min_distance = f; ApplyAttenuation(); }

    virtual float GetMaxDistance() const { return max_distance; }
    virtual void  SetMaxDistance(float f) { max_distance = f; ApplyAttenuation(); }

    virtual void  SetSoundCheck(class SoundCheck* s) { sound_check = s; }
    virtual void  AddToSoundCard(); // in UE, ensures we have a valid World/Context

    const char* GetFilename() const { return filename_ansi; }
    void        SetFilename(const char* s);

public:
    enum FlagEnum : uint32
    {
        AMBIENT = 0x0000,
        LOCALIZED = 0x0001,
        LOC_3D = 0x0002,
        MEMORY = 0x0000,
        STREAMED = 0x0004,
        ONCE = 0x0000,
        LOOP = 0x0008,
        FREE = 0x0000,
        LOCKED = 0x0010,
        DOPPLER = 0x0020,
        INTERFACE = 0x0040,
        OGGVORBIS = 0x4000,
        RESOURCE = 0x8000
    };

    enum StatusEnum : uint32
    {
        UNINITIALIZED,
        INITIALIZING,
        READY,
        PLAYING,
        DONE
    };

private:
    // Core UE asset + component:
    UPROPERTY(Transient)
    TObjectPtr<USoundBase> SoundAsset = nullptr;

    UPROPERTY(Transient)
    TObjectPtr<UAudioComponent> AudioComp = nullptr;

    // Attenuation (optional, built on the fly):
    UPROPERTY(Transient)
    TObjectPtr<class USoundAttenuation> AttenuationAsset = nullptr;

private:
    void EnsureAssetLoaded();
    void EnsureAudioComponent();
    void ApplyAttenuation();

    static UObject* ResolveWorldContext(); // best-effort context resolution
    static USoundBase* LoadSoundAssetFromString(const FString& InRef);

    static float CentibelsToVolumeMultiplier(long Centibels);

private:
    DWORD flags = 0;
    DWORD status = UNINITIALIZED;
    long  volume = 0;       // centibels (0 .. -10000)
    int   looped = 0;

    FVector location = FVector::ZeroVector;
    FVector velocity = FVector::ZeroVector;

    float min_distance = 200.f;
    float max_distance = 4000.f;

    class SoundCheck* sound_check = nullptr;

    // Keep filename exactly like legacy:
    char filename_ansi[64] = { 0 };

    static SoundCard* creator;          // legacy hook (context provider in UE)
    static FVector    ListenerVelocity; // stored for compatibility
};

// +--------------------------------------------------------------------+

class SoundCheck
{
public:
    virtual void Update(USound* /*s*/) {}
};
