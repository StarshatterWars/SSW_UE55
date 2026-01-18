/*  Project Starshatter 4.5
    Fractal Dev Studios
    Copyright © 2025. All Rights Reserved.

    SUBSYSTEM:    Stars.exe
    FILE:         Campaign.cpp
    AUTHOR:       Carlos Bott

    UNREAL PORT:
    - Maintains all variables and methods (names, signatures, members).
    - Uses UE-compatible shims for List/Text/Term/Parser/DataLoader.
*/

#include "Campaign.h"
#include "CampaignPlanStrategic.h"
#include "CampaignPlanAssignment.h"
#include "CampaignPlanEvent.h"
#include "CampaignPlanMission.h"
#include "CampaignPlanMovement.h"
#include "CampaignSituationReport.h"
#include "CampaignSaveGame.h"
#include "Combatant.h"
#include "CombatAction.h"
#include "CombatEvent.h"
#include "CombatGroup.h"
#include "CombatRoster.h"
#include "CombatUnit.h"
#include "CombatZone.h"
#include "Galaxy.h"
#include "Mission.h"
#include "StarSystem.h"
#include "Starshatter.h"
#include "Player.h"
#include "DataLoader.h"
#include "ParseUtil.h"
#include "Random.h"
#include "FormatUtil.h"
#include "GameStructs.h"
#include "Game.h"

// Unreal:
#include "Misc/Paths.h"
#include "HAL/PlatformFilemanager.h"

#ifndef UE_LOG
// If you want, replace with UE_LOG(LogTemp, ...) everywhere.
// Keeping legacy ::Print calls if you still have them in shims.
#endif

// +====================================================================+

static const int32 TIME_NEVER = (int32)1e9;
static const int32 ONE_DAY = (int32)(24 * 3600);

static int RandomIntInclusive(int MinInclusive, int MaxInclusive)
{
    if (MaxInclusive < MinInclusive)
        return MinInclusive;
    return FMath::RandRange(MinInclusive, MaxInclusive);
}

// +====================================================================+
// MissionInfo

MissionInfo::MissionInfo()
    : mission(nullptr)
{
    // Preserve legacy defaults:
    id = 0;
    start = 0;
    type = 0;

    min_rank = 0;
    max_rank = 100;
    action_id = 0;
    action_status = 0;
    exec_once = 0;

    start_before = TIME_NEVER;
    start_after = 0;
}

MissionInfo::~MissionInfo()
{
    delete mission;
    mission = nullptr;
}

bool MissionInfo::IsAvailable()
{
    Campaign* campaign = Campaign::GetCampaign();
    Player* player = Player::GetCurrentPlayer();

    if (!campaign || !player)
        return false;

    CombatGroup* player_group = campaign->GetPlayerGroup();
    if (!player_group)
        return false;

    if (campaign->GetTime() < start_after)
        return false;

    if (campaign->GetTime() > start_before)
        return false;

    if (!region.IsEmpty() && player_group->GetRegion() != region)
        return false;

    if (min_rank && player->Rank() < min_rank)
        return false;

    if (max_rank && player->Rank() > max_rank)
        return false;

    if (exec_once < 0)
        return false;

    if (exec_once > 0)
        exec_once = -1;

    return true;
}

// +====================================================================+
// TemplateList

TemplateList::TemplateList()
{
    mission_type = 0;
    group_type = 0;
    index = 0;
}

TemplateList::~TemplateList()
{
    missions.destroy();
}

// +====================================================================+
// Static campaign registry

static List<Campaign> campaigns;
static Campaign* current_campaign = nullptr;

// +====================================================================+
// Campaign

Campaign::Campaign(int id, const char* n)
    : campaign_id(id)
    , status(CAMPAIGN_INIT)
{
    // Preserve legacy behavior:
    mission_id = -1;
    mission = nullptr;
    net_mission = nullptr;

    scripted = false;
    sequential = false;
    time = 0.0;
    startTime = 0.0;
    loadTime = 0.0;
    updateTime = 0.0;

    player_group = nullptr;
    player_unit = nullptr;

    lockout = 0;
    loaded_from_savegame = false;

    name = n ? FString(n) : FString();

    path.Empty();
    Load();
}

Campaign::Campaign(int id, const char* n, const char* p)
    : campaign_id(id)
    , status(CAMPAIGN_INIT)
{
    mission_id = -1;
    mission = nullptr;
    net_mission = nullptr;

    scripted = false;
    sequential = false;
    time = 0.0;
    startTime = 0.0;
    loadTime = 0.0;
    updateTime = 0.0;

    player_group = nullptr;
    player_unit = nullptr;

    lockout = 0;
    loaded_from_savegame = false;

    name = n ? FString(n) : FString();
    path = p ? FString(p) : FString();

    Load();
}

Campaign::~Campaign()
{
    delete net_mission;
    net_mission = nullptr;

    actions.destroy();
    events.destroy();
    missions.destroy();
    templates.destroy();
    planners.destroy();
    zones.destroy();
    combatants.destroy();
}

void Campaign::Initialize()
{
    Campaign* c = nullptr;
    DataLoader* loader = DataLoader::GetLoader();

    for (int i = 1; i < 100; i++) {
        char legacyPath[256];
        sprintf_s(legacyPath, "Campaigns/%02d/", i);

        loader->UseFileSystem(true);
        loader->SetDataPath(legacyPath);

        if (loader->FindFile("campaign.def")) {
            char txt[256];
            sprintf_s(txt, "Dynamic Campaign %02d", i);
            c = new Campaign(i, txt);
            if (c)
                campaigns.insertSort(c);
        }
    }

    c = new Campaign(SINGLE_MISSIONS, "Single Missions");
    if (c) {
        campaigns.insertSort(c);
        current_campaign = c;
    }

    c = new Campaign(MULTIPLAYER_MISSIONS, "Multiplayer Missions");
    if (c)
        campaigns.insertSort(c);

    c = new Campaign(CUSTOM_MISSIONS, "Custom Missions");
    if (c)
        campaigns.insertSort(c);
}

void Campaign::Close()
{
    current_campaign = nullptr;
    campaigns.destroy();
}

Campaign* Campaign::GetCampaign()
{
    return current_campaign;
}

Campaign* Campaign::SelectCampaign(const char* InName)
{
    Campaign* c = nullptr;
    ListIter<Campaign> iter = campaigns;

    while (++iter && !c) {
        // Legacy used _stricmp; use FString compare ignoring case:
        if (iter.value()->Name().Equals(FString(InName), ESearchCase::IgnoreCase))
            c = iter.value();
    }

    if (c)
        current_campaign = c;

    return c;
}

Campaign* Campaign::CreateCustomCampaign(const char* InName, const char* InPath)
{
    int id = 0;

    if (InName && *InName && InPath && *InPath) {
        ListIter<Campaign> iter = campaigns;

        while (++iter) {
            Campaign* c = iter.value();
            if (c->GetCampaignId() >= id)
                id = c->GetCampaignId() + 1;

            if (c->Name() == FString(InName)) {
                return nullptr;
            }
        }
    }

    if (id == 0)
        id = CUSTOM_MISSIONS + 1;

    Campaign* c = new Campaign(id, InName, InPath);
    campaigns.append(c);

    return c;
}

List<Campaign>& Campaign::GetAllCampaigns()
{
    return campaigns;
}

int Campaign::GetLastCampaignId()
{
    int result = 0;

    for (int i = 0; i < campaigns.size(); i++) {
        Campaign* c = campaigns.at(i);
        if (c->IsDynamic() && c->GetCampaignId() > result)
            result = c->GetCampaignId();
    }

    return result;
}

CombatEvent* Campaign::GetLastEvent()
{
    CombatEvent* result = nullptr;
    if (!events.isEmpty())
        result = events.last();
    return result;
}

int Campaign::CountNewEvents() const
{
    int result = 0;

    for (int i = 0; i < events.size(); i++) {
        if (!events[i]->Visited())
            result++;
    }

    return result;
}

void Campaign::Clear()
{
    missions.destroy();
    planners.destroy();
    combatants.destroy();
    events.destroy();
    actions.destroy();

    player_group = nullptr;
    player_unit = nullptr;

    updateTime = time;
}

void Campaign::Unload()
{
    SetStatus(CAMPAIGN_INIT);

    Game::ResetGameTime();
    StarSystem::SetBaseTime(0);

    startTime = Stardate();
    loadTime = startTime;
    lockout = 0;

    Clear();
    zones.destroy();
}

void Campaign::Load()
{
    // First unload existing:
    Unload();

    if (path.IsEmpty()) {
        switch (campaign_id) {
        case SINGLE_MISSIONS:      path = TEXT("Missions/");       break;
        case CUSTOM_MISSIONS:      path = TEXT("Mods/Missions/");  break;
        case MULTIPLAYER_MISSIONS: path = TEXT("Multiplayer/");    break;
        default:                   path = FString::Printf(TEXT("Campaigns/%02d/"), campaign_id); break;
        }
    }

    DataLoader* loader = DataLoader::GetLoader();
    loader->UseFileSystem(true);
    loader->SetDataPath(TCHAR_TO_ANSI(*path));

    systems.clear();

    if (loader->FindFile("zones.def"))
        zones.append(CombatZone::Load("zones.def"));

    for (int i = 0; i < zones.size(); i++) {
        FString SysName = zones[i]->System();
        bool found = false;

        for (int n = 0; !found && n < systems.size(); n++) {
            if (SysName == systems[n]->Name())
                found = true;
        }

        if (!found)
            systems.append(Galaxy::GetInstance()->GetSystem(TCHAR_TO_ANSI(*SysName)));
    }

    loader->UseFileSystem(Starshatter::UseFileSystem());

    if (loader->FindFile("campaign.def"))
        LoadCampaign(loader);

    if (campaign_id == CUSTOM_MISSIONS) {
        loader->SetDataPath(TCHAR_TO_ANSI(*path));
        LoadCustomMissions(loader);
    }
    else {
        bool found = false;

        if (loader->FindFile("missions.def")) {
            loader->SetDataPath(TCHAR_TO_ANSI(*path));
            LoadMissionList(loader);
            found = true;
        }

        if (loader->FindFile("templates.def")) {
            loader->SetDataPath(TCHAR_TO_ANSI(*path));
            LoadTemplateList(loader);
            found = true;
        }

        if (!found) {
            loader->SetDataPath(TCHAR_TO_ANSI(*path));
            LoadCustomMissions(loader);
        }
    }

    loader->UseFileSystem(true);
    loader->SetDataPath(TCHAR_TO_ANSI(*path));

    // Images removed in header (commented out). Keep behavior if you re-enable Bitmap.
    // if (loader->FindFile("image.pcx")) { ... }

    loader->SetDataPath(nullptr);
    loader->UseFileSystem(Starshatter::UseFileSystem());
}

void Campaign::LoadCampaign(DataLoader* loader, bool full)
{
    BYTE* block = nullptr;
    const char* filename = "campaign.def";

    loader->UseFileSystem(true);
    loader->LoadBuffer(filename, block, true);
    loader->UseFileSystem(Starshatter::UseFileSystem());

    Parser parser(new BlockReader((const char*)block));
    Term* term = parser.ParseTerm();

    if (!term)
        return;

    {
        TermText* file_type = term->isText();
        if (!file_type || file_type->value() != "CAMPAIGN")
            return;
    }

    do {
        delete term; term = nullptr;
        term = parser.ParseTerm();

        if (term) {
            TermDef* def = term->isDef();
            if (def) {
                const FString DefName = def->name()->value().data();

                if (DefName == "name") {
                    if (def->term() && def->term()->isText()) {
                        name = def->term()->isText()->value().data();
                        name = Game::GetText(name);
                    }
                }
                else if (DefName == "desc") {
                    if (def->term() && def->term()->isText()) {
                        description = def->term()->isText()->value().data();
                        description = Game::GetText(description);
                    }
                }
                else if (DefName == "situation") {
                    if (def->term() && def->term()->isText()) {
                        situation = def->term()->isText()->value().data();
                        situation = Game::GetText(situation);
                    }
                }
                else if (DefName == "orders") {
                    if (def->term() && def->term()->isText()) {
                        orders = def->term()->isText()->value().data();
                        orders = Game::GetText(orders);
                    }
                }
                else if (DefName == "scripted") {
                    if (def->term() && def->term()->isBool())
                        scripted = def->term()->isBool()->value();
                }
                else if (DefName == "sequential") {
                    if (def->term() && def->term()->isBool())
                        sequential = def->term()->isBool()->value();
                }
                else if (full && DefName == "combatant") {
                    if (def->term() && def->term()->isStruct()) {
                        TermStruct* val = def->term()->isStruct();

                        char         cname[64];
                        CombatGroup* force = nullptr;
                        CombatGroup* clone = nullptr;

                        ZeroMemory(cname, sizeof(cname));

                        for (int i = 0; i < val->elements()->size(); i++) {
                            TermDef* pdef = val->elements()->at(i)->isDef();
                            if (!pdef) continue;

                            const FString PName = pdef->name()->value().data();

                            if (PName == "name") {
                                GetDefText(cname, pdef, filename);

                                force = CombatRoster::GetInstance()->GetForce(cname);
                                if (force)
                                    clone = force->Clone(false); // shallow copy
                            }
                            else if (PName == "group") {
                                ParseGroup(pdef->term()->isStruct(), force, clone, filename);
                            }
                        }

                        loader->SetDataPath(TCHAR_TO_ANSI(*path));
                        Combatant* c = new Combatant(cname, clone);
                        if (c)
                            combatants.append(c);
                        else {
                            Unload();
                            return;
                        }
                    }
                }
                else if (full && DefName == "action") {
                    if (def->term() && def->term()->isStruct()) {
                        TermStruct* val = def->term()->isStruct();
                        ParseAction(val, filename);
                    }
                }
            }
        }
    } while (term);

    loader->ReleaseBuffer(block);
}

void Campaign::ParseGroup(TermStruct* val, CombatGroup* force, CombatGroup* clone, const char* filename)
{
    if (!val || !force || !clone)
        return;

    int type = 0;
    int id = 0;

    for (int i = 0; i < val->elements()->size(); i++) {
        TermDef* pdef = val->elements()->at(i)->isDef();
        if (!pdef) continue;

        const FString PName = pdef->name()->value().data();

        if (PName == "type") {
            char type_name[64];
            GetDefText(type_name, pdef, filename);
            type = CombatGroup::TypeFromName(type_name);
        }
        else if (PName == "id") {
            GetDefNumber(id, pdef, filename);
        }
    }

    if (type && id) {
        CombatGroup* g = force->FindGroup(type, id);

        if (g && g->GetParent()) {
            CombatGroup* parent = CloneOver(force, clone, g->GetParent());
            parent->AddComponent(g->Clone());
        }
    }
}

void Campaign::ParseAction(TermStruct* val, const char* filename)
{
    if (!val)
        return;

    int   id = 0;
    int   type = 0;
    int   subtype = 0;
    int   opp_type = -1;
    int   team = 0;
    int   source = 0;
    Vec3  loc(0.0f, 0.0f, 0.0f);

    FString system;
    FString region;
    FString file;
    FString image;
    FString scene;
    FString text;

    int   count = 1;
    int   start_before = TIME_NEVER;
    int   start_after = 0;
    int   min_rank = 0;
    int   max_rank = 100;
    int   delay = 0;
    int   probability = 100;

    int   asset_type = 0;
    int   asset_id = 0;
    int   target_type = 0;
    int   target_id = 0;
    int   target_iff = 0;

    CombatAction* action = nullptr;

    for (int i = 0; i < val->elements()->size(); i++) {
        TermDef* pdef = val->elements()->at(i)->isDef();
        if (!pdef) continue;

        const FString PName = pdef->name()->value().data();

        if (PName == "id") {
            GetDefNumber(id, pdef, filename);
        }
        else if (PName == "type") {
            char txt[64];
            GetDefText(txt, pdef, filename);
            type = CombatAction::TypeFromName(txt);
        }
        else if (PName == "subtype") {
            if (pdef->term()->isNumber()) {
                GetDefNumber(subtype, pdef, filename);
            }
            else if (pdef->term()->isText()) {
                char txt[64];
                GetDefText(txt, pdef, filename);

                if (type == CombatAction::MISSION_TEMPLATE)
                    subtype = Mission::TypeFromName(txt);
                else if (type == CombatAction::COMBAT_EVENT)
                    subtype = CombatEvent::TypeFromName(txt);
                else if (type == CombatAction::INTEL_EVENT)
                    subtype = Intel::IntelFromName(txt);
            }
        }
        else if (PName == "opp_type") {
            if (pdef->term()->isNumber()) {
                GetDefNumber(opp_type, pdef, filename);
            }
            else if (pdef->term()->isText()) {
                char txt[64];
                GetDefText(txt, pdef, filename);

                if (type == CombatAction::MISSION_TEMPLATE)
                    opp_type = Mission::TypeFromName(txt);
            }
        }
        else if (PName == "source") {
            char txt[64];
            GetDefText(txt, pdef, filename);
            source = CombatEvent::SourceFromName(txt);
        }
        else if (PName == "team" || PName == "iff") {
            GetDefNumber(team, pdef, filename);
        }
        else if (PName == "count") {
            GetDefNumber(count, pdef, filename);
        }
        else if (PName.Contains("before")) {
            if (pdef->term()->isNumber()) {
                GetDefNumber(start_before, pdef, filename);
            }
            else {
                GetDefTime(start_before, pdef, filename);
                start_before -= ONE_DAY;
            }
        }
        else if (PName.Contains("after")) {
            if (pdef->term()->isNumber()) {
                GetDefNumber(start_after, pdef, filename);
            }
            else {
                GetDefTime(start_after, pdef, filename);
                start_after -= ONE_DAY;
            }
        }
        else if (PName == "min_rank") {
            if (pdef->term()->isNumber()) {
                GetDefNumber(min_rank, pdef, filename);
            }
            else {
                char rank_name[64];
                GetDefText(rank_name, pdef, filename);
                min_rank = Player::RankFromName(rank_name);
            }
        }
        else if (PName == "max_rank") {
            if (pdef->term()->isNumber()) {
                GetDefNumber(max_rank, pdef, filename);
            }
            else {
                char rank_name[64];
                GetDefText(rank_name, pdef, filename);
                max_rank = Player::RankFromName(rank_name);
            }
        }
        else if (PName == "delay") {
            GetDefNumber(delay, pdef, filename);
        }
        else if (PName == "probability") {
            GetDefNumber(probability, pdef, filename);
        }
        else if (PName == "asset_type") {
            char type_name[64];
            GetDefText(type_name, pdef, filename);
            asset_type = CombatGroup::TypeFromName(type_name);
        }
        else if (PName == "target_type") {
            char type_name[64];
            GetDefText(type_name, pdef, filename);
            target_type = CombatGroup::TypeFromName(type_name);
        }
        else if (PName == "location" || PName == "loc") {
            GetDefVec(loc, pdef, filename);
        }
        else if (PName == "system" || PName == "sys") {
            GetDefText(system, pdef, filename);
        }
        else if (PName == "region" || PName == "rgn" || PName == "zone") {
            GetDefText(region, pdef, filename);
        }
        else if (PName == "file") {
            GetDefText(file, pdef, filename);
        }
        else if (PName == "image") {
            GetDefText(image, pdef, filename);
        }
        else if (PName == "scene") {
            GetDefText(scene, pdef, filename);
        }
        else if (PName == "text") {
            GetDefText(text, pdef, filename);
            text = Game::GetText(text);
        }
        else if (PName == "asset_id") {
            GetDefNumber(asset_id, pdef, filename);
        }
        else if (PName == "target_id") {
            GetDefNumber(target_id, pdef, filename);
        }
        else if (PName == "target_iff") {
            GetDefNumber(target_iff, pdef, filename);
        }
        else if (PName == "asset_kill") {
            if (!action)
                action = new CombatAction(id, type, subtype, team);

            if (action) {
                char txt[64];
                GetDefText(txt, pdef, filename);
                action->AssetKills().append(new Text(txt));
            }
        }
        else if (PName == "target_kill") {
            if (!action)
                action = new CombatAction(id, type, subtype, team);

            if (action) {
                char txt[64];
                GetDefText(txt, pdef, filename);
                action->TargetKills().append(new Text(txt));
            }
        }
        else if (PName == "req") {
            if (!action)
                action = new CombatAction(id, type, subtype, team);

            if (!pdef->term() || !pdef->term()->isStruct()) {
                // warning
            }
            else if (action) {
                TermStruct* val2 = pdef->term()->isStruct();

                int  act = 0;
                int  stat = CombatAction::COMPLETE;
                bool notFlag = false;

                Combatant* c1 = nullptr;
                Combatant* c2 = nullptr;
                int comp = 0;
                int score = 0;
                int intel = 0;
                int gtype = 0;
                int gid = 0;

                for (int j = 0; j < val2->elements()->size(); j++) {
                    TermDef* pdef2 = val2->elements()->at(j)->isDef();
                    if (!pdef2) continue;

                    const FString N2 = pdef2->name()->value().data();

                    if (N2 == "action") {
                        GetDefNumber(act, pdef2, filename);
                    }
                    else if (N2 == "status") {
                        char txt[64];
                        GetDefText(txt, pdef2, filename);
                        stat = CombatAction::StatusFromName(txt);
                    }
                    else if (N2 == "not") {
                        GetDefBool(notFlag, pdef2, filename);
                    }
                    else if (N2 == "c1") {
                        char txt[64];
                        GetDefText(txt, pdef2, filename);
                        c1 = GetCombatant(txt);
                    }
                    else if (N2 == "c2") {
                        char txt[64];
                        GetDefText(txt, pdef2, filename);
                        c2 = GetCombatant(txt);
                    }
                    else if (N2 == "comp") {
                        char txt[64];
                        GetDefText(txt, pdef2, filename);
                        comp = CombatActionReq::CompFromName(txt);
                    }
                    else if (N2 == "score") {
                        GetDefNumber(score, pdef2, filename);
                    }
                    else if (N2 == "intel") {
                        if (pdef2->term()->isNumber()) {
                            GetDefNumber(intel, pdef2, filename);
                        }
                        else if (pdef2->term()->isText()) {
                            char txt[64];
                            GetDefText(txt, pdef2, filename);
                            intel = Intel::IntelFromName(txt);
                        }
                    }
                    else if (N2 == "group_type") {
                        char type_name[64];
                        GetDefText(type_name, pdef2, filename);
                        gtype = CombatGroup::TypeFromName(type_name);
                    }
                    else if (N2 == "group_id") {
                        GetDefNumber(gid, pdef2, filename);
                    }
                }

                if (act)
                    action->AddRequirement(act, stat, notFlag);
                else if (gtype)
                    action->AddRequirement(c1, gtype, gid, comp, score, intel);
                else
                    action->AddRequirement(c1, c2, comp, score);
            }
        }
    }

    if (!action)
        action = new CombatAction(id, type, subtype, team);

    if (action) {
        action->SetSource(source);
        action->SetOpposingType(opp_type);
        action->SetLocation(loc);
        action->SetSystem(system);
        action->SetRegion(region);
        action->SetFilename(file);
        action->SetImageFile(image);
        action->SetSceneFile(scene);
        action->SetCount(count);
        action->SetStartAfter(start_after);
        action->SetStartBefore(start_before);
        action->SetMinRank(min_rank);
        action->SetMaxRank(max_rank);
        action->SetDelay(delay);
        action->SetProbability(probability);

        action->SetAssetId(asset_id);
        action->SetAssetType(asset_type);
        action->SetTargetId(target_id);
        action->SetTargetIFF(target_iff);
        action->SetTargetType(target_type);
        action->SetText(text);

        actions.append(action);
    }
}

CombatGroup* Campaign::CloneOver(CombatGroup* force, CombatGroup* clone, CombatGroup* group)
{
    if (!clone || !group)
        return clone;

    CombatGroup* orig_parent = group->GetParent();

    if (orig_parent) {
        CombatGroup* clone_parent = clone->FindGroup(orig_parent->Type(), orig_parent->GetID());

        if (!clone_parent)
            clone_parent = CloneOver(force, clone, orig_parent);

        CombatGroup* new_clone = clone->FindGroup(group->Type(), group->GetID());

        if (!new_clone) {
            new_clone = group->Clone(false);
            clone_parent->AddComponent(new_clone);
        }

        return new_clone;
    }

    return clone;
}

void Campaign::LoadMissionList(DataLoader* loader)
{
    bool        ok = true;
    BYTE* block = nullptr;
    const char* filename = "Missions.def";

    loader->UseFileSystem(true);
    loader->LoadBuffer(filename, block, true);
    loader->UseFileSystem(Starshatter::UseFileSystem());

    Parser parser(new BlockReader((const char*)block));
    Term* term = parser.ParseTerm();

    if (!term)
        return;

    {
        TermText* file_type = term->isText();
        if (!file_type || file_type->value() != "MISSIONLIST") {
            return;
        }
    }

    do {
        delete term; term = nullptr;
        term = parser.ParseTerm();

        if (term) {
            TermDef* def = term->isDef();
            if (def && def->name()->value() == "mission") {
                if (def->term() && def->term()->isStruct()) {
                    TermStruct* val = def->term()->isStruct();

                    int     id = 0;
                    FString mname;
                    FString desc;
                    char    script[256];
                    char    system[256];
                    char    region[256];
                    int     start = 0;
                    int     type = 0;

                    ZeroMemory(script, sizeof(script));
                    strcpy_s(system, "Unknown");
                    strcpy_s(region, "Unknown");

                    for (int i = 0; i < val->elements()->size(); i++) {
                        TermDef* pdef = val->elements()->at(i)->isDef();
                        if (!pdef) continue;

                        const FString PName = pdef->name()->value().data();

                        if (PName == "id") {
                            GetDefNumber(id, pdef, filename);
                        }
                        else if (PName == "name") {
                            GetDefText(mname, pdef, filename);
                            mname = Game::GetText(mname);
                        }
                        else if (PName == "desc") {
                            GetDefText(desc, pdef, filename);
                            if (desc.Len() > 0 && desc.Len() < 32)
                                desc = Game::GetText(desc);
                        }
                        else if (PName == "start") {
                            GetDefTime(start, pdef, filename);
                        }
                        else if (PName == "system") {
                            GetDefText(system, pdef, filename);
                        }
                        else if (PName == "region") {
                            GetDefText(region, pdef, filename);
                        }
                        else if (PName == "script") {
                            GetDefText(script, pdef, filename);
                        }
                        else if (PName == "type") {
                            char typestr[64];
                            GetDefText(typestr, pdef, filename);
                            type = Mission::TypeFromName(typestr);
                        }
                    }

                    MissionInfo* info = new MissionInfo;
                    if (info) {
                        info->id = id;
                        info->name = mname;
                        info->description = desc;
                        info->system = FString(system);
                        info->region = FString(region);
                        info->script = FString(script);
                        info->start = start;
                        info->type = type;
                        info->mission = nullptr;

                        missions.append(info);
                    }
                    else {
                        ok = false;
                    }
                }
            }
        }
    } while (term);

    loader->ReleaseBuffer(block);

    if (!ok)
        Unload();
}

void Campaign::LoadCustomMissions(DataLoader* loader)
{
    bool ok = true;

    List<Text> files;
    loader->UseFileSystem(true);
    loader->ListFiles("*.*", files);

    for (int i = 0; i < files.size(); i++) {
        Text file = *files[i];
        file.setSensitive(false);

        if (file.contains(".def")) {
            BYTE* block = nullptr;
            const char* filename = file.data();

            loader->UseFileSystem(true);
            loader->LoadBuffer(filename, block, true);
            loader->UseFileSystem(Starshatter::UseFileSystem());

            if (strstr((const char*)block, "MISSION") == (const char*)block) {
                FString name;
                FString desc;
                FString system = "Unknown";
                FString region = "Unknown";
                int start = 0;
                int type = 0;
                int msn_id = 0;

                Parser parser(new BlockReader((const char*)block));
                Term* term = parser.ParseTerm();

                if (!term) {
                    loader->ReleaseBuffer(block);
                    continue;
                }

                {
                    TermText* file_type = term->isText();
                    if (!file_type || file_type->value() != "MISSION") {
                        loader->ReleaseBuffer(block);
                        continue;
                    }
                }

                do {
                    delete term; term = nullptr;
                    term = parser.ParseTerm();

                    if (term) {
                        TermDef* def = term->isDef();
                        if (!def) continue;

                        const FString DName = def->name()->value().data();

                        if (DName == "name") {
                            GetDefText(name, def, filename);
                            name = Game::GetText(name);
                        }
                        else if (DName == "type") {
                            char typestr[64];
                            GetDefText(typestr, def, filename);
                            type = Mission::TypeFromName(typestr);
                        }
                        else if (DName == "id") {
                            GetDefNumber(msn_id, def, filename);
                        }
                        else if (DName == "desc") {
                            GetDefText(desc, def, filename);
                            if (desc.Len() > 0 && desc.Len() < 32)
                                desc = Game::GetText(desc);
                        }
                        else if (DName == "system") {
                            GetDefText(system, def, filename);
                        }
                        else if (DName == "region") {
                            GetDefText(region, def, filename);
                        }
                        else if (DName == "start") {
                            GetDefTime(start, def, filename);
                        }
                    }
                } while (term);

                loader->ReleaseBuffer(block);

                // Legacy ID inference:
                if (strstr(filename, "custom") == filename) {
                    sscanf_s(filename + 6, "%d", &msn_id);
                    if (msn_id <= i) msn_id = i + 1;
                }
                else if (msn_id < 1) {
                    msn_id = i + 1;
                }

                MissionInfo* info = new MissionInfo;
                if (info) {
                    info->id = msn_id;
                    info->name = name;
                    info->type = type;
                    info->description = desc;
                    info->system = system;
                    info->region = region;
                    info->script = FString(filename);
                    info->start = start;
                    info->mission = nullptr;

                    missions.append(info);
                }
                else {
                    ok = false;
                }
            }
            else {
                loader->ReleaseBuffer(block);
            }
        }
    }

    files.destroy();

    if (!ok)
        Unload();
    else
        missions.sort();
}

void Campaign::LoadTemplateList(DataLoader* loader)
{
    BYTE* block = nullptr;
    const char* filename = "Templates.def";

    loader->UseFileSystem(true);
    loader->LoadBuffer(filename, block, true);
    loader->UseFileSystem(Starshatter::UseFileSystem());

    Parser parser(new BlockReader((const char*)block));
    Term* term = parser.ParseTerm();

    if (!term)
        return;

    {
        TermText* file_type = term->isText();
        if (!file_type || file_type->value() != "TEMPLATELIST") {
            return;
        }
    }

    do {
        delete term; term = nullptr;
        term = parser.ParseTerm();

        if (term) {
            TermDef* def = term->isDef();
            if (def && def->name()->value() == "mission") {
                if (def->term() && def->term()->isStruct()) {
                    TermStruct* val = def->term()->isStruct();

                    char nameBuf[256];
                    char script[256];
                    char region[256];

                    int id = 0;
                    int msn_type = 0;
                    int grp_type = 0;

                    int min_rank = 0;
                    int max_rank = 0;
                    int action_id = 0;
                    int action_status = 0;
                    int exec_once = 0;
                    int start_before = TIME_NEVER;
                    int start_after = 0;

                    nameBuf[0] = 0;
                    script[0] = 0;
                    region[0] = 0;

                    for (int i = 0; i < val->elements()->size(); i++) {
                        TermDef* pdef = val->elements()->at(i)->isDef();
                        if (!pdef) continue;

                        const FString PName = pdef->name()->value().data();

                        if (PName == "id") GetDefNumber(id, pdef, filename);
                        else if (PName == "name") GetDefText(nameBuf, pdef, filename);
                        else if (PName == "script") GetDefText(script, pdef, filename);
                        else if (PName == "rgn" || PName == "region") GetDefText(region, pdef, filename);
                        else if (PName == "type") {
                            char typestr[64];
                            GetDefText(typestr, pdef, filename);
                            msn_type = Mission::TypeFromName(typestr);
                        }
                        else if (PName == "group") {
                            char typestr[64];
                            GetDefText(typestr, pdef, filename);
                            grp_type = CombatGroup::TypeFromName(typestr);
                        }
                        else if (PName == "min_rank") GetDefNumber(min_rank, pdef, filename);
                        else if (PName == "max_rank") GetDefNumber(max_rank, pdef, filename);
                        else if (PName == "action_id") GetDefNumber(action_id, pdef, filename);
                        else if (PName == "action_status") GetDefNumber(action_status, pdef, filename);
                        else if (PName == "exec_once") GetDefNumber(exec_once, pdef, filename);
                        else if (PName.Contains("before")) {
                            if (pdef->term()->isNumber()) GetDefNumber(start_before, pdef, filename);
                            else { GetDefTime(start_before, pdef, filename); start_before -= ONE_DAY; }
                        }
                        else if (PName.Contains("after")) {
                            if (pdef->term()->isNumber()) GetDefNumber(start_after, pdef, filename);
                            else { GetDefTime(start_after, pdef, filename); start_after -= ONE_DAY; }
                        }
                    }

                    MissionInfo* info = new MissionInfo;
                    if (info) {
                        info->id = id;
                        info->name = FString(nameBuf);
                        info->script = FString(script);
                        info->region = FString(region);
                        info->type = msn_type;
                        info->min_rank = min_rank;
                        info->max_rank = max_rank;
                        info->action_id = action_id;
                        info->action_status = action_status;
                        info->exec_once = exec_once;
                        info->start_before = start_before;
                        info->start_after = start_after;

                        TemplateList* templist = GetTemplateList(msn_type, grp_type);
                        if (!templist) {
                            templist = new TemplateList;
                            templist->mission_type = msn_type;
                            templist->group_type = grp_type;
                            templates.append(templist);
                        }

                        templist->missions.append(info);
                    }
                }
            }
        }
    } while (term);

    loader->ReleaseBuffer(block);
}

void Campaign::CreatePlanners()
{
    if (planners.size() > 0)
        planners.destroy();

    CampaignPlan* p = nullptr;

    // PLAN EVENT MUST BE FIRST PLANNER:
    p = new CampaignPlanEvent(this);
    if (p) planners.append(p);

    p = new CampaignPlanStrategic(this);
    if (p) planners.append(p);

    p = new CampaignPlanAssignment(this);
    if (p) planners.append(p);

    p = new CampaignPlanMovement(this);
    if (p) planners.append(p);

    p = new CampaignPlanMission(this);
    if (p) planners.append(p);

    if (lockout > 0 && planners.size()) {
        ListIter<CampaignPlan> plan = planners;
        while (++plan)
            plan->SetLockout(lockout);
    }
}

int Campaign::GetPlayerIFF()
{
    int iff = 1;
    if (player_group)
        iff = player_group->GetIFF();
    return iff;
}

void Campaign::SetPlayerGroup(CombatGroup* pg)
{
    if (player_group != pg) {
        player_group = pg;
        player_unit = nullptr;

        if (IsDynamic())
            missions.destroy();
    }
}

void Campaign::SetPlayerUnit(CombatUnit* unit)
{
    if (player_unit != unit) {
        player_unit = unit;

        if (unit)
            player_group = unit->GetCombatGroup();

        if (IsDynamic())
            missions.destroy();
    }
}

CombatZone* Campaign::GetZone(const char* rgn)
{
    ListIter<CombatZone> z = zones;
    while (++z) {
        if (z->HasRegion(rgn))
            return z.value();
    }
    return nullptr;
}

StarSystem* Campaign::GetSystem(const char* sys)
{
    return Galaxy::GetInstance()->GetSystem(sys);
}

Combatant* Campaign::GetCombatant(const char* cname)
{
    ListIter<Combatant> iter = combatants;
    while (++iter) {
        Combatant* c = iter.value();
        if (!strcmp(c->Name(), cname))
            return c;
    }
    return nullptr;
}

Mission* Campaign::GetMission()
{
    return GetMission(mission_id);
}

Mission* Campaign::GetMission(int id)
{
    if (id < 0)
        return nullptr;

    if (mission && mission->Identity() == id)
        return mission;

    MissionInfo* info = nullptr;
    for (int i = 0; !info && i < missions.size(); i++)
        if (missions[i]->id == id)
            info = missions[i];

    if (info) {
        if (!info->mission) {
            info->mission = new Mission(id, TCHAR_TO_ANSI(*info->script), TCHAR_TO_ANSI(*path));
            if (info->mission)
                info->mission->Load();
        }

        if (IsDynamic()) {
            if (info->mission) {
                if (info->mission->Situation().Equals(TEXT("Unknown"), ESearchCase::IgnoreCase)) {
                    CampaignSituationReport sitrep(this, info->mission);
                    sitrep.GenerateSituationReport();
                }
            }
        }

        return info->mission;
    }

    return nullptr;
}

Mission* Campaign::GetMissionByFile(const char* filename)
{
    if (!filename || !*filename)
        return nullptr;

    int          id = 0;
    int          maxid = 0;
    MissionInfo* info = nullptr;

    for (int i = 0; !info && i < missions.size(); i++) {
        MissionInfo* m = missions[i];
        if (m->id > maxid) maxid = m->id;
        if (m->script == FString(filename))
            info = m;
    }

    if (info) {
        id = info->id;

        if (!info->mission) {
            info->mission = new Mission(id, TCHAR_TO_ANSI(*info->script), TCHAR_TO_ANSI(*path));
            if (info->mission)
                info->mission->Load();
        }

        if (IsDynamic()) {
            if (info->mission) {
                if (info->mission->Situation().Equals(TEXT("Unknown"), ESearchCase::IgnoreCase)) {
                    CampaignSituationReport sitrep(this, info->mission);
                    sitrep.GenerateSituationReport();
                }
            }
        }
    }
    else {
        info = new MissionInfo;
        if (info) {
            info->id = maxid + 1;
            info->name = TEXT("New Custom Mission");
            info->script = FString(filename);
            info->mission = new Mission(info->id, TCHAR_TO_ANSI(*info->script), "Mods/Missions/");
            info->mission->SetName(info->name);

            missions.append(info);
        }
    }

    return info ? info->mission : nullptr;
}

MissionInfo* Campaign::CreateNewMission()
{
    int maxid = 0;

    if (campaign_id == MULTIPLAYER_MISSIONS)
        maxid = 10;

    for (int i = 0; i < missions.size(); i++) {
        if (missions[i]->id > maxid)
            maxid = missions[i]->id;
    }

    char filename[64];
    sprintf_s(filename, "custom%03d.def", maxid + 1);

    MissionInfo* info = new MissionInfo;
    if (info) {
        info->id = maxid + 1;
        info->name = TEXT("New Custom Mission");
        info->script = FString(filename);
        info->mission = new Mission(info->id, filename, TCHAR_TO_ANSI(*path));
        info->mission->SetName(info->name);

        missions.append(info);
    }

    return info;
}

void Campaign::DeleteMission(int id)
{
    if (id < 0)
        return;

    MissionInfo* m = nullptr;

    for (int i = 0; !m && i < missions.size(); i++) {
        if (missions[i]->id == id)
            m = missions[i];
    }

    if (m) {
        // UE file delete:
        const FString FullPath = FPaths::Combine(path, m->script);
        IPlatformFile& PF = FPlatformFileManager::Get().GetPlatformFile();
        PF.DeleteFile(*FullPath);

        Load();
    }
}

MissionInfo* Campaign::GetMissionInfo(int id)
{
    if (id < 0)
        return nullptr;

    MissionInfo* m = nullptr;
    for (int i = 0; !m && i < missions.size(); i++)
        if (missions[i]->id == id)
            m = missions[i];

    if (m) {
        if (!m->mission) {
            m->mission = new Mission(id, TCHAR_TO_ANSI(*m->script));
            if (m->mission)
                m->mission->Load();
        }
        return m;
    }

    return nullptr;
}

void Campaign::ReloadMission(int id)
{
    if (mission && mission == net_mission) {
        delete net_mission;
        net_mission = nullptr;
    }

    mission = nullptr;

    if (id >= 0 && id < missions.size()) {
        MissionInfo* m = missions[id];
        delete m->mission;
        m->mission = nullptr;
    }
}

void Campaign::LoadNetMission(int id, const char* net_mission_script)
{
    if (mission && mission == net_mission) {
        delete net_mission;
        net_mission = nullptr;
    }

    mission_id = id;
    mission = new Mission(id);

    if (mission && mission->ParseMission(net_mission_script))
        mission->Validate();

    net_mission = mission;
}

CombatAction* Campaign::FindAction(int action_id)
{
    ListIter<CombatAction> iter = actions;
    while (++iter) {
        CombatAction* a = iter.value();
        if (a->Identity() == action_id)
            return a;
    }
    return nullptr;
}

MissionInfo* Campaign::FindMissionTemplate(int mission_type, CombatGroup* in_player_group)
{
    MissionInfo* info = nullptr;
    if (!in_player_group)
        return nullptr;

    TemplateList* templ = GetTemplateList(mission_type, in_player_group->Type());
    if (!templ || !templ->missions.size())
        return nullptr;

    int tries = 0;
    int msize = templ->missions.size();

    while (!info && tries < msize) {
        int index = templ->index;
        if (index >= msize) index = 0;

        info = templ->missions[index];
        templ->index = index + 1;
        tries++;

        if (info) {
            if (info->action_id) {
                CombatAction* a = FindAction(info->action_id);
                if (a && a->Status() != info->action_status)
                    info = nullptr;
            }

            if (info && !info->IsAvailable())
                info = nullptr;
        }
    }

    return info;
}

TemplateList* Campaign::GetTemplateList(int msn_type, int grp_type)
{
    for (int i = 0; i < templates.size(); i++) {
        if (templates[i]->mission_type == msn_type &&
            templates[i]->group_type == grp_type)
            return templates[i];
    }
    return nullptr;
}

void Campaign::SetMissionId(int id)
{
    if (id > 0)
        mission_id = id;
}

double Campaign::Stardate()
{
    return StarSystem::Stardate();
}

void Campaign::SelectDefaultPlayerGroup(CombatGroup* g, int type)
{
    if (player_group || !g) return;

    if (g->Type() == type && !g->IsReserve() && g->Value() > 0) {
        player_group = g;
        player_unit = nullptr;
        return;
    }

    for (int i = 0; i < g->GetComponents().size(); i++)
        SelectDefaultPlayerGroup(g->GetComponents()[i], type);
}

void Campaign::Prep()
{
    if (IsDynamic() && combatants.isEmpty()) {
        DataLoader* loader = DataLoader::GetLoader();
        loader->SetDataPath(TCHAR_TO_ANSI(*path));
        LoadCampaign(loader, true);
    }

    StarSystem::SetBaseTime(loadTime);

    if (IsScripted() && actions.isEmpty()) {
        DataLoader* loader = DataLoader::GetLoader();
        loader->SetDataPath(TCHAR_TO_ANSI(*path));
        LoadCampaign(loader, true);

        ListIter<MissionInfo> m = missions;
        while (++m)
            GetMission(m->id);
    }

    CheckPlayerGroup();
}

void Campaign::Start()
{
    Prep();
    CreatePlanners();
    SetStatus(CAMPAIGN_ACTIVE);
}
// --------------------------------------------------------------------
// ExecFrame onwards
// --------------------------------------------------------------------

void
Campaign::ExecFrame()
{
    if (InCutscene())
        return;

    time = Stardate() - startTime;

    if (status < CAMPAIGN_ACTIVE)
        return;

    if (IsDynamic()) {
        bool completed = false;

        // Starshatter ListIter pattern preserved using your List<> shim.
        ListIter<MissionInfo> m = missions;
        while (++m) {
            if (m->mission && m->mission->IsComplete()) {
                // Replace ::Print with your project logging policy:
                // UE_LOG(LogTemp, Log, TEXT("Campaign::ExecFrame() completed mission %d '%s'"), m->id, *m->name);
                completed = true;
            }
        }

        if (completed) {
            // UE_LOG(LogTemp, Log, TEXT("Campaign::ExecFrame() destroying mission list after completion..."));
            missions.destroy();

            if (!player_group || player_group->IsFighterGroup())
                time += 10 * 3600;
            else
                time += 20 * 3600;

            StarSystem::SetBaseTime(startTime + time - Game::GameTime() / 1000.0);
        }
        else {
            m.reset();

            while (++m) {
                if (m->start < time && !m->mission->IsActive()) {
                    MissionInfo* info = m.removeItem();

                    // if (info) UE_LOG(LogTemp, Log, TEXT("Campaign::ExecFrame() deleting expired mission %d start: %d current: %d"),
                    //     info->id, info->start, (int)time);

                    delete info;
                }
            }
        }

        // PLAN EVENT MUST BE FIRST PLANNER:
        if (loaded_from_savegame && planners.size() > 0) {
            CampaignPlanEvent* plan_event = (CampaignPlanEvent*)planners.first();
            if (plan_event) {
                plan_event->ExecScriptedEvents();
            }

            loaded_from_savegame = false;
        }

        ListIter<CampaignPlan> plan = planners;
        while (++plan) {
            CheckPlayerGroup();
            plan->ExecFrame();
        }

        CheckPlayerGroup();

        // Auto save game AFTER planners have run!
        if (completed) {
            CampaignSaveGame save(this);
            save.SaveAuto();
        }
    }
    else {
        // PLAN EVENT MUST BE FIRST PLANNER:
        if (planners.size() > 0) {
            CampaignPlanEvent* plan_event = (CampaignPlanEvent*)planners.first();
            if (plan_event) {
                plan_event->ExecScriptedEvents();
            }
        }
    }
}

// +--------------------------------------------------------------------+

void
Campaign::LockoutEvents(int seconds)
{
    lockout = seconds;
}

void
Campaign::CheckPlayerGroup()
{
    if (!player_group || player_group->IsReserve() || player_group->CalcValue() < 1) {
        int player_iff = GetPlayerIFF();
        player_group = nullptr;

        CombatGroup* force = nullptr;
        for (int i = 0; i < combatants.size() && !force; i++) {
            if (combatants[i]->GetIFF() == player_iff) {
                force = combatants[i]->GetForce();
            }
        }

        if (force) {
            force->CalcValue();
            SelectDefaultPlayerGroup(force, CombatGroup::WING);

            if (!player_group)
                SelectDefaultPlayerGroup(force, CombatGroup::DESTROYER_SQUADRON);
        }
    }

    if (player_unit && player_unit->GetValue() < 1)
        SetPlayerUnit(nullptr);
}

// +--------------------------------------------------------------------+
// FPU helpers — keep names; implement as no-ops unless you truly need x87 mode
// +--------------------------------------------------------------------+

void FPU2Extended() {}
void FPURestore() {}

void
Campaign::StartMission()
{
    Mission* m = GetMission();

    if (m) {
        // UE_LOG(LogTemp, Log, TEXT("Campaign Start Mission - %d. '%s'"), m->Identity(), *FString(m->Name()));

        if (!scripted) {
            FPU2Extended();

            double gtime = (double)Game::GameTime() / 1000.0;
            double base = startTime + m->Start() - 15 - gtime;

            StarSystem::SetBaseTime(base);

            double current_time = Stardate() - startTime;

            // Original used FormatDayTime; keep your existing port if present.
            // FString CurrentStr = FormatDayTime(current_time);
            // FString StartStr   = FormatDayTime(m->Start());
            // UE_LOG(LogTemp, Log, TEXT("  current time:  %s"), *CurrentStr);
            // UE_LOG(LogTemp, Log, TEXT("  mission start: %s"), *StartStr);

            FPURestore();
        }
    }
}

void
Campaign::RollbackMission()
{
    // UE_LOG(LogTemp, Log, TEXT("Campaign::RollbackMission()"));

    Mission* m = GetMission();

    if (m) {
        if (!scripted) {
            FPU2Extended();

            double gtime = (double)Game::GameTime() / 1000.0;
            double base = startTime + m->Start() - 60 - gtime;

            StarSystem::SetBaseTime(base);

            // double current_time = Stardate() - startTime;
            // UE_LOG(LogTemp, Log, TEXT("  mission start: %d"), m->Start());
            // UE_LOG(LogTemp, Log, TEXT("  current time:  %d"), (int)current_time);

            FPURestore();
        }

        m->SetActive(false);
        m->SetComplete(false);
    }
}

// +--------------------------------------------------------------------+

bool
Campaign::InCutscene() const
{
    Starshatter* stars = Starshatter::GetInstance();
    return stars ? stars->InCutscene() : false;
}

bool
Campaign::IsDynamic() const
{
    return campaign_id >= DYNAMIC_CAMPAIGN &&
        campaign_id < SINGLE_MISSIONS;
}

bool
Campaign::IsTraining() const
{
    return campaign_id == TRAINING_CAMPAIGN;
}

bool
Campaign::IsScripted() const
{
    return scripted;
}

bool
Campaign::IsSequential() const
{
    return sequential;
}

// +--------------------------------------------------------------------+
// FindGroup helpers (static recursion preserved)
// +--------------------------------------------------------------------+

static CombatGroup* FindGroup_Recurse(CombatGroup* g, int type, int id)
{
    if (!g)
        return nullptr;

    if (g->Type() == type && g->GetID() == id)
        return g;

    CombatGroup* result = nullptr;

    ListIter<CombatGroup> subgroup = g->GetComponents();
    while (++subgroup && !result) {
        result = FindGroup_Recurse(subgroup.value(), type, id);
    }

    return result;
}

CombatGroup*
Campaign::FindGroup(int iff, int type, int id)
{
    CombatGroup* result = nullptr;

    ListIter<Combatant> combatant = combatants;
    while (++combatant && !result) {
        if (combatant->GetIFF() == iff) {
            result = FindGroup_Recurse(combatant->GetForce(), type, id);
        }
    }

    return result;
}

// +--------------------------------------------------------------------+

static void FindGroups_Recurse(CombatGroup* g, int type, CombatGroup* near_group, List<CombatGroup>& groups)
{
    if (!g)
        return;

    if (g->GetType() == type && g->IntelLevel() > Intel::RESERVE) {
        if (!near_group || g->GetAssignedZone() == near_group->GetAssignedZone())
            groups.append(g);
    }

    ListIter<CombatGroup> subgroup = g->GetComponents();
    while (++subgroup) {
        FindGroups_Recurse(subgroup.value(), type, near_group, groups);
    }
}

CombatGroup*
Campaign::FindGroup(int iff, int type, CombatGroup* near_group)
{
    CombatGroup* result = nullptr;
    List<CombatGroup> groups;

    ListIter<Combatant> combatant = combatants;
    while (++combatant) {
        if (combatant->GetIFF() == iff) {
            FindGroups_Recurse(combatant->GetForce(), type, near_group, groups);
        }
    }

    if (groups.size() > 0) {
        const int index = RandomIntInclusive(0, groups.size() - 1);
        result = groups[index];
    }

    return result;
}

// +--------------------------------------------------------------------+

static void FindStrikeTargets_Recurse(CombatGroup* g, CombatGroup* strike_group, List<CombatGroup>& groups)
{
    if (!g || !strike_group || !strike_group->GetAssignedZone())
        return;

    if (g->IsStrikeTarget() && g->IntelLevel() > Intel::RESERVE) {
        if (strike_group->GetAssignedZone() == g->GetAssignedZone() ||
            strike_group->GetAssignedZone()->HasRegion(g->GetRegion()))
        {
            groups.append(g);
        }
    }

    ListIter<CombatGroup> subgroup = g->GetComponents();
    while (++subgroup) {
        FindStrikeTargets_Recurse(subgroup.value(), strike_group, groups);
    }
}

CombatGroup*
Campaign::FindStrikeTarget(int iff, CombatGroup* strike_group)
{
    CombatGroup* result = nullptr;
    List<CombatGroup> groups;

    ListIter<Combatant> combatant = GetCombatants();
    while (++combatant) {
        if (combatant->GetIFF() != 0 && combatant->GetIFF() != iff) {
            FindStrikeTargets_Recurse(combatant->GetForce(), strike_group, groups);
        }
    }

    if (groups.size() > 0) {
        const int index = RandomIntInclusive(0, groups.size() - 1);
        result = groups[index];
    }

    return result;
}

// +--------------------------------------------------------------------+

void
Campaign::CommitExpiredActions()
{
    ListIter<CombatAction> iter = actions;
    while (++iter) {
        CombatAction* a = iter.value();

        if (a && a->IsAvailable())
            a->SetStatus(CombatAction::COMPLETE);
    }

    updateTime = time;
}

// +--------------------------------------------------------------------+

int
Campaign::GetPlayerTeamScore()
{
    int score_us = 0;
    int score_them = 0;

    if (player_group) {
        int iff = player_group->GetIFF();

        ListIter<Combatant> iter = combatants;
        while (++iter) {
            Combatant* c = iter.value();
            if (!c) continue;

            if (iff <= 1) {
                if (c->GetIFF() <= 1)
                    score_us += c->Score();
                else
                    score_them += c->Score();
            }
            else {
                if (c->GetIFF() <= 1)
                    score_them += c->Score();
                else
                    score_us += c->Score();
            }
        }
    }

    return score_us - score_them;
}

// +--------------------------------------------------------------------+

void
Campaign::SetStatus(int s)
{
    status = s;

    // record the win in player profile:
    if (status == CAMPAIGN_SUCCESS) {
        Player* player = Player::GetCurrentPlayer();
        if (player)
            player->SetCampaignComplete(campaign_id);
    }

    if (status > CAMPAIGN_ACTIVE) {
        // UE_LOG(LogTemp, Log, TEXT("Campaign::SetStatus() destroying mission list at campaign end"));
        missions.destroy();
    }
}

// +--------------------------------------------------------------------+

static void GetCombatUnits_Recurse(CombatGroup* g, List<CombatUnit>& units)
{
    if (!g)
        return;

    ListIter<CombatUnit> unit = g->GetUnits();
    while (++unit) {
        CombatUnit* u = unit.value();
        if (!u) continue;

        if (u->Count() - u->DeadCount() > 0)
            units.append(u);
    }

    ListIter<CombatGroup> comp = g->GetComponents();
    while (++comp) {
        CombatGroup* g2 = comp.value();
        if (g2 && !g2->IsReserve())
            GetCombatUnits_Recurse(g2, units);
    }
}

int
Campaign::GetAllCombatUnits(int iff, List<CombatUnit>& units)
{
    units.clear();

    ListIter<Combatant> iter = combatants;
    while (++iter) {
        Combatant* c = iter.value();
        if (!c) continue;

        if (iff < 0 || c->GetIFF() == iff) {
            GetCombatUnits_Recurse(c->GetForce(), units);
        }
    }

    return units.size();
}