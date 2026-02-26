/*  Project Starshatter Wars
    Fractal Dev Studios
    Copyright © 2025-2026. All Rights Reserved.

    ORIGINAL AUTHOR AND STUDIO:
    John DiCamillo, Destroyer Studios LLC
    Copyright © 1997-2004. All Rights Reserved.

    SUBSYSTEM:    Stars.exe
    FILE:         Galaxy.cpp
    AUTHOR:       Carlos Bott


    OVERVIEW
    ========
    Galaxy (list of star systems) for a single campaign.
*/

#include "Galaxy.h"
#include "StarSystem.h"
#include "Starshatter.h"

#include "Game.h"
#include "DataLoader.h"
#include "ParseUtil.h"

#include "Math/Vector.h"

#include "Logging/LogMacros.h"

DEFINE_LOG_CATEGORY_STATIC(LogStarshatterWarsGalaxy, Log, All);

static Galaxy* galaxy = nullptr;

// +--------------------------------------------------------------------+

Galaxy::Galaxy(const char* n)
    : name(n), radius(10)
{
}

// +--------------------------------------------------------------------+

Galaxy::~Galaxy()
{
    UE_LOG(LogStarshatterWarsGalaxy, Log, TEXT("DESTROYING GALAXY %s"), ANSI_TO_TCHAR((const char*)name));
    systems.destroy();
    stars.destroy();
}

// +--------------------------------------------------------------------+

void
Galaxy::Initialize()
{
    if (galaxy) {
        delete galaxy;
        galaxy = nullptr;
    }

    galaxy = new Galaxy("Galaxy");
    galaxy->Load();
}

void
Galaxy::Close()
{
    delete galaxy;
    galaxy = nullptr;
}

Galaxy*
Galaxy::GetInstance()
{
    return galaxy;
}

// +--------------------------------------------------------------------+

void
Galaxy::Load()
{
    DataLoader* loader = DataLoader::GetLoader();
    loader->SetDataPath("Galaxy/");

    sprintf_s(filename, "%s.def", (const char*)name);
    Load(filename);

    // load mod galaxies:
    List<Text> mod_galaxies;
    loader->SetDataPath("Mods/Galaxy/");
    loader->ListFiles("*.def", mod_galaxies);

    ListIter<Text> iter = mod_galaxies;
    while (++iter) {
        Text* tname = iter.value();

        if (tname && !tname->contains("/")) {
            loader->SetDataPath("Mods/Galaxy/");
            Load(tname->data());
        }
    }
}

void
Galaxy::Load(const char* in_filename)
{
    UE_LOG(LogStarshatterWarsGalaxy, Log, TEXT("LOADING GALAXY: %s"), ANSI_TO_TCHAR(in_filename));

    BYTE* block = nullptr;
    DataLoader* loader = DataLoader::GetLoader();
    loader->LoadBuffer(in_filename, block, true);

    Parser parser(new BlockReader((const char*)block));

    Term* term = parser.ParseTerm();

    if (!term) {
        UE_LOG(LogStarshatterWarsGalaxy, Warning, TEXT("COULD NOT PARSE '%s'"), ANSI_TO_TCHAR(in_filename));
        loader->ReleaseBuffer(block);
        loader->SetDataPath("");
        return;
    }
    else {
        TermText* file_type = term->isText();
        if (!file_type || file_type->value() != "GALAXY") {
            UE_LOG(LogStarshatterWarsGalaxy, Warning, TEXT("INVALID GALAXY FILE '%s'"), ANSI_TO_TCHAR(in_filename));
            delete term;
            loader->ReleaseBuffer(block);
            loader->SetDataPath("");
            return;
        }
    }

    // parse the galaxy:
    do {
        delete term;
        term = parser.ParseTerm();

        if (term) {
            TermDef* def = term->isDef();
            if (def) {
                if (def->name()->value() == "radius") {
                    GetDefNumber(radius, def, in_filename);
                }

                else if (def->name()->value() == "system") {
                    if (!def->term() || !def->term()->isStruct()) {
                        UE_LOG(LogStarshatterWarsGalaxy, Warning, TEXT("SYSTEM STRUCT MISSING IN '%s'"), ANSI_TO_TCHAR(in_filename));
                    }
                    else {
                        TermStruct* val = def->term()->isStruct();

                        char     sys_name[32];
                        char     classname[32];
                        FVector  sys_loc = FVector::ZeroVector;
                        int      sys_iff = 0;
                        int      star_class = Star::G;

                        sys_name[0] = 0;
                        classname[0] = 0;

                        for (int i = 0; i < val->elements()->size(); i++) {
                            TermDef* pdef = val->elements()->at(i)->isDef();
                            if (pdef) {
                                if (pdef->name()->value() == "name")
                                    GetDefText(sys_name, pdef, in_filename);

                                else if (pdef->name()->value() == "loc")
                                    GetDefVec(sys_loc, pdef, in_filename);

                                else if (pdef->name()->value() == "iff")
                                    GetDefNumber(sys_iff, pdef, in_filename);

                                else if (pdef->name()->value() == "class") {
                                    GetDefText(classname, pdef, in_filename);

                                    switch (classname[0]) {
                                    case 'O':   star_class = Star::O;           break;
                                    case 'B':   star_class = Star::B;           break;
                                    case 'A':   star_class = Star::A;           break;
                                    case 'F':   star_class = Star::F;           break;
                                    case 'G':   star_class = Star::G;           break;
                                    case 'K':   star_class = Star::K;           break;
                                    case 'M':   star_class = Star::M;           break;
                                    case 'R':   star_class = Star::RED_GIANT;   break;
                                    case 'W':   star_class = Star::WHITE_DWARF; break;
                                    case 'Z':   star_class = Star::BLACK_HOLE;  break;
                                    default:    star_class = Star::G;           break;
                                    }
                                }
                            }
                        }

                        if (sys_name[0]) {
                            StarSystem* star_system = new StarSystem(sys_name, sys_loc, sys_iff, star_class);
                            star_system->Load();
                            systems.append(star_system);

                            Star* star = new Star(sys_name, sys_loc, star_class);
                            stars.append(star);
                        }
                    }
                }

                else if (def->name()->value() == "star") {
                    if (!def->term() || !def->term()->isStruct()) {
                        UE_LOG(LogStarshatterWarsGalaxy, Warning, TEXT("STAR STRUCT MISSING IN '%s'"), ANSI_TO_TCHAR(in_filename));
                    }
                    else {
                        TermStruct* val = def->term()->isStruct();

                        char     star_name[32];
                        char     classname[32];
                        FVector  star_loc = FVector::ZeroVector;
                        int      star_class = Star::G;

                        star_name[0] = 0;
                        classname[0] = 0;

                        for (int i = 0; i < val->elements()->size(); i++) {
                            TermDef* pdef = val->elements()->at(i)->isDef();
                            if (pdef) {
                                if (pdef->name()->value() == "name")
                                    GetDefText(star_name, pdef, in_filename);

                                else if (pdef->name()->value() == "loc")
                                    GetDefVec(star_loc, pdef, in_filename);

                                else if (pdef->name()->value() == "class") {
                                    GetDefText(classname, pdef, in_filename);

                                    switch (classname[0]) {
                                    case 'O':   star_class = Star::O;           break;
                                    case 'B':   star_class = Star::B;           break;
                                    case 'A':   star_class = Star::A;           break;
                                    case 'F':   star_class = Star::F;           break;
                                    case 'G':   star_class = Star::G;           break;
                                    case 'K':   star_class = Star::K;           break;
                                    case 'M':   star_class = Star::M;           break;
                                    case 'R':   star_class = Star::RED_GIANT;   break;
                                    case 'W':   star_class = Star::WHITE_DWARF; break;
                                    case 'Z':   star_class = Star::BLACK_HOLE;  break;
                                    default:    star_class = Star::G;           break;
                                    }
                                }
                            }
                        }

                        if (star_name[0]) {
                            Star* star = new Star(star_name, star_loc, star_class);
                            stars.append(star);
                        }
                    }
                }
            }
        }
    } while (term);

    loader->ReleaseBuffer(block);
    loader->SetDataPath("");
}

// +--------------------------------------------------------------------+

void
Galaxy::ExecFrame()
{
    ListIter<StarSystem> sys = systems;
    while (++sys) {
        sys->ExecFrame();
    }
}

// +--------------------------------------------------------------------+

StarSystem*
Galaxy::GetSystem(const char* in_name)
{
    ListIter<StarSystem> sys = systems;
    while (++sys) {
        if (!strcmp(sys->Name(), in_name))
            return sys.value();
    }

    return nullptr;
}

// +--------------------------------------------------------------------+

StarSystem*
Galaxy::FindSystemByRegion(const char* rgn_name)
{
    ListIter<StarSystem> iter = systems;
    while (++iter) {
        StarSystem* sys = iter.value();
        if (sys && sys->FindRegion(rgn_name))
            return sys;
    }

    return nullptr;
}
