/*  Project Starshatter 4.5
    Fractal Dev Studios
    Copyright © 2025. All Rights Reserved.

    SUBSYSTEM:    Stars.exe
    FILE:         CombatAction.cpp
    AUTHOR:       Carlos Bott

    UNREAL PORT:
    - Preserves original logic and flow.
    - Uses FMath::RandRange for probability roll (0..99).
    - String name parsing uses FString (case-insensitive Equals), no _stricmp.
    - Player rank gating routes through UStarshatterPlayerSubsystem (not PlayerCharacter).
*/

#include "CombatAction.h"
#include "Campaign.h"
#include "Combatant.h"
#include "CombatGroup.h"

// Player profile (rank)
#include "StarshatterPlayerSubsystem.h"

#include "Math/UnrealMathUtility.h"

static int32 Rand0_99()
{
    return FMath::RandRange(0, 99);
}

static FString ToFStringSafe(const char* In)
{
    return In ? FString(UTF8_TO_TCHAR(In)) : FString();
}

static bool EqualsI(const FString& A, const TCHAR* B)
{
    return A.Equals(B, ESearchCase::IgnoreCase);
}

CombatAction::CombatAction(int32 InId, int32 InType, int InSubtype, int32 InTeam)
    : id(InId)
    , type((uint8)InType)
    , subtype(InSubtype)
    , opp_type((uint8)0xFF) // -1 in uint8
    , team((uint8)InTeam)
    , status((uint8)PENDING)
    , min_rank(0)
    , max_rank(100)
    , source(ECombatEventSource::NONE)
    , count(0)
    , start_before((int32)1e9)
    , start_after(0)
    , delay(0)
    , probability(100)
    , rval(-1)
    , time(0)
    , asset_type(0)
    , asset_id(0)
    , target_type(0)
    , target_id(0)
    , target_iff(0)
{
}

CombatAction::~CombatAction()
{
    requirements.destroy();
    asset_kills.destroy();
    target_kills.destroy();
}

bool CombatAction::IsAvailable() const
{
    // Preserve original behavior: may mutate rval/status/delay/start_after
    CombatAction* pThis = const_cast<CombatAction*>(this);

    if (rval < 0)
    {
        pThis->rval = Rand0_99();

        if (rval > probability)
            pThis->status = (uint8)SKIPPED;
    }

    if (status != PENDING)
        return false;

    // ------------------------------------------------------------
    // Rank gate (migrated from PlayerCharacter -> PlayerSubsystem)
    // ------------------------------------------------------------
    if (min_rank > 0 || max_rank < 100)
    {
        // NOTE:
        // CombatAction is legacy-core and usually doesn't have a UObject context.
        // This safe accessor returns DefaultRankId if it can't resolve a subsystem.
        // If you later thread a proper WorldContextObject into CombatAction, swap it here.
        const int32 PlayerRank = UStarshatterPlayerSubsystem::GetRankIdSafe(nullptr, /*DefaultRankId=*/0);

        if (PlayerRank < (int)min_rank || PlayerRank >(int)max_rank)
            return false;
    }

    Campaign* campaign = Campaign::GetCampaign();
    if (campaign)
    {
        if (campaign->GetTime() < start_after)
            return false;

        if (campaign->GetTime() > start_before)
        {
            pThis->status = (uint8)FAILED; // too late
            return false;
        }

        // check requirements against actions in current campaign:
        ListIter<CombatActionReq> iter = pThis->requirements;
        while (++iter)
        {
            CombatActionReq* r = iter.value();
            bool ok = false;

            if (r->action > 0)
            {
                ListIter<CombatAction> action = campaign->GetActions();
                while (++action)
                {
                    CombatAction* a = action.value();

                    if (a->Identity() == r->action)
                    {
                        if (r->notx)
                        {
                            if (a->GetStatus() == r->stat)
                                return false;
                        }
                        else
                        {
                            if (a->GetStatus() != r->stat)
                                return false;
                        }
                    }
                }
            }

            // group-based requirement
            else if (r->group_type != ECOMBATGROUP_TYPE::NONE)
            {
                if (r->c1)
                {
                    CombatGroup* group = r->c1->FindGroup(r->group_type, r->group_id);

                    if (group)
                    {
                        int test = 0;
                        int comp = 0;

                        if (r->intel)
                        {
                            test = group->IntelLevel();
                            comp = r->intel;
                        }
                        else
                        {
                            test = group->CalcValue();
                            comp = r->score;
                        }

                        switch (r->comp)
                        {
                        case CombatActionReq::LT: ok = (test < comp);  break;
                        case CombatActionReq::LE: ok = (test <= comp); break;
                        case CombatActionReq::GT: ok = (test > comp);  break;
                        case CombatActionReq::GE: ok = (test >= comp); break;
                        case CombatActionReq::EQ: ok = (test == comp); break;
                        default:                  ok = false;          break;
                        }
                    }

                    if (!ok)
                        return false;
                }
            }

            // score-based requirement
            else
            {
                if (r->comp <= CombatActionReq::EQ) // absolute
                {
                    if (r->c1)
                    {
                        const int test = r->c1->GetScore();

                        switch (r->comp)
                        {
                        case CombatActionReq::LT: ok = (test < r->score);  break;
                        case CombatActionReq::LE: ok = (test <= r->score); break;
                        case CombatActionReq::GT: ok = (test > r->score);  break;
                        case CombatActionReq::GE: ok = (test >= r->score); break;
                        case CombatActionReq::EQ: ok = (test == r->score); break;
                        default:                  ok = false;             break;
                        }
                    }
                }
                else // relative
                {
                    if (r->c1 && r->c2)
                    {
                        const int test = r->c1->GetScore() - r->c2->GetScore();

                        switch (r->comp)
                        {
                        case CombatActionReq::RLT: ok = (test < r->score);  break;
                        case CombatActionReq::RLE: ok = (test <= r->score); break;
                        case CombatActionReq::RGT: ok = (test > r->score);  break;
                        case CombatActionReq::RGE: ok = (test >= r->score); break;
                        case CombatActionReq::REQ: ok = (test == r->score); break;
                        default:                   ok = false;             break;
                        }
                    }
                }

                if (!ok)
                    return false;
            }

            if (delay > 0)
            {
                pThis->start_after = (int)campaign->GetTime() + delay;
                pThis->delay = 0;
                return IsAvailable();
            }
        }
    }

    return true;
}

void CombatAction::FireAction()
{
    Campaign* campaign = Campaign::GetCampaign();
    if (campaign)
        time = (int)campaign->GetTime();

    if (count >= 1)
        count--;

    if (count < 1)
        status = (uint8)COMPLETE;
}

void CombatAction::FailAction()
{
    Campaign* campaign = Campaign::GetCampaign();
    if (campaign)
        time = (int)campaign->GetTime();

    count = 0;
    status = (uint8)FAILED;
}

void CombatAction::AddRequirement(int action, int stat, bool not_req)
{
    requirements.append(new CombatActionReq(action, stat, not_req));
}

void CombatAction::AddRequirement(Combatant* c1, Combatant* c2, int comp, int score)
{
    requirements.append(new CombatActionReq(c1, c2, comp, score));
}

void CombatAction::AddRequirement(Combatant* c1, ECOMBATGROUP_TYPE group_type, int group_id, int comp, int score, int intel)
{
    requirements.append(new CombatActionReq(c1, group_type, group_id, comp, score, intel));
}

int CombatAction::TypeFromName(const char* n)
{
    const FString N = ToFStringSafe(n);
    if (N.IsEmpty())
        return 0;

    if (EqualsI(N, TEXT("NO_ACTION")))             return NO_ACTION;
    if (EqualsI(N, TEXT("MARKER")))                return NO_ACTION;

    if (EqualsI(N, TEXT("STRATEGIC_DIRECTIVE")))   return STRATEGIC_DIRECTIVE;
    if (EqualsI(N, TEXT("STRATEGIC")))             return STRATEGIC_DIRECTIVE;

    if (EqualsI(N, TEXT("ZONE_ASSIGNMENT")))       return ZONE_ASSIGNMENT;
    if (EqualsI(N, TEXT("ZONE")))                  return ZONE_ASSIGNMENT;

    if (EqualsI(N, TEXT("SYSTEM_ASSIGNMENT")))     return SYSTEM_ASSIGNMENT;
    if (EqualsI(N, TEXT("SYSTEM")))                return SYSTEM_ASSIGNMENT;

    if (EqualsI(N, TEXT("MISSION_TEMPLATE")))      return MISSION_TEMPLATE;
    if (EqualsI(N, TEXT("MISSION")))               return MISSION_TEMPLATE;

    if (EqualsI(N, TEXT("COMBAT_EVENT")))          return COMBAT_EVENT;
    if (EqualsI(N, TEXT("EVENT")))                 return COMBAT_EVENT;

    if (EqualsI(N, TEXT("INTEL_EVENT")))           return INTEL_EVENT;
    if (EqualsI(N, TEXT("INTEL")))                 return INTEL_EVENT;

    if (EqualsI(N, TEXT("CAMPAIGN_SITUATION")))    return CAMPAIGN_SITUATION;
    if (EqualsI(N, TEXT("SITREP")))                return CAMPAIGN_SITUATION;

    if (EqualsI(N, TEXT("CAMPAIGN_ORDERS")))       return CAMPAIGN_ORDERS;
    if (EqualsI(N, TEXT("ORDERS")))                return CAMPAIGN_ORDERS;

    return 0;
}

int CombatAction::StatusFromName(const char* n)
{
    const FString N = ToFStringSafe(n);
    if (N.IsEmpty())
        return 0;

    if (EqualsI(N, TEXT("PENDING")))   return PENDING;
    if (EqualsI(N, TEXT("ACTIVE")))    return ACTIVE;
    if (EqualsI(N, TEXT("SKIPPED")))   return SKIPPED;
    if (EqualsI(N, TEXT("FAILED")))    return FAILED;
    if (EqualsI(N, TEXT("COMPLETE")))  return COMPLETE;

    return 0;
}

int CombatActionReq::CompFromName(const char* n)
{
    const FString N = ToFStringSafe(n);
    if (N.IsEmpty())
        return 0;

    if (EqualsI(N, TEXT("LT")))   return LT;
    if (EqualsI(N, TEXT("LE")))   return LE;
    if (EqualsI(N, TEXT("GT")))   return GT;
    if (EqualsI(N, TEXT("GE")))   return GE;
    if (EqualsI(N, TEXT("EQ")))   return EQ;

    if (EqualsI(N, TEXT("RLT")))  return RLT;
    if (EqualsI(N, TEXT("RLE")))  return RLE;
    if (EqualsI(N, TEXT("RGT")))  return RGT;
    if (EqualsI(N, TEXT("RGE")))  return RGE;
    if (EqualsI(N, TEXT("REQ")))  return REQ;

    return 0;
}
