/*  Project Starshatter 4.5
    Destroyer Studios LLC
    Copyright © 1997-2004. All Rights Reserved.

    SUBSYSTEM:    Stars.exe
    FILE:         CombatEvent.cpp
    AUTHOR:       John DiCamillo

    OVERVIEW
    ========
    A significant (newsworthy) event in the dynamic campaign.
*/

#include "CombatEvent.h"
#include "CombatGroup.h"
#include "Campaign.h"
#include "PlayerCharacter.h"
#include "ShipDesign.h"
#include "Ship.h"
#include "Term.h"
#include "ParseUtil.h"
#include "FormatUtil.h"
#include "DataLoader.h"
#include "GameStructs.h"

#include "Misc/Paths.h"
#include "Misc/FileHelper.h"
#include "HAL/PlatformFilemanager.h"

#include "Engine/Texture2D.h"

// +----------------------------------------------------------------------+

CombatEvent::CombatEvent(Campaign* c, int typ, int tim, int tem,
    ECombatEventSource src, const char* rgn)
    : campaign(c), type(typ), time(tim), team(tem), source(src),
    visited(false), loc(FVector::ZeroVector), points(FVector::ZeroVector),
    region(rgn)
{
}

// +----------------------------------------------------------------------+

FString
CombatEvent::GetEventSourceName() const
{
    return GetSourceName(source);
}

FString CombatEvent::GetEventTypeName() const
{
    const UEnum* EnumPtr = StaticEnum<ECombatEventType>();
    if (!EnumPtr)
    {
        return TEXT("Unknown");
    }

    if (!EnumPtr->IsValidEnumValue(type))
    {
        return TEXT("Unknown");
    }

    return EnumPtr->GetDisplayNameTextByValue(type).ToString();
}

// +----------------------------------------------------------------------+
FString CombatEvent::GetTypeName(ECombatEventType InType)
{
    const UEnum* EnumPtr = StaticEnum<ECombatEventType>();
    if (!EnumPtr)
    {
        return TEXT("Unknown");
    }

    return EnumPtr->GetDisplayNameTextByValue((int64)InType).ToString();
}

// ----------------------------------------------------------------------

FString CombatEvent::GetSourceName(ECombatEventSource InSource)
{
    const UEnum* EnumPtr = StaticEnum<ECombatEventSource>();
    if (!EnumPtr)
    {
        return TEXT("Unknown");
    }

    return EnumPtr->GetDisplayNameTextByValue((int64)InSource).ToString();
}

ECombatEventSource CombatEvent::GetSourceFromName(const FString& Name)
{
    const FString N = Name.TrimStartAndEnd();

    if (N.IsEmpty())
        return ECombatEventSource::NONE;   // <-- define this in your enum

    if (N.Equals(TEXT("FORCOM"), ESearchCase::IgnoreCase))
        return ECombatEventSource::FORCOM;

    if (N.Equals(TEXT("TACNET"), ESearchCase::IgnoreCase))
        return ECombatEventSource::TACNET;

    if (N.Equals(TEXT("SECURE"), ESearchCase::IgnoreCase))
        return ECombatEventSource::INTEL;

    if (N.Equals(TEXT("Mail"), ESearchCase::IgnoreCase))
        return ECombatEventSource::MAIL;

    if (N.Equals(TEXT("News"), ESearchCase::IgnoreCase))
        return ECombatEventSource::NEWS;

    return ECombatEventSource::NONE;
}

// ----------------------------------------------------------------------
ECombatEventType CombatEvent::GetTypeFromName(const FString& Name)
{
    const FString N = Name.TrimStartAndEnd();

    if (N.IsEmpty())
        return ECombatEventType::NONE; // add Unknown to the enum

    if (N.Equals(TEXT("ATTACK"), ESearchCase::IgnoreCase))
        return ECombatEventType::ATTACK;

    if (N.Equals(TEXT("DEFEND"), ESearchCase::IgnoreCase))
        return ECombatEventType::DEFEND;

    if (N.Equals(TEXT("MOVE_TO"), ESearchCase::IgnoreCase))
        return ECombatEventType::MOVE_TO;

    if (N.Equals(TEXT("CAPTURE"), ESearchCase::IgnoreCase))
        return ECombatEventType::CAPTURE;

    if (N.Equals(TEXT("STRATEGY"), ESearchCase::IgnoreCase))
        return ECombatEventType::STRATEGY;

    if (N.Equals(TEXT("STORY"), ESearchCase::IgnoreCase))
        return ECombatEventType::STORY;

    if (N.Equals(TEXT("CAMPAIGN_START"), ESearchCase::IgnoreCase))
        return ECombatEventType::CAMPAIGN_START;

    if (N.Equals(TEXT("CAMPAIGN_END"), ESearchCase::IgnoreCase))
        return ECombatEventType::CAMPAIGN_END;

    if (N.Equals(TEXT("CAMPAIGN_FAIL"), ESearchCase::IgnoreCase))
        return ECombatEventType::CAMPAIGN_FAIL;

    return ECombatEventType::NONE;
}

// +----------------------------------------------------------------------+

void CombatEvent::Load()
{
    if (!campaign)
    {
        return;
    }

    // If your legacy member is std::string file; adjust as needed:
    if (file.length() <= 0)
    {
        return;
    }

    const FString CampaignPath = UTF8_TO_TCHAR(campaign->Path());      // folder
    const FString RelativeFile = UTF8_TO_TCHAR(file);          // file name / relative path
    const FString FullPath = FPaths::ConvertRelativePathToFull(FPaths::Combine(CampaignPath, RelativeFile));

    FString LoadedText;
    if (!FFileHelper::LoadFileToString(LoadedText, *FullPath))
    {
        // Keep it silent like legacy, or add logging:
        // UE_LOG(LogTemp, Warning, TEXT("CombatEvent::Load failed: %s"), *FullPath);
        return;
    }

    // Store into your member (prefer FString in UE):
    // If your member 'info' is std::string, convert back at the end.
    FString InfoText = LoadedText;

    // Token replacement (only if needed)
    if (InfoText.Contains(TEXT("$")))
    {
        PlayerCharacter* Player = PlayerCharacter::GetCurrentPlayer();
        CombatGroup* Group = campaign->GetPlayerGroup();

        if (Player)
        {
            // These look legacy-returning const char*; convert as needed:
            InfoText.ReplaceInline(TEXT("$NAME"), UTF8_TO_TCHAR(*Player->Name()));
            InfoText.ReplaceInline(TEXT("$RANK"), UTF8_TO_TCHAR(PlayerCharacter::RankName(Player->GetRank())));
        }

        if (Group)
        {
            // If GetDescription() returns const char*:
            InfoText.ReplaceInline(TEXT("$GROUP"), UTF8_TO_TCHAR(Group->GetDescription()));
        }

        // Time formatting: adapt to your campaign time representation.
        // If campaign->GetTime() is "seconds since campaign start":
        const double CampaignSeconds = campaign->GetTime();

        const int32 TotalSeconds = (int32)FMath::Max(0.0, CampaignSeconds);
        const int32 Hours = (TotalSeconds / 3600) % 24;
        const int32 Minutes = (TotalSeconds / 60) % 60;

        const FString TimeStr = FString::Printf(TEXT("%02d:%02d"), Hours, Minutes);
        InfoText.ReplaceInline(TEXT("$TIME"), *TimeStr);
    }

    // Assign back to your storage:
    // Preferred: make 'info' an FString in the UE port.
    info = TCHAR_TO_UTF8(*InfoText); // if 'info' is std::string
    // If 'info' is FString already: info = InfoText;
}