/*
    Project Starshatter 4.5
    Destroyer Studios LLC
    Copyright © 1997-2004.
    All Rights Reserved.

    SUBSYSTEM:    Stars.exe
    FILE:         PlayerCharacter.h
    AUTHOR:       Carlos Bott

    OVERVIEW
    ========
    Player / Logbook class (renamed for Unreal compatibility)
*/

#pragma once

#include "Types.h"
#include "List.h"
#include "Text.h"
#include "DataLoader.h"
#include "Encrypt.h"
#include "ParseUtil.h"
#include "GameStructs.h"
#include <cstring>


// +-------------------------------------------------------------------+

class Bitmap;
class ShipStats;
class AwardInfo;
class USound;

// +-------------------------------------------------------------------+

class PlayerCharacter
{
public:
    static const char* TYPENAME() { return "PlayerCharacter"; }

    PlayerCharacter(const char* name);
    virtual ~PlayerCharacter();

    int operator == (const PlayerCharacter& u) const { return name == u.name; }

    int            Identity()        const { return uid; }
    const Text& Name()            const { return name; }
    const Text& Password()        const { return pass; }
    const Text& Squadron()        const { return squadron; }
    const Text& Signature()       const { return signature; }
    const Text& ChatMacro(int n)  const;
    int            CreateDate()      const { return create_date; }
    int            GetRank()         const;
    int            Medal(int n)      const;
    int            Points()          const { return points; }
    int            Medals()          const { return medals; }
    int            FlightTime()      const { return flight_time; }
    int            Missions()        const { return missions; }
    int            Kills()           const { return kills; }
    int            Deaths()          const { return deaths; }
    int            Losses()          const { return losses; }
    int            Campaigns()       const { return campaigns; }
    int            Trained()         const { return trained; }

    int            FlightModel()     const { return flight_model; }
    int            FlyingStart()     const { return flying_start; }
    int            LandingModel()    const { return landing_model; }
    int            AILevel()         const { return ai_level; }
    int            HUDMode()         const { return hud_mode; }
    int            HUDColor()        const { return hud_color; }
    int            FriendlyFire()    const { return ff_level; }
    int            GridMode()        const { return grid; }
    int            Gunsight()        const { return gunsight; }

    bool           ShowAward()       const { return award != 0; }
    Text           AwardName()       const;
    Text           AwardDesc()       const;
    Bitmap*        AwardImage()      const;
    USound*         AwardSound()      const;

    bool           CanCommand(int ship_class);

    void           SetName(const char* n);
    void           SetPassword(const char* p);
    void           SetSquadron(const char* s);
    void           SetSignature(const char* s);
    void           SetChatMacro(int n, const char* m);
    void           SetCreateDate(int d);
    void           SetRank(int r);
    void           SetPoints(int p);
    void           SetMedals(int m);
    void           SetCampaigns(int n);
    void           SetTrained(int n);
    void           SetFlightTime(int t);
    void           SetMissions(int m);
    void           SetKills(int k);
    void           SetDeaths(int d);
    void           SetLosses(int l);

    void           AddFlightTime(int t);
    void           AddPoints(int p);
    void           AddMedal(int m);
    void           AddMissions(int m);
    void           AddKills(int k);
    void           AddDeaths(int d);
    void           AddLosses(int l);

    bool           HasTrained(int n)            const;
    bool           HasCompletedCampaign(int id) const;
    void           SetCampaignComplete(int id);

    void           SetFlightModel(int n);
    void           SetFlyingStart(int n);
    void           SetLandingModel(int n);
    void           SetAILevel(int n);
    void           SetHUDMode(int n);
    void           SetHUDColor(int n);
    void           SetFriendlyFire(int n);
    void           SetGridMode(int n);
    void           SetGunsight(int n);

    void           ClearShowAward();

    Text           EncodeStats();
    void           DecodeStats(const char* stats);

    int            GetMissionPoints(ShipStats* stats, DWORD start_time);
    void           ProcessStats(ShipStats* stats, DWORD start_time);
    bool           EarnedAward(AwardInfo* a, ShipStats* s);

    static const char* RankName(int rank);
    static const char* RankAbrv(int rank);
    static int         RankFromName(const char* name);
    static Bitmap*     RankInsignia(int rank, int size);
    static const char* RankDescription(int rank);
    static const char* MedalName(int medal);
    static Bitmap*     MedalInsignia(int medal, int size);
    static const char* MedalDescription(int medal);
    static int           CommandRankRequired(int ship_class);

    static List<PlayerCharacter>& GetRoster();
    static PlayerCharacter*       GetCurrentPlayer();
    static void                   SelectPlayer(PlayerCharacter* p);
    static void                   SetCurrentPlayer(PlayerCharacter* NewPlayer);

    static PlayerCharacter*       EnsureCurrentPlayer();

    static PlayerCharacter* Create(const char* name);
    static void                   Destroy(PlayerCharacter* p);
    static PlayerCharacter* Find(const char* name);
    static void                   Initialize();
    static void                   Close();
    bool                          ConfigExists() const;
    static void                   Load();
    static void                   Save();
    static bool                   SaveToSubsystem(UObject* WorldContext);
    static void                   LoadAwardTables();

    static PlayerCharacter* CreateDefault();
    static void AddToRoster(PlayerCharacter* P);
    static void RemoveFromRoster(PlayerCharacter* P);

protected:
    PlayerCharacter();

    void           CreateUniqueID();


    int            uid;
    Text           name;
    Text           pass;
    Text           squadron;
    Text           signature;
    Text           chat_macros[10];
   
    TArray<EMFDMode> mfd;

    // stats:
    int            create_date;
    int            points;
    int            medals;        // bitmap of earned medals
    int            flight_time;
    int            missions;
    int            kills;
    int            deaths;
    int            losses;
    int            campaigns;     // bitmap of completed campaigns
    int            trained;       // id of highest training mission completed
    int            rank;
    // gameplay options:
    int            flight_model;
    int            flying_start;
    int            landing_model;
    int            ai_level;
    int            hud_mode;
    int            hud_color;
    int            ff_level;
    int            grid;
    int            gunsight;

    // transient:
    AwardInfo* award;
};

// ------------------------------------------------------------
// Inline DEF helpers (Starshatter-compatible, UE-safe)
// ------------------------------------------------------------

inline bool DefNameEquals(const TermDef* Def, const char* Key)
{
    if (!Def || !Def->name() || !Key)
        return false;

    // TermText::value() appears to be legacy Text (not std::string).
    // Avoid operator== (may return int), compare raw C-strings:
    const Text NameText = Def->name()->value();
    const char* Name = NameText.data();

    return Name && (::strcmp(Name, Key) == 0);
}

inline void DefReadBool(bool& Out, TermDef* Def, const char* Filename)
{
    // Legacy helpers typically return int; we don't use the return value as bool.
    GetDefBool(Out, Def, Filename);

    // Normalize to strict bool (optional but safe):
    Out = Out ? true : false;
}

inline void DefReadInt(int& Out, TermDef* Def, const char* Filename)
{
    GetDefNumber(Out, Def, Filename);
}

inline void DefReadText(Text& Out, TermDef* Def, const char* Filename)
{
    GetDefText(Out, Def, Filename);
}

// Enum helper: read numeric value, cast to enum
template<typename TEnum>
inline void DefReadEnum(TEnum& Out, TermDef* Def, const char* Filename)
{
    int Temp = static_cast<int>(Out);
    GetDefNumber(Temp, Def, Filename);
    Out = static_cast<TEnum>(Temp);
}

// ------------------------------------------------------------
// Inline Text helpers (avoid C4800 int->bool)
// ------------------------------------------------------------

inline bool TextStartsWith(const Text& S, const char* Prefix)
{
    const char* Str = S.data();
    if (!Str || !Prefix)
        return false;

    const size_t N = ::strlen(Prefix);
    return ::strncmp(Str, Prefix, N) == 0;
}

inline bool TextContains(const Text& S, const char* Needle)
{
    const char* Str = S.data();
    if (!Str || !Needle)
        return false;

    // strstr returns pointer or null; convert explicitly to bool:
    return (::strstr(Str, Needle) != nullptr);
}

// Optional: parse "chat_0".."chat_9" key
inline bool TryParseChatIndex(const Text& Key, int& OutIndex)
{
    OutIndex = -1;

    const char* Str = Key.data();
    if (!Str)
        return false;

    // Expect "chat_X"
    if (::strncmp(Str, "chat_", 5) != 0)
        return false;

    const char c = Str[5];
    if (c < '0' || c > '9')
        return false;

    OutIndex = (c - '0');
    return true;
}