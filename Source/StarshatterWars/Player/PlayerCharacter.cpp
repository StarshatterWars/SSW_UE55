// /*  Project nGenEx	Fractal Dev Games	Copyright (C) 2024. All Rights Reserved.	SUBSYSTEM:    SSW	FILE:         Game.cpp	AUTHOR:       Carlos Bott*/


/*  Project Starshatter 4.5
    Destroyer Studios LLC
    Copyright © 1997-2004. All Rights Reserved.

    SUBSYSTEM:    Stars.exe
    FILE:         PlayerCharacter.cpp
    AUTHOR:       Carlos Bott

    OVERVIEW
    ========
    Player / Logbook class (renamed for Unreal compatibility)
*/

#include "PlayerCharacter.h"

#include "Ship.h"
#include "SimEvent.h"
#include "Campaign.h"
#include "CampaignSaveGame.h"
#include "Random.h"
#include "HUDView.h"
#include "MFDView.h"
#include "GameStructs.h"

#include "DataLoader.h"
#include "Encrypt.h"
#include "ParseUtil.h"
#include "FormatUtil.h"
#include "Engine/Texture2D.h"
#include "Game.h"

#include "Engine/World.h"
#include "Engine/GameInstance.h"
#include "StarshatterPlayerSubsystem.h"

// +-------------------------------------------------------------------+

static PlayerCharacter* GCurrentPlayer = nullptr;

class AwardInfo
{
public:
    static const char* TYPENAME() { return "AwardInfo"; }

    AwardInfo()
        : type(RANK),
        id(0),
        large_insignia(0),
        small_insignia(0),
        granted_ship_classes(0x7),
        total_points(0),
        mission_points(0),
        total_missions(0),
        kills(0),
        lost(0),
        collision(0),
        campaign_id(0),
        campaign_complete(false),
        dynamic_campaign(false),
        ceremony(true),
        required_awards(0),
        lottery(0),
        min_rank(0),
        max_rank((int)1e9),
        min_ship_class(0),
        max_ship_class((int)1e9)
    {
    }

    ~AwardInfo() {}

    enum TYPE { RANK, MEDAL };

    int      type;
    int      id;
    Text     name;
    Text     abrv;
    Text     desc;
    Text     grant;
    Text     desc_sound;
    Text     grant_sound;
    Bitmap* large_insignia;
    Bitmap* small_insignia;
    int      granted_ship_classes;

    int      total_points;
    int      mission_points;
    int      total_missions;
    int      kills;
    int      lost;
    int      collision;
    int      campaign_id;
    bool     campaign_complete;
    bool     dynamic_campaign;
    bool     ceremony;

    int      required_awards;
    int      lottery;
    int      min_rank;
    int      max_rank;
    int      min_ship_class;
    int      max_ship_class;
};

static List<AwardInfo> rank_table;
static List<AwardInfo> medal_table;
static bool            config_exists = false;

// +-------------------------------------------------------------------+

PlayerCharacter::PlayerCharacter(const char* n)
    : uid(0), name(n), create_date(0), points(0), medals(0), flight_time(0),
    missions(0), kills(0), losses(0), campaigns(0), trained(0),
    flight_model(0), flying_start(0), landing_model(0),
    ai_level(1), hud_mode(0), hud_color(1),
    ff_level(4), grid(1), gunsight(0), award(0)
{
    name.setSensitive(false);

    mfd.SetNum(4);
    mfd[0] = EMFDMode::OFF;
    mfd[1] = EMFDMode::OFF;;
    mfd[2] = EMFDMode::OFF;;
    mfd[3] = EMFDMode::OFF;;
}

PlayerCharacter::PlayerCharacter()
    : uid(0), create_date(0), points(0), medals(0), flight_time(0),
    missions(0), kills(0), losses(0), campaigns(0), trained(0),
    flight_model(0), flying_start(0), landing_model(0),
    ai_level(1), hud_mode(0), hud_color(1),
    ff_level(4), grid(1), gunsight(0), award(0)
{
    name.setSensitive(false);

    mfd.SetNum(4);
    mfd[0] = EMFDMode::OFF;;
    mfd[1] = EMFDMode::OFF;;
    mfd[2] = EMFDMode::OFF;;
    mfd[3] = EMFDMode::OFF;
}

PlayerCharacter* PlayerCharacter::CreateDefault()
{
    // Create a new player with whatever your defaults are
    PlayerCharacter* P = new PlayerCharacter();
    P->SetName("NEW PILOT");          // adjust to your API
    // set other defaults...
    return P;
}

void PlayerCharacter::AddToRoster(PlayerCharacter* P)
{
    if (!P) return;
    GetRoster().append(P);
    Save(); // if you have Save()
}

void PlayerCharacter::RemoveFromRoster(PlayerCharacter* P)
{
    if (!P) return;

    List<PlayerCharacter>& R = GetRoster();
    // remove by pointer:
    for (int i = 0; i < R.size(); ++i)
    {
        if (R[i] == P)
        {
            R.remove(P);   // pointer-based remove
            break;
        }
    }

    delete P;
    Save();
}

PlayerCharacter::~PlayerCharacter()
{
}

// +-------------------------------------------------------------------+

void
PlayerCharacter::SetName(const char* n)
{
    if (n && *n)
        name = n;
}

void
PlayerCharacter::SetPassword(const char* p)
{
    if (p && *p) {
        pass = p;

        if (pass.length() > 16)
            pass = pass.substring(0, 16);
    }
}

void
PlayerCharacter::SetSquadron(const char* s)
{
    if (s && *s)
        squadron = s;
}

void
PlayerCharacter::SetSignature(const char* s)
{
    if (s && *s)
        signature = s;
}

const Text&
PlayerCharacter::ChatMacro(int n) const
{
    if (n >= 0 && n < 10)
        return chat_macros[n];

    return chat_macros[0];
}

void
PlayerCharacter::SetChatMacro(int n, const char* m)
{
    if (n >= 0 && n < 10 && m && *m)
        chat_macros[n] = m;
}

void
PlayerCharacter::SetPoints(int p)
{
    if (p >= 0)
        points = p;
}

void
PlayerCharacter::SetMedals(int m)
{
    medals = m;
}

void
PlayerCharacter::SetCampaigns(int n)
{
    campaigns = n;
}

void
PlayerCharacter::SetTrained(int n)
{
    if (n == 0)
        trained = 0;

    else if (n > 0 && n <= 20)
        trained = trained | (1 << (n - 1));

    else if (n > 20)
        trained = n;
}

bool
PlayerCharacter::HasTrained(int n) const
{
    if (n > 0 && n <= 20)
        return (trained & (1 << (n - 1))) ? true : false;

    return false;
}

bool
PlayerCharacter::HasCompletedCampaign(int id) const
{
    if (id > 0 && id < 30)
        return (campaigns & (1 << id)) ? true : false;

    return false;
}

void
PlayerCharacter::SetCampaignComplete(int id)
{
    if (id > 0 && id < 30) {
        campaigns = campaigns | (1 << id);
        Save();
    }
}

void
PlayerCharacter::SetCreateDate(int d)
{
    if (d >= 0)
        create_date = d;
}

void
PlayerCharacter::SetFlightTime(int t)
{
    if (t >= 0)
        flight_time = t;
}

void
PlayerCharacter::SetMissions(int m)
{
    if (m >= 0)
        missions = m;
}

void
PlayerCharacter::SetKills(int k)
{
    if (k >= 0)
        kills = k;
}

void PlayerCharacter::SetDeaths(int d)
{
    if (d >= 0)
        deaths = d;
}

void
PlayerCharacter::SetLosses(int l)
{
    if (l >= 0)
        losses = l;
}

// +-------------------------------------------------------------------+

void
PlayerCharacter::AddFlightTime(int t)
{
    if (t > 0)
        flight_time += t;
}

void
PlayerCharacter::AddPoints(int p)
{
    if (p > 0)
        points += p;
}

void
PlayerCharacter::AddMissions(int m)
{
    if (m > 0)
        missions += m;
}

void
PlayerCharacter::AddKills(int k)
{
    if (k > 0)
        kills += k;
}

void PlayerCharacter::AddDeaths(int d)
{
    if (d > 0)
        deaths += d;
}

void
PlayerCharacter::AddLosses(int l)
{
    if (l > 0)
        losses += l;
}

// +-------------------------------------------------------------------+

void
PlayerCharacter::SetFlightModel(int n)
{
    if (n >= Ship::FM_STANDARD && n <= Ship::FM_ARCADE) {
        flight_model = n;
        Ship::SetFlightModel(n);
    }
}

void
PlayerCharacter::SetFlyingStart(int n)
{
    flying_start = n;
}

void
PlayerCharacter::SetLandingModel(int n)
{
    if (n >= Ship::LM_STANDARD && n <= Ship::LM_EASIER) {
        landing_model = n;
        Ship::SetLandingModel(landing_model);
    }
}

void
PlayerCharacter::SetAILevel(int n)
{
    ai_level = n;
}

void
PlayerCharacter::SetHUDMode(int n)
{
    hud_mode = n;
    HUDView::SetArcade(n > 0);
}

void
PlayerCharacter::SetHUDColor(int n)
{
    hud_color = n;
    HUDView::SetDefaultColorSet(n);
}

void
PlayerCharacter::SetFriendlyFire(int n)
{
    if (n >= 0 && n <= 4) {
        ff_level = n;
        Ship::SetFriendlyFireLevel(n / 4.0);
    }
}

void
PlayerCharacter::SetGridMode(int n)
{
    if (n >= 0 && n <= 1) {
        grid = n;
    }
}

void
PlayerCharacter::SetGunsight(int n)
{
    if (n >= 0 && n <= 1) {
        gunsight = n;
    }
}

void
PlayerCharacter::ClearShowAward()
{
    award = 0;
}

Text
PlayerCharacter::AwardName() const
{
    if (award)
        return award->name;

    return Text();
}

Text
PlayerCharacter::AwardDesc() const
{
    if (award)
        return award->grant;

    return Text();
}

Bitmap*
PlayerCharacter::AwardImage() const
{
    if (award)
        return award->large_insignia;

    return 0;
}

USound*
PlayerCharacter::AwardSound() const
{
    if (award && award->grant_sound.length()) {
        DataLoader* loader = DataLoader::GetLoader();
        USound* result = 0;

        loader->LoadSound(award->grant_sound, result);
        return result;
    }

    return 0;
}

// +-------------------------------------------------------------------+

const char*
PlayerCharacter::RankName(int rank)
{
    ListIter<AwardInfo> iter = rank_table;
    while (++iter) {
        AwardInfo* award = iter.value();
        if (award->id == rank)
            return award->name;
    }

    return "Conscript";
}

const char*
PlayerCharacter::RankAbrv(int rank)
{
    ListIter<AwardInfo> iter = rank_table;
    while (++iter) {
        AwardInfo* award = iter.value();
        if (award->id == rank)
            return award->abrv;
    }

    return "";
}

Bitmap*
PlayerCharacter::RankInsignia(int rank, int size)
{
    ListIter<AwardInfo> iter = rank_table;
    while (++iter) {
        AwardInfo* award = iter.value();
        if (award->id == rank) {
            if (size == 0)
                return award->small_insignia;

            if (size == 1)
                return award->large_insignia;
        }
    }

    return 0;
}

const char*
PlayerCharacter::RankDescription(int rank)
{
    ListIter<AwardInfo> iter = rank_table;
    while (++iter) {
        AwardInfo* award = iter.value();
        if (award->id == rank)
            return award->desc;
    }

    return "";
}

int
PlayerCharacter::RankFromName(const char* name)
{
    ListIter<AwardInfo> iter = rank_table;
    while (++iter) {
        AwardInfo* award = iter.value();
        if (award->name == name)
            return award->id;
    }

    return 0;
}

int32
PlayerCharacter::GetRank() const
{
    const int32 RankCount = rank_table.size();

    for (int32 Index = RankCount - 1; Index >= 0; --Index) {
        const AwardInfo* Award = rank_table[Index];

        if (points >= Award->total_points) {
            return Award->id;
        }
    }

    return 0;
}

void
PlayerCharacter::SetRank(int32 RankId)
{
    ListIter<AwardInfo> RankIter = rank_table;

    while (++RankIter) {
        AwardInfo* Award = RankIter.value();

        if (Award && RankId == Award->id) {
            points = Award->total_points;
            return;
        }
    } 
}

int
PlayerCharacter::Medal(int n) const
{
    if (n < 0)
        return 0;

    for (int i = 0; i < 16; i++) {
        int selector = 1 << (15 - i);

        // found a medal:
        if (medals & selector) {

            // and it's the nth medal!
            if (n == 0) {
                return selector;
            }

            n--;
        }
    }

    return 0;
}

// +-------------------------------------------------------------------+

const char*
PlayerCharacter::MedalName(int medal)
{
    ListIter<AwardInfo> iter = medal_table;
    while (++iter) {
        AwardInfo* award = iter.value();
        if (award->id == medal)
            return award->name;
    }

    return "";
}

Bitmap*
PlayerCharacter::MedalInsignia(int medal, int size)
{
    ListIter<AwardInfo> iter = medal_table;
    while (++iter) {
        AwardInfo* award = iter.value();
        if (award->id == medal) {
            if (size == 0)
                return award->small_insignia;

            if (size == 1)
                return award->large_insignia;
        }
    }

    return 0;
}

const char*
PlayerCharacter::MedalDescription(int medal)
{
    ListIter<AwardInfo> iter = medal_table;
    while (++iter) {
        AwardInfo* award = iter.value();
        if (award->id == medal)
            return award->desc;
    }

    return "";
}

// +-------------------------------------------------------------------+

bool
PlayerCharacter::CanCommand(int ship_class)
{
    if (ship_class <= (int)CLASSIFICATION::ATTACK)
        return true;

    for (int32 RankIndex = rank_table.size() - 1; RankIndex >= 0; --RankIndex) {
        AwardInfo* RankAward = rank_table[RankIndex];
        if (points > RankAward->total_points) {
            return (ship_class & RankAward->granted_ship_classes) != 0;
        }
    }

    return false;
}

int
PlayerCharacter::CommandRankRequired(int ship_class)
{
    for (int i = 0; i < rank_table.size(); i++) {
        AwardInfo* award = rank_table[i];
        if ((ship_class & award->granted_ship_classes) != 0) {
            return i;
        }
    }

    return rank_table.size() - 1;
}

// +-------------------------------------------------------------------+

int
PlayerCharacter::GetMissionPoints(ShipStats* ShipStatsPtr, DWORD StartTime)
{
    int MissionPoints = 0;

    if (ShipStatsPtr) {
        MissionPoints = ShipStatsPtr->GetPoints();

        const int FlightTimeSeconds = (Game::GameTime() - StartTime) / 1000;

        // If player survived the mission, award experience based on time in action
        if (!ShipStatsPtr->GetDeaths() && !ShipStatsPtr->GetColls()) {
            int MinutesInAction = FlightTimeSeconds / 60;
            MinutesInAction /= 10;
            MinutesInAction *= 10;

            MissionPoints += MinutesInAction;

            if (ShipStatsPtr->HasEvent(SimEvent::DOCK))
                MissionPoints += 100;
        }
        else {
            MissionPoints -= static_cast<int>(2.5 * Ship::Value(ShipStatsPtr->GetShipClass()));
        }

        if (MissionPoints < 0)
            MissionPoints = 0;
    }

    return MissionPoints;
}

// +-------------------------------------------------------------------+

void
PlayerCharacter::ProcessStats(ShipStats* s, DWORD start_time)
{
    if (!s) return;

    int old_rank = GetRank();
    int pts = GetMissionPoints(s, start_time);

    AddPoints(pts);
    AddPoints(s->GetCommandPoints());
    AddKills(s->GetGunKills());
    AddKills(s->GetMissileKills());
    AddLosses(s->GetDeaths());
    AddLosses(s->GetColls());
    AddMissions(1);
    AddFlightTime((Game::GameTime() - start_time) / 1000);

    rank = GetRank();

    // did the player earn a promotion?
    if (old_rank != rank) {
        ListIter<AwardInfo> iter = rank_table;
        while (++iter) {
            AwardInfo* a = iter.value();
            if (rank == a->id) {
                award = a;
            }
        }
    }

    // if not, did the player earn a medal?
    else {
        ListIter<AwardInfo> iter = medal_table;
        while (++iter) {
            AwardInfo* a = iter.value();

            if (EarnedAward(a, s) && a->ceremony) {
                award = a;
                break;
            }
        }
    }

    // persist all stats, promotions, and medals:
    Save();
}

bool
PlayerCharacter::EarnedAward(AwardInfo* a, ShipStats* s)
{
    if (!a || !s)
        return false;

    // already earned this medal?
    if (a->id & medals)
        return false;

    // eligible for this medal?
    rank = GetRank();
    if (a->min_rank > rank || a->max_rank < rank)
        return false;

    if ((a->required_awards & medals) < a->required_awards)
        return false;

    if (a->min_ship_class > s->GetShipClass() || a->max_ship_class < s->GetShipClass())
        return false;

    if (a->total_points > points)
        return false;

    if (a->total_missions > missions)
        return false;

    if (a->campaign_id && a->campaign_complete) {
        if (!HasCompletedCampaign(a->campaign_id))
            return false;
    }
    else {
        // campaign related requirements
        Campaign* c = Campaign::GetCampaign();

        if (c) {
            if (a->dynamic_campaign && !c->IsDynamic())
                return false;
        }
    }

    // sufficient merit for this medal?
    if (a->mission_points > s->GetPoints())
        return false;

    if (a->kills > s->GetGunKills() + s->GetMissileKills())
        return false;

    if (a->mission_points > s->GetPoints())
        return false;

    // player must survive mission if lost = -1
    if (a->lost < 0 && (s->GetDeaths() || s->GetColls()))
        return false;

    // do we need to be wounded in battle?
    if (a->lost > s->GetDeaths() || a->collision > s->GetColls())
        return false;

    // final lottery check:
    if (a->lottery < 2 || RandomChance(1, a->lottery)) {
        medals |= a->id;
        return true;
    }

    // what do we have for the losers, judge?
    return false;
}

// +-------------------------------------------------------------------+

static List<PlayerCharacter>  player_roster;
static PlayerCharacter* current_player = 0;

List<PlayerCharacter>&
PlayerCharacter::GetRoster()
{
    return player_roster;
}

PlayerCharacter*
PlayerCharacter::GetCurrentPlayer()
{
    return GCurrentPlayer;;
}

void PlayerCharacter::SetCurrentPlayer(PlayerCharacter* NewPlayer)
{
    if (GCurrentPlayer == NewPlayer)
        return;

    // Optional: clean up previous player if ownership is clear
    // delete GCurrentPlayer;

    GCurrentPlayer = NewPlayer;
}

void
PlayerCharacter::SelectPlayer(PlayerCharacter* p)
{
    HUDView* hud = HUDView::GetInstance();

    if (current_player && current_player != p) {
        if (hud) {
            for (int i = 0; i < 3; i++) {
                MFDView* mfd = hud->GetMFD(i);

                if (mfd)
                    current_player->mfd[i] = mfd->GetMode();
            }
        }
    }

    if (player_roster.contains(p)) {
        current_player = p;

        Ship::SetFlightModel(p->flight_model);
        Ship::SetLandingModel(p->landing_model);
        HUDView::SetArcade(p->hud_mode > 0);
        HUDView::SetDefaultColorSet(p->hud_color);

        if (hud) {
            for (int i = 0; i < 3; i++) {
                if ((int)p->mfd[i] >= 0) {
                    MFDView* mfd = hud->GetMFD(i);

                    if (mfd)
                        mfd->SetMode(p->mfd[i]);
                }
            }
        }
    }
}

PlayerCharacter*
PlayerCharacter::Find(const char* name)
{
    for (int i = 0; i < player_roster.size(); i++) {
        PlayerCharacter* p = player_roster.at(i);
        if (p->Name() == name)
            return p;
    }

    return 0;
}

PlayerCharacter*
PlayerCharacter::Create(const char* name)
{
    if (name && *name) {
        // check for existence:
        if (Find(name))
            return 0;

        PlayerCharacter* newbie = new PlayerCharacter(name);
        player_roster.append(newbie);
        newbie->CreateUniqueID();
        return newbie;
    }

    return 0;
}

void
PlayerCharacter::Destroy(PlayerCharacter* p)
{
    if (p) {
        player_roster.remove(p);

        if (p == current_player) {
            current_player = 0;

            if (player_roster.size())
                current_player = player_roster.at(0);
        }

        CampaignSaveGame::RemovePlayer(p);
        delete p;
    }
}

// +-------------------------------------------------------------------+

void
PlayerCharacter::Initialize()
{
    LoadAwardTables();
    Load();

    if (!current_player) {
        if (!player_roster.size()) {
            Create("Pilot");
        }

        SelectPlayer(player_roster.at(0));
    }
}

void
PlayerCharacter::Close()
{
    if (current_player && !player_roster.contains(current_player))
        delete current_player;

    player_roster.destroy();
    current_player = 0;

    rank_table.destroy();
    medal_table.destroy();
}

// +-------------------------------------------------------------------+

bool PlayerCharacter::ConfigExists() const
{
    return config_exists;
}
// +-------------------------------------------------------------------+

#define GET_DEF_BOOL(x) if(pdef->name()->value()==(#x))GetDefBool(player->x,pdef,filename)
#define GET_DEF_TEXT(x) if(pdef->name()->value()==(#x))GetDefText(player->x,pdef,filename)
#define GET_DEF_NUM(x)  if(pdef->name()->value()==(#x))GetDefNumber(player->x,pdef,filename)

void PlayerCharacter::Load()
{
    config_exists = false;

    // ------------------------------------------------------------
    // Read config file into memory block
    // ------------------------------------------------------------
    BYTE* block = nullptr;
    int   blocklen = 0;

    char filename[64];
    strcpy_s(filename, "player.cfg");

    FILE* f = nullptr;
    ::fopen_s(&f, filename, "rb");

    if (f)
    {
        config_exists = true;

        ::fseek(f, 0, SEEK_END);
        blocklen = static_cast<int>(::ftell(f));
        ::fseek(f, 0, SEEK_SET);

        block = new BYTE[blocklen + 1];
        block[blocklen] = 0;

        ::fread(block, blocklen, 1, f);
        ::fclose(f);
    }

    if (blocklen == 0)
        return;

    // ------------------------------------------------------------
    // Parse file
    // ------------------------------------------------------------
    Parser parser(new BlockReader((const char*)block, blocklen));
    Term* term = parser.ParseTerm();

    if (!term)
    {
        UE_LOG(LogTemp, Error,
            TEXT("ERROR: could not parse '%s'."),
            ANSI_TO_TCHAR(filename));

        delete[] block;
        return;
    }
    else
    {
        TermText* file_type = term->isText();
        if (!file_type || file_type->value() != "PLAYER_CONFIG")
        {
            UE_LOG(LogTemp, Warning,
                TEXT("WARNING: invalid '%s' file. Using defaults"),
                ANSI_TO_TCHAR(filename));

            delete term;
            delete[] block;
            return;
        }
    }

    // ------------------------------------------------------------
    // Reset roster
    // ------------------------------------------------------------
    if (current_player && !player_roster.contains(current_player))
        delete current_player;

    player_roster.destroy();
    current_player = nullptr;

    // ------------------------------------------------------------
    // Inline DEF helpers (must exist above this method or in a header)
    // ------------------------------------------------------------
    // inline bool DefNameEquals(const TermDef* Def, const char* Key);
    // inline void DefReadBool(bool& Out, TermDef* Def, const char* Filename);
    // inline void DefReadInt(int& Out, TermDef* Def, const char* Filename);
    // inline void DefReadText(Text& Out, TermDef* Def, const char* Filename);
    // template<typename TEnum> inline void DefReadEnum(TEnum& Out, TermDef* Def, const char* Filename);

    // ------------------------------------------------------------
    // Parse terms until EOF
    // ------------------------------------------------------------
    do
    {
        delete term;
        term = parser.ParseTerm();

        if (!term)
            break;

        TermDef* def = term->isDef();
        if (!def)
        {
            UE_LOG(LogTemp, Warning,
                TEXT("WARNING: term ignored in '%s'"),
                ANSI_TO_TCHAR(filename));
            term->print();
            continue;
        }

        if (!DefNameEquals(def, "player"))
        {
            UE_LOG(LogTemp, Warning,
                TEXT("WARNING: unknown label '%s' in '%s'"),
                ANSI_TO_TCHAR(def->name()->value().data()),
                ANSI_TO_TCHAR(filename));

            continue;
        }

        if (!def->term() || !def->term()->isStruct())
        {
            UE_LOG(LogTemp, Warning,
                TEXT("WARNING: player structure missing in '%s'"),
                ANSI_TO_TCHAR(filename));

            continue;
        }

        PlayerCharacter* player = new PlayerCharacter;
        bool             current = false;

        TermStruct* val = def->term()->isStruct();

        // Make sure arrays have enough slots before assignment:
        // - chat_macros (legacy) assumed at least 10
        // - mfd is now UE TArray<EMFDMode> (ensure >= 3)
        if (player->mfd.Num() < 3)
            player->mfd.SetNum(3);

        for (int i = 0; i < val->elements()->size(); i++)
        {
            TermDef* pdef = val->elements()->at(i)->isDef();
            if (!pdef)
                continue;

            // ----------------------------------------------------
            // Common keys (Text)
            // ----------------------------------------------------
            if (DefNameEquals(pdef, "name")) { DefReadText(player->name, pdef, filename); }
            else if (DefNameEquals(pdef, "squadron")) { DefReadText(player->squadron, pdef, filename); }
            else if (DefNameEquals(pdef, "signature")) { DefReadText(player->signature, pdef, filename); }

            // ----------------------------------------------------
            // Common keys (Numbers)
            // ----------------------------------------------------
            else if (DefNameEquals(pdef, "uid")) { DefReadInt(player->uid, pdef, filename); }
            else if (DefNameEquals(pdef, "flight_model")) { DefReadInt(player->flight_model, pdef, filename); }
            else if (DefNameEquals(pdef, "flying_start")) { DefReadInt(player->flying_start, pdef, filename); }
            else if (DefNameEquals(pdef, "landing_model")) { DefReadInt(player->landing_model, pdef, filename); }
            else if (DefNameEquals(pdef, "ai_level")) { DefReadInt(player->ai_level, pdef, filename); }
            else if (DefNameEquals(pdef, "hud_mode")) { DefReadInt(player->hud_mode, pdef, filename); }
            else if (DefNameEquals(pdef, "hud_color")) { DefReadInt(player->hud_color, pdef, filename); }
            else if (DefNameEquals(pdef, "ff_level")) { DefReadInt(player->ff_level, pdef, filename); }
            else if (DefNameEquals(pdef, "grid")) { DefReadInt(player->grid, pdef, filename); }
            else if (DefNameEquals(pdef, "gunsight")) { DefReadInt(player->gunsight, pdef, filename); }

            // ----------------------------------------------------
            // Chat macros: chat_0 .. chat_9
            // ----------------------------------------------------
            else
            {
                const Text Key = pdef->name()->value();

                if (Key.indexOf("chat_") == 0)
                {
                    int idx = -1;
                    const char* Raw = Key.data();
                    if (Raw && ::strlen(Raw) >= 6)
                        idx = Raw[5] - '0';

                    if (idx >= 0 && idx < 10)
                        GetDefText(player->chat_macros[idx], pdef, filename);
                }

                // ------------------------------------------------
                // MFD modes: mfd0..mfd2 (enum)
                // ------------------------------------------------
                else if (Key == "mfd0") { DefReadEnum(player->mfd[0], pdef, filename); }
                else if (Key == "mfd1") { DefReadEnum(player->mfd[1], pdef, filename); }
                else if (Key == "mfd2") { DefReadEnum(player->mfd[2], pdef, filename); }

                // ------------------------------------------------
                // Bool / other special keys
                // ------------------------------------------------
                else if (Key == "current")
                {
                    DefReadBool(current, pdef, filename);
                }
                else if (Key == "trained")
                {
                    DefReadInt(player->trained, pdef, filename);
                }
                else if (Key == "stats")
                {
                    Text stats;
                    DefReadText(stats, pdef, filename);
                    player->DecodeStats(stats);
                }

                // ------------------------------------------------
                // Cheat keys (legacy)
                // ------------------------------------------------
                else if (Key.indexOf("XXX_CHEAT_A1B2C3_") == 0)
                {
                    if (Key.contains("points"))
                        DefReadInt(player->points, pdef, filename);

                    else if (Key.contains("rank"))
                    {
                        int rank = 0;
                        DefReadInt(rank, pdef, filename);
                        player->SetRank(rank);
                    }
                    else if (Key.contains("medals"))
                        DefReadInt(player->medals, pdef, filename);
                    else if (Key.contains("campaigns"))
                        DefReadInt(player->campaigns, pdef, filename);
                    else if (Key.contains("missions"))
                        DefReadInt(player->missions, pdef, filename);
                    else if (Key.contains("kills"))
                        DefReadInt(player->kills, pdef, filename);
                    else if (Key.contains("losses"))
                        DefReadInt(player->losses, pdef, filename);
                    else if (Key.contains("flight_time"))
                        DefReadInt(player->flight_time, pdef, filename);
                }
            }
        }

        player_roster.append(player);
        player->CreateUniqueID();

        if (current)
            SelectPlayer(player);

    } while (term);

    delete[] block;
}


// +-------------------------------------------------------------------+

void
PlayerCharacter::Save()
{
    HUDView* hud = HUDView::GetInstance();
    if (hud && current_player) {
        for (int i = 0; i < 3; i++) {
            MFDView* mfd = hud->GetMFD(i);

            if (mfd)
                current_player->mfd[i] = mfd->GetMode();
        }
    }

    FILE* f;
    fopen_s(&f, "player.cfg", "w");
    if (f) {
        fprintf(f, "PLAYER_CONFIG\n\n");

        ListIter<PlayerCharacter> iter = player_roster;
        while (++iter) {
            PlayerCharacter* p = iter.value();

            fprintf(f, "player: {\n");
            fprintf(f, "   uid:           %d,\n", p->uid);
            fprintf(f, "   name:          \"%s\",\n", SafeQuotes(p->name.data()));
            fprintf(f, "   squadron:      \"%s\",\n", SafeQuotes(p->squadron.data()));
            fprintf(f, "   signature:     \"%s\",\n", SafeQuotes(p->signature.data()));

            Text stat_data = p->EncodeStats();

            if (stat_data.length() > 32) {
                char tmp[64];
                int  len = stat_data.length();

                for (int n = 0; n < len; n += 32) {
                    ZeroMemory(tmp, sizeof(tmp));
                    const char* ptxt = stat_data.data() + n;
                    strncpy(tmp, ptxt, 32);

                    if (n == 0)
                        fprintf(f, "   stats:         \"%s\"\n", tmp);
                    else if (n < len - 32)
                        fprintf(f, "                  \"%s\"\n", tmp);
                    else
                        fprintf(f, "                  \"%s\",\n", tmp);
                }
            }

            if (p == current_player)
                fprintf(f, "   current:       true,\n");
            else
                fprintf(f, "   current:       false,\n");

            fprintf(f, "   trained:       %d,\n", p->trained);
            fprintf(f, "   flight_model:  %d,\n", p->flight_model);
            fprintf(f, "   flying_start:  %d,\n", p->flying_start);
            fprintf(f, "   landing_model: %d,\n", p->landing_model);
            fprintf(f, "   ai_level:      %d,\n", p->ai_level);
            fprintf(f, "   hud_mode:      %d,\n", p->hud_mode);
            fprintf(f, "   hud_color:     %d,\n", p->hud_color);
            fprintf(f, "   ff_level:      %d,\n", p->ff_level);
            fprintf(f, "   grid:          %d,\n", p->grid);
            fprintf(f, "   gunsight:      %d,\n", p->gunsight);

            for (int i = 0; i < 10; i++) {
                fprintf(f, "   chat_%d:       \"%s\",\n", i, SafeQuotes(p->chat_macros[i].data()));
            }

            for (int i = 0; i < 3; i++) {
                if ((int)p->mfd[i] >= 0) {
                    fprintf(f, "   mfd%d:         %d,\n", i, p->mfd[i]);
                }
            }

            fprintf(f, "}\n\n");
        }

        fclose(f);

        config_exists = true;
    }
}

bool PlayerCharacter::SaveToSubsystem(UObject* WorldContext)
{
    if (!WorldContext)
        return false;

    UWorld* World = WorldContext->GetWorld();
    if (!World)
        return false;

    UGameInstance* GI = World->GetGameInstance();
    if (!GI)
        return false;

    UStarshatterPlayerSubsystem* PlayerSS = GI->GetSubsystem<UStarshatterPlayerSubsystem>();
    if (!PlayerSS)
        return false;

    PlayerCharacter* P = PlayerCharacter::GetCurrentPlayer();
    if (!P)
        return false;

    // --- legacy: capture MFD modes from HUD into player object ---
    HUDView* hud = HUDView::GetInstance();
    if (hud)
    {
        for (int i = 0; i < 3; i++)
        {
            MFDView* mfd = hud->GetMFD(i);
            if (mfd)
                P->mfd[i] = mfd->GetMode();
        }
    }

    // --- write into subsystem struct ---
    FS_PlayerGameInfo& Info = PlayerSS->GetMutablePlayerInfo();

    // Identity
    Info.Id = P->uid;
    Info.Name = UTF8_TO_TCHAR(P->name.data());
    // Optional: treat Nickname like legacy "squadron/callsign" if you want:
    // Info.Nickname = UTF8_TO_TCHAR(P->squadron.data());
    Info.Signature = UTF8_TO_TCHAR(P->signature.data());

    // Gameplay prefs
    Info.Rank = P->GetRank();          // if you have it; otherwise leave as-is
    Info.Trained = P->trained;
    Info.FlightModel = P->flight_model;
    Info.LandingMode = P->landing_model;
    Info.FlyingStart = (P->flying_start != 0);

    Info.AILevel = P->ai_level;
    Info.HudMode = P->hud_mode;
    Info.HudColor = P->hud_color;
    Info.ForceFeedbackLevel = P->ff_level;

    Info.GridMode = (P->grid != 0);
    Info.GunSightMode = (P->gunsight != 0);

    // Chat macros
    Info.ChatMacros.SetNum(10);
    for (int i = 0; i < 10; i++)
    {
        Info.ChatMacros[i] = UTF8_TO_TCHAR(P->chat_macros[i].data());
    }

    // MFD modes
    Info.MfdModes.SetNum(3);
    for (int i = 0; i < 3; i++)
    {
        Info.MfdModes[i] = (int32)P->mfd[i];
    }

    // Persist
    return PlayerSS->SavePlayer(true);
}
// +-------------------------------------------------------------------+

static char stat_buf[280];
static char code_buf[280];

Text
PlayerCharacter::EncodeStats()
{
    ZeroMemory(stat_buf, 280);
    ZeroMemory(code_buf, 280);

    sprintf_s(stat_buf,
        "%-16s%08x%08x%08x%08x%08x%08x%08x%08x%08x%08x%08x%08x%08x%08x%08x%08x%08x%08x%08x%08x%08x%08x%08x%08x%08x%08x%08x%08x%08x%08x",
        pass.data(),
        create_date,
        points,
        flight_time,
        missions,
        kills,
        losses,
        medals,
        campaigns,
        11, 12, 13, 14, 15, 16,
        17, 18, 19, 20, 21, 22, 23, 24,
        25, 26, 27, 28, 29, 30, 31, 32);

    for (int i = 0; i < 16; i++)
        for (int j = 0; j < 16; j++)
            code_buf[i * 16 + j] = stat_buf[j * 16 + i];

    return Encryption::Encode(Encryption::Encrypt(code_buf));
}

void
PlayerCharacter::DecodeStats(const char* stats)
{
    ZeroMemory(stat_buf, 280);
    ZeroMemory(code_buf, 280);

    if (!stats || !*stats) {
        UE_LOG(LogTemp, Warning,
            TEXT("PlayerCharacter::DecodeStats() invalid or missing stats"));

        return;
    }

    Text plain = Encryption::Decrypt(Encryption::Decode(stats));

    if (plain.length() == 64) {
        for (int i = 0; i < 8; i++)
            for (int j = 0; j < 8; j++)
                stat_buf[j * 8 + i] = plain[i * 8 + j];
    }

    else if (plain.length() == 256) {
        for (int i = 0; i < 16; i++)
            for (int j = 0; j < 16; j++)
                stat_buf[j * 16 + i] = plain[i * 16 + j];
    }

    else {
        const int32 PlainLen = static_cast<int32>(plain.length());

        UE_LOG(LogTemp, Warning,
            TEXT("PlayerCharacter::DecodeStats() invalid plain text length %d"),
            PlainLen);

        return;
    }

    char work[32];
    ZeroMemory(work, 32);
    CopyMemory(work, stat_buf, 16);
    for (int i = 15; i > 0; i--)
        if (work[i] == ' ') work[i] = 0;
        else break;
    pass = work;

    ZeroMemory(work, 16);
    CopyMemory(work, stat_buf + 16, 8);
    sscanf_s(work, "%x", &create_date);

    ZeroMemory(work, 16);
    CopyMemory(work, stat_buf + 24, 8);
    sscanf_s(work, "%x", &points);
    if (points < 0) points = 0;

    ZeroMemory(work, 16);
    CopyMemory(work, stat_buf + 32, 8);
    sscanf_s(work, "%x", &flight_time);
    if (flight_time < 0) flight_time = 0;

    ZeroMemory(work, 16);
    CopyMemory(work, stat_buf + 40, 8);
    sscanf_s(work, "%x", &missions);
    if (missions < 0) missions = 0;

    ZeroMemory(work, 16);
    CopyMemory(work, stat_buf + 48, 8);
    sscanf_s(work, "%x", &kills);
    if (kills < 0) kills = 0;

    ZeroMemory(work, 16);
    CopyMemory(work, stat_buf + 56, 8);
    sscanf_s(work, "%x", &losses);
    if (losses < 0) losses = 0;

    if (plain.length() > 64) {
        ZeroMemory(work, 16);
        CopyMemory(work, stat_buf + 64, 8);
        sscanf_s(work, "%x", &medals);

        ZeroMemory(work, 16);
        CopyMemory(work, stat_buf + 72, 8);
        sscanf_s(work, "%x", &campaigns);
    }

    if (create_date == 0) {
        UE_LOG(LogTemp, Warning,
            TEXT("WARNING - loaded player with zero stats '%s'"),
            ANSI_TO_TCHAR(name.data()));
    }
}

// +-------------------------------------------------------------------+

void
PlayerCharacter::LoadAwardTables()
{
    DataLoader* loader = DataLoader::GetLoader();
    if (!loader) return;

    BYTE* block = 0;
    const char* filename = "awards.def";

    loader->SetDataPath("Awards/");
    loader->LoadBuffer(filename, block, true);
    Parser parser(new  BlockReader((const char*)block));

    Term* term = parser.ParseTerm();

    if (!term) {
        return;
    }
    else {
        TermText* file_type = term->isText();
        if (!file_type || file_type->value() != "AWARDS") {
            return;
        }
    }

    rank_table.destroy();
    medal_table.destroy();

    UE_LOG(LogTemp, Log,
        TEXT("Loading Ranks and Medals"));

    do {
        delete term; term = 0;
        term = parser.ParseTerm();

        if (term) {
            TermDef* def = term->isDef();
            if (def) {
                if (def->name()->value() == "award") {

                    if (!def->term() || !def->term()->isStruct()) {
                        UE_LOG(LogTemp, Warning,
                            TEXT("WARNING: award structure missing in '%s'"),
                            ANSI_TO_TCHAR(filename));
                    }
                    else {
                        AwardInfo* award = new  AwardInfo;
                        TermStruct* val = def->term()->isStruct();

                        for (int i = 0; i < val->elements()->size(); i++) {
                            TermDef* pdef = val->elements()->at(i)->isDef();
                            if (pdef) {
                                if (pdef->name()->value() == ("name")) {
                                    GetDefText(award->name, pdef, filename);
                                    award->name = Game::GetText(award->name);
                                }

                                else if (pdef->name()->value() == ("abrv")) {
                                    GetDefText(award->abrv, pdef, filename);
                                    award->abrv = Game::GetText(award->abrv);
                                }

                                else if (pdef->name()->value() == ("desc")) {
                                    GetDefText(award->desc, pdef, filename);
                                    if (award->desc.length() <= 40)
                                        award->desc = Game::GetText(award->desc);
                                }

                                else if (pdef->name()->value() == ("award")) {
                                    GetDefText(award->grant, pdef, filename);
                                    if (award->grant.length() <= 40)
                                        award->grant = Game::GetText(award->grant);
                                }

                                else if (pdef->name()->value() == ("desc_sound"))
                                    GetDefText(award->desc_sound, pdef, filename);

                                else if (pdef->name()->value() == ("award_sound"))
                                    GetDefText(award->grant_sound, pdef, filename);

                                else if (pdef->name()->value().indexOf("large") == 0) {
                                    Text txt;
                                    GetDefText(txt, pdef, filename);
                                    txt.setSensitive(false);

                                    if (!txt.contains(".pcx"))
                                        txt.append(".pcx");
                                }

                                else if (pdef->name()->value().indexOf("small") == 0) {
                                    Text txt;
                                    GetDefText(txt, pdef, filename);
                                    txt.setSensitive(false);

                                    if (!txt.contains(".pcx"))
                                        txt.append(".pcx");

                                    if (award->small_insignia)
                                        award->small_insignia->AutoMask();
                                }

                                else if (pdef->name()->value() == ("type")) {
                                    Text txt;
                                    GetDefText(txt, pdef, filename);
                                    txt.setSensitive(false);

                                    if (txt == "rank")
                                        award->type = AwardInfo::RANK;

                                    else if (txt == "medal")
                                        award->type = AwardInfo::MEDAL;
                                }

                                else if (pdef->name()->value() == ("id"))
                                    GetDefNumber(award->id, pdef, filename);

                                else if (pdef->name()->value() == ("total_points"))
                                    GetDefNumber(award->total_points, pdef, filename);

                                else if (pdef->name()->value() == ("mission_points"))
                                    GetDefNumber(award->mission_points, pdef, filename);

                                else if (pdef->name()->value() == ("total_missions"))
                                    GetDefNumber(award->total_missions, pdef, filename);

                                else if (pdef->name()->value() == ("kills"))
                                    GetDefNumber(award->kills, pdef, filename);

                                else if (pdef->name()->value() == ("lost"))
                                    GetDefNumber(award->lost, pdef, filename);

                                else if (pdef->name()->value() == ("collision"))
                                    GetDefNumber(award->collision, pdef, filename);

                                else if (pdef->name()->value() == ("campaign_id"))
                                    GetDefNumber(award->campaign_id, pdef, filename);

                                else if (pdef->name()->value() == ("campaign_complete"))
                                    GetDefBool(award->campaign_complete, pdef, filename);

                                else if (pdef->name()->value() == ("dynamic_campaign"))
                                    GetDefBool(award->dynamic_campaign, pdef, filename);

                                else if (pdef->name()->value() == ("ceremony"))
                                    GetDefBool(award->ceremony, pdef, filename);

                                else if (pdef->name()->value() == ("required_awards"))
                                    GetDefNumber(award->required_awards, pdef, filename);

                                else if (pdef->name()->value() == ("lottery"))
                                    GetDefNumber(award->lottery, pdef, filename);

                                else if (pdef->name()->value() == ("min_rank"))
                                    GetDefNumber(award->min_rank, pdef, filename);

                                else if (pdef->name()->value() == ("max_rank"))
                                    GetDefNumber(award->max_rank, pdef, filename);

                                else if (pdef->name()->value() == ("min_ship_class")) {
                                    Text classname;
                                    GetDefText(classname, pdef, filename);
                                    award->min_ship_class = Ship::ClassForName(classname);
                                }

                                else if (pdef->name()->value() == ("max_ship_class")) {
                                    Text classname;
                                    GetDefText(classname, pdef, filename);
                                    award->max_ship_class = Ship::ClassForName(classname);
                                }

                                else if (pdef->name()->value().indexOf("grant") == 0)
                                    GetDefNumber(award->granted_ship_classes, pdef, filename);
                            }
                        }

                        if (award->type == AwardInfo::RANK) {
                            rank_table.append(award);
                        }
                        else if (award->type == AwardInfo::MEDAL) {
                            medal_table.append(award);
                        }
                        else {
                            delete award;
                        }
                    }
                }
                else {
                    UE_LOG(LogTemp, Warning,
                        TEXT("WARNING: unknown label '%s' in '%s'"),
                        ANSI_TO_TCHAR(def->name()->value().data()),
                        ANSI_TO_TCHAR(filename));
                }
            }
            else {
                UE_LOG(LogTemp, Warning,
                    TEXT("WARNING: term ignored in '%s'"),
                    ANSI_TO_TCHAR(filename));
                    term->print();
            }
        }
    } while (term);

    loader->ReleaseBuffer(block);
    loader->SetDataPath(0);
}

// +-------------------------------------------------------------------+

void
PlayerCharacter::CreateUniqueID()
{
    ListIter<PlayerCharacter> iter = player_roster;
    while (++iter) {
        PlayerCharacter* p = iter.value();

        if (p != this && p->uid >= uid)
            uid = p->uid + 1;
    }

    if (uid < 1)
        uid = 1;
}

PlayerCharacter* PlayerCharacter::EnsureCurrentPlayer()
{
    PlayerCharacter* P = PlayerCharacter::GetCurrentPlayer();
    if (P)
        return P;

    // Create a default player object (whatever your class expects)
    P = new PlayerCharacter();
    PlayerCharacter::SetCurrentPlayer(P); // implement if you don't have it
    return P;
}