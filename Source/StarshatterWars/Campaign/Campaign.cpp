/*  Project Starshatter Wars
    Fractal Dev Studios
    Copyright © 2025-2026. All Rights Reserved.

    SUBSYSTEM:    Stars.exe
    FILE:         Campaign.cpp
    AUTHOR:       Carlos Bott

    ORIGINAL AUTHOR AND STUDIO
    ==========================
    John DiCamillo / Destroyer Studios LLC

    OVERVIEW
    ========
    Campaign defines a strategic military scenario.
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
#include "MissionInfo.h"
#include "StarSystem.h"
#include "Starshatter.h"
#include "PlayerCharacter.h"

#include "Game.h"
#include "Bitmap.h"
#include "DataLoader.h"
#include "ParseUtil.h"
#include "FormatUtil.h"
#include "GameScreen.h"
#include "AwardInfoRegistry.h"
#include "StarshatterPlayerSubsystem.h"

// Unreal minimal support:
#include "CoreMinimal.h"
#include "HAL/FileManager.h"
#include "Misc/Paths.h"


DEFINE_LOG_CATEGORY_STATIC(LogCampaign, Log, All);

const int TIME_NEVER = (int)1e9;
const int ONE_DAY = (int)24 * 3600;

TemplateList::TemplateList()
    : mission_type(0)
    , group_type(0)
    , index(0)
{
}

TemplateList::~TemplateList()
{
    missions.destroy();
}

// +====================================================================+

static List<Campaign> campaigns;
static Campaign* current_campaign = 0;

// +--------------------------------------------------------------------+

Campaign::Campaign(int id, const char* n)
    : campaign_id(id)
    , name(n)
    , mission_id(-1)
    , mission(0)
    , net_mission(0)
    , scripted(false)
    , sequential(false)
    , time(0)
    , startTime(0)
    , loadTime(0)
    , player_group(0)
    , player_unit(0)
    , campaign_status(ECampaignStatus::INIT)
    , lockout(0)
    , loaded_from_savegame(false)
{
    FMemory::Memzero(path, sizeof(path));
    Load();
}

Campaign::Campaign(int id, const char* n, const char* p)
    : campaign_id(id)
    , name(n)
    , mission_id(-1)
    , mission(0)
    , net_mission(0)
    , scripted(false)
    , sequential(false)
    , time(0)
    , startTime(0)
    , loadTime(0)
    , player_group(0)
    , player_unit(0)
    , campaign_status(ECampaignStatus::INIT)
    , lockout(0)
    , loaded_from_savegame(false)
{
    FMemory::Memzero(path, sizeof(path));
    FCStringAnsi::Strncpy(path, p ? p : "", (int)sizeof(path));
    Load();
}

// +--------------------------------------------------------------------+

Campaign::~Campaign()
{
    for (int i = 0; i < NUM_IMAGES; i++)
        image[i].ClearImage();

    delete net_mission;

    actions.destroy();
    events.destroy();
    missions.destroy();
    templates.destroy();
    planners.destroy();
    zones.destroy();
    combatants.destroy();
}

// +--------------------------------------------------------------------+

void
Campaign::Initialize()
{
    Campaign* c = 0;
    DataLoader* loader = DataLoader::GetLoader();

    for (int i = 1; i < 100; i++) {
        char TempPath[256];
        FCStringAnsi::Snprintf(TempPath, sizeof(TempPath), "Campaigns/%02d/", i);

        loader->UseFileSystem(true);
        loader->SetDataPath(TempPath);

        if (loader->FindFile("campaign.def")) {
            char txt[256];
            FCStringAnsi::Snprintf(txt, sizeof(txt), "Dynamic Campaign %02d", i);

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
    if (c) {
        campaigns.insertSort(c);
    }

    c = new Campaign(CUSTOM_MISSIONS, "Custom Missions");
    if (c) {
        campaigns.insertSort(c);
    }
}

void
Campaign::Close()
{
    UE_LOG(LogCampaign, Log, TEXT("Campaign::Close() - destroying all campaigns"));
    current_campaign = 0;
    campaigns.destroy();
}

Campaign*
Campaign::GetCampaign()
{
    return current_campaign;
}

Campaign*
Campaign::SelectCampaign(const char* InName)
{
    Campaign* c = 0;
    ListIter<Campaign> iter = campaigns;

    while (++iter && !c) {
        if (InName && FCStringAnsi::Stricmp(iter->Name(), InName) == 0)
            c = iter.value();
    }

    if (c) {
        UE_LOG(LogCampaign, Log, TEXT("Campaign: Selected '%s'"), ANSI_TO_TCHAR(c->Name()));
        current_campaign = c;
    }
    else {
        UE_LOG(LogCampaign, Warning, TEXT("Campaign: could not find '%s'"), InName ? ANSI_TO_TCHAR(InName) : TEXT("(null)"));
    }

    return c;
}

Campaign*
Campaign::CreateCustomCampaign(const char* InName, const char* InPath)
{
    int id = 0;

    if (InName && *InName && InPath && *InPath) {
        ListIter<Campaign> iter = campaigns;

        while (++iter) {
            Campaign* c = iter.value();

            if (c->GetCampaignId() >= id)
                id = c->GetCampaignId() + 1;

            if (FCStringAnsi::Strcmp(c->Name(), InName) == 0) {
                UE_LOG(LogCampaign, Warning, TEXT("Campaign: custom campaign '%s' already exists."), ANSI_TO_TCHAR(InName));
                return 0;
            }
        }
    }

    if (id == 0)
        id = CUSTOM_MISSIONS + 1;

    Campaign* c = new Campaign(id, InName, InPath);
    UE_LOG(LogCampaign, Log, TEXT("Campaign: created custom campaign %d '%s'"), id, InName ? ANSI_TO_TCHAR(InName) : TEXT("(null)"));
    campaigns.append(c);

    return c;
}

List<Campaign>&
Campaign::GetAllCampaigns()
{
    return campaigns;
}

int
Campaign::GetLastCampaignId()
{
    int result = 0;

    for (int i = 0; i < campaigns.size(); i++) {
        Campaign* c = campaigns.at(i);

        if (c->IsDynamic() && c->GetCampaignId() > result) {
            result = c->GetCampaignId();
        }
    }

    return result;
}

// +--------------------------------------------------------------------+

CombatEvent*
Campaign::GetLastEvent()
{
    CombatEvent* result = 0;

    if (!events.isEmpty())
        result = events.last();

    return result;
}

// +--------------------------------------------------------------------+

int
Campaign::CountNewEvents() const
{
    int result = 0;

    for (int i = 0; i < events.size(); i++) {
        if (!events[i]->Visited())
            result++;
    }

    return result;
}

// +--------------------------------------------------------------------+

void
Campaign::Clear()
{
    missions.destroy();
    planners.destroy();
    combatants.destroy();
    events.destroy();
    actions.destroy();

    player_group = 0;
    player_unit = 0;

    updateTime = time;
}

// +--------------------------------------------------------------------+

void
Campaign::Load()
{
    // first, unload any existing data:
    Unload();

    if (!path[0]) {
        // then load the campaign from files:
        switch (campaign_id) {
        case SINGLE_MISSIONS:      FCStringAnsi::Strcpy(path, "Missions/");       break;
        case CUSTOM_MISSIONS:      FCStringAnsi::Strcpy(path, "Mods/Missions/");  break;
        case MULTIPLAYER_MISSIONS: FCStringAnsi::Strcpy(path, "Multiplayer/");    break;
        default:                   FCStringAnsi::Snprintf(path, sizeof(path), "Campaigns/%02d/", campaign_id); break;
        }
    }

    DataLoader* loader = DataLoader::GetLoader();
    loader->UseFileSystem(true);
    loader->SetDataPath(path);
    systems.clear();

    if (loader->FindFile("zones.def"))
        zones.append(CombatZone::Load("zones.def"));

    for (int i = 0; i < zones.size(); i++) {
        Text s = zones[i]->GetSystem();
        bool found = false;

        for (int n = 0; !found && n < systems.size(); n++) {
            if (s == systems[n]->Name())
                found = true;
        }

        if (!found)
            systems.append(Galaxy::GetInstance()->GetSystem(s));
    }

    loader->UseFileSystem(Starshatter::UseFileSystem());

    if (loader->FindFile("campaign.def"))
        LoadCampaign(loader);

    if (campaign_id == CUSTOM_MISSIONS) {
        loader->SetDataPath(path);
        LoadCustomMissions(loader);
    }
    else {
        bool found = false;

        if (loader->FindFile("missions.def")) {
            loader->SetDataPath(path);
            LoadMissionList(loader);
            found = true;
        }

        if (loader->FindFile("templates.def")) {
            loader->SetDataPath(path);
            LoadTemplateList(loader);
            found = true;
        }

        if (!found) {
            loader->SetDataPath(path);
            LoadCustomMissions(loader);
        }
    }

    loader->UseFileSystem(true);
    loader->SetDataPath(path);

    if (loader->FindFile("image.pcx")) {
        loader->LoadGameBitmap("image.pcx", image[0]);
        loader->LoadGameBitmap("selected.pcx", image[1]);
        loader->LoadGameBitmap("unavail.pcx", image[2]);
        loader->LoadGameBitmap("banner.pcx", image[3]);
    }

    loader->SetDataPath(0);
    loader->UseFileSystem(Starshatter::UseFileSystem());
}

void
Campaign::Unload()
{
    SetCampaignStatus(ECampaignStatus::INIT);

    Game::ResetGameTime();
    StarSystem::SetBaseTime(0);

    startTime = Stardate();
    loadTime = startTime;
    lockout = 0;

    for (int i = 0; i < NUM_IMAGES; i++)
        image[i].ClearImage();

    Clear();

    zones.destroy();
}

void
Campaign::LoadCampaign(DataLoader* loader, bool full)
{
    BYTE* block = 0;
    const char* SourceFilename = "campaign.def";

    loader->UseFileSystem(true);
    loader->LoadBuffer(SourceFilename, block, true);
    loader->UseFileSystem(Starshatter::UseFileSystem());

    Parser parser(new BlockReader((const char*)block));
    Term* term = parser.ParseTerm();

    if (!term) {
        return;
    }
    else {
        TermText* file_type = term->isText();
        if (!file_type || file_type->value() != "CAMPAIGN") {
            return;
        }
    }

    do {
        delete term; term = 0;
        term = parser.ParseTerm();

        if (term) {
            TermDef* def = term->isDef();
            if (def) {
                if (def->name()->value() == "name") {
                    if (!def->term() || !def->term()->isText()) {
                        UE_LOG(LogCampaign, Warning, TEXT("WARNING: name missing in '%s/%s'"),
                            ANSI_TO_TCHAR(loader->GetDataPath() ? loader->GetDataPath() : ""),
                            ANSI_TO_TCHAR(SourceFilename));
                    }
                    else {
                        name = def->term()->isText()->value();
                        name = Game::GetText(name);
                    }
                }
                else if (def->name()->value() == "desc") {
                    if (!def->term() || !def->term()->isText()) {
                        UE_LOG(LogCampaign, Warning, TEXT("WARNING: description missing in '%s/%s'"),
                            ANSI_TO_TCHAR(loader->GetDataPath() ? loader->GetDataPath() : ""),
                            ANSI_TO_TCHAR(SourceFilename));
                    }
                    else {
                        description = def->term()->isText()->value();
                        description = Game::GetText(description);
                    }
                }
                else if (def->name()->value() == "situation") {
                    if (!def->term() || !def->term()->isText()) {
                        UE_LOG(LogCampaign, Warning, TEXT("WARNING: situation missing in '%s/%s'"),
                            ANSI_TO_TCHAR(loader->GetDataPath() ? loader->GetDataPath() : ""),
                            ANSI_TO_TCHAR(SourceFilename));
                    }
                    else {
                        situation = def->term()->isText()->value();
                        situation = Game::GetText(situation);
                    }
                }
                else if (def->name()->value() == "orders") {
                    if (!def->term() || !def->term()->isText()) {
                        UE_LOG(LogCampaign, Warning, TEXT("WARNING: orders missing in '%s/%s'"),
                            ANSI_TO_TCHAR(loader->GetDataPath() ? loader->GetDataPath() : ""),
                            ANSI_TO_TCHAR(SourceFilename));
                    }
                    else {
                        orders = def->term()->isText()->value();
                        orders = Game::GetText(orders);
                    }
                }
                else if (def->name()->value() == "scripted") {
                    if (def->term() && def->term()->isBool()) {
                        scripted = def->term()->isBool()->value();
                    }
                }
                else if (def->name()->value() == "sequential") {
                    if (def->term() && def->term()->isBool()) {
                        sequential = def->term()->isBool()->value();
                    }
                }
                else if (full && def->name()->value() == "combatant") {
                    if (!def->term() || !def->term()->isStruct()) {
                        UE_LOG(LogCampaign, Warning, TEXT("WARNING: combatant struct missing in '%s/%s'"),
                            ANSI_TO_TCHAR(loader->GetDataPath() ? loader->GetDataPath() : ""),
                            ANSI_TO_TCHAR(SourceFilename));
                    }
                    else {
                        TermStruct* val = def->term()->isStruct();

                        char        cname[64];
                        CombatGroup* force = 0;
                        CombatGroup* clone = 0;

                        FMemory::Memzero(cname, sizeof(cname));

                        for (int i = 0; i < val->elements()->size(); i++) {
                            TermDef* pdef = val->elements()->at(i)->isDef();
                            if (pdef) {
                                if (pdef->name()->value() == "name") {
                                    GetDefText(cname, pdef, SourceFilename);

                                    force = CombatRoster::GetInstance()->GetForce(cname);

                                    if (force)
                                        clone = force->Clone(false); // shallow copy
                                }
                                else if (pdef->name()->value() == "group") {
                                    ParseGroup(pdef->term()->isStruct(), force, clone, SourceFilename);
                                }
                            }
                        }

                        loader->SetDataPath(path);
                        Combatant* c = new Combatant(cname, clone);
                        if (c) {
                            combatants.append(c);
                        }
                        else {
                            Unload();
                            return;
                        }
                    }
                }
                else if (full && def->name()->value() == "action") {
                    if (!def->term() || !def->term()->isStruct()) {
                        UE_LOG(LogCampaign, Warning, TEXT("WARNING: action struct missing in '%s/%s'"),
                            ANSI_TO_TCHAR(loader->GetDataPath() ? loader->GetDataPath() : ""),
                            ANSI_TO_TCHAR(SourceFilename));
                    }
                    else {
                        TermStruct* val = def->term()->isStruct();
                        ParseAction(val, SourceFilename);
                    }
                }
            }
        }
    } while (term);

    loader->ReleaseBuffer(block);
}

// +--------------------------------------------------------------------+

void
Campaign::ParseGroup(TermStruct* val, CombatGroup* force, CombatGroup* clone, const char* SourceFilename)
{
    if (!val) {
        UE_LOG(LogCampaign, Warning, TEXT("invalid combat group in campaign %s"), ANSI_TO_TCHAR(name.data()));
        return;
    }

    ECOMBATGROUP_TYPE type = ECOMBATGROUP_TYPE::NONE;
    int id = 0;

    for (int i = 0; i < val->elements()->size(); i++) {
        TermDef* pdef = val->elements()->at(i)->isDef();
        if (pdef) {
            if (pdef->name()->value() == "type") {
                char type_name[64];
                GetDefText(type_name, pdef, SourceFilename);
                type = CombatGroup::TypeFromName(type_name);
            }
            else if (pdef->name()->value() == "id") {
                GetDefNumber(id, pdef, SourceFilename);
            }
        }
    }

    if ((int) type && id && force && clone) {
        CombatGroup* g = force->FindGroup(type, id);

        // found original group, now clone it over
        if (g && g->GetParent()) {
            CombatGroup* parent = CloneOver(force, clone, g->GetParent());
            if (parent)
                parent->AddComponent(g->Clone());
        }
    }
}

// +--------------------------------------------------------------------+

void
Campaign::ParseAction(TermStruct* val, const char* SourceFilename)
{
    if (!val) {
        UE_LOG(LogCampaign, Warning, TEXT("invalid action in campaign %s"), ANSI_TO_TCHAR(name.data()));
        return;
    }

    int     id = 0;
    int     type = 0;
    int     subtype = 0;
    int     opp_type = -1;
    int     team = 0;
    int     source = 0;
    FVector loc(0.0f, 0.0f, 0.0f);

    Text    system;
    Text    region;
    Text    file;
    Text    image_file;
    Text    scene_file;
    Text    text;

    int     count = 1;
    int     start_before = TIME_NEVER;
    int     start_after = 0;
    int     min_rank = 0;
    int     max_rank = 100;
    int     delay = 0;
    int     probability = 100;

    int     asset_type = 0;
    int     asset_id = 0;
    int     target_type = 0;
    int     target_id = 0;
    int     target_iff = 0;

    CombatAction* action = 0;

    for (int i = 0; i < val->elements()->size(); i++) {
        TermDef* pdef = val->elements()->at(i)->isDef();
        if (pdef) {
            if (pdef->name()->value() == "id") {
                GetDefNumber(id, pdef, SourceFilename);
            }
            else if (pdef->name()->value() == "type") {
                char txt[64];
                GetDefText(txt, pdef, SourceFilename);
                type = CombatAction::TypeFromName(txt);
            }
            else if (pdef->name()->value() == "subtype") {
                if (pdef->term()->isNumber()) {
                    GetDefNumber(subtype, pdef, SourceFilename);
                }
                else if (pdef->term()->isText()) {
                    char txt[64];
                    GetDefText(txt, pdef, SourceFilename);

                    if (type == CombatAction::MISSION_TEMPLATE)
                        subtype = Mission::TypeFromName(txt);
                    else if (type == CombatAction::COMBAT_EVENT)
                        subtype = CombatEvent::TypeFromName(txt);
                    else if (type == CombatAction::INTEL_EVENT)
                        subtype = Intel::IntelFromName(txt);
                }
            }
            else if (pdef->name()->value() == "opp_type") {
                if (pdef->term()->isNumber()) {
                    GetDefNumber(opp_type, pdef, SourceFilename);
                }
                else if (pdef->term()->isText()) {
                    char txt[64];
                    GetDefText(txt, pdef, SourceFilename);

                    if (type == CombatAction::MISSION_TEMPLATE)
                        opp_type = Mission::TypeFromName(txt);
                }
            }
            else if (pdef->name()->value() == "source") {
                char txt[64];
                GetDefText(txt, pdef, SourceFilename);
                source = CombatEvent::SourceFromName(txt);
            }
            else if (pdef->name()->value() == "team" || pdef->name()->value() == "iff") {
                GetDefNumber(team, pdef, SourceFilename);
            }
            else if (pdef->name()->value() == "count") {
                GetDefNumber(count, pdef, SourceFilename);
            }
            else if (pdef->name()->value().contains("before")) {
                if (pdef->term()->isNumber()) {
                    GetDefNumber(start_before, pdef, SourceFilename);
                }
                else {
                    GetDefTime(start_before, pdef, SourceFilename);
                    start_before -= ONE_DAY;
                }
            }
            else if (pdef->name()->value().contains("after")) {
                if (pdef->term()->isNumber()) {
                    GetDefNumber(start_after, pdef, SourceFilename);
                }
                else {
                    GetDefTime(start_after, pdef, SourceFilename);
                    start_after -= ONE_DAY;
                }
            }
            else if (pdef->name()->value() == "min_rank") {
                if (pdef->term()->isNumber()) {
                    GetDefNumber(min_rank, pdef, SourceFilename);
                }
                else {
                    char rank_name[64];
                    GetDefText(rank_name, pdef, SourceFilename);
                    int32 rank = UAwardInfoRegistry::RankFromName(rank_name);
                }
            }
            else if (pdef->name()->value() == "max_rank") {
                if (pdef->term()->isNumber()) {
                    GetDefNumber(max_rank, pdef, SourceFilename);
                }
                else {
                    char rank_name[64];
                    GetDefText(rank_name, pdef, SourceFilename);
                    max_rank = UAwardInfoRegistry::RankFromName(rank_name);
                }
            }
            else if (pdef->name()->value() == "delay") {
                GetDefNumber(delay, pdef, SourceFilename);
            }
            else if (pdef->name()->value() == "probability") {
                GetDefNumber(probability, pdef, SourceFilename);
            }
            else if (pdef->name()->value() == "asset_type") {
                char type_name[64];
                GetDefText(type_name, pdef, SourceFilename);
                asset_type = (int) CombatGroup::TypeFromName(type_name);
            }
            else if (pdef->name()->value() == "target_type") {
                char type_name[64];
                GetDefText(type_name, pdef, SourceFilename);
                target_type = (int) CombatGroup::TypeFromName(type_name);
            }
            else if (pdef->name()->value() == "location" || pdef->name()->value() == "loc") {
                GetDefVec(loc, pdef, SourceFilename);
            }
            else if (pdef->name()->value() == "system" || pdef->name()->value() == "sys") {
                GetDefText(system, pdef, SourceFilename);
            }
            else if (pdef->name()->value() == "region" || pdef->name()->value() == "rgn" || pdef->name()->value() == "zone") {
                GetDefText(region, pdef, SourceFilename);
            }
            else if (pdef->name()->value() == "file") {
                GetDefText(file, pdef, SourceFilename);
            }
            else if (pdef->name()->value() == "image") {
                GetDefText(image_file, pdef, SourceFilename);
            }
            else if (pdef->name()->value() == "scene") {
                GetDefText(scene_file, pdef, SourceFilename);
            }
            else if (pdef->name()->value() == "text") {
                GetDefText(text, pdef, SourceFilename);
                text = Game::GetText(text);
            }
            else if (pdef->name()->value() == "asset_id") {
                GetDefNumber(asset_id, pdef, SourceFilename);
            }
            else if (pdef->name()->value() == "target_id") {
                GetDefNumber(target_id, pdef, SourceFilename);
            }
            else if (pdef->name()->value() == "target_iff") {
                GetDefNumber(target_iff, pdef, SourceFilename);
            }
            else if (pdef->name()->value() == "asset_kill") {
                if (!action)
                    action = new CombatAction(id, type, subtype, team);

                if (action) {
                    char txt[64];
                    GetDefText(txt, pdef, SourceFilename);
                    action->AssetKills().append(new Text(txt));
                }
            }
            else if (pdef->name()->value() == "target_kill") {
                if (!action)
                    action = new CombatAction(id, type, subtype, team);

                if (action) {
                    char txt[64];
                    GetDefText(txt, pdef, SourceFilename);
                    action->TargetKills().append(new Text(txt));
                }
            }
            else if (pdef->name()->value() == "req") {
                if (!action)
                    action = new CombatAction(id, type, subtype, team);

                if (!pdef->term() || !pdef->term()->isStruct()) {
                    UE_LOG(LogCampaign, Warning, TEXT("WARNING: action req struct missing in '%s'"),
                        SourceFilename ? ANSI_TO_TCHAR(SourceFilename) : TEXT("(null)"));
                }
                else if (action) {
                    TermStruct* val2 = pdef->term()->isStruct();

                    int  act = 0;
                    int  stat = CombatAction::COMPLETE;
                    bool not_flag = false;

                    Combatant* c1 = 0;
                    Combatant* c2 = 0;
                    int        comp = 0;
                    int        score = 0;
                    int        intel = 0;
                    ECOMBATGROUP_TYPE gtype = ECOMBATGROUP_TYPE::NONE;
                    int        gid = 0;

                    for (int j = 0; j < val2->elements()->size(); j++) {
                        TermDef* pdef2 = val2->elements()->at(j)->isDef();
                        if (pdef2) {
                            if (pdef2->name()->value() == "action") {
                                GetDefNumber(act, pdef2, SourceFilename);
                            }
                            else if (pdef2->name()->value() == "status") {
                                char txt[64];
                                GetDefText(txt, pdef2, SourceFilename);
                                stat = CombatAction::StatusFromName(txt);
                            }
                            else if (pdef2->name()->value() == "not") {
                                GetDefBool(not_flag, pdef2, SourceFilename);
                            }
                            else if (pdef2->name()->value() == "c1") {
                                char txt[64];
                                GetDefText(txt, pdef2, SourceFilename);
                                c1 = GetCombatant(txt);
                            }
                            else if (pdef2->name()->value() == "c2") {
                                char txt[64];
                                GetDefText(txt, pdef2, SourceFilename);
                                c2 = GetCombatant(txt);
                            }
                            else if (pdef2->name()->value() == "comp") {
                                char txt[64];
                                GetDefText(txt, pdef2, SourceFilename);
                                comp = CombatActionReq::CompFromName(txt);
                            }
                            else if (pdef2->name()->value() == "score") {
                                GetDefNumber(score, pdef2, SourceFilename);
                            }
                            else if (pdef2->name()->value() == "intel") {
                                if (pdef2->term()->isNumber()) {
                                    GetDefNumber(intel, pdef2, SourceFilename);
                                }
                                else if (pdef2->term()->isText()) {
                                    char txt[64];
                                    GetDefText(txt, pdef2, SourceFilename);
                                    intel = Intel::IntelFromName(txt);
                                }
                            }
                            else if (pdef2->name()->value() == "group_type") {
                                char type_name[64];
                                GetDefText(type_name, pdef2, SourceFilename);
                                gtype = CombatGroup::TypeFromName(type_name);
                            }
                            else if (pdef2->name()->value() == "group_id") {
                                GetDefNumber(gid, pdef2, SourceFilename);
                            }
                        }
                    }

                    if (act)
                        action->AddRequirement(act, stat, not_flag);
                    else if ((int) gtype)
                        action->AddRequirement(c1, gtype, gid, comp, score, intel);
                    else
                        action->AddRequirement(c1, c2, comp, score);
                }
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
        action->SetImageFile(image_file);
        action->SetSceneFile(scene_file);

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

// +--------------------------------------------------------------------+

CombatGroup*
Campaign::CloneOver(CombatGroup* force, CombatGroup* clone, CombatGroup* group)
{
    CombatGroup* orig_parent = group ? group->GetParent() : 0;

    if (orig_parent) {
        CombatGroup* clone_parent = clone ? clone->FindGroup(orig_parent->GetType(), orig_parent->GetID()) : 0;

        if (!clone_parent)
            clone_parent = CloneOver(force, clone, orig_parent);

        CombatGroup* new_clone = clone ? clone->FindGroup(group->GetType(), group->GetID()) : 0;

        if (!new_clone) {
            new_clone = group->Clone(false);
            if (clone_parent)
                clone_parent->AddComponent(new_clone);
        }

        return new_clone;
    }

    return clone;
}

// +--------------------------------------------------------------------+

void
Campaign::LoadMissionList(DataLoader* loader)
{
    bool        ok = true;
    BYTE* block = 0;
    const char* SourceFilename = "Missions.def";

    loader->UseFileSystem(true);
    loader->LoadBuffer(SourceFilename, block, true);
    loader->UseFileSystem(Starshatter::UseFileSystem());

    Parser parser(new BlockReader((const char*)block));
    Term* term = parser.ParseTerm();

    if (!term) {
        return;
    }
    else {
        TermText* file_type = term->isText();
        if (!file_type || file_type->value() != "MISSIONLIST") {
            UE_LOG(LogCampaign, Warning, TEXT("WARNING: invalid mission list file '%s'"),
                ANSI_TO_TCHAR(SourceFilename));
            return;
        }
    }

    do {
        delete term; term = 0;
        term = parser.ParseTerm();

        if (term) {
            TermDef* def = term->isDef();
            if (def && def->name()->value() == "mission") {
                if (!def->term() || !def->term()->isStruct()) {
                    UE_LOG(LogCampaign, Warning, TEXT("WARNING: mission struct missing in '%s'"),
                        ANSI_TO_TCHAR(SourceFilename));
                }
                else {
                    TermStruct* val = def->term()->isStruct();

                    int   id = 0;
                    Text  mname;
                    Text  desc;
                    char  script[256];
                    char  system[256];
                    char  region[256];
                    int   start = 0;
                    int   type = 0;

                    FMemory::Memzero(script, sizeof(script));
                    FCStringAnsi::Strcpy(system, "Unknown");
                    FCStringAnsi::Strcpy(region, "Unknown");

                    for (int i = 0; i < val->elements()->size(); i++) {
                        TermDef* pdef = val->elements()->at(i)->isDef();
                        if (!pdef) continue;

                        if (pdef->name()->value() == "id") {
                            GetDefNumber(id, pdef, SourceFilename);
                        }
                        else if (pdef->name()->value() == "name") {
                            GetDefText(mname, pdef, SourceFilename);
                            mname = Game::GetText(mname);
                        }
                        else if (pdef->name()->value() == "desc") {
                            GetDefText(desc, pdef, SourceFilename);
                            if (desc.length() > 0 && desc.length() < 32)
                                desc = Game::GetText(desc);
                        }
                        else if (pdef->name()->value() == "start") {
                            GetDefTime(start, pdef, SourceFilename);
                        }
                        else if (pdef->name()->value() == "system") {
                            GetDefText(system, pdef, SourceFilename);
                        }
                        else if (pdef->name()->value() == "region") {
                            GetDefText(region, pdef, SourceFilename);
                        }
                        else if (pdef->name()->value() == "script") {
                            GetDefText(script, pdef, SourceFilename);
                        }
                        else if (pdef->name()->value() == "type") {
                            char typestr[64];
                            GetDefText(typestr, pdef, SourceFilename);
                            type = Mission::TypeFromName(typestr);
                        }
                    }

                    MissionInfo* info = new MissionInfo;
                    if (info) {
                        info->id = id;
                        info->name = mname;
                        info->description = desc;
                        info->system = system;
                        info->region = region;
                        info->script = script;
                        info->start = start;
                        info->type = type;
                        info->mission = 0;

                        info->script.setSensitive(false);

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

void
Campaign::LoadCustomMissions(DataLoader* loader)
{
    bool       ok = true;
    List<Text> files;

    loader->UseFileSystem(true);
    loader->ListFiles("*.*", files);

    for (int i = 0; i < files.size(); i++) {
        Text file = *files[i];
        file.setSensitive(false);

        if (file.contains(".def")) {
            BYTE* block = 0;
            const char* SourceFilename = file.data();

            loader->UseFileSystem(true);
            loader->LoadBuffer(SourceFilename, block, true);
            loader->UseFileSystem(Starshatter::UseFileSystem());

            if (strstr((const char*)block, "MISSION") == (const char*)block) {
                Text  mname;
                Text  desc;
                Text  system = "Unknown";
                Text  region = "Unknown";
                int   start = 0;
                int   type = 0;
                int   msn_id = 0;

                Parser parser(new BlockReader((const char*)block));
                Term* term = parser.ParseTerm();

                if (!term) {
                    UE_LOG(LogCampaign, Warning, TEXT("ERROR: could not parse '%s'"), ANSI_TO_TCHAR(SourceFilename));
                    loader->ReleaseBuffer(block);
                    continue;
                }
                else {
                    TermText* file_type = term->isText();
                    if (!file_type || file_type->value() != "MISSION") {
                        UE_LOG(LogCampaign, Warning, TEXT("ERROR: invalid mission file '%s'"), ANSI_TO_TCHAR(SourceFilename));
                        delete term;
                        loader->ReleaseBuffer(block);
                        continue;
                    }
                }

                do {
                    delete term; term = 0;
                    term = parser.ParseTerm();

                    if (term) {
                        TermDef* def = term->isDef();
                        if (!def) continue;

                        if (def->name()->value() == "name") {
                            GetDefText(mname, def, SourceFilename);
                            mname = Game::GetText(mname);
                        }
                        else if (def->name()->value() == "type") {
                            char typestr[64];
                            GetDefText(typestr, def, SourceFilename);
                            type = Mission::TypeFromName(typestr);
                        }
                        else if (def->name()->value() == "id") {
                            GetDefNumber(msn_id, def, SourceFilename);
                        }
                        else if (def->name()->value() == "desc") {
                            GetDefText(desc, def, SourceFilename);
                            if (desc.length() > 0 && desc.length() < 32)
                                desc = Game::GetText(desc);
                        }
                        else if (def->name()->value() == "system") {
                            GetDefText(system, def, SourceFilename);
                        }
                        else if (def->name()->value() == "region") {
                            GetDefText(region, def, SourceFilename);
                        }
                        else if (def->name()->value() == "start") {
                            GetDefTime(start, def, SourceFilename);
                        }
                    }
                } while (term);

                loader->ReleaseBuffer(block);

                // Legacy ID inference:
                if (strstr(SourceFilename, "custom") == SourceFilename) {
                    sscanf_s(SourceFilename + 6, "%d", &msn_id);
                    if (msn_id <= i) msn_id = i + 1;
                }
                else if (msn_id < 1) {
                    msn_id = i + 1;
                }

                MissionInfo* info = new MissionInfo;
                if (info) {
                    info->id = msn_id;
                    info->name = mname;
                    info->type = type;
                    info->description = desc;
                    info->system = system;
                    info->region = region;
                    info->script = SourceFilename;
                    info->start = start;
                    info->mission = 0;

                    info->script.setSensitive(false);

                    missions.append(info);
                }
                else {
                    ok = false;
                }
            }

            loader->ReleaseBuffer(block);
        }
    }

    files.destroy();

    if (!ok)
        Unload();
    else
        missions.sort();
}

void
Campaign::LoadTemplateList(DataLoader* loader)
{
    BYTE* block = 0;
    const char* SourceFilename = "Templates.def";

    loader->UseFileSystem(true);
    loader->LoadBuffer(SourceFilename, block, true);
    loader->UseFileSystem(Starshatter::UseFileSystem());

    Parser parser(new BlockReader((const char*)block));
    Term* term = parser.ParseTerm();

    if (!term) {
        return;
    }
    else {
        TermText* file_type = term->isText();
        if (!file_type || file_type->value() != "TEMPLATELIST") {
            UE_LOG(LogCampaign, Warning, TEXT("WARNING: invalid template list file '%s'"),
                ANSI_TO_TCHAR(SourceFilename));
            return;
        }
    }

    do {
        delete term; term = 0;
        term = parser.ParseTerm();

        if (term) {
            TermDef* def = term->isDef();
            if (def && def->name()->value() == "mission") {
                if (!def->term() || !def->term()->isStruct()) {
                    UE_LOG(LogCampaign, Warning, TEXT("WARNING: mission struct missing in '%s'"),
                        ANSI_TO_TCHAR(SourceFilename));
                }
                else {
                    TermStruct* val = def->term()->isStruct();

                    char name_buf[256];
                    char script[256];
                    char region_buf[256];

                    int  id = 0;
                    int  msn_type = 0;
                    int  grp_type = 0;

                    int  min_rank = 0;
                    int  max_rank = 0;
                    int  action_id = 0;
                    int  action_status = 0;
                    int  exec_once = 0;
                    int  start_before = TIME_NEVER;
                    int  start_after = 0;

                    name_buf[0] = 0;
                    script[0] = 0;
                    region_buf[0] = 0;

                    for (int i = 0; i < val->elements()->size(); i++) {
                        TermDef* pdef = val->elements()->at(i)->isDef();
                        if (!pdef) continue;

                        if (pdef->name()->value() == "id")
                            GetDefNumber(id, pdef, SourceFilename);

                        else if (pdef->name()->value() == "name")
                            GetDefText(name_buf, pdef, SourceFilename);

                        else if (pdef->name()->value() == "script")
                            GetDefText(script, pdef, SourceFilename);

                        else if (pdef->name()->value() == "rgn" || pdef->name()->value() == "region")
                            GetDefText(region_buf, pdef, SourceFilename);

                        else if (pdef->name()->value() == "type") {
                            char typestr[64];
                            GetDefText(typestr, pdef, SourceFilename);
                            msn_type = Mission::TypeFromName(typestr);
                        }

                        else if (pdef->name()->value() == "group") {
                            char typestr[64];
                            GetDefText(typestr, pdef, SourceFilename);
                            grp_type = (int) CombatGroup::TypeFromName(typestr);
                        }

                        else if (pdef->name()->value() == "min_rank")
                            GetDefNumber(min_rank, pdef, SourceFilename);

                        else if (pdef->name()->value() == "max_rank")
                            GetDefNumber(max_rank, pdef, SourceFilename);

                        else if (pdef->name()->value() == "action_id")
                            GetDefNumber(action_id, pdef, SourceFilename);

                        else if (pdef->name()->value() == "action_status")
                            GetDefNumber(action_status, pdef, SourceFilename);

                        else if (pdef->name()->value() == "exec_once")
                            GetDefNumber(exec_once, pdef, SourceFilename);

                        else if (pdef->name()->value().contains("before")) {
                            if (pdef->term()->isNumber()) {
                                GetDefNumber(start_before, pdef, SourceFilename);
                            }
                            else {
                                GetDefTime(start_before, pdef, SourceFilename);
                                start_before -= ONE_DAY;
                            }
                        }

                        else if (pdef->name()->value().contains("after")) {
                            if (pdef->term()->isNumber()) {
                                GetDefNumber(start_after, pdef, SourceFilename);
                            }
                            else {
                                GetDefTime(start_after, pdef, SourceFilename);
                                start_after -= ONE_DAY;
                            }
                        }
                    }

                    MissionInfo* info = new MissionInfo;
                    if (info) {
                        info->id = id;
                        info->name = name_buf;
                        info->script = script;
                        info->region = region_buf;
                        info->type = msn_type;
                        info->min_rank = min_rank;
                        info->max_rank = max_rank;
                        info->action_id = action_id;
                        info->action_status = action_status;
                        info->exec_once = exec_once;
                        info->start_before = start_before;
                        info->start_after = start_after;

                        info->script.setSensitive(false);

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

// +--------------------------------------------------------------------+

void
Campaign::CreatePlanners()
{
    if (planners.size() > 0)
        planners.destroy();

    CampaignPlan* p = 0;

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

// +--------------------------------------------------------------------+

int
Campaign::GetPlayerIFF()
{
    int iff = 1;

    if (player_group)
        iff = player_group->GetIFF();

    return iff;
}

void
Campaign::SetPlayerGroup(CombatGroup* pg)
{
    if (player_group != pg) {
        UE_LOG(LogCampaign, Log, TEXT("Campaign::SetPlayerGroup(%s)"),
            pg ? ANSI_TO_TCHAR(pg->Name().data()) : TEXT("0"));

        player_group = pg;
        player_unit = 0;

        // need to regenerate missions when changing player combat group:
        if (IsDynamic()) {
            UE_LOG(LogCampaign, Log, TEXT("  destroying mission list..."));
            missions.destroy();
        }
    }
}

void
Campaign::SetPlayerUnit(CombatUnit* unit)
{
    if (player_unit != unit) {
        UE_LOG(LogCampaign, Log, TEXT("Campaign::SetPlayerUnit(%s)"),
            unit ? ANSI_TO_TCHAR(unit->Name().data()) : TEXT("0"));

        player_unit = unit;

        if (unit)
            player_group = unit->GetCombatGroup();

        // need to regenerate missions when changing player combat unit:
        if (IsDynamic()) {
            UE_LOG(LogCampaign, Log, TEXT("  destroying mission list..."));
            missions.destroy();
        }
    }
}

// +--------------------------------------------------------------------+

CombatZone*
Campaign::GetZone(const char* rgn)
{
    ListIter<CombatZone> z = zones;
    while (++z) {
        if (z->HasRegion(rgn))
            return z.value();
    }

    return 0;
}

StarSystem*
Campaign::GetSystem(const char* sys)
{
    return Galaxy::GetInstance()->GetSystem(sys);
}

Combatant*
Campaign::GetCombatant(const char* cname)
{
    ListIter<Combatant> iter = combatants;
    while (++iter) {
        Combatant* c = iter.value();
        if (FCStringAnsi::Strcmp(c->GetName(), cname) == 0)
            return c;
    }

    return 0;
}

// +--------------------------------------------------------------------+

Mission*
Campaign::GetMission()
{
    return GetMission(mission_id);
}

Mission*
Campaign::GetMission(int id)
{
    if (id < 0) {
        UE_LOG(LogCampaign, Error, TEXT("ERROR - Campaign::GetMission(%d) invalid mission id"), id);
        return 0;
    }

    if (mission && mission->Identity() == id) {
        return mission;
    }

    MissionInfo* info = 0;
    for (int i = 0; !info && i < missions.size(); i++)
        if (missions[i]->id == id)
            info = missions[i];

    if (info) {
        if (!info->mission) {
            UE_LOG(LogCampaign, Log, TEXT("Campaign::GetMission(%d) loading mission..."), id);
            info->mission = new Mission(id, info->script, path);
            if (info->mission)
                info->mission->Load();
        }

        if (IsDynamic()) {
            if (info->mission) {
                if (FCStringAnsi::Stricmp(info->mission->Situation(), "Unknown") == 0) {
                    UE_LOG(LogCampaign, Log, TEXT("Campaign::GetMission(%d) generating sitrep..."), id);
                    CampaignSituationReport sitrep(this, info->mission);
                    sitrep.GenerateSituationReport();
                }
            }
            else {
                UE_LOG(LogCampaign, Warning, TEXT("Campaign::GetMission(%d) could not find/load mission."), id);
            }
        }

        return info->mission;
    }

    return 0;
}

Mission*
Campaign::GetMissionByFile(const char* InFilename)
{
    if (!InFilename || !*InFilename) {
        UE_LOG(LogCampaign, Error, TEXT("ERROR - Campaign::GetMissionByFile() invalid filename"));
        return 0;
    }

    int          id = 0;
    int          maxid = 0;
    MissionInfo* info = 0;

    for (int i = 0; !info && i < missions.size(); i++) {
        MissionInfo* m = missions[i];

        if (m->id > maxid)
            maxid = m->id;

        if (m->script == InFilename)
            info = m;
    }

    if (info) {
        id = info->id;

        if (!info->mission) {
            UE_LOG(LogCampaign, Log, TEXT("Campaign::GetMission(%d) loading mission..."), id);
            info->mission = new Mission(id, info->script, path);
            if (info->mission)
                info->mission->Load();
        }

        if (IsDynamic()) {
            if (info->mission) {
                if (FCStringAnsi::Stricmp(info->mission->Situation(), "Unknown") == 0) {
                    UE_LOG(LogCampaign, Log, TEXT("Campaign::GetMission(%d) generating sitrep..."), id);
                    CampaignSituationReport sitrep(this, info->mission);
                    sitrep.GenerateSituationReport();
                }
            }
            else {
                UE_LOG(LogCampaign, Warning, TEXT("Campaign::GetMission(%d) could not find/load mission."), id);
            }
        }
    }
    else {
        info = new MissionInfo;
        if (info) {
            info->id = maxid + 1;
            info->name = "New Custom Mission";
            info->script = InFilename;

            info->mission = new Mission(info->id, info->script, "Mods/Missions/");
            info->mission->SetName(info->name);

            info->script.setSensitive(false);

            missions.append(info);
        }
    }

    return info ? info->mission : 0;
}

MissionInfo*
Campaign::CreateNewMission()
{
    int          maxid = 0;
    MissionInfo* info = 0;

    if (campaign_id == MULTIPLAYER_MISSIONS)
        maxid = 10;

    for (int i = 0; i < missions.size(); i++) {
        MissionInfo* m = missions[i];
        if (m->id > maxid)
            maxid = m->id;
    }

    char NewScript[64];
    FCStringAnsi::Snprintf(NewScript, sizeof(NewScript), "custom%03d.def", maxid + 1);

    info = new MissionInfo;
    if (info) {
        info->id = maxid + 1;
        info->name = "New Custom Mission";
        info->script = NewScript;
        info->mission = new Mission(info->id, NewScript, path);
        info->mission->SetName(info->name);

        info->script.setSensitive(false);

        missions.append(info);
    }

    return info;
}

void
Campaign::DeleteMission(int id)
{
    if (id < 0) {
        UE_LOG(LogCampaign, Error, TEXT("ERROR - Campaign::DeleteMission(%d) invalid mission id"), id);
        return;
    }

    MissionInfo* m = 0;
    int          index = -1;

    for (int i = 0; !m && i < missions.size(); i++) {
        if (missions[i]->id == id) {
            m = missions[i];
            index = i;
        }
    }

    if (m) {
        char full_path[256];

        if (path[FCStringAnsi::Strlen(path) - 1] == '/')
            FCStringAnsi::Snprintf(full_path, sizeof(full_path), "%s%s", path, m->script.data());
        else
            FCStringAnsi::Snprintf(full_path, sizeof(full_path), "%s/%s", path, m->script.data());

        // Unreal-friendly delete:
        IFileManager::Get().Delete(ANSI_TO_TCHAR(full_path), /*RequireExists*/false, /*EvenReadOnly*/true, /*Quiet*/false);

        Load();
    }
    else {
        UE_LOG(LogCampaign, Error, TEXT("ERROR - Campaign::DeleteMission(%d) could not find mission"), id);
    }
}

MissionInfo*
Campaign::GetMissionInfo(int id)
{
    if (id < 0) {
        UE_LOG(LogCampaign, Error, TEXT("ERROR - Campaign::GetMissionInfo(%d) invalid mission id"), id);
        return 0;
    }

    MissionInfo* m = 0;
    for (int i = 0; !m && i < missions.size(); i++)
        if (missions[i]->id == id)
            m = missions[i];

    if (m) {
        if (!m->mission) {
            m->mission = new Mission(id, m->script);
            if (m->mission)
                m->mission->Load();
        }

        return m;
    }

    UE_LOG(LogCampaign, Error, TEXT("ERROR - Campaign::GetMissionInfo(%d) could not find mission"), id);
    return 0;
}

void
Campaign::ReloadMission(int id)
{
    if (mission && mission == net_mission) {
        delete net_mission;
        net_mission = 0;
    }

    mission = 0;

    if (id >= 0 && id < missions.size()) {
        MissionInfo* m = missions[id];
        delete m->mission;
        m->mission = 0;
    }
}

void
Campaign::LoadNetMission(int id, const char* net_mission_script)
{
    if (mission && mission == net_mission) {
        delete net_mission;
        net_mission = 0;
    }

    mission_id = id;
    mission = new Mission(id);

    if (mission && mission->ParseMission(net_mission_script))
        mission->Validate();

    net_mission = mission;
}

// +--------------------------------------------------------------------+

CombatAction*
Campaign::FindAction(int action_id)
{
    ListIter<CombatAction> iter = actions;
    while (++iter) {
        CombatAction* a = iter.value();

        if (a->Identity() == action_id)
            return a;
    }

    return 0;
}

// +--------------------------------------------------------------------+

MissionInfo*
Campaign::FindMissionTemplate(int mission_type, CombatGroup* in_player_group)
{
    MissionInfo* info = 0;

    if (!in_player_group)
        return info;

    TemplateList* templ = GetTemplateList(
        mission_type,
        (int)in_player_group->GetType()
    );

    if (!templ || !templ->missions.size())
        return info;

    int tries = 0;
    int msize = templ->missions.size();

    while (!info && tries < msize) {
        int index = templ->index;
        if (index >= msize)
            index = 0;

        info = templ->missions[index];
        templ->index = index + 1;
        tries++;

        if (info) {
            if (info->action_id) {
                CombatAction* a = FindAction(info->action_id);
                if (a && a->Status() != info->action_status)
                    info = 0;
            }

            if (info && !info->IsAvailable())
                info = 0;
        }
    }

    return info;
}

// +--------------------------------------------------------------------+

TemplateList*
Campaign::GetTemplateList(int msn_type, int grp_type)
{
    for (int i = 0; i < templates.size(); i++) {
        if (templates[i]->mission_type == msn_type &&
            templates[i]->group_type == grp_type)
            return templates[i];
    }

    return 0;
}

// +--------------------------------------------------------------------+

void
Campaign::SetMissionId(int id)
{
    UE_LOG(LogCampaign, Log, TEXT("Campaign::SetMissionId(%d)"), id);

    if (id > 0)
        mission_id = id;
    else
        UE_LOG(LogCampaign, Log, TEXT("   retaining mission id = %d"), mission_id);
}

// +--------------------------------------------------------------------+

double
Campaign::Stardate()
{
    return StarSystem::Stardate();
}

// +--------------------------------------------------------------------+

void
Campaign::SelectDefaultPlayerGroup(CombatGroup* g, int type)
{
    if (player_group || !g) return;

    if ((int) g->GetType() == type && !g->IsReserve() && g->Value() > 0) {
        player_group = g;
        player_unit = 0;
        return;
    }

    for (int i = 0; i < g->GetComponents().size(); i++)
        SelectDefaultPlayerGroup(g->GetComponents()[i], type);
}

// +--------------------------------------------------------------------+

void
Campaign::Prep()
{
    if (IsDynamic() && combatants.isEmpty()) {
        DataLoader* loader = DataLoader::GetLoader();
        loader->SetDataPath(path);
        LoadCampaign(loader, true);
    }

    StarSystem::SetBaseTime(loadTime);

    if (IsScripted() && actions.isEmpty()) {
        DataLoader* loader = DataLoader::GetLoader();
        loader->SetDataPath(path);
        LoadCampaign(loader, true);

        ListIter<MissionInfo> m = missions;
        while (++m) {
            GetMission(m->id);
        }
    }

    CheckPlayerGroup();
}

void
Campaign::Start()
{
    UE_LOG(LogCampaign, Log, TEXT("Campaign::Start()"));

    Prep();

    CreatePlanners();
    SetCampaignStatus(ECampaignStatus::ACTIVE);
}

void
Campaign::ExecFrame()
{
    if (InCutscene())
        return;

    time = Stardate() - startTime;

    if (campaign_status < ECampaignStatus::ACTIVE)
        return;

    if (IsDynamic()) {
        bool completed = false;

        ListIter<MissionInfo> m = missions;
        while (++m) {
            if (m->mission && m->mission->IsComplete()) {
                UE_LOG(LogCampaign, Log, TEXT("Campaign::ExecFrame() completed mission %d '%s'"),
                    m->id, ANSI_TO_TCHAR(m->name.data()));
                completed = true;
            }
        }

        if (completed) {
            UE_LOG(LogCampaign, Log, TEXT("Campaign::ExecFrame() destroying mission list after completion..."));
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

                    if (info) {
                        UE_LOG(LogCampaign, Log, TEXT("Campaign::ExecFrame() deleting expired mission %d start: %d current: %d"),
                            info->id, info->start, (int)time);
                        delete info;
                    }
                }
            }
        }

        // PLAN EVENT MUST BE FIRST PLANNER:
        if (loaded_from_savegame && planners.size() > 0) {
            CampaignPlanEvent* plan_event = (CampaignPlanEvent*)planners.first();
            plan_event->ExecScriptedEvents();
            loaded_from_savegame = false;
        }

        ListIter<CampaignPlan> plan = planners;
        while (++plan) {
            CheckPlayerGroup();
            plan->ExecFrame();
        }

        CheckPlayerGroup();

        // Auto save AFTER planners have run:
        if (completed) {
            CampaignSaveGame save(this);
            if (completed) {
                CampaignSaveGame save(this);
                save.SaveAuto(PlayerId);   
            }
        }
    }
    else {
        // PLAN EVENT MUST BE FIRST PLANNER:
        if (planners.size() > 0) {
            CampaignPlanEvent* plan_event = (CampaignPlanEvent*)planners.first();
            plan_event->ExecScriptedEvents();
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
        player_group = 0;

        CombatGroup* force = 0;
        for (int i = 0; i < combatants.size() && !force; i++) {
            if (combatants[i]->GetIFF() == player_iff) {
                force = combatants[i]->GetForce();
            }
        }

        if (force) {
            force->CalcValue();
            SelectDefaultPlayerGroup(force, (int) ECOMBATGROUP_TYPE::WING);

            if (!player_group)
                SelectDefaultPlayerGroup(force, (int) ECOMBATGROUP_TYPE::DESTROYER_SQUADRON);
        }
    }

    if (player_unit && player_unit->GetValue() < 1)
        SetPlayerUnit(0);
}

// +--------------------------------------------------------------------+

void
Campaign::StartMission()
{
    Mission* m = GetMission();

    if (m) {
        UE_LOG(LogCampaign, Log, TEXT("Campaign Start Mission - %d. '%s'"),
            m->Identity(), ANSI_TO_TCHAR(m->Name()));

        if (!scripted) {

            double gtime = (double)Game::GameTime() / 1000.0;
            double base = startTime + m->Start() - 15 - gtime;

            StarSystem::SetBaseTime(base);

            double current_time = Stardate() - startTime;

            char buffer[32];
            FormatDayTime(buffer, current_time);
            UE_LOG(LogCampaign, Log, TEXT("  current time:  %s"), ANSI_TO_TCHAR(buffer));

            FormatDayTime(buffer, m->Start());
            UE_LOG(LogCampaign, Log, TEXT("  mission start: %s"), ANSI_TO_TCHAR(buffer));
        }
    }
}

void
Campaign::RollbackMission()
{
    UE_LOG(LogCampaign, Log, TEXT("Campaign::RollbackMission()"));

    Mission* m = GetMission();

    if (m) {
        if (!scripted) {

            double gtime = (double)Game::GameTime() / 1000.0;
            double base = startTime + m->Start() - 60 - gtime;

            StarSystem::SetBaseTime(base);

            double current_time = Stardate() - startTime;
            UE_LOG(LogCampaign, Log, TEXT("  mission start: %d"), m->Start());
            UE_LOG(LogCampaign, Log, TEXT("  current time:  %d"), (int)current_time);
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
    return campaign_id >= DYNAMIC_CAMPAIGN && campaign_id < SINGLE_MISSIONS;
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

static CombatGroup* FindGroup_r(CombatGroup* g, int type, int id)
{
    if ((int) g->GetType() == type && g->GetID() == id)
        return g;

    CombatGroup* result = 0;

    ListIter<CombatGroup> subgroup = g->GetComponents();
    while (++subgroup && !result)
        result = FindGroup_r(subgroup.value(), type, id);

    return result;
}

CombatGroup*
Campaign::FindGroup(int iff, int type, int id)
{
    CombatGroup* result = 0;

    ListIter<Combatant> combatant = combatants;
    while (++combatant && !result) {
        if (combatant->GetIFF() == iff) {
            result = FindGroup_r(combatant->GetForce(), type, id);
        }
    }

    return result;
}

// +--------------------------------------------------------------------+

static void FindGroups(CombatGroup* g, int type, CombatGroup* near_group, List<CombatGroup>& groups)
{
    if ((int) g->GetType() == type && g->IntelLevel() > Intel::RESERVE) {
        if (!near_group || g->GetAssignedZone() == near_group->GetAssignedZone())
            groups.append(g);
    }

    ListIter<CombatGroup> subgroup = g->GetComponents();
    while (++subgroup)
        FindGroups(subgroup.value(), type, near_group, groups);
}

CombatGroup*
Campaign::FindGroup(int iff, int type, CombatGroup* near_group)
{
    CombatGroup* result = 0;
    List<CombatGroup> groups;

    ListIter<Combatant> combatant = combatants;
    while (++combatant) {
        if (combatant->GetIFF() == iff) {
            FindGroups(combatant->GetForce(), type, near_group, groups);
        }
    }

    if (groups.size() > 0) {
        const int MaxIndex = groups.size() - 1;
        const int index = MaxIndex > 0 ? FMath::RandRange(0, MaxIndex) : 0;
        result = groups[index];
    }

    return result;
}

// +--------------------------------------------------------------------+

static void FindStrikeTargets(CombatGroup* g, CombatGroup* strike_group, List<CombatGroup>& groups)
{
    if (!strike_group || !strike_group->GetAssignedZone()) return;

    if (g->IsStrikeTarget() && g->IntelLevel() > Intel::RESERVE) {
        if (strike_group->GetAssignedZone() == g->GetAssignedZone() ||
            strike_group->GetAssignedZone()->HasRegion(g->GetRegion()))
            groups.append(g);
    }

    ListIter<CombatGroup> subgroup = g->GetComponents();
    while (++subgroup)
        FindStrikeTargets(subgroup.value(), strike_group, groups);
}

CombatGroup*
Campaign::FindStrikeTarget(int iff, CombatGroup* strike_group)
{
    CombatGroup* result = 0;
    List<CombatGroup> groups;

    ListIter<Combatant> combatant = GetCombatants();
    while (++combatant) {
        if (combatant->GetIFF() != 0 && combatant->GetIFF() != iff) {
            FindStrikeTargets(combatant->GetForce(), strike_group, groups);
        }
    }

    if (groups.size() > 0) {
        const int MaxIndex = groups.size() - 1;
        const int index = MaxIndex > 0 ? FMath::RandRange(0, MaxIndex) : 0;
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

        if (a->IsAvailable())
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

            if (iff <= 1) {
                if (c->GetIFF() <= 1) score_us += c->GetScore();
                else                  score_them += c->GetScore();
            }
            else {
                if (c->GetIFF() <= 1) score_them += c->GetScore();
                else                  score_us += c->GetScore();
            }
        }
    }

    return score_us - score_them;
}

// +--------------------------------------------------------------------+

void Campaign::SetCampaignStatus(UObject* WorldContextObject, ECampaignStatus s)
{
    campaign_status = s;

    if (campaign_status == ECampaignStatus::SUCCESS)
    {
        if (UStarshatterPlayerSubsystem* PlayerSSW = UStarshatterPlayerSubsystem::Get(WorldContextObject))
        {
            FS_PlayerGameInfo& Info = PlayerSSW->GetMutablePlayerInfo();

            // legacy campaign_id is usually 1-based:
            const int32 CampaignBitIndex = campaign_id - 1;
            Info.SetCampaignComplete(CampaignBitIndex, true);

            PlayerSS->SavePlayer(true);
        }
    }

    if (campaign_status > ECampaignStatus::ACTIVE)
    {
        UE_LOG(LogCampaign, Log, TEXT("Campaign::SetStatus() destroying mission list at campaign end"));
        missions.destroy();
    }
}
// +--------------------------------------------------------------------+

static void GetCombatUnits(CombatGroup* g, List<CombatUnit>& units)
{
    if (g) {
        ListIter<CombatUnit> unit = g->GetUnits();
        while (++unit) {
            CombatUnit* u = unit.value();

            if (u->Count() - u->DeadCount() > 0)
                units.append(u);
        }

        ListIter<CombatGroup> comp = g->GetComponents();
        while (++comp) {
            CombatGroup* g2 = comp.value();

            if (!g2->IsReserve())
                GetCombatUnits(g2, units);
        }
    }
}

int
Campaign::GetAllCombatUnits(int iff, List<CombatUnit>& units)
{
    units.clear();

    ListIter<Combatant> iter = combatants;
    while (++iter) {
        Combatant* c = iter.value();

        if (iff < 0 || c->GetIFF() == iff) {
            GetCombatUnits(c->GetForce(), units);
        }
    }

    return units.size();
}
