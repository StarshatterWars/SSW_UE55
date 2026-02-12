/*  Project Starsha/*  Project Starshatter Wars
    Fractal Dev Studios
    Copyright (C) 2025-2026. All Rights Reserved.

    SUBSYSTEM:    Stars.exe
    FILE:         Starshatter.cpp
    AUTHOR:       Carlos Bott

    ORIGINAL AUTHOR AND STUDIO:
    John DiCamillo / Destroyer Studios LLC
*/
#include "Starshatter.h"
#include "CoreMinimal.h"
#include "Math/UnrealMathUtility.h"

// UE helpers (for finding a UWorld from legacy/non-UObject code):
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Engine/GameInstance.h"

// Subsystems:
#include "StarshatterPlayerSubsystem.h"

// UI:
#include "MenuScreen.h"
#include "LoadScreen.h"
#include "MissionPlanner.h"
#include "CmpnScreen.h"
#include "CmpLoadDlg.h"

#include "AudioConfig.h"
#include "MusicManager.h"
#include "HUDSounds.h"

// NOTE: PlayerCharacter legacy facade removed:
// #include "PlayerCharacter.h"

#include "SimShot.h"
#include "Drive.h"
#include "LandingGear.h"
#include "Explosion.h"
#include "FlightDeck.h"
#include "NavLight.h"
#include "Debris.h"
#include "SimContact.h"
#include "QuantumDrive.h"
#include "Sensor.h"
#include "Power.h"
#include "SystemDesign.h"
#include "WeaponDesign.h"

#include "Campaign.h"
#include "CampaignSaveGame.h"
#include "CombatRoster.h"
#include "CombatZone.h"
#include "CampaignPlan.h"

#include "Galaxy.h"
#include "StarSystem.h"
#include "Mission.h"
#include "Sim.h"
#include "SimEvent.h"
#include "SimElement.h"
#include "Ship.h"
#include "ShipManager.h"
#include "ShipDesign.h"
#include "HUDView.h"
#include "MFDView.h"
#include "RadioMessage.h"
#include "RadioTraffic.h"
#include "RadioVox.h"
#include "CameraManager.h"
#include "KeyMap.h"

#include "View.h"
#include "GameScreen.h"

#include "QuantumView.h"
#include "QuitView.h"
#include "RadioView.h"
#include "TacticalView.h"
#include "DisplayView.h"

#include "LoadDlg.h"
#include "TacRefDlg.h"

#include "Terrain.h"

#include "ParseUtil.h"
#include "Token.h"

#include "Game.h"
#include "VideoFactory.h"
#include "Screen.h"
#include "ActiveWindow.h"
#include "UIButton.h"
#include "CameraView.h"
#include "ImageView.h"
#include "FadeView.h"
#include "Bitmap.h"
#include "SystemFont.h"
#include "FontManager.h"
#include "Keyboard.h"
#include "Joystick.h"
#include "MouseController.h"
#include "Mouse.h"
#include "EventDispatch.h"
#include "MultiController.h"
#include "DataLoader.h"
#include "Resource.h"
#include "SimUniverse.h"
#include "Video.h"
#include "VideoSettings.h"
#include "UIAudioManager.h"
#include "GameStructs.h"
#include "StarshatterWarsLog.h"

// +--------------------------------------------------------------------+
// World/Subsytem access from legacy singleton code
// +--------------------------------------------------------------------+

static UWorld* SSW_GetAnyGameWorld()
{
    if (!GEngine)
        return nullptr;

    for (const FWorldContext& Ctx : GEngine->GetWorldContexts())
    {
        if (Ctx.WorldType == EWorldType::Game || Ctx.WorldType == EWorldType::PIE)
        {
            return Ctx.World();
        }
    }

    // Fallback: editor world (rarely desirable, but prevents null in edge cases)
    for (const FWorldContext& Ctx : GEngine->GetWorldContexts())
    {
        if (Ctx.WorldType == EWorldType::Editor)
        {
            return Ctx.World();
        }
    }

    return nullptr;
}

static UStarshatterPlayerSubsystem* SSW_GetPlayerSubsystem()
{
    if (UWorld* W = SSW_GetAnyGameWorld())
    {
        return UStarshatterPlayerSubsystem::Get(W);
    }
    return nullptr;
}

// +--------------------------------------------------------------------+

DEFINE_LOG_CATEGORY(LogStarshatterWars);

int            quick_mode = 0;
char           quick_mission_name[64];
Mission* quick_mission = 0;

int            Starshatter::keymap[256];
int            Starshatter::keyalt[256];
Starshatter* Starshatter::instance = 0;

static Mission* current_mission = 0;
static Mission* cutscene_mission = 0;
static double     cutscene_basetime = 0;
static int        cut_efx_volume = 100;
static int        cut_wrn_volume = 100;
static double     time_til_change = 0;
static bool       exit_latch = true;
static bool       show_missions = false;
static bool       use_file_system = false;
static bool       no_splash = false;

enum CHAT_MODES {
    CHAT_BROADCAST = 1,
    CHAT_TEAM = 2,
    CHAT_WING = 3,
    CHAT_UNIT = 4
};

// +--------------------------------------------------------------------+

Starshatter::Starshatter()
    : gamewin(0), menuscreen(0), loadscreen(0), planscreen(0),
    cmpnscreen(0), gamescreen(0), splash(0), splash_index(0),
    input(0), loader(0), cam_dir(0), music_dir(0),
    field_of_view(2), time_mark(0), minutes(0),
    player_ship(0),
    spinning(false), tactical(false), mouse_x(0), mouse_y(0),
    game_mode(EGameMode::MENU), mouse_input(0),
    terminal(0), verdana(0), limerick18(0), limerick12(0),
    HUDfont(0), GUIfont(0), GUI_small_font(0), title_font(0),
    ocrb(0), req_change_video(0), video_changed(0),
    lens_flare(true), corona(true), nebula(true), dust(0),
    load_step(0), load_progress(0),
    chat_mode(0), exit_time(1.2), cutscene(0)
{
    if (!instance)
        instance = this;

    app_name = "Starshatter Wars";
    title_text = "Starshatter Wars";

    gamma = 128; // default - flat gamma ramp

    if (!DataLoader::GetLoader())
        DataLoader::Initialize();

    LoadVideoConfig("video.cfg");

    // create the fonts
    loader->SetDataPath("Fonts/");

    HUDfont = new  SystemFont("HUDfont");
    FontManager::Register("HUD", HUDfont);

    GUIfont = new  SystemFont("GUIfont");
    FontManager::Register("GUI", GUIfont);

    GUI_small_font = new  SystemFont("GUIsmall");
    FontManager::Register("GUIsmall", GUI_small_font);

    limerick12 = new  SystemFont("Limerick12");
    limerick18 = new  SystemFont("Limerick18");
    terminal = new  SystemFont("Terminal");
    verdana = new  SystemFont("Verdana");
    ocrb = new  SystemFont("OCRB");

    FontManager::Register("Limerick12", limerick12);
    FontManager::Register("Limerick18", limerick18);
    FontManager::Register("Terminal", terminal);
    FontManager::Register("Verdana", verdana);
    FontManager::Register("OCRB", ocrb);

    loader->SetDataPath(0);

    ZeroMemory(keymap, sizeof(keymap));
    ZeroMemory(keyalt, sizeof(keyalt));
}

Starshatter::~Starshatter()
{
    if (video_changed) {
        SaveVideoConfig("video.cfg");
    }

    // Legacy: PlayerCharacter::Save();
    // Unreal port: persist via PlayerSubsystem if available.
    if (Status() <= EXIT)
    {
        if (UStarshatterPlayerSubsystem* SS = SSW_GetPlayerSubsystem())
        {
            SS->SavePlayer(true);
        }
    }

    delete planscreen;
    planscreen = 0;

    music_dir = 0;

    // delete all the ships and stuff
    // BEFORE getting rid of the system
    // and weapons catalogs!
    delete world;
    world = 0; // don't let base class double delete the world

    delete quick_mission;

    AudioConfig::Close();
    HUDSounds::Close();
    MusicManager::Close();

    // Legacy: PlayerCharacter::Close();
    // In UE, subsystems are owned by GI lifecycle.

    Drive::Close();
    LandingGear::Close();
    MFDView::Close();
    Explosion::Close();
    FlightDeck::Close();
    Campaign::Close();
    CombatRoster::Close();
    Galaxy::Close();
    RadioTraffic::Close();
    RadioVox::Close();
    Ship::Close();
    WeaponDesign::Close();
    SystemDesign::Close();
    TacticalView::Close();
    QuantumView::Close();
    QuitView::Close();
    RadioView::Close();

    Mouse::Close();
    EventDispatch::Close();
    FontManager::Close();
    UIButton::Close();
    DataLoader::Close();

    delete ocrb;
    delete limerick12;
    delete limerick18;
    delete verdana;
    delete terminal;
    delete HUDfont;
    delete GUIfont;
    delete GUI_small_font;
    delete input;

    instance = 0;
}

void Starshatter::Exit()
{
    MusicManager::SetMode(MusicManager::NONE);
    SetGameMode(EGameMode::EXIT);
}

// +--------------------------------------------------------------------+

void Starshatter::MapKeys()
{
    int nkeys = keycfg.GetNumKeys();

    if (nkeys > 0) {
        Starshatter::MapKeys(&keycfg, nkeys);
        input->MapKeys(keycfg.GetMapping(), nkeys);
    }
}

void Starshatter::MapKeys(KeyMap* mapping, int nkeys)
{
    for (int i = 0; i < nkeys; i++) {
        KeyMapEntry* k = mapping->GetKeyMap(i);

        if (k->act >= KEY_MAP_FIRST && k->act <= KEY_MAP_LAST)
            MapKey(k->act, k->key, k->alt);
    }
}

void Starshatter::MapKey(int act, int key, int alt)
{
    keymap[act] = key;
    keyalt[act] = alt;

    GetAsyncKeyState(key);
    GetAsyncKeyState(alt);
}

// +--------------------------------------------------------------------+

bool Starshatter::Init()
{
    return Game::Init();
}

// +--------------------------------------------------------------------+

bool Starshatter::InitGame()
{
    if (!Game::InitGame())
        return false;

    // Starshatter RandomInit() replacement:
    // Seed Unreal's global RNG once (non-deterministic by design).
    FMath::RandInit(static_cast<int32>(FPlatformTime::Cycles()));

    AudioConfig::Initialize();

    InitMouse();

    UIButton::Initialize();
    EventDispatch::Create();

    // Legacy: PlayerCharacter::Initialize();
    // UE: load player save if subsystem exists; Boot may already do this.
    if (UStarshatterPlayerSubsystem* SS = SSW_GetPlayerSubsystem())
    {
        // Safe to call multiple times; your subsystem guards loaded state.
        SS->LoadFromBoot(); // or SS->LoadPlayer();
    }

    HUDSounds::Initialize();

    int nkeys = keycfg.LoadKeyMap("key.cfg", 256);

    if (nkeys)
        UE_LOG(LogStarshatterWars, Log, TEXT("Loaded key.cfg"));

    // create the appropriate motion controller and player_ship
    input = new  MultiController;
    Keyboard* k = new  Keyboard;
    input->AddController(k);
    ActivateKeyboardLayout(GetKeyboardLayout(0), 0);

    mouse_input = new  MouseController;
    input->AddController(mouse_input);

    UE_LOG(LogStarshatterWars, Verbose, TEXT("Starshatter::InitGame() create joystick"));
    Joystick* j = new  Joystick;
    j->SetSensitivity(15, 5000);
    input->AddController(j);

    Joystick::EnumerateDevices();

    MapKeys();

    SystemDesign::Initialize("sys.def");
    WeaponDesign::Initialize("wep.def");
    MusicManager::Initialize();

    // if no splashes, we need to initialize the campaign engine now
    if (no_splash) {
        Ship::Initialize();
        Galaxy::Initialize();
        CombatRoster::Initialize();
        Campaign::Initialize();
    }

    // otherwise, the campaign engine will get initialized during the splashes
    else {
        SetupSplash();
    }

    time_mark = Game::GameTime();
    minutes = 0;

    return true;
}

void Starshatter::InitMouse()
{
    if (loader) {
        loader->SetDataPath(0);

        Mouse::Create(screen);
        Mouse::Show(false);
        Mouse::LoadCursor(Mouse::ARROW, "MouseArrow.pcx", Mouse::HOTSPOT_NW);
        Mouse::LoadCursor(Mouse::CROSS, "MouseCross.pcx", Mouse::HOTSPOT_CTR);
        Mouse::LoadCursor(Mouse::DRAG, "MouseDrag.pcx", Mouse::HOTSPOT_NW);
        Mouse::SetCursor(Mouse::ARROW);
    }
}

// +--------------------------------------------------------------------+

void Starshatter::RequestChangeVideo()
{
    req_change_video = true;
}

int Starshatter::GetScreenWidth()
{
    if (video_settings)
        return video_settings->GetWidth();

    return 0;
}

// +--------------------------------------------------------------------+

int Starshatter::GetScreenHeight()
{
    if (video_settings)
        return video_settings->GetHeight();

    return 0;
}

// +--------------------------------------------------------------------+

void Starshatter::StartOrResumeGame()
{
    if (game_mode != EGameMode::MENU && game_mode != EGameMode::CMPN)
        return;

    UStarshatterPlayerSubsystem* SS = SSW_GetPlayerSubsystem();
    if (!SS)
        return;

    const FS_PlayerGameInfo& P = SS->GetPlayerInfo();

    List<Campaign>& list = Campaign::GetAllCampaigns();
    Campaign* c = 0;
    Text            saved = CampaignSaveGame::GetResumeFile();

    // resume saved game?
    if (saved.length()) {
        CampaignSaveGame savegame;
        savegame.Load(saved);
        c = savegame.GetCampaign();
    }

    // start training campaign?
    else if (P.Trained < 255) {
        c = list[0];
        c->Load();
    }

    // start new dynamic campaign sequence?
    else {
        c = list[1];
        c->Load();
    }

    if (c)
        Campaign::SelectCampaign(c->Name());

    Mouse::Show(false);
    SetGameMode(EGameMode::CLOD);
}

// +--------------------------------------------------------------------+

bool Starshatter::UseFileSystem()
{
    return use_file_system;
}

// +--------------------------------------------------------------------+

void Starshatter::OpenTacticalReference()
{
    if (menuscreen && game_mode == EGameMode::MENU) {
        menuscreen->ShowLoadDlg();

        ULoadDlg* load_dlg = menuscreen->GetLoadDlg();

        if (load_dlg && load_dlg->IsShown()) {
            load_activity = Game::GetText("Starshatter.load.tac-ref");
            load_progress = 1;
            catalog_index = 0;
        }
        else {
            menuscreen->ShowTacRefDlg();
        }
    }
}

// +--------------------------------------------------------------------+

void Starshatter::SetGameMode(EGameMode m)
{
    if (game_mode == m)
        return;

    MouseController* mouse_con = MouseController::GetInstance();
    if (mouse_con)
        mouse_con->SetActive(false);

    if (m == EGameMode::CLOD || m == EGameMode::PREP || m == EGameMode::LOAD) {
        load_step = 0;
        load_progress = 0;
        load_activity = Game::GetText("Starshatter.load.general");
        paused = true;
    }

    else if (m == EGameMode::CMPN) {
        load_step = 0;
        load_progress = 100;
        load_activity = Game::GetText("Starshatter.load.complete");
        paused = false;
    }

    else if (m == EGameMode::PLAY) {
        UE_LOG(LogStarshatterWars, Log, TEXT("Starting Game..."));

        player_ship = 0;
        load_progress = 100;
        load_activity = Game::GetText("Starshatter.load.complete");

        if (!world) {
            CreateWorld();
            InstantiateMission();
        }

        if (gamescreen)
            gamescreen->SetFieldOfView(field_of_view);

        HUDView::ClearMessages();
        RadioView::ClearMessages();

        SetTimeCompression(1);
        Pause(false);

        UE_LOG(LogStarshatterWars, Log, TEXT("Stardate: %.1f"), StarSystem::GetBaseTime());
    }

    else if (m == EGameMode::PLAN) {
        if (game_mode == EGameMode::PLAY) {
            UE_LOG(LogStarshatterWars, Log, TEXT("Returning to Plan Mode..."));
            if (soundcard)
                soundcard->StopSoundEffects();

            //StopNetGame();
            Pause(true);
            UE_LOG(LogStarshatterWars, Log, TEXT("Stardate: %.1f"), StarSystem::GetBaseTime());
        }
    }

    else if (m == EGameMode::MENU) {
        UE_LOG(LogStarshatterWars, Log, TEXT("Returning to Main Menu..."));

        if (game_mode == EGameMode::PLAN || game_mode == EGameMode::PLAY) {
            if (soundcard)
                soundcard->StopSoundEffects();

            //StopNetGame();
        }

        paused = true;
    }

    if (m == EGameMode::EXIT) {
        UE_LOG(LogStarshatterWars, Log, TEXT("Shutting Down (Returning to Windows)..."));

        if (game_mode == EGameMode::PLAN || game_mode == EGameMode::PLAY) {
            if (soundcard)
                soundcard->StopSoundEffects();
        }

        UE_LOG(LogStarshatterWars, Log, TEXT("Stardate: %.1f"), StarSystem::GetBaseTime());
        UE_LOG(LogStarshatterWars, Log, TEXT("Bitmap Cache Footprint: %d KB"), Bitmap::CacheMemoryFootprint() / 1024);

        paused = true;
    }

    FlushKeys();
    game_mode = m;
}

// +--------------------------------------------------------------------+

bool Starshatter::ChangeVideo()
{
    bool result = false;

    loader->SetDataPath(0);

    LoadVideoConfig("video2.cfg");

    result = ResetVideo();

    InitMouse();

    req_change_video = false;
    video_changed = true;

    return result;
}

bool Starshatter::ResizeVideo()
{
    if (Game::ResizeVideo()) {
        InitMouse();
        Mouse::Show(true);

        return true;
    }

    return false;
}

// +--------------------------------------------------------------------+

void Starshatter::CreateWorld()
{
    RadioTraffic::Initialize();
    RadioView::Initialize();
    RadioVox::Initialize();
    QuantumView::Initialize();
    //QuitView::Initialize();
    TacticalView::Initialize();

    // create world
    if (!world) {
        Sim* sim = new  Sim(input);
        world = sim;
        UE_LOG(LogStarshatterWars, Log, TEXT("World Created."));
    }

    cam_dir = CameraManager::GetInstance();
}

void Starshatter::InstantiateMission()
{
    current_mission = 0;

    if (Campaign::GetCampaign()) {
        current_mission = Campaign::GetCampaign()->GetMission();
    }

    Sim* sim = (Sim*)world;

    if (sim) {
        bool        dynamic = false;
        Campaign* campaign = Campaign::GetCampaign();

        if (campaign && campaign->IsDynamic())
            dynamic = true;

        sim->UnloadMission();
        sim->LoadMission(current_mission);
        sim->ExecMission();
        sim->SetTestMode(test_mode && !dynamic ? true : false);

        UE_LOG(LogStarshatterWars, Log, TEXT("Mission Instantiated."));
    }
}

// +--------------------------------------------------------------------+

int Starshatter::KeyDown(int action) const
{
    int k = Joystick::KeyDownMap(action) ||
        Keyboard::KeyDownMap(action);

    return k;
}

// +--------------------------------------------------------------------+

bool Starshatter::GameLoop()
{
    cam_dir = CameraManager::GetInstance();

    if (active && paused) {
        // Route Events to EventTargets
        EventDispatch* ed = EventDispatch::GetInstance();
        if (ed)
            ed->Dispatch();

        UpdateWorld();
        GameState();
        UpdateScreen();
        CollectStats();
    }

    Game::GameLoop();
    return false;  // must return false to keep processing
}

// +--------------------------------------------------------------------+

void Starshatter::UpdateWorld()
{
    long   new_time = real_time;
    double delta = new_time - frame_time; // in milliseconds
    seconds = max_frame_length;      // in seconds
    gui_seconds = delta * 0.001;

    if (frame_time == 0)
        gui_seconds = 0;

    if (delta < time_comp * max_frame_length * 1000) {
        seconds = time_comp * delta * 0.001;
    }
    else {
        seconds = time_comp * max_frame_length;
    }

    frame_time = new_time;

    Galaxy* galaxy = Galaxy::GetInstance();
    if (galaxy) galaxy->ExecFrame();

    if (!cutscene_mission) {
        Campaign* campaign = Campaign::GetCampaign();
        if (campaign) campaign->ExecFrame();
    }

    if (paused) {
        if (world)
            world->ExecFrame(0);
    }

    else {
        game_time += (DWORD)(seconds * 1000);

        Drive::StartFrame();

        if (world)
            world->ExecFrame(seconds);
    }

    if (game_mode == EGameMode::PLAY || InCutscene()) {
        if (cam_dir) {
            cam_dir->ExecFrame(gui_seconds);
        }

        Sim* sim = Sim::GetSim();
        SimRegion* rgn = sim ? sim->GetActiveRegion() : 0;

        if (rgn) {
            ListIter<Ship> iter = rgn->GetShips();
            while (++iter) {
                Ship* s = iter.value();
                s->SelectDetail(seconds);
            }
        }
    }
}

// +--------------------------------------------------------------------+

void Starshatter::GameState()
{
    if (splash) {
        static bool quick_splash = false;

        if (GetKey() != 0)
            quick_splash = true;

        if (quick_splash) {
            splash->FadeIn(0);
            splash->StopHold();
            splash->FadeOut(0);
        }

        if (splash->Done()) {
            splash = 0; // this will get deleted along with gamewin
            splash_index++;

            if (gamewin) {
                screen->DelWindow(gamewin);
                delete gamewin;
                gamewin = 0;
            }

            if (splash_index < 2) {
                Ship::Initialize();
                Galaxy::Initialize();
                SetupSplash();
            }
            else {
                CombatRoster::Initialize();
                Campaign::Initialize();
                SetupMenuScreen();
            }

            FlushKeys();
        }
    }

    else if (game_mode == EGameMode::MENU) {
        bool campaign_select = false;

        if (cmpnscreen) {
            campaign_select = cmpnscreen->IsShown();
            cmpnscreen->Hide();
        }

        if (gamescreen)
            gamescreen->Hide();

        if (planscreen)
            planscreen->Hide();

        if (loadscreen)
            loadscreen->Hide();

        if (!menuscreen) {
            SetupMenuScreen();
        }
        else {
            menuscreen->Show();

            if (campaign_select)
                menuscreen->ShowCampaignSelectDlg();
        }

        if (MusicManager::GetInstance() &&
            MusicManager::GetInstance()->GetMode() != MusicManager::CREDITS)
            MusicManager::SetMode(MusicManager::MENU);

        DoMenuScreenFrame();
    }

    else if (game_mode == EGameMode::CLOD ||
        game_mode == EGameMode::PREP ||
        game_mode == EGameMode::LOAD) {
        if (menuscreen)
            menuscreen->Hide();

        if (planscreen)
            planscreen->Hide();

        if (cmpnscreen)
            cmpnscreen->Hide();

        if (!loadscreen)
            SetupLoadScreen();
        else
            loadscreen->Show();

        if (game_mode == EGameMode::CLOD)
            MusicManager::SetMode(MusicManager::MENU);
        else
            MusicManager::SetMode(MusicManager::BRIEFING);

        DoLoadScreenFrame();
    }

    else if (game_mode == EGameMode::PLAN) {
        if (menuscreen)
            menuscreen->Hide();

        if (cmpnscreen)
            menuscreen->Hide();

        if (loadscreen)
            loadscreen->Hide();

        if (gamescreen)
            gamescreen->Hide();

        if (!planscreen)
            SetupPlanScreen();
        else
            planscreen->Show();

        // Legacy: PlayerCharacter award state -> PlayerSubsystem
        if (UStarshatterPlayerSubsystem* SS = SSW_GetPlayerSubsystem())
        {
            if (SS->GetShowAward())
            {
                if (!planscreen->IsAwardShown())
                    planscreen->ShowAwardDlg();
            }
        }

        else if (ShipStats::NumStats()) {
            if (!planscreen->IsDebriefShown()) {
                planscreen->ShowDebriefDlg();
                show_missions = true;
            }
        }

        else {
            if (!planscreen->IsMsnShown() && !planscreen->IsNavShown())
                planscreen->ShowMsnDlg();
        }

        MusicManager::SetMode(MusicManager::BRIEFING);

        DoPlanScreenFrame();
    }

    else if (game_mode == EGameMode::CMPN) {
        if (menuscreen)
            menuscreen->Hide();

        if (planscreen)
            planscreen->Hide();

        if (loadscreen)
            loadscreen->Hide();

        if (gamescreen)
            gamescreen->Hide();

        if (!cmpnscreen)
            SetupCmpnScreen();
        else
            cmpnscreen->Show();

        DoCmpnScreenFrame();
    }

    else if (game_mode == EGameMode::PLAY) {
        if (menuscreen)
            menuscreen->Hide();

        if (cmpnscreen)
            cmpnscreen->Hide();

        if (planscreen)
            planscreen->Hide();

        if (loadscreen)
            loadscreen->Hide();

        if (!gamescreen)
            SetupGameScreen();
        else
            gamescreen->Show();

        DoGameScreenFrame();
    }

    if (game_mode == EGameMode::EXIT) {
        exit_time -= Game::GUITime();

        if (exit_time <= 0)
            Game::Exit();
    }

    if (music_dir)
        music_dir->ExecFrame();
}

// +--------------------------------------------------------------------+

void Starshatter::DoMenuScreenFrame()
{
    if (!Mouse::RButton()) {
        Mouse::SetCursor(Mouse::ARROW);
        Mouse::Show(true);
    }

    if (time_til_change > 0)
        time_til_change -= Game::GUITime();

    if (!menuscreen)
        return;

    if (KeyDown(KEY_EXIT)) {
        if (time_til_change <= 0) {
            time_til_change = 0.5;

            if (!exit_latch && !menuscreen->CloseTopmost()) {
                menuscreen->ShowExitDlg();
            }
        }

        exit_latch = true;
    }
    else {
        exit_latch = false;
    }

    ULoadDlg* load_dlg = menuscreen->GetLoadDlg();

    if (load_dlg && load_dlg->IsShown()) {
        if (catalog_index < ShipDesign::StandardCatalogSize()) {
            ShipDesign::PreloadCatalog(catalog_index++);
            load_activity = Game::GetText("Starshatter.load.tac-ref");

            if (load_progress < 95)
                load_progress++;
        }

        else {
            menuscreen->ShowTacRefDlg();
        }
    }

    if (show_missions) {
        menuscreen->ShowMissionSelectDlg();
        show_missions = false;
    }

    menuscreen->ExecFrame(0);

    if (req_change_video) {
        ChangeVideo();
        SetupMenuScreen();
    }
}

// +--------------------------------------------------------------------+

void Starshatter::DoPlanScreenFrame()
{
    Mouse::SetCursor(Mouse::ARROW);

    if (time_til_change > 0)
        time_til_change -= Game::GUITime();

    if (KeyDown(KEY_EXIT)) {
        if (time_til_change <= 0) {
            time_til_change = 1;

            if (!exit_latch && !planscreen->CloseTopmost()) {
                Campaign* campaign = Campaign::GetCampaign();
                if (campaign && (campaign->IsDynamic() || campaign->IsTraining()))
                    SetGameMode(EGameMode::CMPN);
                else
                    SetGameMode(EGameMode::MENU);
            }
        }

        exit_latch = true;
    }
    else {
        exit_latch = false;
    }

    planscreen->ExecFrame(0);
    show_missions = true;
}

// +--------------------------------------------------------------------+
// NOTE: Everything below this point is unchanged EXCEPT:
// - DoChatMode(): removed PlayerCharacter usage
// - EndCutscene(): restored FlightModel via subsystem or default
// +--------------------------------------------------------------------+

void Starshatter::DoChatMode()
{
    UStarshatterPlayerSubsystem* SS = SSW_GetPlayerSubsystem();

    bool     send_chat = false;
    Text     name = "Player";

    if (player_ship)
        name = player_ship->Name();
    else if (SS)
        name = TCHAR_TO_UTF8(*SS->GetPlayerInfo().Name);

    // If you haven't migrated chat macros yet, skip macro expansion safely.
    if (chat_text.length()) {
        if (chat_text[0] >= '0' && chat_text[0] <= '9') {
            // TODO: route to a Keyboard/Controls settings subsystem once implemented:
            // chat_text = <macro>;
            // send_chat = true;
        }
    }

    if (KeyDown(KEY_EXIT)) {
        SetChatMode(0);
        time_til_change = 0.5;
    }

    else if (send_chat || Keyboard::KeyDown(VK_RETURN)) {
        switch (chat_mode) {
        default:
        case CHAT_BROADCAST:
            break;

        case CHAT_TEAM:
            if (player_ship) {
            }
            break;

        case CHAT_WING:
            if (player_ship) {
                SimElement* elem = player_ship->GetElement();
                if (elem) {
                    for (int i = 1; i <= elem->NumShips(); i++) {
                        Ship* s = elem->GetShip(i);
                        if (s && s != player_ship) {
                        }
                    }
                }
            }
            break;

        case CHAT_UNIT:
        {
            Sim* sim = Sim::GetSim();
            ListIter<Ship> seln = sim->GetSelection();

            while (++seln) {
                Ship* s = seln.value();
                if (s != player_ship) {
                }
            }
        }
        break;
        }

        UE_LOG(LogTemp, Log, TEXT("%s> %s"),
            UTF8_TO_TCHAR(name.data()),
            UTF8_TO_TCHAR(chat_text.data()));

        SetChatMode(0);
        time_til_change = 0.5;
    }

    else {
        int key = 0;
        int shift = 0;

        while (GetKeyPlus(key, shift)) {
            if (key >= 'A' && key <= 'Z') {
                if (shift & 1)
                    chat_text += (char)key;
                else
                    chat_text += (char)tolower(key);
            }
            else {
                switch (key) {
                case VK_BACK:
                    chat_text = chat_text.substring(0, chat_text.length() - 1);
                    break;

                case VK_SPACE: chat_text += ' '; break;
                case '0': if (shift & 1) chat_text += ')'; else chat_text += '0'; break;
                case '1': if (shift & 1) chat_text += '!'; else chat_text += '1'; break;
                case '2': if (shift & 1) chat_text += '@'; else chat_text += '2'; break;
                case '3': if (shift & 1) chat_text += '#'; else chat_text += '3'; break;
                case '4': if (shift & 1) chat_text += '$'; else chat_text += '4'; break;
                case '5': if (shift & 1) chat_text += '%'; else chat_text += '5'; break;
                case '6': if (shift & 1) chat_text += '^'; else chat_text += '6'; break;
                case '7': if (shift & 1) chat_text += '&'; else chat_text += '7'; break;
                case '8': if (shift & 1) chat_text += '*'; else chat_text += '8'; break;
                case '9': if (shift & 1) chat_text += '('; else chat_text += '9'; break;
                case 186: if (shift & 1) chat_text += ':'; else chat_text += ';'; break;
                case 187: if (shift & 1) chat_text += '+'; else chat_text += '='; break;
                case 188: if (shift & 1) chat_text += '<'; else chat_text += ','; break;
                case 189: if (shift & 1) chat_text += '_'; else chat_text += '-'; break;
                case 190: if (shift & 1) chat_text += '>'; else chat_text += '.'; break;
                case 191: if (shift & 1) chat_text += '?'; else chat_text += '/'; break;
                case 192: if (shift & 1) chat_text += '~'; else chat_text += '`'; break;
                case 219: if (shift & 1) chat_text += '{'; else chat_text += '['; break;
                case 221: if (shift & 1) chat_text += '}'; else chat_text += ']'; break;
                case 220: if (shift & 1) chat_text += '|'; else chat_text += '\\'; break;
                case 222: if (shift & 1) chat_text += '"'; else chat_text += '\''; break;
                }
            }
        }
    }
}

// ... keep the rest of your original file the same ...

void Starshatter::EndCutscene()
{
    cutscene--;

    if (cutscene == 0) {
        DisplayView* disp_view = DisplayView::GetInstance();
        if (disp_view)
            disp_view->ClearDisplay();

        HUDView* hud_view = HUDView::GetInstance();
        if (hud_view)
            hud_view->SetHUDMode(EHUDMode::Tactical);

        Sim* sim = Sim::GetSim();
        if (sim->GetPlayerShip())
            sim->GetPlayerShip()->SetControls(sim->GetControls());

        if (cam_dir) {
            cam_dir->SetViewOrbital(0);
            CameraManager::SetRangeLimits(10, CameraManager::GetRangeLimit());
            cam_dir->SetOrbitPoint(PI / 4, PI / 4, 1);
            cam_dir->SetOrbitRates(0, 0, 0);
        }

        AudioConfig* audio_cfg = AudioConfig::GetInstance();

        if (audio_cfg) {
            audio_cfg->SetEfxVolume(cut_efx_volume);
            audio_cfg->SetWrnVolume(cut_wrn_volume);
        }

        // Legacy: Ship::SetFlightModel(p->FlightModel());
        // If you haven't migrated FlightModel into FS_PlayerGameInfo yet, use a stable default:
        if (UStarshatterPlayerSubsystem* SS = SSW_GetPlayerSubsystem())
        {
            // If you DO have a field, replace this with your real value:
            // Ship::SetFlightModel(SS->GetPlayerInfo().FlightModel);
            Ship::SetFlightModel(Ship::FM_ARCADE);
        }
        else
        {
            Ship::SetFlightModel(Ship::FM_ARCADE);
        }
    }
}

Mission*
Starshatter::GetCutsceneMission() const
{
	return cutscene_mission;
}

const char*
Starshatter::GetSubtitles() const
{
	if (cutscene_mission)
		return cutscene_mission->Subtitles();

	return "";
}






