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

void
CombatEvent::Load()
{
    DataLoader* loader = DataLoader::GetLoader();

    if (!campaign || !loader)
        return;

    loader->SetDataPath(campaign->Path());

    if (file.length() > 0) {
        const char* filename = file.data();
        BYTE* block = 0;

        loader->LoadBuffer(filename, block, true);
        info = (const char*)block;
        loader->ReleaseBuffer(block);

        if (info.contains('$')) {
            PlayerCharacter* player = PlayerCharacter::GetCurrentPlayer();
            CombatGroup* group = campaign->GetPlayerGroup();

            if (player) {
                info = FormatTextReplace(info, "$NAME", player->Name().data());
                info = FormatTextReplace(info, "$RANK", PlayerCharacter::RankName(player->GetRank()));
            }

            if (group) {
                info = FormatTextReplace(info, "$GROUP", group->GetDescription());
            }

            char timestr[32];
            FormatDayTime(timestr, campaign->GetTime(), true);
            info = FormatTextReplace(info, "$TIME", timestr);
        }
    }

    // Bitmap loading removed/commented to match template header (Bitmap disabled):
    // if (type < CAMPAIGN_END && image_file.length() > 0) {
    //     loader->LoadBitmap(image_file, image);
    // }

    loader->SetDataPath("");
}
