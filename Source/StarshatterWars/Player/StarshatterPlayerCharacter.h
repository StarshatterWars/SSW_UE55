#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "GameStructs.h"
#include "StarshatterPlayerCharacter.generated.h"

class UStarshatterPlayerSubsystem;

UCLASS(BlueprintType)
class STARSHATTERWARS_API UStarshatterPlayerCharacter : public UObject
{
    GENERATED_BODY()

public:
    // Subsystem sets this after NewObject<>() so this object can save without hacks.
    void Initialize(UStarshatterPlayerSubsystem* InOwnerSubsystem);

    // ----------------------------
    // Identity / display
    // ----------------------------
    UFUNCTION(BlueprintPure, Category = "Starshatter|Player")
    int32 GetIdentity() const { return PlayerId; }

    UFUNCTION(BlueprintPure, Category = "Starshatter|Player")
    const FString& GetPlayerName() const { return PlayerName; }

    UFUNCTION(BlueprintCallable, Category = "Starshatter|Player")
    void SetPlayerName(const FString& InName);

    // ----------------------------
    // Awards (your registry-driven lookups)
    // ----------------------------
    UFUNCTION(BlueprintPure, Category = "Starshatter|Player|Awards")
    bool ShowAward() const { return PendingAwardId != 0; }

    UFUNCTION(BlueprintPure, Category = "Starshatter|Player|Awards")
    FString GetAwardName() const;

    UFUNCTION(BlueprintPure, Category = "Starshatter|Player|Awards")
    FString GetAwardDesc() const;

    UFUNCTION(BlueprintCallable, Category = "Starshatter|Player|Awards")
    void ClearShowAward();

    // ----------------------------
    // Save / load sync
    // ----------------------------
    void FromPlayerInfo(const FS_PlayerGameInfo& InInfo);
    void ToPlayerInfo(FS_PlayerGameInfo& OutInfo) const;

    // “Commit” writes changes into subsystem + saves.
    UFUNCTION(BlueprintCallable, Category = "Starshatter|Player")
    bool Commit(bool bForceSave = true);

private:
    // Not UPROPERTY on purpose (subsystem owns us; we just reference it).
    // If you ever want hot-reload safety, make it TWeakObjectPtr.
    UStarshatterPlayerSubsystem* OwnerSubsystem = nullptr;

private:
    // ----------------------------
    // UE-backed state
    // ----------------------------
    UPROPERTY(VisibleAnywhere, Category = "Starshatter|Player")
    int32 PlayerId = 0;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Starshatter|Player", meta = (AllowPrivateAccess = "true"))
    FString PlayerName;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Starshatter|Player", meta = (AllowPrivateAccess = "true"))
    FString PlayerSignature;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Starshatter|Player", meta = (AllowPrivateAccess = "true"))
    FString PlayerSquadron;

    UPROPERTY(VisibleAnywhere, Category = "Starshatter|Player|Stats")
    int32 PlayerPoints = 0;

    UPROPERTY(VisibleAnywhere, Category = "Starshatter|Player|Stats")
    int32 CachedRankId = 0;

    UPROPERTY(VisibleAnywhere, Category = "Starshatter|Player|Awards")
    int32 PendingAwardId = 0;

    UPROPERTY(VisibleAnywhere, Category = "Starshatter|Player|Awards")
    bool bPendingAwardIsRank = false;

    UPROPERTY(VisibleAnywhere, Category = "Starshatter|Player")
    bool bDirty = false;
};
