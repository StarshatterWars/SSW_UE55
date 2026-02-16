#include "StarshatterPlayerCharacter.h"
#include "GameStructs.h"
#include "StarshatterPlayerSubsystem.h"
#include "AwardInfoRegistry.h"

void UStarshatterPlayerCharacter::Initialize(UStarshatterPlayerSubsystem* InOwnerSubsystem)
{
    OwnerSubsystem = InOwnerSubsystem;
}

void UStarshatterPlayerCharacter::SetPlayerName(const FString& InName)
{
    if (!InName.IsEmpty() && PlayerName != InName)
    {
        PlayerName = InName;
        bDirty = true;
    }
}

FString UStarshatterPlayerCharacter::GetAwardName() const
{
    if (PendingAwardId == 0)
        return FString();

    return bPendingAwardIsRank
        ? FString(UAwardInfoRegistry::RankName(PendingAwardId))
        : FString(UAwardInfoRegistry::MedalName(PendingAwardId));
}

FString UStarshatterPlayerCharacter::GetAwardDesc() const
{
    if (PendingAwardId == 0)
        return FString();

    return bPendingAwardIsRank
        ? FString(UAwardInfoRegistry::RankDescription(PendingAwardId))
        : FString(UAwardInfoRegistry::MedalDescription(PendingAwardId));
}

void UStarshatterPlayerCharacter::ClearShowAward()
{
    PendingAwardId = 0;
    bPendingAwardIsRank = false;
    bDirty = true;
}

void UStarshatterPlayerCharacter::FromPlayerInfo(const FS_PlayerGameInfo& InInfo)
{
    PlayerId = InInfo.Id;
    PlayerName = InInfo.Name;
    PlayerSignature = InInfo.Signature;
    PlayerSquadron = InInfo.Nickname;

    PlayerPoints = InInfo.PlayerPoints;
    CachedRankId = InInfo.Rank;

    // If you persist award presentation later, wire it here.
    PendingAwardId = 0;
    bPendingAwardIsRank = false;

    bDirty = false;
}

void UStarshatterPlayerCharacter::ToPlayerInfo(FS_PlayerGameInfo& OutInfo) const
{
    OutInfo.Id = PlayerId;
    OutInfo.Name = PlayerName;
    OutInfo.Signature = PlayerSignature;
    OutInfo.Nickname = PlayerSquadron;

    OutInfo.PlayerPoints = PlayerPoints;
    OutInfo.Rank = CachedRankId;

    // Leave the rest alone unless you mirror everything.
}

bool UStarshatterPlayerCharacter::Commit(bool bForceSave)
{
    if (!OwnerSubsystem)
        return false;

    FS_PlayerGameInfo& MutableInfo = OwnerSubsystem->GetMutablePlayerInfo();
    ToPlayerInfo(MutableInfo);

    const bool bOk = OwnerSubsystem->SavePlayer(bForceSave);
    if (bOk)
        bDirty = false;

    return bOk;
}
