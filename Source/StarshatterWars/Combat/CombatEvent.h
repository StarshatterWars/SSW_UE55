#pragma once

#include "Types.h"
#include "Geometry.h"
#include "Text.h"
#include "List.h"
#include "GameStructs.h"
#include "Math/Vector.h"   // FVector

// +--------------------------------------------------------------------+

class Campaign;
class CombatGroup;
class CombatUnit;
class UTexture2D;

// +--------------------------------------------------------------------+

class CombatEvent
{
public:
    static const char* TYPENAME() { return "CombatEvent"; }

    CombatEvent(Campaign* c, int type, int time, int team, ECombatEventSource source, const char* rgn);

    int operator==(const CombatEvent& u) const { return this == &u; }

    // accessors:
    int                  GetType()      const { return type; }
    int                  Time()         const { return time; }
    int                  GetIFF()       const { return team; }
    const FVector&       GetPoints()    const { return points; }
    ECombatEventSource   GetSource()    const { return source; }
    FVector              Location()     const { return loc; }
    const char* Region()       const { return region; }
    const char* Title()        const { return title; }
    const char* Information()  const { return info; }
    const char* Filename()     const { return file; }
    const char* ImageFile()    const { return image_file; }
    const char* SceneFile()    const { return scene_file; }
    //Bitmap& Image() { return image; }
    bool                 Visited()      const { return visited; }

    FString GetEventSourceName()   const;
    FString GetTypeName()     const;

    // mutators:
    void SetType(int t) { type = t; }
    void SetTime(int t) { time = t; }
    void SetIFF(int t) { team = t; }
    void SetPoints(const FVector& p) { points = p; }
    void SetSource(ECombatEventSource s) { source = s; }
    void SetLocation(const FVector& p) { loc = p; }
    void SetRegion(Text rgn) { region = rgn; }
    void SetTitle(Text t) { title = t; }
    void SetInformation(Text t) { info = t; }
    void SetFilename(Text f) { file = f; }
    void SetImageFile(Text f) { image_file = f; }
    void SetSceneFile(Text f) { scene_file = f; }
    void SetVisited(bool v) { visited = v; }

    // operations:
    void Load();

    // utilities
    static FString GetTypeName(ECombatEventType Type);
    static ECombatEventSource GetSourceFromName(const FString& Name);
    static ECombatEventType GetTypeFromName(const FString& Name);
    static FString GetSourceName(ECombatEventSource Source);

private:
    Campaign* campaign;

    int                  type;
    int                  time;
    int                  team;
    ECombatEventSource   source;
    bool                 visited;

    FVector              loc;
    FVector              points;     

    Text                 region;
    Text                 title;
    Text                 info;
    Text                 file;
    Text                 image_file;
    Text                 scene_file;

    UTexture2D           image;
};
