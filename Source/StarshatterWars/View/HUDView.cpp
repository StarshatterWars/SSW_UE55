/*  Project Starshatter Wars
	Fractal Dev Studios
	Copyright (C) 2025-2026. All Rights Reserved.

	SUBSYSTEM:    Stars.exe
	FILE:         HUDView.cpp
	AUTHOR:       Carlos Bott

	ORIGINAL AUTHOR AND STUDIO: John DiCamillo / Destroyer Studios LLC


	OVERVIEW
	========
	View class for Heads Up Display
*/

#include "HUDView.h"
#include "CoreMinimal.h"          // UE_LOG, FMemory, basic types
#include "Math/Vector.h"          // FVector

#include "HUDSounds.h"
#include "Ship.h"
#include "SimElement.h"
#include "Computer.h"
#include "Drive.h"
#include "Instruction.h"
#include "NavSystem.h"
#include "Power.h"
#include "Shield.h"
#include "Sensor.h"
#include "SimContact.h"
#include "ShipDesign.h"
#include "SimShot.h"
#include "Drone.h"
#include "Thruster.h"
#include "Weapon.h"
#include "WeaponGroup.h"
#include "FlightDeck.h"
#include "SteerAI.h"
#include "Sim.h"
#include "StarSystem.h"
#include "Starshatter.h"
#include "CameraDirector.h"
#include "MFD.h"
#include "RadioView.h"
#include "FormatUtil.h"
#include "Hoop.h"
#include "QuantumDrive.h"
#include "KeyMap.h"
#include "AudioConfig.h"
#include "PlayerCharacter.h"

#include "NetGame.h"
#include "NetPlayer.h"

#include "Color.h"
#include "CameraView.h"
#include "Screen.h"
#include "DataLoader.h"
#include "SimScene.h"
#include "FontMgr.h"
#include "Graphic.h"
#include "Sprite.h"
#include "Keyboard.h"
#include "Mouse.h"
#include "MouseController.h"
#include "Polygon.h"
#include "Sound.h"
#include "Game.h"
#include "Window.h"

// Unreal logging (local to this translation unit)
DEFINE_LOG_CATEGORY_STATIC(LogHUDView, Log, All);

// NOTE:
// Per porting rules, Bitmap render assets are replaced by UTexture2D*.
// These are forward-declared in headers; here we only use pointers.
class UTexture2D;

// ---------------------------------------------------------------------
// HUD textures (formerly Bitmap)

static UTexture2D* hud_left_air_tex = nullptr;
static UTexture2D* hud_right_air_tex = nullptr;
static UTexture2D* hud_left_fighter_tex = nullptr;
static UTexture2D* hud_right_fighter_tex = nullptr;
static UTexture2D* hud_left_starship_tex = nullptr;
static UTexture2D* hud_right_starship_tex = nullptr;
static UTexture2D* instr_left_tex = nullptr;
static UTexture2D* instr_right_tex = nullptr;
static UTexture2D* warn_left_tex = nullptr;
static UTexture2D* warn_right_tex = nullptr;
static UTexture2D* lead_tex = nullptr;
static UTexture2D* cross_tex = nullptr;
static UTexture2D* cross1_tex = nullptr;
static UTexture2D* cross2_tex = nullptr;
static UTexture2D* cross3_tex = nullptr;
static UTexture2D* cross4_tex = nullptr;
static UTexture2D* fpm_tex = nullptr;
static UTexture2D* hpm_tex = nullptr;
static UTexture2D* pitch_ladder_pos_tex = nullptr;
static UTexture2D* pitch_ladder_neg_tex = nullptr;
static UTexture2D* chase_left_tex = nullptr;
static UTexture2D* chase_right_tex = nullptr;
static UTexture2D* chase_top_tex = nullptr;
static UTexture2D* chase_bottom_tex = nullptr;
static UTexture2D* icon_ship_tex = nullptr;
static UTexture2D* icon_target_tex = nullptr;

// Shades are raw 8-bit buffers (formerly BYTE*)
static uint8* hud_left_shade_air = nullptr;
static uint8* hud_right_shade_air = nullptr;
static uint8* hud_left_shade_fighter = nullptr;
static uint8* hud_right_shade_fighter = nullptr;
static uint8* hud_left_shade_starship = nullptr;
static uint8* hud_right_shade_starship = nullptr;
static uint8* instr_left_shade = nullptr;
static uint8* instr_right_shade = nullptr;
static uint8* warn_left_shade = nullptr;
static uint8* warn_right_shade = nullptr;
static uint8* lead_shade = nullptr;
static uint8* cross_shade = nullptr;
static uint8* cross1_shade = nullptr;
static uint8* cross2_shade = nullptr;
static uint8* cross3_shade = nullptr;
static uint8* cross4_shade = nullptr;
static uint8* fpm_shade = nullptr;
static uint8* hpm_shade = nullptr;
static uint8* pitch_ladder_pos_shade = nullptr;
static uint8* pitch_ladder_neg_shade = nullptr;
static uint8* chase_left_shade = nullptr;
static uint8* chase_right_shade = nullptr;
static uint8* chase_top_shade = nullptr;
static uint8* chase_bottom_shade = nullptr;
static uint8* icon_ship_shade = nullptr;
static uint8* icon_target_shade = nullptr;

static Sprite* hud_left_sprite = nullptr;
static Sprite* hud_right_sprite = nullptr;
static Sprite* fpm_sprite = nullptr;
static Sprite* hpm_sprite = nullptr;
static Sprite* lead_sprite = nullptr;
static Sprite* aim_sprite = nullptr;
static Sprite* tgt1_sprite = nullptr;
static Sprite* tgt2_sprite = nullptr;
static Sprite* tgt3_sprite = nullptr;
static Sprite* tgt4_sprite = nullptr;
static Sprite* chase_sprite = nullptr;
static Sprite* instr_left_sprite = nullptr;
static Sprite* instr_right_sprite = nullptr;
static Sprite* warn_left_sprite = nullptr;
static Sprite* warn_right_sprite = nullptr;
static Sprite* icon_ship_sprite = nullptr;
static Sprite* icon_target_sprite = nullptr;

static Sound* missile_lock_sound = nullptr;

const int NUM_HUD_COLORS = 4;

Color standard_hud_colors[NUM_HUD_COLORS] = {
	Color(130,190,140),  // green
	Color(130,200,220),  // cyan
	Color(250,170, 80),  // orange
	// Color(220,220,100),  // yellow
	Color(16, 16, 16)   // dark gray
};

Color standard_txt_colors[NUM_HUD_COLORS] = {
	Color(150,200,170),  // green w/ green gray
	Color(220,220,180),  // cyan  w/ light yellow
	Color(220,220, 80),  // orange w/ yellow
	// Color(180,200,220),  // yellow w/ white
	Color(32, 32, 32)   // dark gray
};

Color night_vision_colors[NUM_HUD_COLORS] = {
	Color(20, 80, 20),  // green
	Color(30, 80, 80),  // cyan
	Color(80, 80, 20),  // yellow
	// Color(180,200,220),  // not used
	Color(0,  0,  0)   // no night vision
};

static SystemFont* hud_font = nullptr;
static SystemFont* big_font = nullptr;

static bool   mouse_in = false;
static int    mouse_latch = 0;
static int    mouse_index = -1;

static int ship_status = SimSystem::NOMINAL;
static int tgt_status = SimSystem::NOMINAL;

// +--------------------------------------------------------------------+

static enum TXT {
	MAX_CONTACT = 50,

	TXT_CAUTION_TXT = 0,
	TXT_LAST_CAUTION = 23,
	TXT_CAM_ANGLE,
	TXT_CAM_MODE,
	TXT_PAUSED,
	TXT_GEAR_DOWN,

	TXT_HUD_MODE,
	TXT_PRIMARY_WEP,
	TXT_SECONDARY_WEP,
	TXT_DECOY,
	TXT_SHIELD,
	TXT_AUTO,
	TXT_SHOOT,
	TXT_NAV_INDEX,
	TXT_NAV_ACTION,
	TXT_NAV_FORMATION,
	TXT_NAV_SPEED,
	TXT_NAV_ETR,
	TXT_NAV_HOLD,

	TXT_SPEED,
	TXT_RANGE,
	TXT_CLOSING_SPEED,
	TXT_THREAT_WARN,
	TXT_COMPASS,
	TXT_HEADING,
	TXT_PITCH,
	TXT_ALTITUDE,
	TXT_GFORCE,
	TXT_MISSILE_T1,
	TXT_MISSILE_T2,
	TXT_ICON_SHIP_TYPE,
	TXT_ICON_TARGET_TYPE,
	TXT_TARGET_NAME,
	TXT_TARGET_DESIGN,
	TXT_TARGET_SHIELD,
	TXT_TARGET_HULL,
	TXT_TARGET_SUB,
	TXT_TARGET_ETA,

	TXT_MSG_1,
	TXT_MSG_2,
	TXT_MSG_3,
	TXT_MSG_4,
	TXT_MSG_5,
	TXT_MSG_6,

	TXT_NAV_PT,
	TXT_SELF,
	TXT_SELF_NAME,
	TXT_CONTACT_NAME,
	TXT_CONTACT_INFO = TXT_CONTACT_NAME + MAX_CONTACT,
	TXT_LAST = TXT_CONTACT_INFO + MAX_CONTACT,

	TXT_LAST_ACTIVE = TXT_NAV_HOLD,
	TXT_INSTR_PAGE = TXT_CAUTION_TXT + 6,
};

static HUDText hud_text[TXT_LAST];

void
HUDView::DrawHUDText(int index, const char* txt, Rect& rect, int align, int upcase, bool box)
{
	if (index < 0 || index >= TXT_LAST)
		return;

	HUDText& ht = hud_text[index];
	Color    hc = ht.color;

	char txt_buf[256];
	int  n = (int)strlen(txt);

	if (n > 250) n = 250;

	int i;
	for (i = 0; i < n; i++) {
		if (upcase && islower((unsigned char)txt[i]))
			txt_buf[i] = (char)toupper((unsigned char)txt[i]);
		else
			txt_buf[i] = txt[i];
	}

	txt_buf[i] = 0;

	if (box) {
		ht.font->DrawText(txt_buf, n, rect, DT_LEFT | DT_SINGLELINE | DT_CALCRECT);

		if ((align & DT_CENTER) != 0) {
			int cx = width / 2;
			rect.x = cx - rect.w / 2;
		}
	}

	if (!cockpit_hud_texture && rect.Contains(Mouse::X(), Mouse::Y())) {
		mouse_in = true;

		if (index <= TXT_LAST_ACTIVE)
			hc = Color::White;

		if (Mouse::LButton() && !mouse_latch) {
			mouse_latch = 2;
			mouse_index = index;
		}
	}

	if (cockpit_hud_texture &&
		index >= TXT_HUD_MODE &&
		index <= TXT_TARGET_ETA &&
		ht.font != big_font) {

		Sprite* s = hud_sprite[0];

		int cx = (int)s->Location().x;
		int cy = (int)s->Location().y;
		int w2 = s->Width() / 2;
		int h2 = s->Height() / 2;

		Rect txt_rect(rect);
		txt_rect.x -= (cx - w2);
		txt_rect.y -= (cy - h2);

		if (index == TXT_ICON_SHIP_TYPE)
			txt_rect = Rect(0, 500, 128, 12);

		else if (index == TXT_ICON_TARGET_TYPE)
			txt_rect = Rect(128, 500, 128, 12);

		ht.font->SetColor(hc);
		ht.font->DrawText(txt_buf, n, txt_rect, align | DT_SINGLELINE, cockpit_hud_texture);
		ht.hidden = false;
	}
	else {
		ht.font->SetColor(hc);
		ht.font->DrawText(txt_buf, n, rect, align | DT_SINGLELINE);
		ht.rect = rect;
		ht.hidden = false;

		if (box) {
			rect.Inflate(3, 2);
			rect.h--;
			window->DrawRect(rect, hud_color);
		}
	}
}

void
HUDView::HideHUDText(int index)
{
	if (index >= TXT_LAST)
		return;

	hud_text[index].hidden = true;
}

// +--------------------------------------------------------------------+

HUDView* HUDView::hud_view = nullptr;
bool     HUDView::arcade = false;
bool     HUDView::show_fps = false;
int      HUDView::def_color_set = 1;
int      HUDView::gunsight = 1;

// +--------------------------------------------------------------------+

HUDView::HUDView(Window* c)
	: View(c), projector(0), camview(0),
	sim(0), ship(0), target(0), mode(HUD_MODE_TAC),
	tactical(0), overlay(0), cockpit_hud_texture(0),
	threat(0), active_region(0), transition(false), docking(false),
	az_ring(0), az_pointer(0), el_ring(0), el_pointer(0), compass_scale(1),
	show_warn(false), show_inst(false), inst_page(0)
{
	hud_view = this;

	sim = Sim::GetSim();

	if (sim)
		sim->ShowGrid(false);

	int i;

	width = window->Width();
	height = window->Height();
	xcenter = (width / 2.0) - 0.5;
	ycenter = (height / 2.0) + 0.5;

	// NOTE:
	// In the Unreal port, bitmap loading/colorization should be reimplemented
	// to populate UTexture2D* (and optional shade buffers). Calls below remain
	// as legacy placeholders until the loader is migrated.
	PrepareBitmap("HUDleftA.pcx", hud_left_air_tex, hud_left_shade_air);
	PrepareBitmap("HUDrightA.pcx", hud_right_air_tex, hud_right_shade_air);
	PrepareBitmap("HUDleft.pcx", hud_left_fighter_tex, hud_left_shade_fighter);
	PrepareBitmap("HUDright.pcx", hud_right_fighter_tex, hud_right_shade_fighter);
	PrepareBitmap("HUDleft1.pcx", hud_left_starship_tex, hud_left_shade_starship);
	PrepareBitmap("HUDright1.pcx", hud_right_starship_tex, hud_right_shade_starship);
	PrepareBitmap("INSTR_left.pcx", instr_left_tex, instr_left_shade);
	PrepareBitmap("INSTR_right.pcx", instr_right_tex, instr_right_shade);
	PrepareBitmap("CAUTION_left.pcx", warn_left_tex, warn_left_shade);
	PrepareBitmap("CAUTION_right.pcx", warn_right_tex, warn_right_shade);
	PrepareBitmap("hud_icon.pcx", icon_ship_tex, icon_ship_shade);
	PrepareBitmap("hud_icon.pcx", icon_target_tex, icon_target_shade);

	PrepareBitmap("lead.pcx", lead_tex, lead_shade);
	PrepareBitmap("cross.pcx", cross_tex, cross_shade);
	PrepareBitmap("cross1.pcx", cross1_tex, cross1_shade);
	PrepareBitmap("cross2.pcx", cross2_tex, cross2_shade);
	PrepareBitmap("cross3.pcx", cross3_tex, cross3_shade);
	PrepareBitmap("cross4.pcx", cross4_tex, cross4_shade);
	PrepareBitmap("fpm.pcx", fpm_tex, fpm_shade);
	PrepareBitmap("hpm.pcx", hpm_tex, hpm_shade);
	PrepareBitmap("chase_l.pcx", chase_left_tex, chase_left_shade);
	PrepareBitmap("chase_r.pcx", chase_right_tex, chase_right_shade);
	PrepareBitmap("chase_t.pcx", chase_top_tex, chase_top_shade);
	PrepareBitmap("chase_b.pcx", chase_bottom_tex, chase_bottom_shade);
	PrepareBitmap("ladder1.pcx", pitch_ladder_pos_tex,
		pitch_ladder_pos_shade);
	PrepareBitmap("ladder2.pcx", pitch_ladder_neg_tex,
		pitch_ladder_neg_shade);

	// Sprite animation sources now refer to textures (frame binding is handled inside Sprite in the port).
	hud_left_sprite = new Sprite(hud_left_fighter_tex);
	hud_right_sprite = new Sprite(hud_right_fighter_tex);
	instr_left_sprite = new Sprite(instr_left_tex);
	instr_right_sprite = new Sprite(instr_right_tex);
	warn_left_sprite = new Sprite(warn_left_tex);
	warn_right_sprite = new Sprite(warn_right_tex);
	icon_ship_sprite = new Sprite(icon_ship_tex);
	icon_target_sprite = new Sprite(icon_target_tex);
	fpm_sprite = new Sprite(fpm_tex);
	hpm_sprite = new Sprite(hpm_tex);
	lead_sprite = new Sprite(lead_tex);
	aim_sprite = new Sprite(cross_tex);
	tgt1_sprite = new Sprite(cross1_tex);
	tgt2_sprite = new Sprite(cross2_tex);
	tgt3_sprite = new Sprite(cross3_tex);
	tgt4_sprite = new Sprite(cross4_tex);
	chase_sprite = new Sprite(chase_left_tex);

	FMemory::Memzero(hud_sprite, sizeof(hud_sprite));

	hud_sprite[0] = hud_left_sprite;
	hud_sprite[1] = hud_right_sprite;
	hud_sprite[2] = instr_left_sprite;
	hud_sprite[3] = instr_right_sprite;
	hud_sprite[4] = warn_left_sprite;
	hud_sprite[5] = warn_right_sprite;
	hud_sprite[6] = icon_ship_sprite;
	hud_sprite[7] = icon_target_sprite;
	hud_sprite[8] = fpm_sprite;
	hud_sprite[9] = hpm_sprite;
	hud_sprite[10] = lead_sprite;
	hud_sprite[11] = aim_sprite;
	hud_sprite[12] = tgt1_sprite;
	hud_sprite[13] = tgt2_sprite;
	hud_sprite[14] = tgt3_sprite;
	hud_sprite[15] = tgt4_sprite;
	hud_sprite[16] = chase_sprite;

	double pitch_ladder_UV[8] = { 0.125,0.0625, 0.875,0.0625, 0.875,0, 0.125,0 };
	double UV[8];

	for (i = 0; i < 15; i++) {
		pitch_ladder[i] = new Sprite(pitch_ladder_pos_tex);

		FMemory::Memcpy(UV, pitch_ladder_UV, sizeof(UV));
		UV[1] = UV[3] = (pitch_ladder_UV[1] * (i));
		UV[5] = UV[7] = (pitch_ladder_UV[1] * (i + 1));

		pitch_ladder[i]->Reshape(192, 16);
		pitch_ladder[i]->SetTexCoords(UV);
		pitch_ladder[i]->SetBlendMode(2);
		pitch_ladder[i]->Hide();
	}

	// zero mark at i=15
	{
		pitch_ladder[i] = new Sprite(pitch_ladder_pos_tex);

		UV[0] = UV[6] = 0;
		UV[2] = UV[4] = 1;
		UV[1] = UV[3] = (pitch_ladder_UV[1] * (i + 1));
		UV[5] = UV[7] = (pitch_ladder_UV[1] * (i));

		pitch_ladder[i]->Reshape(256, 16);
		pitch_ladder[i]->SetTexCoords(UV);
		pitch_ladder[i]->SetBlendMode(2);
		pitch_ladder[i]->Hide();
	}

	for (i = 16; i < 31; i++) {
		pitch_ladder[i] = new Sprite(pitch_ladder_neg_tex);

		FMemory::Memcpy(UV, pitch_ladder_UV, sizeof(UV));
		UV[1] = UV[3] = (pitch_ladder_UV[1] * (30 - i));
		UV[5] = UV[7] = (pitch_ladder_UV[1] * (30 - i + 1));

		pitch_ladder[i]->Reshape(192, 16);
		pitch_ladder[i]->SetTexCoords(UV);
		pitch_ladder[i]->SetBlendMode(2);
		pitch_ladder[i]->Hide();
	}

	for (i = 0; i < 3; i++)
		mfd[i] = new MFD(window, i);

	mfd[0]->SetRect(Rect(8, height - 136, 128, 128));
	mfd[1]->SetRect(Rect(width - 136, height - 136, 128, 128));
	mfd[2]->SetRect(Rect(8, 8, 128, 128));

	hud_left_sprite->MoveTo(FVector((float)(width / 2 - 128), (float)(height / 2), 1.0f));
	hud_right_sprite->MoveTo(FVector((float)(width / 2 + 128), (float)(height / 2), 1.0f));
	hud_left_sprite->SetBlendMode(2);
	hud_left_sprite->SetFilter(0);
	hud_right_sprite->SetBlendMode(2);
	hud_right_sprite->SetFilter(0);

	instr_left_sprite->MoveTo(FVector((float)(width / 2 - 128), (float)(height - 128), 1.0f));
	instr_right_sprite->MoveTo(FVector((float)(width / 2 + 128), (float)(height - 128), 1.0f));
	instr_left_sprite->SetBlendMode(2);
	instr_left_sprite->SetFilter(0);
	instr_right_sprite->SetBlendMode(2);
	instr_right_sprite->SetFilter(0);

	warn_left_sprite->MoveTo(FVector((float)(width / 2 - 128), (float)(height - 128), 1.0f));
	warn_right_sprite->MoveTo(FVector((float)(width / 2 + 128), (float)(height - 128), 1.0f));
	warn_left_sprite->SetBlendMode(2);
	warn_left_sprite->SetFilter(0);
	warn_right_sprite->SetBlendMode(2);
	warn_right_sprite->SetFilter(0);

	icon_ship_sprite->MoveTo(FVector(184.0f, (float)(height - 72), 1.0f));
	icon_target_sprite->MoveTo(FVector((float)(width - 184), (float)(height - 72), 1.0f));
	icon_ship_sprite->SetBlendMode(2);
	icon_ship_sprite->SetFilter(0);
	icon_target_sprite->SetBlendMode(2);
	icon_target_sprite->SetFilter(0);

	fpm_sprite->MoveTo(FVector((float)(width / 2), (float)(height / 2), 1.0f));
	hpm_sprite->MoveTo(FVector((float)(width / 2), (float)(height / 2), 1.0f));
	lead_sprite->MoveTo(FVector((float)(width / 2), (float)(height / 2), 1.0f));
	aim_sprite->MoveTo(FVector((float)(width / 2), (float)(height / 2), 1.0f));
	tgt1_sprite->MoveTo(FVector((float)(width / 2), (float)(height / 2), 1.0f));
	tgt2_sprite->MoveTo(FVector((float)(width / 2), (float)(height / 2), 1.0f));
	tgt3_sprite->MoveTo(FVector((float)(width / 2), (float)(height / 2), 1.0f));
	tgt4_sprite->MoveTo(FVector((float)(width / 2), (float)(height / 2), 1.0f));

	fpm_sprite->SetBlendMode(2);
	hpm_sprite->SetBlendMode(2);
	lead_sprite->SetBlendMode(2);
	aim_sprite->SetBlendMode(2);
	tgt1_sprite->SetBlendMode(2);
	tgt2_sprite->SetBlendMode(2);
	tgt3_sprite->SetBlendMode(2);
	tgt4_sprite->SetBlendMode(2);
	chase_sprite->SetBlendMode(2);

	fpm_sprite->SetFilter(0);
	hpm_sprite->SetFilter(0);
	lead_sprite->SetFilter(0);
	aim_sprite->SetFilter(0);
	tgt1_sprite->SetFilter(0);
	tgt2_sprite->SetFilter(0);
	tgt3_sprite->SetFilter(0);
	tgt4_sprite->SetFilter(0);
	chase_sprite->SetFilter(0);

	lead_sprite->Hide();
	aim_sprite->Hide();
	tgt1_sprite->Hide();
	tgt2_sprite->Hide();
	tgt3_sprite->Hide();
	tgt4_sprite->Hide();
	chase_sprite->Hide();

	aw = chase_sprite ? (chase_sprite->Width() / 2) : 0;
	ah = chase_sprite ? (chase_sprite->Height() / 2) : 0;

	mfd[0]->SetMode(MFD::MFD_MODE_SHIP);
	mfd[1]->SetMode(MFD::MFD_MODE_FOV);
	mfd[2]->SetMode(MFD::MFD_MODE_GAME);

	hud_font = FontMgr::Find("HUD");
	big_font = FontMgr::Find("GUI");

	for (i = 0; i < TXT_LAST; i++) {
		hud_text[i].font = hud_font;
	}

	hud_text[TXT_THREAT_WARN].font = big_font;
	hud_text[TXT_SHOOT].font = big_font;
	hud_text[TXT_AUTO].font = big_font;

	SetHUDColorSet(def_color_set);
	MFD::SetColor(standard_hud_colors[color]);

	DataLoader* loader = DataLoader::GetLoader();
	loader->SetDataPath("HUD/");

	az_ring = new Solid;
	az_pointer = new Solid;
	el_ring = new Solid;
	el_pointer = new Solid;

	az_ring->Load("CompassRing.mag", compass_scale);
	az_pointer->Load("CompassPointer.mag", compass_scale);
	el_ring->Load("PitchRing.mag", compass_scale);
	el_pointer->Load("CompassPointer.mag", compass_scale);

	loader->SetDataPath("Sounds/");
	loader->LoadSound("MissileLock.wav", missile_lock_sound, Sound::LOOP | Sound::LOCKED);

	loader->SetDataPath(0);

	for (i = 0; i < MAX_MSG; i++)
		msg_time[i] = 0;
}

HUDView::~HUDView()
{
	HideCompass();

	if (missile_lock_sound) {
		missile_lock_sound->Stop();
		missile_lock_sound->Release();
		missile_lock_sound = 0;
	}

	for (int i = 0; i < 3; i++) {
		delete mfd[i];
		mfd[i] = 0;
	}

	for (int i = 0; i < 32; i++) {
		GRAPHIC_DESTROY(hud_sprite[i]);
	}

	delete[] fpm_shade;
	delete[] hpm_shade;
	delete[] lead_shade;
	delete[] cross_shade;
	delete[] cross1_shade;
	delete[] cross2_shade;
	delete[] cross3_shade;
	delete[] cross4_shade;
	delete[] hud_left_shade_air;
	delete[] hud_right_shade_air;
	delete[] hud_left_shade_fighter;
	delete[] hud_right_shade_fighter;
	delete[] hud_left_shade_starship;
	delete[] hud_right_shade_starship;
	delete[] instr_left_shade;
	delete[] instr_right_shade;
	delete[] warn_left_shade;
	delete[] warn_right_shade;
	delete[] icon_ship_shade;
	delete[] icon_target_shade;
	delete[] chase_left_shade;
	delete[] chase_right_shade;
	delete[] chase_top_shade;
	delete[] chase_bottom_shade;
	delete[] pitch_ladder_pos_shade;
	delete[] pitch_ladder_neg_shade;

	delete az_ring;
	delete az_pointer;
	delete el_ring;
	delete el_pointer;

	fpm_shade = nullptr;
	hpm_shade = nullptr;
	cross_shade = nullptr;
	cross1_shade = nullptr;
	cross2_shade = nullptr;
	cross3_shade = nullptr;
	cross4_shade = nullptr;
	hud_left_shade_air = nullptr;
	hud_right_shade_air = nullptr;
	hud_left_shade_fighter = nullptr;
	hud_right_shade_fighter = nullptr;
	hud_left_shade_starship = nullptr;
	hud_right_shade_starship = nullptr;
	instr_left_shade = nullptr;
	instr_right_shade = nullptr;
	warn_left_shade = nullptr;
	warn_right_shade = nullptr;
	icon_ship_shade = nullptr;
	icon_target_shade = nullptr;
	chase_left_shade = nullptr;
	chase_right_shade = nullptr;
	chase_top_shade = nullptr;
	chase_bottom_shade = nullptr;
	pitch_ladder_pos_shade = nullptr;
	pitch_ladder_neg_shade = nullptr;

	az_ring = nullptr;
	az_pointer = nullptr;
	el_ring = nullptr;
	el_pointer = nullptr;

	hud_view = nullptr;
}

void
HUDView::OnWindowMove()
{
	width = window->Width();
	height = window->Height();
	xcenter = (width / 2.0) - 0.5;
	ycenter = (height / 2.0) + 0.5;

	mfd[0]->SetRect(Rect(8, height - 136, 128, 128));
	mfd[1]->SetRect(Rect(width - 136, height - 136, 128, 128));
	mfd[2]->SetRect(Rect(8, 8, 128, 128));

	hud_left_sprite->MoveTo(FVector((float)(width / 2 - 128), (float)(height / 2), 1.0f));
	hud_right_sprite->MoveTo(FVector((float)(width / 2 + 128), (float)(height / 2), 1.0f));

	instr_left_sprite->MoveTo(FVector((float)(width / 2 - 128), (float)(height - 128), 1.0f));
	instr_right_sprite->MoveTo(FVector((float)(width / 2 + 128), (float)(height - 128), 1.0f));
	warn_left_sprite->MoveTo(FVector((float)(width / 2 - 128), (float)(height - 128), 1.0f));
	warn_right_sprite->MoveTo(FVector((float)(width / 2 + 128), (float)(height - 128), 1.0f));
	icon_ship_sprite->MoveTo(FVector(184.0f, (float)(height - 72), 1.0f));
	icon_target_sprite->MoveTo(FVector((float)(width - 184), (float)(height - 72), 1.0f));

	for (int i = 0; i < TXT_LAST; i++) {
		hud_text[i].font = hud_font;
		hud_text[i].color = standard_txt_colors[color];
	}

	if (big_font) {
		hud_text[TXT_THREAT_WARN].font = big_font;
		hud_text[TXT_SHOOT].font = big_font;
		hud_text[TXT_AUTO].font = big_font;
	}

	MFD::SetColor(standard_hud_colors[color]);

	// cached, if needed:
	// int cx = width/2;
	// int cy = height/2;
}

// +--------------------------------------------------------------------+

void
HUDView::SetTacticalMode(int mode_in)
{
	if (tactical != mode_in) {
		tactical = mode_in;

		if (tactical) {
			hud_left_sprite->Hide();
			hud_right_sprite->Hide();

			for (int i = 0; i < 31; i++)
				pitch_ladder[i]->Hide();
		}
		else if (Game::MaxTexSize() > 128) {
			hud_left_sprite->Show();
			hud_right_sprite->Show();
		}
	}
}

void
HUDView::SetOverlayMode(int mode_in)
{
	if (overlay != mode_in) {
		overlay = mode_in;
	}
}

// +--------------------------------------------------------------------+

bool
HUDView::Update(SimObject* obj)
{
	if (obj == ship) {
		if (target)
			SetTarget(0);

		ship = 0;

		for (int i = 0; i < 3; i++)
			mfd[i]->SetShip(ship);
	}

	if (obj == target) {
		target = 0;
		PrepareBitmap("hud_icon.pcx", icon_target_tex, icon_target_shade);
		ColorizeBitmap(icon_target_tex, icon_target_shade, txt_color);
	}

	return SimObserver::Update(obj);
}

const char*
HUDView::GetObserverName() const
{
	return "HUDView";
}

// +--------------------------------------------------------------------+

void
HUDView::UseCameraView(CameraView* v)
{
	if (v && camview != v) {
		camview = v;

		for (int i = 0; i < 3; i++)
			mfd[i]->UseCameraView(camview);

		projector = camview->GetProjector();
	}
}

// +--------------------------------------------------------------------+

Color
HUDView::MarkerColor(SimContact* contact)
{
	Color c(80, 80, 80);

	if (contact) {
		Sim* sim_local = Sim::GetSim();
		Ship* ship_local = sim_local->GetPlayerShip();

		int c_iff = contact->GetIFF(ship_local);

		c = Ship::IFFColor(c_iff) * contact->Age();

		if (contact->GetShot() && contact->Threat(ship_local)) {
			if ((Game::RealTime() / 500) & 1)
				c = c * 2;
			else
				c = c * 0.5;
		}
	}

	return c;
}

// +--------------------------------------------------------------------+

void
HUDView::DrawContactMarkers()
{
	threat = 0;

	for (int i = 0; i < MAX_CONTACT; i++) {
		HideHUDText(TXT_CONTACT_NAME + i);
		HideHUDText(TXT_CONTACT_INFO + i);
	}

	if (!ship)
		return;

	int               index = 0;
	ListIter<SimContact> contact = ship->ContactList();

	// draw own sensor contacts:
	while (++contact) {
		SimContact* c = contact.value();

		// draw track ladder:
		if (c->TrackLength() > 0 && c->GetShip() != ship) {
			DrawTrack(c);
		}

		DrawContact(c, index++);
	}

	Color c = ship->MarkerColor();

	// draw own ship track ladder:
	if (CameraDirector::GetCameraMode() == CameraDirector::MODE_ORBIT && ship->TrackLength() > 0) {
		int ctl = ship->TrackLength();

		FVector t1 = ship->Location();
		FVector t2 = ship->TrackPoint(0);

		if (t1 != t2)
			DrawTrackSegment(t1, t2, c);

		for (int i = 0; i < ctl - 1; i++) {
			t1 = ship->TrackPoint(i);
			t2 = ship->TrackPoint(i + 1);

			if (t1 != t2)
				DrawTrackSegment(t1, t2, c * ((double)(ctl - i) / (double)ctl));
		}
	}

	// draw own ship marker:
	FVector mark_pt = ship->Location();
	projector->Transform(mark_pt);

	// clip:
	if (CameraDirector::GetCameraMode() == CameraDirector::MODE_ORBIT && mark_pt.Z > 1.0) {
		projector->Project(mark_pt);

		int x = (int)mark_pt.X;
		int y = (int)mark_pt.Y;

		if (x > 4 && x < width - 4 &&
			y > 4 && y < height - 4) {

			DrawDiamond(x, y, 5, c);

			if (tactical) {
				Rect self_rect(x + 8, y - 4, 200, 12);
				DrawHUDText(TXT_SELF, ship->Name(), self_rect, DT_LEFT, HUD_MIXED_CASE);

				if (NetGame::GetInstance()) {
					PlayerCharacter* p = PlayerCharacter::GetCurrentPlayer();
					if (p) {
						Rect net_name_rect(x + 8, y + 6, 120, 12);
						DrawHUDText(TXT_SELF_NAME, p->Name(), net_name_rect, DT_LEFT, HUD_MIXED_CASE);
					}
				}
			}
		}
	}

	// draw life bars on targeted ship:
	if (target && target->Type() == SimObject::SIM_SHIP && target->Rep()) {
		Ship* tgt_ship = (Ship*)target;
		if (tgt_ship == nullptr) {
			UE_LOG(LogHUDView, Warning, TEXT("Null pointer in HUDView::DrawContactMarkers(). Please investigate."));
			return;
		}
		Graphic* g = tgt_ship->Rep();
		Rect     r = g->ScreenRect();

		FVector tgt_loc = tgt_ship ? tgt_ship->Location() : FVector::ZeroVector;

		projector->Transform(tgt_loc);

		// clip:
		if (tgt_loc.Z > 1.0) {
			projector->Project(tgt_loc);

			int x = (int)tgt_loc.X;
			int y = r.y;

			if (y >= 2000)
				y = (int)tgt_loc.Y;

			if (x > 4 && x < width - 4 &&
				y > 4 && y < height - 4) {

				const int BAR_LENGTH = 40;

				// life bars:
				int sx = x - BAR_LENGTH / 2;
				int sy = y - 8;

				double hull_strength = tgt_ship->Integrity() / tgt_ship->Design()->integrity;

				int hw = (int)(BAR_LENGTH * hull_strength);
				int sw = (int)(BAR_LENGTH * (tgt_ship->ShieldStrength() / 100.0));

				SimSystem::STATUS s = SimSystem::NOMINAL;

				if (hull_strength < 0.30)        s = SimSystem::CRITICAL;
				else if (hull_strength < 0.60)   s = SimSystem::DEGRADED;

				Color hc = GetStatusColor(s);
				Color sc = hud_color;

				window->FillRect(sx, sy, sx + hw, sy + 1, hc);
				window->FillRect(sx, sy + 3, sx + sw, sy + 4, sc);
			}
		}
	}
}

// +--------------------------------------------------------------------+

void
HUDView::DrawContact(SimContact* contact, int index)
{
	if (index >= MAX_CONTACT) return;

	Color  c = MarkerColor(contact);
	int    c_iff = contact->GetIFF(ship);
	Ship* c_ship = contact->GetShip();
	SimShot* c_shot = contact->GetShot();
	FVector mark_pt = contact->Location();
	double distance = 0;

	if ((!c_ship && !c_shot) || c_ship == ship)
		return;

	if (c_ship && c_ship->GetFlightPhase() < Ship::ACTIVE)
		return;

	if (c_ship) {
		mark_pt = c_ship->Location();

		if (c_ship->IsGroundUnit())
			mark_pt += FVector(0, 150, 0);
	}
	else {
		mark_pt = c_shot->Location();
	}

	projector->Transform(mark_pt);

	// clip:
	if (mark_pt.Z > 1.0) {
		distance = mark_pt.Size();

		projector->Project(mark_pt);

		int x = (int)mark_pt.X;
		int y = (int)mark_pt.Y;

		if (x > 4 && x < width - 4 &&
			y > 4 && y < height - 4) {

			DrawDiamond(x, y, 3, c);

			if (contact->Threat(ship)) {
				if (c_ship) {
					window->DrawEllipse(x - 6, y - 6, x + 6, y + 6, c);
				}
				else {
					DrawDiamond(x, y, 7, c);
				}
			}

			bool name_crowded = false;

			if (x < width - 8) {
				char code = *(Game::GetText("HUDView.symbol.fighter").data());

				if (c_ship) {
					if (c_ship->Class() > Ship::LCA)
						code = *(Game::GetText("HUDView.symbol.starship").data());
				}
				else if (c_shot) {
					code = *(Game::GetText("HUDView.symbol.torpedo").data());
				}

				Sensor* sensor = ship->GetSensor();
				double  limit = 75e3;

				if (sensor)
					limit = sensor->GetBeamRange();

				double  range = contact->Range(ship, limit);

				char contact_buf[256];
				Rect contact_rect(x + 8, y - 4, 120, 12);

				if (range == 0) {
					sprintf_s(contact_buf, "%c *", code);
				}
				else {
					bool mega = false;

					if (range > 999e3) {
						range /= 1e6;
						mega = true;
					}
					else if (range < 1e3)
						range = 1;
					else
						range /= 1000;

					if (arcade) {
						if (c_ship)
							strcpy_s(contact_buf, c_ship->Name());
						else if (!mega)
							sprintf_s(contact_buf, "%c %d", code, (int)range);
						else
							sprintf_s(contact_buf, "%c %.1f M", code, range);
					}
					else {
						char  closing = '+';
						FVector delta_v = FVector::ZeroVector;

						if (c_ship)
							delta_v = ship->Velocity() - c_ship->Velocity();
						else if (c_shot)
							delta_v = ship->Velocity() - c_shot->Velocity();

						if (FVector::DotProduct(delta_v, ship->Velocity()) < 0)    // losing ground
							closing = '-';

						if (!mega)
							sprintf_s(contact_buf, "%c %d%c", code, (int)range, closing);
						else
							sprintf_s(contact_buf, "%c %.1f M", code, range);
					}
				}

				if (!IsNameCrowded(x, y)) {
					DrawHUDText(TXT_CONTACT_INFO + index, contact_buf, contact_rect, DT_LEFT, HUD_MIXED_CASE);

					if (c_shot || (c_ship && (c_ship->IsDropship() || c_ship->IsStatic())))
						name_crowded = distance > 50e3;
				}
				else {
					name_crowded = true;
				}
			}

			bool name_drawn = false;
			if (NetGame::GetInstance() && c_ship) {
				NetPlayer* netp = NetGame::GetInstance()->FindPlayerByObjID(c_ship->GetObjID());
				if (netp && strcmp(netp->Name(), "Server A.I. Ship")) {
					Rect contact_rect(x + 8, y + 6, 120, 12);
					DrawHUDText(TXT_CONTACT_NAME + index, netp->Name(), contact_rect, DT_LEFT, HUD_MIXED_CASE);
					name_drawn = true;
				}
			}

			if (!name_drawn && !name_crowded && c_ship && c_iff < 10 && !arcade) {
				Rect contact_rect(x + 8, y + 6, 120, 12);
				DrawHUDText(TXT_CONTACT_NAME + index, c_ship->Name(), contact_rect, DT_LEFT, HUD_MIXED_CASE);
			}
		}
	}

	if (contact->Threat(ship) && !ship->IsStarship()) {
		if (threat < 1 && c_ship && !c_ship->IsStarship())
			threat = 1;

		if (c_shot)
			threat = 2;
	}
}

// +--------------------------------------------------------------------+

void
HUDView::DrawTrackSegment(FVector& t1, FVector& t2, Color c)
{
	int x1, y1, x2, y2;

	projector->Transform(t1);
	projector->Transform(t2);

	const double CLIP_Z = 0.1;

	if (t1.Z < CLIP_Z && t2.Z < CLIP_Z)
		return;

	if (t1.Z < CLIP_Z && t2.Z >= CLIP_Z) {
		double dx = t2.X - t1.X;
		double dy = t2.Y - t1.Y;
		double s = (CLIP_Z - t1.Z) / (t2.Z - t1.Z);

		t1.X += (float)(dx * s);
		t1.Y += (float)(dy * s);
		t1.Z = (float)CLIP_Z;
	}

	else if (t2.Z < CLIP_Z && t1.Z >= CLIP_Z) {
		double dx = t1.X - t2.X;
		double dy = t1.Y - t2.Y;
		double s = (CLIP_Z - t2.Z) / (t1.Z - t2.Z);

		t2.X += (float)(dx * s);
		t2.Y += (float)(dy * s);
		t2.Z = (float)CLIP_Z;
	}

	if (t1.Z >= CLIP_Z && t2.Z >= CLIP_Z) {
		projector->Project(t1, false);
		projector->Project(t2, false);

		x1 = (int)t1.X;
		y1 = (int)t1.Y;
		x2 = (int)t2.X;
		y2 = (int)t2.Y;

		if (window->ClipLine(x1, y1, x2, y2))
			window->DrawLine(x1, y1, x2, y2, c);
	}
}

void
HUDView::DrawTrack(Contact* contact)
{
	Ship* c_ship = contact->GetShip();

	if (c_ship && c_ship->GetFlightPhase() < Ship::ACTIVE)
		return;

	int   ctl = contact->TrackLength();
	Color c = MarkerColor(contact);

	FVector t1 = contact->Location();
	FVector t2 = contact->TrackPoint(0);

	if (t1 != t2)
		DrawTrackSegment(t1, t2, c);

	for (int i = 0; i < ctl - 1; i++) {
		t1 = contact->TrackPoint(i);
		t2 = contact->TrackPoint(i + 1);

		if (t1 != t2)
			DrawTrackSegment(t1, t2, c * ((double)(ctl - i) / (double)ctl));
	}
}

// +--------------------------------------------------------------------+

void
HUDView::Message(const char* fmt, ...)
{
	if (!fmt)
		return;

	char msg[512];

	// Legacy varargs formatting retained:
	vsprintf(msg, fmt, (char*)(&fmt + 1));

	char* newline = strchr(msg, '\n');
	if (newline)
		*newline = 0;

	UE_LOG(LogHUDView, Log, TEXT("%hs"), msg);

	if (hud_view) {
		int index = -1;

		for (int i = 0; i < MAX_MSG; i++) {
			if (hud_view->msg_time[i] <= 0) {
				index = i;
				break;
			}
		}

		// no space; advance pipeline:
		if (index < 0) {
			for (int i = 0; i < MAX_MSG - 1; i++) {
				hud_view->msg_text[i] = hud_view->msg_text[i + 1];
				hud_view->msg_time[i] = hud_view->msg_time[i + 1];
			}

			index = MAX_MSG - 1;
		}

		hud_view->msg_text[index] = msg;
		hud_view->msg_time[index] = 10;
	}
}

void
HUDView::ClearMessages()
{
	if (hud_view) {
		for (int i = 0; i < MAX_MSG - 1; i++) {
			hud_view->msg_text[i] = Text();
			hud_view->msg_time[i] = 0;
		}
	}
}

// +--------------------------------------------------------------------+
//
// NOTE: The original paste provided ends mid-function ("void...").
// The remaining HUDView.cpp content must be ported in the same style:
// - Convert Point/Vec3 -> FVector
// - Replace Print debugging with UE_LOG(LogHUDView, ...)
// - Replace ZeroMemory/CopyMemory -> FMemory
// - Replace Bitmap usage -> UTexture2D* (and rework loader/colorize accordingly)
//
// +--------------------------------------------------------------------+

void
HUDView::PrepareBitmap(const char* name, UTexture2D*& texture, uint8*& shades)
{
	delete[] shades;
	shades = nullptr;

	DataLoader* loader = DataLoader::GetLoader();
	if (!loader || !name || !*name)
		return;

	loader->SetDataPath("HUD/");

	// Unreal port requirement:
	// Implement a UE-aware loader path that returns a UTexture2D* for a named asset.
	// Keep the call name consistent with your DataLoader migration strategy.
	const int loaded = loader->LoadTexture(name, texture, /*bTransparent=*/true);

	loader->SetDataPath(0);

	if (!loaded || !texture)
		return;

	const int32 w = texture->GetSizeX();
	const int32 h = texture->GetSizeY();

	shades = new uint8[w * h];

	// Shade generation requires reading pixel data from the texture.
	// Provide a loader utility that can read back pixels into an array (CPU-side).
	TArray<FColor> pixels;
	if (loader->ReadTexturePixels(texture, pixels) && pixels.Num() == (w * h)) {
		for (int32 i = 0; i < w * h; ++i) {
			// Match original: shade = Red * 0.66
			shades[i] = (uint8)FMath::Clamp<int32>((int32)(pixels[i].R * 0.66f), 0, 255);
		}
	}
	else {
		// Fallback: safe default, avoids undefined behavior downstream.
		FMemory::Memset(shades, 128, w * h);
	}
}

void
HUDView::TransferBitmap(const UTexture2D* src, UTexture2D*& texture, uint8*& shades)
{
	delete[] shades;
	shades = nullptr;

	if (!src || !texture)
		return;

	if (src->GetSizeX() != texture->GetSizeX() || src->GetSizeY() != texture->GetSizeY())
		return;

	DataLoader* loader = DataLoader::GetLoader();
	if (!loader)
		return;

	// Unreal port requirement:
	// Implement a texture copy helper (or re-create texture resource) as needed.
	loader->CopyTexture(src, texture);

	const int32 w = texture->GetSizeX();
	const int32 h = texture->GetSizeY();

	shades = new uint8[w * h];

	TArray<FColor> pixels;
	if (loader->ReadTexturePixels(texture, pixels) && pixels.Num() == (w * h)) {
		for (int32 i = 0; i < w * h; ++i) {
			// Match original: shade = Red * 0.5
			shades[i] = (uint8)FMath::Clamp<int32>((int32)(pixels[i].R * 0.5f), 0, 255);
		}
	}
	else {
		FMemory::Memset(shades, 128, w * h);
	}
}

void
HUDView::ColorizeBitmap(UTexture2D*& texture, const uint8* shades, Color color, bool force_alpha)
{
	if (!texture || !shades)
		return;

	const int32 w = texture->GetSizeX();
	const int32 h = texture->GetSizeY();

	DataLoader* loader = DataLoader::GetLoader();
	if (!loader)
		return;

	// Ensure minimum texture size behavior (ported from legacy MaxTexSize clamp).
	const int max_tex_size = Game::MaxTexSize();
	if (max_tex_size < 128)
		Game::SetMaxTexSize(128);

	TArray<FColor> outPixels;
	outPixels.SetNumUninitialized(w * h);

	// Original behavior:
	// - If cockpit HUD texture exists and !force_alpha: write dimmed color into RGB and opaque alpha
	// - Else: write solid color with alpha from shades
	if (hud_view && hud_view->cockpit_hud_texture && !force_alpha) {
		for (int32 i = 0; i < w * h; ++i) {
			const uint8 s = shades[i];

			if (s) {
				// color.dim(...) is Starshatter's function; preserve semantics by using it,
				// then convert into FColor.
				const Color dimmed = color.dim((double)s / 200.0);
				outPixels[i] = FColor(
					(uint8)FMath::Clamp<int32>(dimmed.Red(), 0, 255),
					(uint8)FMath::Clamp<int32>(dimmed.Green(), 0, 255),
					(uint8)FMath::Clamp<int32>(dimmed.Blue(), 0, 255),
					255);
			}
			else {
				outPixels[i] = FColor(0, 0, 0, 255);
			}
		}
	}
	else {
		const uint8 cr = (uint8)FMath::Clamp<int32>(color.Red(), 0, 255);
		const uint8 cg = (uint8)FMath::Clamp<int32>(color.Green(), 0, 255);
		const uint8 cb = (uint8)FMath::Clamp<int32>(color.Blue(), 0, 255);

		for (int32 i = 0; i < w * h; ++i) {
			outPixels[i] = FColor(cr, cg, cb, shades[i]);
		}
	}

	// Unreal port requirement:
	// Implement write-back to the texture (UpdateResource, mip locking, etc.) in your loader.
	if (!loader->WriteTexturePixels(texture, outPixels)) {
		UE_LOG(LogHUDView, Warning, TEXT("ColorizeBitmap: failed to write pixels for texture '%s'"), *texture->GetName());
	}

	if (max_tex_size < 128)
		Game::SetMaxTexSize(max_tex_size);
}

// +--------------------------------------------------------------------+

void
HUDView::MouseFrame()
{
	MouseController* ctrl = MouseController::GetInstance();
	if (ctrl && ctrl->Active())
		return;

	if (mouse_index >= TXT_CAUTION_TXT && mouse_index <= TXT_LAST_CAUTION) {
		if (show_inst) {
			if (mouse_index == TXT_INSTR_PAGE) {
				if (Mouse::X() > width / 2 + 125)
					CycleInstructions(1);
				else if (Mouse::X() < width / 2 + 65)
					CycleInstructions(-1);
			}
			else {
				show_inst = false;
			}
		}
		else {
			CycleHUDWarn();
		}
		return;
	}

	Starshatter* stars = Starshatter::GetInstance();
	if (mouse_index == TXT_PAUSED && stars)
		stars->Pause(!Game::Paused());

	if (mouse_index == TXT_GEAR_DOWN && ship)
		ship->ToggleGear();

	if (mouse_index == TXT_HUD_MODE) {
		CycleHUDMode();

		if (mode == HUD_MODE_OFF)
			CycleHUDMode();
	}

	if (mouse_index == TXT_PRIMARY_WEP && ship) {
		HUDSounds::PlaySound(HUDSounds::SND_WEP_MODE);
		ship->CyclePrimary();
	}

	if (mouse_index == TXT_SECONDARY_WEP && ship) {
		HUDSounds::PlaySound(HUDSounds::SND_WEP_MODE);
		ship->CycleSecondary();
	}

	if (mouse_index == TXT_DECOY && ship)
		ship->FireDecoy();

	if (mouse_index == TXT_SHIELD && ship) {
		Shield* shield = ship->GetShield();

		if (shield) {
			const double level = shield->GetPowerLevel();

			const Rect& r = hud_text[TXT_SHIELD].rect;
			if (Mouse::X() < r.x + r.w * 0.75)
				shield->SetPowerLevel(level - 10);
			else
				shield->SetPowerLevel(level + 10);

			HUDSounds::PlaySound(HUDSounds::SND_SHIELD_LEVEL);
		}
	}

	if (mouse_index == TXT_AUTO && ship)
		ship->TimeSkip();

	if (mouse_index >= TXT_NAV_INDEX && mouse_index <= TXT_NAV_ETR && ship) {
		ship->SetAutoNav(!ship->IsAutoNavEngaged());
		SetHUDMode(HUD_MODE_TAC);
	}
}

// +--------------------------------------------------------------------+

bool
HUDView::IsMouseLatched()
{
	bool result = mouse_in;

	if (!result) {
		HUDView* hud = HUDView::GetInstance();

		for (int i = 0; i < 3; i++)
			result = result || hud->mfd[i]->IsMouseLatched();
	}

	return result;
}

// +--------------------------------------------------------------------+

bool
HUDView::IsNameCrowded(int x, int y)
{
	for (int i = 0; i < MAX_CONTACT; i++) {
		HUDText& testName = hud_text[TXT_CONTACT_NAME + i];

		if (!testName.hidden) {
			Rect r = testName.rect;

			const int dx = r.x - x;
			const int dy = r.y - y;
			const int d = dx * dx + dy * dy;

			if (d <= 400)
				return true;
		}

		HUDText& testInfo = hud_text[TXT_CONTACT_INFO + i];

		if (!testInfo.hidden) {
			Rect r = testInfo.rect;

			const int dx = r.x - x;
			const int dy = r.y - y;
			const int d = dx * dx + dy * dy;

			if (d <= 400)
				return true;
		}
	}

	return false;
}

void
HUDView::DrawDiamond(int x, int y, int r, Color c)
{
	// Replace Win32 POINT with a small local struct compatible with Window::DrawPoly signature expectations.
	struct PolyPoint
	{
		int x;
		int y;
	};

	PolyPoint diamond[4];

	diamond[0].x = x;
	diamond[0].y = y - r;

	diamond[1].x = x + r;
	diamond[1].y = y;

	diamond[2].x = x;
	diamond[2].y = y + r;

	diamond[3].x = x - r;
	diamond[3].y = y;

	// If Window::DrawPoly still expects POINT*, update it to a Starshatter-native point type
	// (or overload) rather than relying on Win32.
	window->DrawPoly(4, (const void*)diamond, c);
}
