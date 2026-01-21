/*  Project Starshatter Wars
    Fractal Dev Studios
    Copyright (C) 2025-2026. All Rights Reserved.

    SUBSYSTEM:    Stars.exe
    FILE:         Explosion.cpp
    AUTHOR:       Carlos Bott

    ORIGINAL AUTHOR AND STUDIO:
    John DiCamillo / Destroyer Studios LLC

    OVERVIEW
    ========
    Explosion Sprite animation class
*/

#include "Explosion.h"
#include "QuantumFlash.h"
#include "Particles.h"
#include "Ship.h"
#include "Sim.h"
#include "CameraManager.h"
#include "AudioConfig.h"

#include "SimLight.h"
#include "Sprite.h"
#include "Solid.h"
#include "DataLoader.h"
#include "Game.h"
#include "SimScene.h"
#include "ParseUtil.h"

// Unreal logging (for legacy Print replacement):
#include "Logging/LogMacros.h"

// +--------------------------------------------------------------------+
//
// NOTE:
// - MemDebug.h removed (Unreal build).
// - Bitmap assets replaced with UTexture2D* (forward-declared here to keep includes light).
// - All legacy Print(...) converted to UE_LOG(...).
// - Vec3/Point migrated to FVector; minimal include added.
// +--------------------------------------------------------------------+

#include "Math/Vector.h"

class UTexture2D;

// +--------------------------------------------------------------------+

DEFINE_LOG_CATEGORY_STATIC(LogStarshatterWarsExplos, Log, All);

// +--------------------------------------------------------------------+

const int MAX_EXPLOSION_TYPES = 32;

static float      lifetimes[MAX_EXPLOSION_TYPES];
static float      scales[MAX_EXPLOSION_TYPES];

// Formerly: Bitmap* bitmaps[] / particle_bitmaps[]
static UTexture2D* bitmaps[MAX_EXPLOSION_TYPES];
static UTexture2D* particle_bitmaps[MAX_EXPLOSION_TYPES];

static int        part_frames[MAX_EXPLOSION_TYPES];
static int        lengths[MAX_EXPLOSION_TYPES];
static float      light_levels[MAX_EXPLOSION_TYPES];
static float      light_decays[MAX_EXPLOSION_TYPES];
static Color      light_colors[MAX_EXPLOSION_TYPES];
static int        num_parts[MAX_EXPLOSION_TYPES];
static int        part_types[MAX_EXPLOSION_TYPES];
static float      part_speeds[MAX_EXPLOSION_TYPES];
static float      part_drags[MAX_EXPLOSION_TYPES];
static float      part_scales[MAX_EXPLOSION_TYPES];
static float      part_blooms[MAX_EXPLOSION_TYPES];
static float      part_rates[MAX_EXPLOSION_TYPES];
static float      part_decays[MAX_EXPLOSION_TYPES];
static bool       part_trails[MAX_EXPLOSION_TYPES];
static int        part_alphas[MAX_EXPLOSION_TYPES];
static Sound* sounds[MAX_EXPLOSION_TYPES];
static bool       recycles[MAX_EXPLOSION_TYPES];

// +--------------------------------------------------------------------+

Explosion::Explosion(int t, const FVector& pos, const FVector& vel,
    float exp_scale, float part_scale,
    SimRegion* rgn, SimObject* src)
    : SimObject("Explosion", t), type(t), particles(0), source(src)
{
    Observe(source);

    MoveTo(pos);
    velocity = vel;
    drag = 0.3f;
    rep = 0;
    light = 0;
    life = 0;

    if (type == QUANTUM_FLASH) {
        life = 1.1;

        QuantumFlash* q = new QuantumFlash();
        rep = q;

        light = new Light(1e9, 0.66f);
        light->SetColor(Color(180, 200, 255));
    }

    else if (type >= 0 && type < MAX_EXPLOSION_TYPES) {
        life = lifetimes[type];

        if (source) {
            Matrix src_orient = source->Cam().Orientation();
            src_orient.Transpose();
            mount_rel = (pos - source->Location()) * src_orient;
        }

        if (lengths[type] > 0) {
            int repeat = (lengths[type] == 1);

            // Sprite API in this port is assumed to accept UTexture2D*.
            // If your Sprite expects a Starshatter Bitmap* wrapper, adapt Sprite/Loader accordingly.
            Sprite* s = new Sprite(bitmaps[type], lengths[type], repeat);
            s->Scale(exp_scale * scales[type]);
            s->SetAngle(PI * rand() / 16384.0);
            s->SetLuminous(true);
            rep = s;
        }

        if (light_levels[type] > 0) {
            light = new SimLight(light_levels[type], light_decays[type]);
            light->SetColor(light_colors[type]);
        }

        if (num_parts[type] > 0) {
            // Particles API in this port is assumed to accept UTexture2D*.
            particles = new Particles(particle_bitmaps[type],
                num_parts[type],
                pos,
                FVector(0.0f, 0.0f, 0.0f),
                part_speeds[type] * part_scale,
                part_drags[type],
                part_scales[type] * part_scale,
                part_blooms[type] * part_scale,
                part_decays[type],
                part_rates[type],
                recycles[type],
                part_trails[type],
                (rgn && rgn->IsAirSpace()),
                part_alphas[type],
                part_frames[type]);
        }
    }

    if (rep)
        rep->MoveTo(pos);

    if (light)
        light->MoveTo(pos);
}

// +--------------------------------------------------------------------+

Explosion::~Explosion()
{
    GRAPHIC_DESTROY(particles);
}

// +--------------------------------------------------------------------+

bool
Explosion::Update(SimObject* obj)
{
    if (obj == source) {
        source = 0;

        if (life < 0 || life > 4)
            life = 4;

        if (particles)
            particles->StopEmitting();
    }

    return SimObserver::Update(obj);
}

const char*
Explosion::GetObserverName() const
{
    static char name[128];

    if (source)
        sprintf_s(name, "Explosion(%s)", source->Name());
    else
        sprintf_s(name, "Explosion");

    return name;
}

// +--------------------------------------------------------------------+

void
Explosion::Initialize()
{
    static int initialized = 0;
    if (initialized) return;

    ZeroMemory(lifetimes, sizeof(lifetimes));
    ZeroMemory(scales, sizeof(scales));
    ZeroMemory(bitmaps, sizeof(bitmaps));
    ZeroMemory(particle_bitmaps, sizeof(particle_bitmaps));
    ZeroMemory(lengths, sizeof(lengths));
    ZeroMemory(light_levels, sizeof(light_levels));
    ZeroMemory(light_decays, sizeof(light_decays));
    ZeroMemory(light_colors, sizeof(light_colors));
    ZeroMemory(num_parts, sizeof(num_parts));
    ZeroMemory(part_types, sizeof(part_types));
    ZeroMemory(part_speeds, sizeof(part_speeds));
    ZeroMemory(part_drags, sizeof(part_drags));
    ZeroMemory(part_scales, sizeof(part_scales));
    ZeroMemory(part_blooms, sizeof(part_blooms));
    ZeroMemory(part_decays, sizeof(part_decays));
    ZeroMemory(part_rates, sizeof(part_rates));
    ZeroMemory(part_trails, sizeof(part_trails));
    ZeroMemory(part_alphas, sizeof(part_alphas));
    ZeroMemory(sounds, sizeof(sounds));
    ZeroMemory(recycles, sizeof(recycles));

    const char* filename = "Explosions.def";
    UE_LOG(LogStarshatterWarsExplos, Log, TEXT("Loading Explosion Defs '%hs'"), filename);

    // Load Design File:
    DataLoader* loader = DataLoader::GetLoader();
    BYTE* block = 0;

    loader->SetDataPath("Explosions/");
    int blocklen = loader->LoadBuffer(filename, block, true);
    Parser parser(new BlockReader((const char*)block, blocklen));
    Term* term = parser.ParseTerm();

    if (!term) {
        UE_LOG(LogStarshatterWarsExplos, Error, TEXT("ERROR: could notxparse '%hs'"), filename);
        return;
    }
    else {
        TermText* file_type = term->isText();
        if (!file_type || file_type->value() != "EXPLOSION") {
            UE_LOG(LogStarshatterWarsExplos, Error, TEXT("ERROR: invalid explosion def file '%hs'"), filename);
            delete term;
            loader->ReleaseBuffer(block);
            loader->SetDataPath(0);
            return;
        }
    }

    do {
        delete term;
        term = parser.ParseTerm();

        if (term) {
            TermDef* def = term->isDef();
            if (def) {
                if (def->name()->value() == "explosion") {

                    if (!def->term() || !def->term()->isStruct()) {
                        UE_LOG(LogStarshatterWarsExplos, Warning,
                            TEXT("WARNING: explosion structure missing in '%hs'"), filename);
                    }
                    else {
                        TermStruct* val = def->term()->isStruct();
                        int         type = -1;
                        char        type_name[32];
                        char        bitmap[32];
                        char        particle_bitmap[32];
                        char        sound_file[32];
                        float       lifetime = 0.0f;
                        float       scale = 1.0f;
                        int         length = 0;
                        float       light_level = 0.0f;
                        float       light_decay = 1.0f;
                        Color       light_color;
                        int         num_part = 0;
                        int         part_type = 0;
                        float       part_speed = 0.0f;
                        float       part_drag = 1.0f;
                        float       part_scale = 1.0f;
                        float       part_bloom = 0.0f;
                        float       part_decay = 100.0f;
                        float       part_rate = 1.0f;
                        bool        part_trail = true;
                        bool        continuous = false;
                        int         part_alpha = 4;
                        int         part_nframes = 1;
                        float       sound_min_dist = 1.0f;
                        float       sound_max_dist = 1.0e5f;

                        type_name[0] = 0;
                        bitmap[0] = 0;
                        particle_bitmap[0] = 0;
                        sound_file[0] = 0;

                        for (int i = 0; i < val->elements()->size(); i++) {
                            TermDef* pdef = val->elements()->at(i)->isDef();
                            if (pdef) {

                                if (pdef->name()->value() == "type") {
                                    if (pdef->term()->isText()) {
                                        GetDefText(type_name, pdef, filename);

                                        const char* names[15] = {
                                            "SHIELD_FLASH",
                                            "HULL_FLASH",
                                            "BEAM_FLASH",
                                            "SHOT_BLAST",
                                            "HULL_BURST",
                                            "HULL_FIRE",
                                            "PLASMA_LEAK",
                                            "SMOKE_TRAIL",
                                            "SMALL_FIRE",
                                            "SMALL_EXPLOSION",
                                            "LARGE_EXPLOSION",
                                            "LARGE_BURST",
                                            "NUKE_EXPLOSION",
                                            "QUANTUM_FLASH",
                                            "HYPER_FLASH"
                                        };

                                        for (int n = 0; n < 15; n++)
                                            if (!_stricmp(type_name, names[n]))
                                                type = n + 1;
                                    }
                                    else if (pdef->term()->isNumber()) {
                                        GetDefNumber(type, pdef, filename);

                                        if (type < 0 || type >= MAX_EXPLOSION_TYPES) {
                                            UE_LOG(LogStarshatterWarsExplos, Warning,
                                                TEXT("Warning - invalid explosion type %d ignored"), type);
                                        }
                                    }
                                    else {
                                        UE_LOG(LogStarshatterWarsExplos, Warning,
                                            TEXT("Warning - weird explosion type term encountered"));
                                        pdef->print();
                                    }
                                }

                                else if (pdef->name()->value() == "image" ||
                                    pdef->name()->value() == "bitmap")
                                    GetDefText(bitmap, pdef, filename);

                                else if (pdef->name()->value() == "particles" ||
                                    pdef->name()->value() == "particle_bitmap")
                                    GetDefText(particle_bitmap, pdef, filename);

                                else if (pdef->name()->value() == "sound")
                                    GetDefText(sound_file, pdef, filename);

                                else if (pdef->name()->value() == "lifetime")
                                    GetDefNumber(lifetime, pdef, filename);

                                else if (pdef->name()->value() == "scale")
                                    GetDefNumber(scale, pdef, filename);

                                else if (pdef->name()->value() == "length")
                                    GetDefNumber(length, pdef, filename);

                                else if (pdef->name()->value() == "light_level")
                                    GetDefNumber(light_level, pdef, filename);

                                else if (pdef->name()->value() == "light_decay")
                                    GetDefNumber(light_decay, pdef, filename);

                                else if (pdef->name()->value() == "light_color")
                                    GetDefColor(light_color, pdef, filename);

                                else if (pdef->name()->value() == "num_parts")
                                    GetDefNumber(num_part, pdef, filename);

                                else if (pdef->name()->value() == "part_frames")
                                    GetDefNumber(part_nframes, pdef, filename);

                                else if (pdef->name()->value() == "part_type")
                                    GetDefNumber(part_type, pdef, filename);

                                else if (pdef->name()->value() == "part_speed")
                                    GetDefNumber(part_speed, pdef, filename);

                                else if (pdef->name()->value() == "part_drag")
                                    GetDefNumber(part_drag, pdef, filename);

                                else if (pdef->name()->value() == "part_scale")
                                    GetDefNumber(part_scale, pdef, filename);

                                else if (pdef->name()->value() == "part_bloom")
                                    GetDefNumber(part_bloom, pdef, filename);

                                else if (pdef->name()->value() == "part_decay")
                                    GetDefNumber(part_decay, pdef, filename);

                                else if (pdef->name()->value() == "part_rate")
                                    GetDefNumber(part_rate, pdef, filename);

                                else if (pdef->name()->value() == "part_trail")
                                    GetDefBool(part_trail, pdef, filename);

                                else if (pdef->name()->value() == "part_alpha")
                                    GetDefNumber(part_alpha, pdef, filename);

                                else if (pdef->name()->value() == "continuous")
                                    GetDefBool(continuous, pdef, filename);

                                else if (pdef->name()->value() == "sound_min_dist")
                                    GetDefNumber(sound_min_dist, pdef, filename);

                                else if (pdef->name()->value() == "sound_max_dist")
                                    GetDefNumber(sound_max_dist, pdef, filename);

                                else {
                                    UE_LOG(LogStarshatterWarsExplos, Warning,
                                        TEXT("WARNING: parameter '%hs' ignored in '%hs'"),
                                        pdef->name()->value().data(), filename);
                                }
                            }
                            else {
                                UE_LOG(LogStarshatterWarsExplos, Warning,
                                    TEXT("WARNING: term ignored in '%hs'"), filename);
                                val->elements()->at(i)->print();
                            }
                        }

                        if (type >= 0 && type < MAX_EXPLOSION_TYPES) {
                            if (part_alpha > 2)
                                part_alpha = 4;

                            lengths[type] = length;
                            lifetimes[type] = lifetime;
                            scales[type] = scale;
                            light_levels[type] = light_level;
                            light_decays[type] = light_decay;
                            light_colors[type] = light_color;
                            num_parts[type] = num_part;
                            part_types[type] = part_type;
                            part_speeds[type] = part_speed;
                            part_drags[type] = part_drag;
                            part_scales[type] = part_scale;
                            part_blooms[type] = part_bloom;
                            part_frames[type] = part_nframes;
                            part_decays[type] = part_decay;
                            part_rates[type] = part_rate;
                            part_trails[type] = part_trail;
                            part_alphas[type] = part_alpha;
                            recycles[type] = continuous;

                            // IMPORTANT:
                            // Legacy code loaded PCX bitmaps into Bitmap arrays.
                            // In Unreal, you should resolve these textures via your content pipeline
                            // (e.g., DataLoader returning UTexture2D*, or an indirection asset table).
                            //
                            // This port keeps the variables but leaves loading as a TODO in DataLoader.
                            //
                            // Example expectation:
                            // loader->LoadTextureSequence(bitmap, length, bitmaps[type]);
                            // loader->LoadTextureSequence(particle_bitmap, part_nframes, particle_bitmaps[type]);

                            if (sound_file[0]) {
                                loader->SetDataPath("Sounds/");
                                loader->LoadSound(sound_file, sounds[type], Sound::LOCALIZED | Sound::LOC_3D);
                                loader->SetDataPath("Explosions/");

                                if (sounds[type]) {
                                    sounds[type]->SetMinDistance(sound_min_dist);
                                    sounds[type]->SetMaxDistance(sound_max_dist);
                                }
                            }
                        }
                    }
                }
                else {
                    UE_LOG(LogStarshatterWarsExplos, Warning,
                        TEXT("WARNING: unknown definition '%hs' in '%hs'"),
                        def->name()->value().data(), filename);
                }
            }
            else {
                UE_LOG(LogStarshatterWarsExplos, Warning,
                    TEXT("WARNING: term ignored in '%hs'"), filename);
                term->print();
            }
        }
    } while (term);

    loader->ReleaseBuffer(block);
    loader->SetDataPath(0);
    initialized = 1;
}

// +--------------------------------------------------------------------+

void
Explosion::Close()
{
    // Texture lifetime is expected to be managed by Unreal / DataLoader, notxmanual delete.
    for (int t = 0; t < MAX_EXPLOSION_TYPES; t++) {
        bitmaps[t] = 0;
        particle_bitmaps[t] = 0;

        delete sounds[t];
        sounds[t] = 0;
    }
}

// +--------------------------------------------------------------------+

void
Explosion::ExecFrame(double seconds)
{
    if (source) {
        const Matrix& orientation = source->Cam().Orientation();
        MoveTo((mount_rel * orientation) + source->Location());

        if (rep)       rep->Show();
        if (particles) particles->Show();

        if (source == Sim::GetSim()->GetPlayerShip()) {
            Ship* ship = (Ship*)source;
            if (CameraManager::GetCameraMode() == CameraManager::MODE_COCKPIT &&
                !ship->IsDying()) {
                if (rep)       rep->Hide();
                if (particles) particles->Hide();
            }
        }
    }

    life -= seconds;

    if (rep) {
        rep->MoveTo(Location());

        if (rep->Life() == 0) {
            rep = 0; // about to be GC'd
        }
        else if (rep->IsSprite()) {
            Sprite* s = (Sprite*)rep;
            s->SetAngle(s->Angle() + seconds * 0.5);
        }
        else if (type == QUANTUM_FLASH) {
            QuantumFlash* q = (QuantumFlash*)rep;
            q->SetShade(q->Shade() - seconds);
        }
    }

    if (light) {
        light->Update();

        if (light->Life() == 0)
            light = 0; // about to be GC'd
    }

    if (particles) {
        particles->MoveTo(Location());
        particles->ExecFrame(seconds);
    }

    if (source && source->Life() == 0)
        life = 0;
}

// +--------------------------------------------------------------------+

void
Explosion::Activate(SimScene& scene)
{
    bool filter = false;

    CameraManager* cam_dir = CameraManager::GetInstance();
    if (cam_dir && cam_dir->GetCamera()) {
        if (FVector(cam_dir->GetCamera()->Pos() - Location()).Length() < 100.0)
            filter = true;
    }

    if (rep && !filter)
        scene.AddGraphic(rep);

    if (light)
        scene.AddLight(light);

    if (particles && !filter)
        scene.AddGraphic(particles);

    if (sounds[obj_type]) {
        Sound* sound = sounds[obj_type]->Duplicate();

        // fire and forget:
        if (sound) {
            sound->SetLocation(Location());
            sound->SetVolume(AudioConfig::EfxVolume());
            sound->Play();
        }
    }

    active = true;
}

void
Explosion::Deactivate(SimScene& scene)
{
    SimObject::Deactivate(scene);

    if (particles)
        scene.DelGraphic(particles);
}

// +--------------------------------------------------------------------+
