/*  Project Starshatter Wars
	Fractal Dev Studios
	Copyright (c) 2025-2026.

	SUBSYSTEM:    Stars.exe
	FILE:         HUDView.cpp
	AUTHOR:       Carlos Bott

	OVERVIEW
	========
	View class for Heads Up Display

	Original Author: John DiCamillo
	Original Studio: Destroyer Studios LLC (Starshatter 4.5, 1997-2004)
*/

#include "HUDView.h"

#include "View.h"

#include "Engine/Engine.h"
#include "Engine/GameViewportClient.h"
#include "GameFramework/PlayerController.h"
#include "HAL/PlatformTime.h"
#include "Misc/VarArgs.h"
#include "InputCoreTypes.h"

#include "GameStructs.h"

// ---- Starshatter legacy includes (kept as-is; types mapped in .h/.cpp) ----
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
#include "CameraManager.h"
#include "MFDView.h"
#include "RadioView.h"
#include "FormatUtil.h"
#include "Hoop.h"
#include "QuantumDrive.h"
#include "KeyMap.h"
#include "AudioConfig.h"
#include "PlayerCharacter.h"

#include "CameraView.h"
#include "Screen.h"
#include "DataLoader.h"
#include "SimScene.h"
#include "FontManager.h"
#include "Graphic.h"
#include "Sprite.h"
#include "Keyboard.h"
#include "Mouse.h"
#include "MouseController.h"
#include "Polygon.h"
#include "Sound.h"
#include "Game.h"
#include "Window.h"
#include "GameStructs.h"

// ------------------------------------------------------------
// Static legacy globals

static Bitmap hud_left_air;
static Bitmap hud_right_air;
static Bitmap hud_left_fighter;
static Bitmap hud_right_fighter;
static Bitmap hud_left_starship;
static Bitmap hud_right_starship;
static Bitmap instr_left;
static Bitmap instr_right;
static Bitmap warn_left;
static Bitmap warn_right;
static Bitmap lead;
static Bitmap cross;
static Bitmap cross1;
static Bitmap cross2;
static Bitmap cross3;
static Bitmap cross4;
static Bitmap fpm;
static Bitmap hpm;
static Bitmap pitch_ladder_pos;
static Bitmap pitch_ladder_neg;
static Bitmap chase_left;
static Bitmap chase_right;
static Bitmap chase_top;
static Bitmap chase_bottom;
static Bitmap icon_ship;
static Bitmap icon_target;

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

// NOTE: Colors converted to Unreal FColor (used throughout HUDView now):
static FColor standard_hud_colors[NUM_HUD_COLORS] = {
	FColor(130,190,140,255),  // green
	FColor(130,200,220,255),  // cyan
	FColor(250,170, 80,255),  // orange
	FColor(16, 16, 16,255)   // dark gray
};

static FColor standard_txt_colors[NUM_HUD_COLORS] = {
	FColor(150,200,170,255),  // green w/ green gray
	FColor(220,220,180,255),  // cyan  w/ light yellow
	FColor(220,220, 80,255),  // orange w/ yellow
	FColor(32, 32, 32,255)   // dark gray
};

static FColor night_vision_colors[NUM_HUD_COLORS] = {
	FColor(20, 80, 20,255),  // green
	FColor(30, 80, 80,255),  // cyan
	FColor(80, 80, 20,255),  // yellow
	FColor(0,  0,  0,255)   // no night vision
};

static SystemFont* hud_font = nullptr;
static SystemFont* big_font = nullptr;

static bool  mouse_in = false;
static int   mouse_latch = 0;
static int   mouse_index = -1;

// System::STATUS is mapped in the .h to SimSystem::STATUS (per your rules).
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

static inline FColor DimFColor(const FColor& In, float Scalar01)
{
	Scalar01 = FMath::Clamp(Scalar01, 0.0f, 1.0f);

	// Do it in linear to avoid ugly gamma artifacts:
	const FLinearColor Lin = FLinearColor(In) * Scalar01;

	// Preserve incoming alpha:
	FColor Out = Lin.ToFColor(true);
	Out.A = In.A;
	return Out;
}

// +--------------------------------------------------------------------+
// HUD text rendering helpers

void UHUDView::DrawHUDText(int index, const char* txt, Rect& rect, int align, int upcase, bool box)
{
	if (index < 0 || index >= TXT_LAST)
		return;

	HUDText& ht = hud_text[index];
	FColor   hc = ht.color;

	char txt_buf[256];
	int  n = (int)strlen(txt);

	if (n > 250) n = 250;

	int i = 0;
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
			const int cx = width / 2;
			rect.x = cx - rect.w / 2;
		}
	}

	if (!cockpit_hud_texture && rect.Contains(Mouse::X(), Mouse::Y())) {
		mouse_in = true;

		if (index <= TXT_LAST_ACTIVE)
			hc = FColor::White;

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

		const int cx = (int)s->Location().X;
		const int cy = (int)s->Location().Y;
		const int w2 = s->Width() / 2;
		const int h2 = s->Height() / 2;

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

void UHUDView::HideHUDText(int index)
{
	if (index >= TXT_LAST)
		return;

	hud_text[index].hidden = true;
}

// +--------------------------------------------------------------------+
// Static members

UHUDView* UHUDView::hud_view = nullptr;
bool      UHUDView::arcade = false;
bool      UHUDView::show_fps = false;
int       UHUDView::def_color_set = 1;
int       UHUDView::gunsight = 1;

// +--------------------------------------------------------------------+
// Construction / Destruction

UHUDView::UHUDView(Window* c)
	: UView(c)
	, projector(nullptr)
	, camview(nullptr)
	, sim(nullptr)
	, ship(nullptr)
	, target(nullptr)
	, mode((int)HUD_MODE::HUD_MODE_TAC)
	, tactical(0)
	, overlay(0)
	, cockpit_hud_texture(nullptr)
	, threat(0)
	, active_region(nullptr)
	, transition(false)
	, docking(false)
	, az_ring(nullptr)
	, az_pointer(nullptr)
	, el_ring(nullptr)
	, el_pointer(nullptr)
	, compass_scale(1)
	, show_warn(false)
	, show_inst(false)
	, inst_page(0)
{
	hud_view = this;

	sim = Sim::GetSim();
	if (sim)
		sim->ShowGrid(false);

	width = window->Width();
	height = window->Height();
	xcenter = (width / 2.0) - 0.5;
	ycenter = (height / 2.0) + 0.5;

	PrepareBitmap("HUDleftA.pcx", hud_left_air, hud_left_shade_air);
	PrepareBitmap("HUDrightA.pcx", hud_right_air, hud_right_shade_air);
	PrepareBitmap("HUDleft.pcx", hud_left_fighter, hud_left_shade_fighter);
	PrepareBitmap("HUDright.pcx", hud_right_fighter, hud_right_shade_fighter);
	PrepareBitmap("HUDleft1.pcx", hud_left_starship, hud_left_shade_starship);
	PrepareBitmap("HUDright1.pcx", hud_right_starship, hud_right_shade_starship);
	PrepareBitmap("INSTR_left.pcx", instr_left, instr_left_shade);
	PrepareBitmap("INSTR_right.pcx", instr_right, instr_right_shade);
	PrepareBitmap("CAUTION_left.pcx", warn_left, warn_left_shade);
	PrepareBitmap("CAUTION_right.pcx", warn_right, warn_right_shade);
	PrepareBitmap("hud_icon.pcx", icon_ship, icon_ship_shade);
	PrepareBitmap("hud_icon.pcx", icon_target, icon_target_shade);

	PrepareBitmap("lead.pcx", lead, lead_shade);
	PrepareBitmap("cross.pcx", cross, cross_shade);
	PrepareBitmap("cross1.pcx", cross1, cross1_shade);
	PrepareBitmap("cross2.pcx", cross2, cross2_shade);
	PrepareBitmap("cross3.pcx", cross3, cross3_shade);
	PrepareBitmap("cross4.pcx", cross4, cross4_shade);
	PrepareBitmap("fpm.pcx", fpm, fpm_shade);
	PrepareBitmap("hpm.pcx", hpm, hpm_shade);
	PrepareBitmap("chase_l.pcx", chase_left, chase_left_shade);
	PrepareBitmap("chase_r.pcx", chase_right, chase_right_shade);
	PrepareBitmap("chase_t.pcx", chase_top, chase_top_shade);
	PrepareBitmap("chase_b.pcx", chase_bottom, chase_bottom_shade);
	PrepareBitmap("ladder1.pcx", pitch_ladder_pos, pitch_ladder_pos_shade);
	PrepareBitmap("ladder2.pcx", pitch_ladder_neg, pitch_ladder_neg_shade);

	hud_left_air.SetType(Bitmap::BMP_TRANSLUCENT);
	hud_right_air.SetType(Bitmap::BMP_TRANSLUCENT);
	hud_left_fighter.SetType(Bitmap::BMP_TRANSLUCENT);
	hud_right_fighter.SetType(Bitmap::BMP_TRANSLUCENT);
	hud_left_starship.SetType(Bitmap::BMP_TRANSLUCENT);
	hud_right_starship.SetType(Bitmap::BMP_TRANSLUCENT);
	instr_left.SetType(Bitmap::BMP_TRANSLUCENT);
	instr_right.SetType(Bitmap::BMP_TRANSLUCENT);
	warn_left.SetType(Bitmap::BMP_TRANSLUCENT);
	warn_right.SetType(Bitmap::BMP_TRANSLUCENT);
	icon_ship.SetType(Bitmap::BMP_TRANSLUCENT);
	icon_target.SetType(Bitmap::BMP_TRANSLUCENT);
	fpm.SetType(Bitmap::BMP_TRANSLUCENT);
	hpm.SetType(Bitmap::BMP_TRANSLUCENT);
	lead.SetType(Bitmap::BMP_TRANSLUCENT);
	cross.SetType(Bitmap::BMP_TRANSLUCENT);
	cross1.SetType(Bitmap::BMP_TRANSLUCENT);
	cross2.SetType(Bitmap::BMP_TRANSLUCENT);
	cross3.SetType(Bitmap::BMP_TRANSLUCENT);
	cross4.SetType(Bitmap::BMP_TRANSLUCENT);
	chase_left.SetType(Bitmap::BMP_TRANSLUCENT);
	chase_right.SetType(Bitmap::BMP_TRANSLUCENT);
	chase_top.SetType(Bitmap::BMP_TRANSLUCENT);
	chase_bottom.SetType(Bitmap::BMP_TRANSLUCENT);
	pitch_ladder_pos.SetType(Bitmap::BMP_TRANSLUCENT);
	pitch_ladder_neg.SetType(Bitmap::BMP_TRANSLUCENT);

	// Unreal: remove debug allocator (__FILE__, __LINE__)
	hud_left_sprite = new Sprite(&hud_left_fighter);
	hud_right_sprite = new Sprite(&hud_right_fighter);
	instr_left_sprite = new Sprite(&instr_left);
	instr_right_sprite = new Sprite(&instr_right);
	warn_left_sprite = new Sprite(&warn_left);
	warn_right_sprite = new Sprite(&warn_right);
	icon_ship_sprite = new Sprite(&icon_ship);
	icon_target_sprite = new Sprite(&icon_target);
	fpm_sprite = new Sprite(&fpm);
	hpm_sprite = new Sprite(&hpm);
	lead_sprite = new Sprite(&lead);
	aim_sprite = new Sprite(&cross);
	tgt1_sprite = new Sprite(&cross1);
	tgt2_sprite = new Sprite(&cross2);
	tgt3_sprite = new Sprite(&cross3);
	tgt4_sprite = new Sprite(&cross4);
	chase_sprite = new Sprite(&chase_left);

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

	int i = 0;
	for (i = 0; i < 15; i++) {
		pitch_ladder[i] = new Sprite(&pitch_ladder_pos);

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
		pitch_ladder[i] = new Sprite(&pitch_ladder_pos);

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
		pitch_ladder[i] = new Sprite(&pitch_ladder_neg);

		FMemory::Memcpy(UV, pitch_ladder_UV, sizeof(UV));
		UV[1] = UV[3] = (pitch_ladder_UV[1] * (30 - i));
		UV[5] = UV[7] = (pitch_ladder_UV[1] * (30 - i + 1));

		pitch_ladder[i]->Reshape(192, 16);
		pitch_ladder[i]->SetTexCoords(UV);
		pitch_ladder[i]->SetBlendMode(2);
		pitch_ladder[i]->Hide();
	}

	MFDViews.SetNum(4);

	for (int32 i = 0; i < 4; ++i)
	{
		UMFDView* MFD = CreateWidget<UMFDView>(
			GetWorld(),
			UMFDView::StaticClass()
		);
	}

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

	aw = chase_left.Width() / 2;
	ah = chase_left.Height() / 2;

	mfd[0]->SetMode(EMFDMode::SHIP);
	mfd[1]->SetMode(EMFDMode::FOV);
	mfd[2]->SetMode(EMFDMode::GAME);

	hud_font = (SystemFont*)FontManager::Find("HUD");
	big_font = (SystemFont*)FontManager::Find("GUI");

	for (i = 0; i < TXT_LAST; i++) {
		hud_text[i].font = hud_font;
	}

	hud_text[TXT_THREAT_WARN].font = big_font;
	hud_text[TXT_SHOOT].font = big_font;
	hud_text[TXT_AUTO].font = big_font;

	SetHUDColorSet(def_color_set);
	UMFDView::SetColor(standard_hud_colors[color]);

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

	loader->SetDataPath(nullptr);

	for (i = 0; i < MAX_MSG; i++)
		msg_time[i] = 0;
}

UHUDView::~UHUDView()
{
	HideCompass();

	if (missile_lock_sound) {
		missile_lock_sound->Stop();
		missile_lock_sound->Release();
		missile_lock_sound = nullptr;
	}

	for (int i = 0; i < 3; i++) {
		delete mfd[i];
		mfd[i] = nullptr;
	}

	for (int i = 0; i < 32; i++) {
		GRAPHIC_DESTROY(hud_sprite[i]);
	}

	fpm.ClearImage();
	hpm.ClearImage();
	lead.ClearImage();
	cross.ClearImage();
	cross1.ClearImage();
	cross2.ClearImage();
	cross3.ClearImage();
	cross4.ClearImage();
	hud_left_air.ClearImage();
	hud_right_air.ClearImage();
	hud_left_fighter.ClearImage();
	hud_right_fighter.ClearImage();
	hud_left_starship.ClearImage();
	hud_right_starship.ClearImage();
	instr_left.ClearImage();
	instr_right.ClearImage();
	warn_left.ClearImage();
	warn_right.ClearImage();
	icon_ship.ClearImage();
	icon_target.ClearImage();
	chase_left.ClearImage();
	chase_right.ClearImage();
	chase_top.ClearImage();
	chase_bottom.ClearImage();
	pitch_ladder_pos.ClearImage();
	pitch_ladder_neg.ClearImage();

	delete[] fpm_shade;                fpm_shade = nullptr;
	delete[] hpm_shade;                hpm_shade = nullptr;
	delete[] lead_shade;               lead_shade = nullptr;
	delete[] cross_shade;              cross_shade = nullptr;
	delete[] cross1_shade;             cross1_shade = nullptr;
	delete[] cross2_shade;             cross2_shade = nullptr;
	delete[] cross3_shade;             cross3_shade = nullptr;
	delete[] cross4_shade;             cross4_shade = nullptr;
	delete[] hud_left_shade_air;       hud_left_shade_air = nullptr;
	delete[] hud_right_shade_air;      hud_right_shade_air = nullptr;
	delete[] hud_left_shade_fighter;   hud_left_shade_fighter = nullptr;
	delete[] hud_right_shade_fighter;  hud_right_shade_fighter = nullptr;
	delete[] hud_left_shade_starship;  hud_left_shade_starship = nullptr;
	delete[] hud_right_shade_starship; hud_right_shade_starship = nullptr;
	delete[] instr_left_shade;         instr_left_shade = nullptr;
	delete[] instr_right_shade;        instr_right_shade = nullptr;
	delete[] warn_left_shade;          warn_left_shade = nullptr;
	delete[] warn_right_shade;         warn_right_shade = nullptr;
	delete[] icon_ship_shade;          icon_ship_shade = nullptr;
	delete[] icon_target_shade;        icon_target_shade = nullptr;
	delete[] chase_left_shade;         chase_left_shade = nullptr;
	delete[] chase_right_shade;        chase_right_shade = nullptr;
	delete[] chase_top_shade;          chase_top_shade = nullptr;
	delete[] chase_bottom_shade;       chase_bottom_shade = nullptr;
	delete[] pitch_ladder_pos_shade;   pitch_ladder_pos_shade = nullptr;
	delete[] pitch_ladder_neg_shade;   pitch_ladder_neg_shade = nullptr;

	delete az_ring;     az_ring = nullptr;
	delete az_pointer;  az_pointer = nullptr;
	delete el_ring;     el_ring = nullptr;
	delete el_pointer;  el_pointer = nullptr;

	hud_view = nullptr;
}

void UHUDView::OnWindowMove()
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

	UMFDView::SetColor(standard_hud_colors[color]);

	// int cx = width/2;
	// int cy = height/2;
}

// +--------------------------------------------------------------------+

void UHUDView::SetTacticalMode(int InMode)
{
	if (tactical != InMode) {
		tactical = InMode;

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

void UHUDView::SetOverlayMode(int InMode)
{
	if (overlay != InMode) {
		overlay = InMode;
	}
}

// +--------------------------------------------------------------------+

bool UHUDView::Update(SimObject* obj)
{
	if (obj == ship) {
		if (target)
			SetTarget(nullptr);

		ship = nullptr;

		for (int i = 0; i < 3; i++)
			mfd[i]->SetShip(ship);
	}

	if (obj == target) {
		target = nullptr;
		PrepareBitmap("hud_icon.pcx", icon_target, icon_target_shade);
		ColorizeBitmap(icon_target, icon_target_shade, txt_color);
	}

	return SimObserver::Update(obj);
}

const char* UHUDView::GetObserverName() const
{
	return "HUDView";
}

// +--------------------------------------------------------------------+

void UHUDView::UseCameraView(CameraView* v)
{
	if (v && camview != v) {
		camview = v;

		for (int i = 0; i < 3; i++)
			mfd[i]->UseCameraView(camview);

		projector = camview->GetProjector();
	}
}

// +--------------------------------------------------------------------+
// Contact marker color

FColor UHUDView::MarkerColor(SimContact* Contact)
{
	FColor Out(80, 80, 80, 255);

	if (!Contact)
		return Out;

	Sim* LocalSim = Sim::GetSim();
	Ship* LocalShip = LocalSim ? LocalSim->GetPlayerShip() : nullptr;
	if (!LocalShip)
		return Out;

	const int32 Iff = Contact->GetIFF(LocalShip);

	// Age is assumed to be a 0..1 style fade. Clamp to be safe.
	const float Age = FMath::Clamp((float)Contact->Age(), 0.0f, 1.0f);

	// Ship::IFFColor should return FColor (or something convertible to it).
	const FColor BaseColor = Ship::IFFColor(Iff);

	FLinearColor LC = FLinearColor(BaseColor) * Age;

	// Threat flash for shots:
	if (Contact->GetShot() && Contact->Threat(LocalShip))
	{
		const bool bBlinkOn = (((Game::RealTime() / 500) & 1) != 0);
		const float Scale = bBlinkOn ? 2.0f : 0.5f;
		LC *= Scale;
	}

	// Clamp to valid color range and convert back to FColor (sRGB)
	LC.R = FMath::Clamp(LC.R, 0.0f, 1.0f);
	LC.G = FMath::Clamp(LC.G, 0.0f, 1.0f);
	LC.B = FMath::Clamp(LC.B, 0.0f, 1.0f);
	LC.A = 1.0f;

	Out = LC.ToFColor(true);
	return Out;
}

// +--------------------------------------------------------------------+

void
UHUDView::DrawContactMarkers()
{
	threat = 0;

	for (int i = 0; i < MAX_CONTACT; i++) {
		HideHUDText(TXT_CONTACT_NAME + i);
		HideHUDText(TXT_CONTACT_INFO + i);
	}

	if (!ship)
		return;

	int                     index = 0;
	ListIter<SimContact>     contact = ship->ContactList();

	// draw own sensor contacts:
	while (++contact) {
		SimContact* c = contact.value();

		// draw track ladder:
		if (c->TrackLength() > 0 && c->GetShip() != ship) {
			DrawTrack(c);
		}

		DrawContact(c, index++);
	}

	FColor c = ship->MarkerColor();

	// draw own ship track ladder:
	if (CameraManager::GetCameraMode() == CameraManager::MODE_ORBIT && ship->TrackLength() > 0) {
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
	if (CameraManager::GetCameraMode() == CameraManager::MODE_ORBIT && mark_pt.Z > 1.0) {
		projector->Project(mark_pt);

		int x = (int)mark_pt.X;
		int y = (int)mark_pt.Y;

		if (x > 4 && x < width - 4 &&
			y > 4 && y < height - 4) {

			DrawDiamond(x, y, 5, c);

			if (tactical) {
				Rect self_rect(x + 8, y - 4, 200, 12);
				DrawHUDText(TXT_SELF, ship->Name(), self_rect, DT_LEFT, HUD_MIXED_CASE);
			}
		}
	}

	// draw life bars on targeted ship:
	if (target && target->Type() == SimObject::SIM_SHIP && target->Rep()) {
		Ship* tgt_ship = (Ship*)target;

		if (!tgt_ship) {
			UE_LOG(LogTemp, Warning, TEXT("Null Pointer in UHUDView::DrawContactMarkers(). Please investigate."));
			return;
		}

		Graphic* g = tgt_ship->Rep();
		Rect     r = g->ScreenRect();

		FVector tgt_pt = tgt_ship->Location();
		projector->Transform(tgt_pt);

		// clip:
		if (tgt_pt.Z > 1.0) {
			projector->Project(tgt_pt);

			int x = (int)tgt_pt.X;
			int y = r.y;

			if (y >= 2000)
				y = (int)tgt_pt.Y;

			if (x > 4 && x < width - 4 &&
				y > 4 && y < height - 4) {

				const int BAR_LENGTH = 40;

				// life bars:
				int sx = x - BAR_LENGTH / 2;
				int sy = y - 8;

				double hull_strength = tgt_ship->Integrity() / tgt_ship->Design()->integrity;

				int hw = (int)(BAR_LENGTH * hull_strength);
				int sw = (int)(BAR_LENGTH * (tgt_ship->ShieldStrength() / 100.0));

				SYSTEM_STATUS s = SYSTEM_STATUS::NOMINAL;

				if (hull_strength < 0.30)       s = SYSTEM_STATUS::CRITICAL;
				else if (hull_strength < 0.60)  s = SYSTEM_STATUS::DEGRADED;

				FColor hc = GetStatusColor(s);
				FColor sc = hud_color;

				window->FillRect(sx, sy, sx + hw, sy + 1, hc);
				window->FillRect(sx, sy + 3, sx + sw, sy + 4, sc);
			}
		}
	}
}

// +--------------------------------------------------------------------+

void
UHUDView::DrawContact(SimContact* contact, int index)
{
	if (index >= MAX_CONTACT)
		return;

	FColor   c = MarkerColor(contact);
	int      c_iff = contact->GetIFF(ship);
	Ship* c_ship = contact->GetShip();
	SimShot* c_shot = contact->GetShot();
	FVector  mark_pt = contact->Location();
	double   distance = 0;

	if ((!c_ship && !c_shot) || c_ship == ship)
		return;

	if (c_ship && c_ship->GetFlightPhase() < Ship::ACTIVE)
		return;

	if (c_ship) {
		mark_pt = c_ship->Location();

		if (c_ship->IsGroundUnit())
			mark_pt += FVector(0.f, 150.f, 0.f);
	}
	else {
		mark_pt = c_shot->Location();
	}

	projector->Transform(mark_pt);

	// clip:
	if (mark_pt.Z > 1.0) {
		distance = mark_pt.Length();

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
					if (c_ship->Class() > CLASSIFICATION::LCA)
						code = *(Game::GetText("HUDView.symbol.starship").data());
				}
				else if (c_shot) {
					code = *(Game::GetText("HUDView.symbol.torpedo").data());
				}

				Sensor* sensor = ship->GetSensor();
				double  limit = 75e3;

				if (sensor)
					limit = sensor->GetBeamRange();

				double range = contact->Range(ship, limit);

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
					else if (range < 1e3) {
						range = 1;
					}
					else {
						range /= 1000;
					}

					if (arcade) {
						if (c_ship) {
							strcpy_s(contact_buf, c_ship->Name());
						}
						else if (!mega) {
							sprintf_s(contact_buf, "%c %d", code, (int)range);
						}
						else {
							sprintf_s(contact_buf, "%c %.1f M", code, range);
						}
					}
					else {
						char    closing = '+';
						FVector delta_v;

						if (c_ship)
							delta_v = ship->Velocity() - c_ship->Velocity();
						else if (c_shot)
							delta_v = ship->Velocity() - c_shot->Velocity();

						// losing ground:
						if (FVector::DotProduct(delta_v, ship->Velocity()) < 0.0f)
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
UHUDView::DrawTrackSegment(FVector& t1, FVector& t2, FColor c)
{
	int x1, y1, x2, y2;

	projector->Transform(t1);
	projector->Transform(t2);

	const double CLIP_Z = 0.1;

	if (t1.Z < CLIP_Z && t2.Z < CLIP_Z)
		return;

	if (t1.Z < CLIP_Z && t2.Z >= CLIP_Z) {
		double dx = (double)t2.X - (double)t1.X;
		double dy = (double)t2.Y - (double)t1.Y;
		double s = (CLIP_Z - (double)t1.Z) / ((double)t2.Z - (double)t1.Z);

		t1.X = (float)((double)t1.X + dx * s);
		t1.Y = (float)((double)t1.Y + dy * s);
		t1.Z = (float)CLIP_Z;
	}
	else if (t2.Z < CLIP_Z && t1.Z >= CLIP_Z) {
		double dx = (double)t1.X - (double)t2.X;
		double dy = (double)t1.Y - (double)t2.Y;
		double s = (CLIP_Z - (double)t2.Z) / ((double)t1.Z - (double)t2.Z);

		t2.X = (float)((double)t2.X + dx * s);
		t2.Y = (float)((double)t2.Y + dy * s);
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
UHUDView::DrawTrack(SimContact* contact)
{
	Ship* c_ship = contact->GetShip();

	if (c_ship && c_ship->GetFlightPhase() < Ship::ACTIVE)
		return;

	int    ctl = contact->TrackLength();
	FColor c = MarkerColor(contact);

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
UHUDView::DrawRect(SimObject* targ)
{
	Graphic* g = targ->Rep();
	Rect     r = g->ScreenRect();
	FColor   c;

	if (targ->Type() == SimObject::SIM_SHIP)
		c = ((Ship*)targ)->MarkerColor();
	else
		c = ((SimShot*)targ)->MarkerColor();

	if (r.w > 0 && r.h > 0) {
		if (r.w < 8) {
			r.x -= (8 - r.w) / 2;
			r.w = 8;
		}

		if (r.h < 8) {
			r.y -= (8 - r.h) / 2;
			r.h = 8;
		}
	}
	else {
		FVector mark_pt = targ->Location();
		projector->Transform(mark_pt);

		// clip:
		if (mark_pt.Z < 1.0)
			return;

		projector->Project(mark_pt);

		int x = (int)mark_pt.X;
		int y = (int)mark_pt.Y;

		if (x < 4 || x > width - 4 || y < 4 || y > height - 4)
			return;

		r.x = x - 4;
		r.y = y - 4;
		r.w = 8;
		r.h = 8;
	}

	// horizontal
	window->DrawLine(r.x, r.y, r.x + 8, r.y, c);
	window->DrawLine(r.x + r.w - 8, r.y, r.x + r.w, r.y, c);
	window->DrawLine(r.x, r.y + r.h, r.x + 8, r.y + r.h, c);
	window->DrawLine(r.x + r.w - 8, r.y + r.h, r.x + r.w, r.y + r.h, c);
	// vertical
	window->DrawLine(r.x, r.y, r.x, r.y + 8, c);
	window->DrawLine(r.x, r.y + r.h - 8, r.x, r.y + r.h, c);
	window->DrawLine(r.x + r.w, r.y, r.x + r.w, r.y + 8, c);
	window->DrawLine(r.x + r.w, r.y + r.h - 8, r.x + r.w, r.y + r.h, c);
}
// +--------------------------------------------------------------------+

void
UHUDView::DrawFPM()
{
	fpm_sprite->Hide();

	if (!ship)
		return;

	if (ship->Velocity().Length() > 50.0) {
		double xtarg = xcenter;
		double ytarg = ycenter;

		FVector svel = ship->Velocity();
		svel.Normalize();

		FVector tloc = ship->Location() + svel * 1e8f;

		// Translate into camera relative:
		projector->Transform(tloc);

		const bool behind = (tloc.Z < 0.0f);
		if (behind)
			return;

		// Project into screen coordinates:
		projector->Project(tloc);

		xtarg = tloc.X;
		ytarg = tloc.Y;

		fpm_sprite->Show();
		fpm_sprite->MoveTo(FVector((float)xtarg, (float)ytarg, 1.0f));
	}
}

// +--------------------------------------------------------------------+

void
UHUDView::DrawPitchLadder()
{
	for (int i = 0; i < 31; i++)
		pitch_ladder[i]->Hide();

	if (!ship)
		return;

	if (ship->IsAirborne() && Game::MaxTexSize() > 128) {
		double xtarg = xcenter;
		double ytarg = ycenter;

		FVector uvec(0.f, 1.f, 0.f);
		FVector svel = ship->Velocity();

		if (svel.Length() == 0.0f)
			svel = ship->Heading();

		if (svel.X == 0.0f && svel.Z == 0.0f)
			return;

		svel.Y = 0.0f;
		svel.Normalize();

		FVector gloc = ship->Location();
		gloc.Y = 0.0f;

		const double baseline = 1e9;
		const double clip_angle = 20 * DEGREES;

		FVector tloc = gloc + svel * (float)baseline;

		// Translate into camera relative:
		projector->Transform(tloc);

		// Project into screen coordinates:
		projector->Project(tloc);

		xtarg = tloc.X;
		ytarg = tloc.Y;

		// compute roll angle:
		double roll_angle = 0;
		double pitch_angle = 0;

		FVector heading = ship->Heading();
		heading.Normalize();

		if (heading.X != 0.0f || heading.Z != 0.0f) {
			FVector gheading = heading;
			gheading.Y = 0.0f;
			gheading.Normalize();

			double dot = FVector::DotProduct(gheading, heading);

			if (heading.Y < 0.0f)
				dot = -dot;

			pitch_angle = acos(dot);

			if (pitch_angle > PI / 2)
				pitch_angle -= PI;

			double s0 = sin(pitch_angle);
			double c0 = cos(pitch_angle);
			double s1 = sin(pitch_angle + 10 * DEGREES);
			double c1 = cos(pitch_angle + 10 * DEGREES);

			tloc = gloc + (svel * (float)(baseline * c0)) + (uvec * (float)(baseline * s0));
			projector->Transform(tloc);

			double x0 = tloc.X;
			double y0 = tloc.Y;

			tloc = gloc + (svel * (float)(baseline * c1)) + (uvec * (float)(baseline * s1));
			projector->Transform(tloc);

			double x1 = tloc.X;
			double y1 = tloc.Y;

			double dx = x1 - x0;
			double dy = y1 - y0;

			roll_angle = atan2(-dy, dx) + PI / 2;
		}

		const double alias_limit = 0.1 * DEGREES;

		if (fabs(roll_angle) <= alias_limit) {
			if (roll_angle > 0)
				roll_angle = alias_limit;
			else
				roll_angle = -alias_limit;
		}
		else if (fabs(roll_angle - PI) <= alias_limit) {
			roll_angle = PI - alias_limit;
		}

		if (fabs(pitch_angle) <= clip_angle) {
			pitch_ladder[15]->Show();
			pitch_ladder[15]->MoveTo(FVector((float)xtarg, (float)ytarg, 1.0f));
			pitch_ladder[15]->SetAngle(roll_angle);
		}

		for (int i = 1; i <= 15; i++) {
			double angle = i * 5 * DEGREES;

			if (i > 12)
				angle = (60 + (i - 12) * 10) * DEGREES;

			double s = sin(angle);
			double c = cos(angle);

			if (fabs(pitch_angle - angle) <= clip_angle) {
				// positive angle:
				tloc = gloc + (svel * (float)(baseline * c)) + (uvec * (float)(baseline * s));
				projector->Transform(tloc);

				if (tloc.Z > 0.0f) {
					projector->Project(tloc);
					pitch_ladder[15 - i]->Show();
					pitch_ladder[15 - i]->MoveTo(FVector(tloc.X, tloc.Y, 1.0f));
					pitch_ladder[15 - i]->SetAngle(roll_angle);
				}
			}

			if (fabs(pitch_angle + angle) <= clip_angle) {
				// negative angle:
				tloc = gloc + (svel * (float)(baseline * c)) + (uvec * (float)(-baseline * s));
				projector->Transform(tloc);

				if (tloc.Z > 0.0f) {
					projector->Project(tloc);
					pitch_ladder[15 + i]->Show();
					pitch_ladder[15 + i]->MoveTo(FVector(tloc.X, tloc.Y, 1.0f));
					pitch_ladder[15 + i]->SetAngle(roll_angle);
				}
			}
		}
	}
}

// +--------------------------------------------------------------------+

void
UHUDView::DrawHPM()
{
	hpm_sprite->Hide();

	if (!ship)
		return;

	double xtarg = xcenter;
	double ytarg = ycenter;

	double az = ship->GetHelmHeading() - PI;
	double el = ship->GetHelmPitch();

	FVector hvec((float)sin(az), (float)sin(el), (float)cos(az));
	hvec.Normalize();

	FVector tloc = ship->Location() + hvec * 1e8f;

	// Translate into camera relative:
	projector->Transform(tloc);

	const bool behind = (tloc.Z < 0.0f);
	if (behind)
		return;

	// Project into screen coordinates:
	projector->Project(tloc);

	xtarg = tloc.X;
	ytarg = tloc.Y;

	hpm_sprite->Show();
	hpm_sprite->MoveTo(FVector((float)xtarg, (float)ytarg, 1.0f));
}

// +--------------------------------------------------------------------+

void
UHUDView::HideCompass()
{
	az_ring->Hide();
	az_pointer->Hide();
	el_ring->Hide();
	el_pointer->Hide();

	SimScene* scene = az_ring->GetScene();
	if (scene) {
		scene->DelGraphic(az_ring);
		scene->DelGraphic(az_pointer);
		scene->DelGraphic(el_ring);
		scene->DelGraphic(el_pointer);
	}
}

// +--------------------------------------------------------------------+

static FORCEINLINE FRotator RotFromRadians(double YawRad, double PitchRad = 0.0, double RollRad = 0.0)
{
	// UE Rotator is (Pitch, Yaw, Roll) in DEGREES
	return FRotator(
		FMath::RadiansToDegrees(PitchRad),
		FMath::RadiansToDegrees(YawRad),
		FMath::RadiansToDegrees(RollRad)
	);
}

void UHUDView::DrawCompass()
{
	if (!ship || !ship->Rep())
		return;

	Solid* SolidRep = static_cast<Solid*>(ship->Rep());
	const FVector Loc = SolidRep->Location();

	az_ring->MoveTo(Loc);
	az_pointer->MoveTo(Loc);
	el_ring->MoveTo(Loc);
	el_pointer->MoveTo(Loc);

	const double HelmHeading = ship->GetHelmHeading();
	const double HelmPitch = ship->GetHelmPitch();
	const double CurrHeading = ship->CompassHeading();
	const double CurrPitch = ship->CompassPitch();

	const bool bShowAz = FMath::Abs(HelmHeading - CurrHeading) > 5.0 * DEGREES;
	const bool bShowEl = FMath::Abs(HelmPitch - CurrPitch) > 5.0 * DEGREES;

	SimScene* Scene = camview ? camview->GetScene() : nullptr;
	if (!Scene)
		return;

	if (bShowAz || bShowEl)
	{
		Scene->AddGraphic(az_ring);
		az_ring->Show();

		if (bShowEl || FMath::Abs(HelmPitch) > 5.0 * DEGREES)
		{
			Scene->AddGraphic(el_ring);

			// === EL RING ORIENTATION ===
			{
				const FRotator Rot(
					FMath::RadiansToDegrees(0.0),                 // Pitch
					FMath::RadiansToDegrees(HelmHeading + PI),     // Yaw
					0.0                                           // Roll
				);
				el_ring->SetOrientation(FRotationMatrix(Rot));
				el_ring->Show();
			}

			Scene->AddGraphic(el_pointer);

			// === EL POINTER ORIENTATION ===
			{
				const FRotator Rot(
					FMath::RadiansToDegrees(-HelmPitch),           // Pitch
					FMath::RadiansToDegrees(HelmHeading + PI),     // Yaw
					FMath::RadiansToDegrees(PI / 2.0)              // Roll
				);
				el_pointer->SetOrientation(FRotationMatrix(Rot));
				el_pointer->Show();
			}
		}
		else
		{
			Scene->AddGraphic(az_pointer);

			// === AZ POINTER ORIENTATION ===
			{
				const FRotator Rot(
					0.0,
					FMath::RadiansToDegrees(HelmHeading + PI),
					0.0
				);
				az_pointer->SetOrientation(FRotationMatrix(Rot));
				az_pointer->Show();
			}
		}
	}
}

// +--------------------------------------------------------------------+

void
UHUDView::DrawLCOS(SimObject* targ, double dist)
{
	lead_sprite->Hide();
	aim_sprite->Hide();
	chase_sprite->Hide();

	double xtarg = xcenter;
	double ytarg = ycenter;

	Weapon* prim = ship ? ship->GetPrimary() : nullptr;
	if (!prim)
		return;

	FVector tloc = targ->Location();

	// Translate into camera relative:
	projector->Transform(tloc);

	const bool behind = (tloc.Z < 0.0f);

	if (behind)
		tloc.Z = -tloc.Z;

	// Project into screen coordinates:
	projector->Project(tloc);

	// DRAW THE OFFSCREEN CHASE INDICATOR:
	if (behind ||
		tloc.X <= 0.0f || tloc.X >= (float)(width - 1) ||
		tloc.Y <= 0.0f || tloc.Y >= (float)(height - 1)) {

		// Left side:
		if (tloc.X <= 0.0f || (behind && tloc.X < width / 2.0f)) {
			if (tloc.Y < ah) tloc.Y = (float)ah;
			else if (tloc.Y >= height - ah) tloc.Y = (float)(height - 1 - ah);

			chase_sprite->Show();
			chase_sprite->SetAnimation(&chase_left);
			chase_sprite->MoveTo(FVector((float)aw, tloc.Y, 1.0f));
		}

		// Right side:
		else if (tloc.X >= (float)(width - 1) || behind) {
			if (tloc.Y < ah) tloc.Y = (float)ah;
			else if (tloc.Y >= height - ah) tloc.Y = (float)(height - 1 - ah);

			chase_sprite->Show();
			chase_sprite->SetAnimation(&chase_right);
			chase_sprite->MoveTo(FVector((float)(width - 1 - aw), tloc.Y, 1.0f));
		}
		else {
			if (tloc.X < aw) tloc.X = (float)aw;
			else if (tloc.X >= width - aw) tloc.X = (float)(width - 1 - aw);

			// Top edge:
			if (tloc.Y <= 0.0f) {
				chase_sprite->Show();
				chase_sprite->SetAnimation(&chase_top);
				chase_sprite->MoveTo(FVector(tloc.X, (float)ah, 1.0f));
			}

			// Bottom edge:
			else if (tloc.Y >= (float)(height - 1)) {
				chase_sprite->Show();
				chase_sprite->SetAnimation(&chase_bottom);
				chase_sprite->MoveTo(FVector(tloc.X, (float)(height - 1 - ah), 1.0f));
			}
		}
	}

	// DRAW THE LCOS:
	else {
		if (!ship->IsStarship()) {
			FVector aim_vec = ship->Heading();
			aim_vec.Normalize();

			// shot speed is relative to ship speed:
			const FVector shot_vel = ship->Velocity() + aim_vec * (float)prim->Design()->speed;
			const double  shot_speed = shot_vel.Length();

			// time for shot to reach target
			const double time = (shot_speed > 0.0) ? (dist / shot_speed) : 0.0;

			// LCOS (Lead Computing Optical Sight)
			if (gunsight == 0) {
				// where the shot will be when it is the same distance
				// away from the ship as the target:
				const FVector impact = ship->Location() + (shot_vel * (float)time);

				// where the target will be when the shot reaches it:
				const FVector targ_vel = targ->Velocity();
				const FVector dest = targ->Location() + (targ_vel * (float)time);
				const FVector delta = impact - dest;

				// draw the gun sight here in 3d world coordinates:
				FVector sight = targ->Location() + delta;

				// Project into screen coordinates:
				projector->Transform(sight);
				projector->Project(sight);

				xtarg = sight.X;
				ytarg = sight.Y;

				aim_sprite->Show();
				aim_sprite->MoveTo(FVector((float)xtarg, (float)ytarg, 1.0f));
			}

			// Wing Commander style lead indicator
			else {
				// where the target will be when the shot reaches it:
				const FVector targ_vel = targ->Velocity() - ship->Velocity();
				FVector dest = targ->Location() + (targ_vel * (float)time);

				// Translate into camera relative:
				projector->Transform(dest);
				projector->Project(dest);

				xtarg = dest.X;
				ytarg = dest.Y;

				lead_sprite->Show();
				lead_sprite->MoveTo(FVector((float)xtarg, (float)ytarg, 1.0f));
			}
		}
	}
}

// +--------------------------------------------------------------------+

void
UHUDView::DrawTarget()
{
	const int   bar_width = 256;
	const int   bar_height = 192;
	const int   box_width = 120;

	SimObject* old_target = target;

	if (!ship) {
		target = old_target;
		return;
	}

	if (mode == (int)HUD_MODE::HUD_MODE_ILS) {
		Ship* controller = ship->GetController();
		if (controller && !target)
			target = controller;
	}

	if (target && target->Rep()) {
		Sensor* sensor = ship->GetSensor();
		SimContact* contact = 0;

		if (sensor && target->Type() == SimObject::SIM_SHIP) {
			contact = sensor->FindContact((Ship*)target);
		}

		int     cx = width / 2;
		int     cy = height / 2;
		int     l = cx - bar_width / 2;
		int     r = cx + bar_width / 2;
		int     t = cy - bar_height / 2;
		int     b = cy + bar_height / 2;
		FVector delta = target->Location() - ship->Location();
		double  distance = delta.Length();
		FVector delta_v = ship->Velocity() - target->Velocity();
		double  speed = delta_v.Length();
		char    txt[256];

		if (mode == (int)HUD_MODE::HUD_MODE_ILS && ship->GetInbound() && ship->GetInbound()->GetDeck()) {
			delta = ship->GetInbound()->GetDeck()->EndPoint() - ship->Location();
			distance = delta.Length();
		}

		if (FVector::DotProduct(delta, ship->Velocity()) > 0) {        // in front
			if (FVector::DotProduct(delta_v, ship->Velocity()) < 0)     // losing ground
				speed = -speed;
		}
		else {                                                        // behind
			if (FVector::DotProduct(delta_v, ship->Velocity()) > 0)     // passing
				speed = -speed;
		}

		Rect range_rect(r - 20, cy - 5, box_width, 12);

		if (tactical)
			range_rect.x = width - range_rect.w - 8;

		if (contact) {
			Sensor* sensor2 = ship->GetSensor();
			double  limit = 75e3;

			if (sensor2)
				limit = sensor2->GetBeamRange();

			distance = contact->Range(ship, limit);

			if (!contact->ActLock() && !contact->PasLock()) {
				strcpy_s(txt, Game::GetText("HUDView.No-Range").data());
				speed = 0;
			}
			else {
				FormatNumber(txt, distance);
			}
		}
		else {
			FormatNumber(txt, distance);
		}

		DrawHUDText(TXT_RANGE, txt, range_rect, DT_RIGHT);

		if (arcade) {
			target = old_target;
			return;
		}

		range_rect.y += 18;
		FormatNumber(txt, speed);
		DrawHUDText(TXT_CLOSING_SPEED, txt, range_rect, DT_RIGHT);

		// target info:
		if (!tactical) {
			range_rect.y = cy - 76;
		}
		else {
			range_rect.x = width - 2 * box_width - 8;
			range_rect.y = cy - 76;
			range_rect.w = 2 * box_width;
		}

		DrawHUDText(TXT_TARGET_NAME, target->Name(), range_rect, DT_RIGHT);

		if (target->Type() == SimObject::SIM_SHIP) {
			Ship* tgt_ship = (Ship*)target;

			range_rect.y += 10;
			DrawHUDText(TXT_TARGET_DESIGN, tgt_ship->Design()->display_name, range_rect, DT_RIGHT);

			if (mode != (int)HUD_MODE::HUD_MODE_ILS) {
				if (tgt_ship->IsStarship()) {
					range_rect.y += 10;
					sprintf_s(txt, "%s %03d",
						Game::GetText("HUDView.symbol.shield").data(),
						(int)tgt_ship->ShieldStrength());
					DrawHUDText(TXT_TARGET_SHIELD, txt, range_rect, DT_RIGHT);
				}

				range_rect.y += 10;
				sprintf_s(txt, "%s %03d",
					Game::GetText("HUDView.symbol.hull").data(),
					(int)(tgt_ship->Integrity() / tgt_ship->Design()->integrity * 100));
				DrawHUDText(TXT_TARGET_HULL, txt, range_rect, DT_RIGHT);

				SimSystem* sys = ship->GetSubTarget();
				if (sys) {
					FColor        stat = hud_color;
					static DWORD blink = Game::RealTime();

					int blink_delta = Game::RealTime() - blink;
					sprintf_s(txt, "%s %03d", sys->Abbreviation(), (int)sys->Availability());

					switch (sys->Status()) {
					case SimSystem::DEGRADED:   stat = FColor(255, 255, 0);  break;
					case SimSystem::CRITICAL:
					case SimSystem::DESTROYED:  stat = FColor(255, 0, 0);    break;
					case SimSystem::MAINT:
						if (blink_delta < 250)
							stat = FColor(8, 8, 8);
						break;
					}

					if (blink_delta > 500)
						blink = Game::RealTime();

					range_rect.y += 10;
					DrawHUDText(TXT_TARGET_SUB, txt, range_rect, DT_RIGHT);
				}
			}
		}

		else if (target->Type() == SimObject::SIM_DRONE) {
			Drone* tgt_drone = (Drone*)target;

			range_rect.y += 10;
			DrawHUDText(TXT_TARGET_DESIGN, tgt_drone->DesignName(), range_rect, DT_RIGHT);

			range_rect.y += 10;
			int eta = tgt_drone->GetEta();

			if (eta > 0) {
				int minutes = (eta / 60) % 60;
				int seconds = (eta) % 60;

				char eta_buf[16];
				sprintf_s(eta_buf, "T %d:%02d", minutes, seconds);
				DrawHUDText(TXT_TARGET_ETA, eta_buf, range_rect, DT_RIGHT);
			}
		}
	}

	target = old_target;
}

// +--------------------------------------------------------------------+

void
UHUDView::DrawNavInfo()
{
	const int   bar_width = 256;
	const int   bar_height = 192;
	const int   box_width = 120;

	if (!ship)
		return;

	if (arcade) {
		if (ship->IsAutoNavEngaged()) {
			Rect info_rect(width / 2 - box_width, height / 2 + bar_height, box_width * 2, 12);

			if (big_font)
				hud_text[TXT_NAV_INDEX].font = big_font;

			DrawHUDText(TXT_NAV_INDEX, Game::GetText("HUDView.Auto-Nav"), info_rect, DT_CENTER);
		}

		return;
	}

	hud_text[TXT_NAV_INDEX].font = hud_font;

	Instruction* navpt = ship->GetNextNavPoint();

	if (navpt) {
		int    cx = width / 2;
		int    cy = height / 2;
		int    l = cx - bar_width / 2;
		int    r = cx + bar_width / 2;
		int    t = cy - bar_height / 2;
		int    b = cy + bar_height / 2;

		int    index = ship->GetNavIndex(navpt);
		double distance = ship->RangeToNavPoint(navpt);
		double speed = ship->Velocity().Length();
		int    etr = 0;
		char   txt[256];

		if (speed > 10)
			etr = (int)(distance / speed);

		Rect info_rect(r - 20, cy + 32, box_width, 12);

		if (tactical)
			info_rect.x = width - info_rect.w - 8;

		if (ship->IsAutoNavEngaged())
			sprintf_s(txt, "%s %d", Game::GetText("HUDView.Auto-Nav").data(), index);
		else
			sprintf_s(txt, "%s %d", Game::GetText("HUDView.Nav").data(), index);

		DrawHUDText(TXT_NAV_INDEX, txt, info_rect, DT_RIGHT);

		info_rect.y += 10;
		if (navpt->Action())
			DrawHUDText(TXT_NAV_ACTION, Instruction::ActionName(navpt->Action()), info_rect, DT_RIGHT);

		info_rect.y += 10;
		FormatNumber(txt, navpt->Speed());
		DrawHUDText(TXT_NAV_SPEED, txt, info_rect, DT_RIGHT);

		if (etr > 3600) {
			info_rect.y += 10;
			sprintf_s(txt, "%s XX:XX", Game::GetText("HUDView.time-enroute").data());
			DrawHUDText(TXT_NAV_ETR, txt, info_rect, DT_RIGHT);
		}
		else if (etr > 0) {
			info_rect.y += 10;

			int minutes = (etr / 60) % 60;
			int seconds = (etr) % 60;
			sprintf_s(txt, "%s %2d:%02d", Game::GetText("HUDView.time-enroute").data(), minutes, seconds);
			DrawHUDText(TXT_NAV_ETR, txt, info_rect, DT_RIGHT);
		}

		if (navpt->HoldTime() > 0) {
			info_rect.y += 10;

			int hold = (int)navpt->HoldTime();
			int minutes = (hold / 60) % 60;
			int seconds = (hold) % 60;
			sprintf_s(txt, "%s %2d:%02d", Game::GetText("HUDView.HOLD").data(), minutes, seconds);
			DrawHUDText(TXT_NAV_HOLD, txt, info_rect, DT_RIGHT);
		}
	}
}

// +--------------------------------------------------------------------+

void
UHUDView::DrawSight()
{
	if (!ship)
		return;

	if (target && target->Rep()) {
		FVector delta = target->Location() - ship->Location();
		double  distance = delta.Length();

		// draw LCOS on target:
		if (!tactical)
			DrawLCOS(target, distance);
	}
}

// +--------------------------------------------------------------------+

void
UHUDView::DrawDesignators()
{
	double     xtarg = xcenter;
	double     ytarg = ycenter;
	SimObject* t1 = 0;
	SimObject* t2 = 0;
	SimObject* t3 = 0;
	Sprite* sprite = 0;

	if (!ship)
		return;

	tgt1_sprite->Hide();
	tgt2_sprite->Hide();
	tgt3_sprite->Hide();
	tgt4_sprite->Hide();

	// fighters just show primary target:
	if (ship->IsDropship()) {
		SimObject* t = ship->GetTarget();
		SimSystem* s = ship->GetSubTarget();

		if (t) {
			FVector tloc = t->Location();
			if (s) {
				tloc = s->MountLocation();
			}
			else if (t->Type() == SimObject::SIM_SHIP) {
				Ship* tgt_ship = (Ship*)t;

				if (tgt_ship->IsGroundUnit())
					tloc += FVector(0.f, 150.f, 0.f);
			}

			projector->Transform(tloc);

			if (tloc.Z > 0.0f) {
				projector->Project(tloc);

				xtarg = tloc.X;
				ytarg = tloc.Y;

				if (xtarg > 0 && xtarg < width - 1 && ytarg > 0 && ytarg < height - 1) {
					double range = (t->Location() - ship->Location()).Length();

					// use out-of-range crosshair if out of range:
					if (!ship->GetPrimaryDesign() || ship->GetPrimaryDesign()->max_range < range) {
						tgt4_sprite->Show();
						tgt4_sprite->MoveTo(FVector((float)xtarg, (float)ytarg, 1.0f));
					}

					// else, use in-range primary crosshair:
					else {
						tgt1_sprite->Show();
						tgt1_sprite->MoveTo(FVector((float)xtarg, (float)ytarg, 1.0f));
					}
				}
			}
		}
	}

	// starships show up to three targets:
	else {
		ListIter<WeaponGroup> w = ship->Weapons();
		while (!t3 && ++w) {
			SimObject* t = w->GetTarget();
			SimSystem* s = w->GetSubTarget();

			if (w->Contains(ship->GetPrimary())) {
				if (t == 0) t = ship->GetTarget();
				t1 = t;
				sprite = tgt1_sprite;
			}

			else if (t && w->Contains(ship->GetSecondary())) {
				t2 = t;
				sprite = tgt2_sprite;

				if (t2 == t1)
					continue; // don't overlap target designators
			}

			else if (t) {
				t3 = t;
				sprite = tgt3_sprite;

				if (t3 == t1 || t3 == t2)
					continue; // don't overlap target designators
			}

			if (t) {
				FVector tloc = t->Location();

				if (s)
					tloc = s->MountLocation();

				projector->Transform(tloc);

				if (tloc.Z > 0.0f) {
					projector->Project(tloc);

					xtarg = tloc.X;
					ytarg = tloc.Y;

					if (xtarg > 0 && xtarg < width - 1 && ytarg > 0 && ytarg < height - 1) {
						double range = (t->Location() - ship->Location()).Length();

						// flip to out-of-range crosshair
						if (sprite == tgt1_sprite) {
							if (!ship->GetPrimaryDesign() || ship->GetPrimaryDesign()->max_range < range) {
								sprite = tgt4_sprite;
							}
						}

						sprite->Show();
						sprite->MoveTo(FVector((float)xtarg, (float)ytarg, 1.0f));
					}
				}
			}
		}
	}
}

// +--------------------------------------------------------------------+

FColor
UHUDView::GetStatusColor(SYSTEM_STATUS status)
{
	FColor sc;

	switch (status) {
	default:
	case SYSTEM_STATUS::NOMINAL:    sc = FColor(32, 192, 32);  break;
	case SYSTEM_STATUS::DEGRADED:   sc = FColor(255, 255, 0);  break;
	case SYSTEM_STATUS::CRITICAL:   sc = FColor(255, 0, 0);    break;
	case SYSTEM_STATUS::DESTROYED:  sc = FColor(0, 0, 0);      break;
	}

	return sc;
}

void
UHUDView::SetStatusColor(SYSTEM_STATUS status)
{
	switch (status) {
	default:
	case SYSTEM_STATUS::NOMINAL:    status_color = txt_color;          break;
	case SYSTEM_STATUS::DEGRADED:   status_color = FColor(255, 255, 0); break;
	case SYSTEM_STATUS::CRITICAL:   status_color = FColor(255, 0, 0);   break;
	case SYSTEM_STATUS::DESTROYED:  status_color = FColor(0, 0, 0);     break;
	}
}

// +--------------------------------------------------------------------+

static int GetReactorStatus(Ship* ship)
{
	if (!ship || ship->Reactors().size() < 1)
		return -1;

	int  status = SimSystem::NOMINAL;
	bool maint = false;

	ListIter<PowerSource> iter = ship->Reactors();
	while (++iter) {
		PowerSource* s = iter.value();

		if (s->Status() < status)
			status = s->Status();
	}

	if (maint && status == SimSystem::NOMINAL)
		status = SimSystem::MAINT;

	return status;
}

static int GetDriveStatus(Ship* ship)
{
	if (!ship || ship->Drives().size() < 1)
		return -1;

	int  status = SimSystem::NOMINAL;
	bool maint = false;

	ListIter<Drive> iter = ship->Drives();
	while (++iter) {
		Drive* s = iter.value();

		if (s->Status() < status)
			status = s->Status();
		else if (s->Status() == SimSystem::MAINT)
			maint = true;
	}

	if (maint && status == SimSystem::NOMINAL)
		status = SimSystem::MAINT;

	return status;
}

static int GetQuantumStatus(Ship* ship)
{
	if (!ship || ship->GetQuantumDrive() == 0)
		return -1;

	QuantumDrive* s = ship->GetQuantumDrive();
	return s->Status();
}

static int GetThrusterStatus(Ship* ship)
{
	if (!ship || ship->GetThruster() == 0)
		return -1;

	Thruster* s = ship->GetThruster();
	return s->Status();
}

static int GetShieldStatus(Ship* ship)
{
	if (!ship)
		return -1;

	Shield* s = ship->GetShield();
	Weapon* d = ship->GetDecoy();

	if (!s && !d)
		return -1;

	int  status = SimSystem::NOMINAL;
	bool maint = false;

	if (s) {
		if (s->Status() < status)
			status = s->Status();
		else if (s->Status() == SimSystem::MAINT)
			maint = true;
	}

	if (d) {
		if (d->Status() < status)
			status = d->Status();
		else if (d->Status() == SimSystem::MAINT)
			maint = true;
	}

	if (maint && status == SimSystem::NOMINAL)
		status = SimSystem::MAINT;

	return status;
}

static int GetWeaponStatus(Ship* ship, int index)
{
	if (!ship || ship->Weapons().size() <= index)
		return -1;

	WeaponGroup* group = ship->Weapons().at(index);

	int  status = SimSystem::NOMINAL;
	bool maint = false;

	ListIter<Weapon> iter = group->GetWeapons();
	while (++iter) {
		Weapon* s = iter.value();

		if (s->Status() < status)
			status = s->Status();
		else if (s->Status() == SimSystem::MAINT)
			maint = true;
	}

	if (maint && status == SimSystem::NOMINAL)
		status = SimSystem::MAINT;

	return status;
}

static int GetSensorStatus(Ship* ship)
{
	if (!ship || ship->GetSensor() == 0)
		return -1;

	Sensor* s = ship->GetSensor();
	Weapon* p = ship->GetProbeLauncher();

	int  status = s->Status();
	bool maint = (s->Status() == SimSystem::MAINT);

	if (p) {
		if (p->Status() < status)
			status = p->Status();
		else if (p->Status() == SimSystem::MAINT)
			maint = true;
	}

	if (maint && status == SimSystem::NOMINAL)
		status = SimSystem::MAINT;

	return status;
}

static int GetComputerStatus(Ship* ship)
{
	if (!ship || ship->Computers().size() < 1)
		return -1;

	int  status = SimSystem::NOMINAL;
	bool maint = false;

	ListIter<Computer> iter = ship->Computers();
	while (++iter) {
		Computer* s = iter.value();

		if (s->Status() < status)
			status = s->Status();
		else if (s->Status() == SimSystem::MAINT)
			maint = true;
	}

	if (ship->GetNavSystem()) {
		NavSystem* s = ship->GetNavSystem();

		if (s->Status() < status)
			status = s->Status();
		else if (s->Status() == SimSystem::MAINT)
			maint = true;
	}

	if (maint && status == SimSystem::NOMINAL)
		status = SimSystem::MAINT;

	return status;
}

static int GetFlightDeckStatus(Ship* ship)
{
	if (!ship || ship->FlightDecks().size() < 1)
		return -1;

	int  status = SimSystem::NOMINAL;
	bool maint = false;

	ListIter<FlightDeck> iter = ship->FlightDecks();
	while (++iter) {
		FlightDeck* s = iter.value();

		if (s->Status() < status)
			status = s->Status();
		else if (s->Status() == SimSystem::MAINT)
			maint = true;
	}

	if (maint && status == SimSystem::NOMINAL)
		status = SimSystem::MAINT;

	return status;
}
 
// +--------------------------------------------------------------------+

void
UHUDView::DrawWarningPanel()
{
	int box_width = 75;
	int box_height = 17;
	int row_height = 28;
	int box_left = width / 2 - box_width * 2;

	if (cockpit_hud_texture) {
		box_left = 275;
		box_height = 18;
		row_height = 18;
	}

	if (ship) {
		if (Game::MaxTexSize() > 128) {
			warn_left_sprite->Show();
			warn_right_sprite->Show();
		}

		int x = box_left;
		int y = cockpit_hud_texture ? 410 : height - 97;
		int c = cockpit_hud_texture ? 3 : 4;

		static DWORD blink = Game::RealTime();

		for (int index = 0; index < 12; index++) {
			int  stat = -1;
			Text abrv = Game::GetText("HUDView.UNKNOWN");

			switch (index) {
			case 0:  stat = GetReactorStatus(ship);    abrv = Game::GetText("HUDView.REACTOR");  break;
			case 1:  stat = GetDriveStatus(ship);      abrv = Game::GetText("HUDView.DRIVE");    break;
			case 2:  stat = GetQuantumStatus(ship);    abrv = Game::GetText("HUDView.QUANTUM");  break;
			case 3:  stat = GetShieldStatus(ship);     abrv = Game::GetText("HUDView.SHIELD");
				if (ship->GetShield() == 0 && ship->GetDecoy())
					abrv = Game::GetText("HUDView.DECOY");
				break;

			case 4:
			case 5:
			case 6:
			case 7:  stat = GetWeaponStatus(ship, index - 4);
				if (stat >= 0) {
					WeaponGroup* g = ship->Weapons().at(index - 4);
					abrv = g->Name();
				}
				break;

			case 8:  stat = GetSensorStatus(ship);     abrv = Game::GetText("HUDView.SENSOR");   break;
			case 9:  stat = GetComputerStatus(ship);   abrv = Game::GetText("HUDView.COMPUTER"); break;
			case 10: stat = GetThrusterStatus(ship);   abrv = Game::GetText("HUDView.THRUSTER"); break;
			case 11: stat = GetFlightDeckStatus(ship); abrv = Game::GetText("HUDView.FLTDECK");  break;
			}

			Rect warn_rect(x, y, box_width, box_height);

			if (cockpit_hud_texture)
				cockpit_hud_texture->DrawRect(warn_rect, FColor(64, 64, 64));

			if (stat >= 0) {
				SetStatusColor((SYSTEM_STATUS)stat);
				FColor tc = status_color;

				if (stat != (int)SYSTEM_STATUS::NOMINAL) {
					if (Game::RealTime() - blink < 250) {
						tc = cockpit_hud_texture ? txt_color : FColor(8, 8, 8);
					}
				}

				if (cockpit_hud_texture) {
					if (tc != txt_color) {
						Rect r2 = warn_rect;
						r2.Inset(1, 1, 1, 1);
						cockpit_hud_texture->FillRect(r2, tc);
						tc = FColor::Black;
					}

					warn_rect.y += 4;

					hud_font->SetColor(tc);
					hud_font->DrawText(abrv, -1,
						warn_rect,
						DT_CENTER | DT_SINGLELINE,
						cockpit_hud_texture);

					warn_rect.y -= 4;
				}
				else {
					DrawHUDText(TXT_CAUTION_TXT + index,
						abrv,
						warn_rect,
						DT_CENTER);

					hud_text[TXT_CAUTION_TXT + index].color = tc;
				}
			}

			x += box_width;

			if (--c <= 0) {
				c = cockpit_hud_texture ? 3 : 4;
				x = box_left;
				y += row_height;
			}
		}

		if (Game::RealTime() - blink > 500)
			blink = Game::RealTime();

		// reset for next time
		SetStatusColor(SYSTEM_STATUS::NOMINAL);
	}
}

// +--------------------------------------------------------------------+

void
UHUDView::DrawInstructions()
{
	if (!ship) return;

	if (Game::MaxTexSize() > 128) {
		instr_left_sprite->Show();
		instr_right_sprite->Show();
	}

	int          ninst = 0;
	int          nobj = 0;
	SimElement* elem = ship->GetElement();

	if (elem) {
		ninst = elem->NumInstructions();
		nobj = elem->NumObjectives();
	}

	Rect r = Rect(width / 2 - 143, height - 105, 290, 17);

	if (ninst) {
		int npages = ninst / 6 + (ninst % 6 ? 1 : 0);

		if (inst_page >= npages)
			inst_page = npages - 1;
		else if (inst_page < 0)
			inst_page = 0;

		int first = inst_page * 6;
		int last = first + 6;
		if (last > ninst) last = ninst;

		int n = TXT_CAUTION_TXT;

		for (int i = first; i < last; i++) {
			hud_text[n].color = standard_txt_colors[color];
			DrawHUDText(n++, FormatInstruction(elem->GetInstruction(i)), r, DT_LEFT, HUD_MIXED_CASE);
			r.y += 14;
		}

		char page[32];
		sprintf_s(page, "%d / %d", inst_page + 1, npages);
		r = Rect(width / 2 + 40, height - 16, 110, 16);
		DrawHUDText(TXT_INSTR_PAGE, page, r, DT_CENTER, HUD_MIXED_CASE);
	}

	else if (nobj) {
		int n = TXT_CAUTION_TXT;

		for (int i = 0; i < nobj; i++) {
			char desc[256];
			sprintf_s(desc, "* %s", elem->GetObjective(i)->GetShortDescription());
			hud_text[n].color = standard_txt_colors[color];
			DrawHUDText(n++, desc, r, DT_LEFT, HUD_MIXED_CASE);
			r.y += 14;
		}
	}

	else {
		hud_text[TXT_CAUTION_TXT].color = standard_txt_colors[color];
		DrawHUDText(TXT_CAUTION_TXT, Game::GetText("HUDView.No-Instructions"), r, DT_LEFT, HUD_MIXED_CASE);
	}
}

// +--------------------------------------------------------------------+

const char*
UHUDView::FormatInstruction(Text instr)
{
	if (!instr.contains('$'))
		return (const char*)instr;

	static char result[256];

	const char* s = (const char*)instr;
	char* d = result;

	KeyMap& keymap = Starshatter::GetInstance()->GetKeyMap();

	while (*s) {
		if (*s == '$') {
			s++;
			char action[32];
			char* a = action;
			while (*s && (isupper(*s) || isdigit(*s) || *s == '_')) *a++ = *s++;
			*a = 0;

			int         act = KeyMap::GetKeyAction(action);
			int         key = keymap.FindMapIndex(act);
			const char* s2 = keymap.DescribeKey(key);

			if (!s2) s2 = action;
			while (*s2) *d++ = *s2++;
		}
		else {
			*d++ = *s++;
		}
	}

	*d = 0;

	return result;
}

// +--------------------------------------------------------------------+

void
UHUDView::CycleInstructions(int direction)
{
	if (direction > 0)
		inst_page++;
	else
		inst_page--;
}

// +--------------------------------------------------------------------+

void
UHUDView::DrawMessages()
{
	int message_queue_empty = true;

	// age messages:
	for (int i = 0; i < MAX_MSG; i++) {
		if (msg_time[i] > 0) {
			msg_time[i] -= Game::GUITime();

			if (msg_time[i] <= 0) {
				msg_time[i] = 0;
				msg_text[i] = "";
			}

			message_queue_empty = false;
		}
	}

	if (!message_queue_empty) {
		// advance message pipeline:
		for (int i = 0; i < MAX_MSG; i++) {
			if (msg_time[0] == 0) {
				for (int j = 0; j < MAX_MSG - 1; j++) {
					msg_time[j] = msg_time[j + 1];
					msg_text[j] = msg_text[j + 1];
				}

				msg_time[MAX_MSG - 1] = 0;
				msg_text[MAX_MSG - 1] = "";
			}
		}

		// draw messages:
		for (int i = 0; i < MAX_MSG; i++) {
			int index = TXT_MSG_1 + i;

			if (msg_time[i] > 0) {
				Rect msg_rect(10, 95 + i * 10, width - 20, 12);
				DrawHUDText(index, msg_text[i], msg_rect, DT_LEFT, HUD_MIXED_CASE);

				if (msg_time[i] > 1)
					hud_text[index].color = txt_color;
				else
					{
						const float Alpha = FMath::Clamp(0.5f + 0.5f * (float)msg_time[i], 0.0f, 1.0f);

						hud_text[index].color = FColor(
							(uint8)FMath::Clamp(FMath::RoundToInt(txt_color.R * Alpha), 0, 255),
							(uint8)FMath::Clamp(FMath::RoundToInt(txt_color.G * Alpha), 0, 255),
							(uint8)FMath::Clamp(FMath::RoundToInt(txt_color.B * Alpha), 0, 255),
							txt_color.A
						);
					}
			}
		}
	}
}

// +--------------------------------------------------------------------+

void
UHUDView::DrawNav()
{
	if (!sim)
		return;

	active_region = sim->GetActiveRegion();

	if (ship) {
		int          nav_index = 1;
		Instruction* next = ship->GetNextNavPoint();

		if (mode == (int)HUD_MODE::HUD_MODE_NAV) {
			if (next && next->Action() == Instruction::LAUNCH)
				DrawNavPoint(*next, 0, true);

			ListIter<Instruction> navpt = ship->GetFlightPlan();
			while (++navpt) {
				DrawNavPoint(*navpt.value(), nav_index++, (navpt.value() == next));
			}
		}
		else if (next) {
			DrawNavPoint(*next, 0, true);
		}
	}
}

void
UHUDView::DrawILS()
{
	if (ship) {
		bool hoops_drawn = false;
		bool same_sector = false;

		InboundSlot* inbound = ship->GetInbound();
		if (inbound) {
			FlightDeck* fd = inbound->GetDeck();

			if (fd && fd->IsRecoveryDeck() && fd->GetCarrier()) {
				if (fd->GetCarrier()->GetRegion() == ship->GetRegion())
					same_sector = true;

				if (same_sector && mode == (int)HUD_MODE::HUD_MODE_ILS && !transition && !docking) {
					FVector dst = fd->MountLocation();
					projector->Transform(dst);

					if (dst.Z > 1.0f) {
						projector->Project(dst);

						int x = (int)dst.X;
						int y = (int)dst.Y;

						if (x > 4 && x < width - 4 &&
							y > 4 && y < height - 4) {

							this->window->DrawLine(x - 6, y - 6, x + 6, y + 6, hud_color);
							window->DrawLine(x + 6, y - 6, x - 6, y + 6, hud_color);
						}
					}
				}

				// draw the hoops for this flight deck:
				SimScene* scene = camview->GetScene();
				for (int h = 0; h < fd->NumHoops(); h++) {
					Hoop* hoop = fd->GetHoops() + h;
					if (hoop && scene) {
						if (same_sector && mode == (int)HUD_MODE::HUD_MODE_ILS && !transition && !docking) {
							scene->AddGraphic(hoop);
							hoop->Show();

							hoops_drawn = true;
						}
						else {
							hoop->Hide();
							scene->DelGraphic(hoop);
						}
					}
				}
			}
		}

		if (!hoops_drawn) {
			ListIter<Ship> iter = ship->GetRegion()->GetCarriers();
			while (++iter) {
				Ship* carrier = iter.value();

				bool ours = (carrier->GetIFF() == ship->GetIFF()) ||
					(carrier->GetIFF() == 0);

				for (int i = 0; i < carrier->NumFlightDecks(); i++) {
					FlightDeck* fd = carrier->GetFlightDeck(i);

					if (fd && fd->IsRecoveryDeck()) {
						if (mode == (int)HUD_MODE::vHUD_MODE_ILS && ours && !transition && !docking) {
							FVector dst = fd->MountLocation();
							projector->Transform(dst);

							if (dst.Z > 1.0f) {
								projector->Project(dst);

								int x = (int)dst.X;
								int y = (int)dst.Y;

								if (x > 4 && x < width - 4 &&
									y > 4 && y < height - 4) {

									window->DrawLine(x - 6, y - 6, x + 6, y + 6, hud_color);
									window->DrawLine(x + 6, y - 6, x - 6, y + 6, hud_color);
								}
							}
						}

						// draw the hoops for this flight deck:
						SimScene* scene = camview->GetScene();
						for (int h = 0; h < fd->NumHoops(); h++) {
							Hoop* hoop = fd->GetHoops() + h;
							if (hoop && scene) {
								if (mode == (int)HUD_MODE::HUD_MODE_ILS && ours && !transition && !docking) {
									hoop->Show();
									if (!hoop->GetScene())
										scene->AddGraphic(hoop);
								}
								else {
									hoop->Hide();
									if (hoop->GetScene())
										scene->DelGraphic(hoop);
								}
							}
						}
					}
				}
			}
		}
	}
}

void
UHUDView::DrawObjective()
{
	if (ship && ship->GetDirector() && ship->GetDirector()->Type() >= SteerAI::SEEKER) {
		SteerAI* steer = (SteerAI*)ship->GetDirector();
		FVector  obj = steer->GetObjective();
		projector->Transform(obj);

		if (obj.Z > 1.0f) {
			projector->Project(obj);

			int x = (int)obj.X;
			int y = (int)obj.Y;

			if (x > 4 && x < width - 4 &&
				y > 4 && y < height - 4) {

				Color c = Color::Cyan;
				window->DrawRect(x - 6, y - 6, x + 6, y + 6, c);
				window->DrawLine(x - 6, y - 6, x + 6, y + 6, c);
				window->DrawLine(x + 6, y - 6, x - 6, y + 6, c);
			}
		}

		if (steer->GetOther()) {
			obj = steer->GetOther()->Location();
			projector->Transform(obj);

			if (obj.Z > 1.0f) {
				projector->Project(obj);

				int x = (int)obj.X;
				int y = (int)obj.Y;

				if (x > 4 && x < width - 4 &&
					y > 4 && y < height - 4) {

					Color c = Color::Orange;
					window->DrawRect(x - 6, y - 6, x + 6, y + 6, c);
					window->DrawLine(x - 6, y - 6, x + 6, y + 6, c);
					window->DrawLine(x + 6, y - 6, x - 6, y + 6, c);
				}
			}
		}
	}
	/***/
}

void
UHUDView::DrawNavPoint(Instruction& navpt, int index, int next)
{
	if (index >= 15 || !navpt.Region()) return;

	// transform from starsystem to world coordinates:
	FVector npt = navpt.Region()->GetLocation() + navpt.Location();

	if (active_region)
		npt -= active_region->GetLocation();

	npt = npt.OtherHand();

	// transform from world to camera:
	projector->Transform(npt);

	// clip:
	if (npt.Z > 1.0f) {

		// project:
		projector->Project(npt);

		int x = (int)npt.X;
		int y = (int)npt.Y;

		// clip:
		if (x > 4 && x < width - 4 &&
			y > 4 && y < height - 4) {

			Color c = Color::White;
			if (navpt.Status() > Instruction::ACTIVE && navpt.HoldTime() <= 0)
				c = Color::DarkGray;

			// draw:
			if (next)
				window->DrawEllipse(x - 6, y - 6, x + 5, y + 5, c);

			window->DrawLine(x - 6, y - 6, x + 6, y + 6, c);
			window->DrawLine(x + 6, y - 6, x - 6, y + 6, c);

			if (index > 0) {
				char npt_buf[32];
				Rect npt_rect(x + 10, y - 4, 200, 12);

				if (navpt.Status() == Instruction::COMPLETE && navpt.HoldTime() > 0) {
					char hold_time[32];
					FormatTime(hold_time, navpt.HoldTime());
					sprintf_s(npt_buf, "%d %s", index, hold_time);
				}
				else {
					sprintf_s(npt_buf, "%d", index);
				}

				DrawHUDText(TXT_NAV_PT + index, npt_buf, npt_rect, DT_LEFT);
			}
		}
	}

	if (next && mode == (int)HUD_MODE::HUD_MODE_NAV && navpt.Region() == ship->GetRegion()) {

		// Translate into camera relative:
		FVector tloc = navpt.Location().OtherHand();
		projector->Transform(tloc);

		int behind = tloc.Z < 0;

		if (behind)
			tloc.Z = -tloc.Z;

		// Project into screen coordinates:
		projector->Project(tloc);

		// DRAW THE OFFSCREEN CHASE INDICATOR:
		if (behind ||
			tloc.X <= 0 || tloc.X >= width - 1 ||
			tloc.Y <= 0 || tloc.Y >= height - 1) {

			// Left side:
			if (tloc.X <= 0 || (behind && tloc.X < width / 2)) {
				if (tloc.Y < ah) tloc.Y = ah;
				else if (tloc.Y >= height - ah) tloc.Y = height - 1 - ah;

				chase_sprite->Show();
				chase_sprite->SetAnimation(&chase_left);
				chase_sprite->MoveTo(FVector((float)aw, (float)tloc.Y, 1.0f));
			}

			// Right side:
			else if (tloc.X >= width - 1 || behind) {
				if (tloc.Y < ah) tloc.Y = ah;
				else if (tloc.Y >= height - ah) tloc.Y = height - 1 - ah;

				chase_sprite->Show();
				chase_sprite->SetAnimation(&chase_right);
				chase_sprite->MoveTo(FVector((float)(width - 1 - aw), (float)tloc.Y, 1.0f));
			}
			else {
				if (tloc.X < aw) tloc.X = aw;
				else if (tloc.X >= width - aw) tloc.X = width - 1 - aw;

				// Top edge:
				if (tloc.Y <= 0) {
					chase_sprite->Show();
					chase_sprite->SetAnimation(&chase_top);
					chase_sprite->MoveTo(FVector((float)tloc.X, (float)ah, 1.0f));
				}

				// Bottom edge:
				else if (tloc.Y >= height - 1) {
					chase_sprite->Show();
					chase_sprite->SetAnimation(&chase_bottom);
					chase_sprite->MoveTo(FVector((float)tloc.X, (float)(height - 1 - ah), 1.0f));
				}
			}
		}
	}
}

// +--------------------------------------------------------------------+

void
UHUDView::SetShip(Ship* s)
{
	if (ship != s) {
		double new_scale = 1;

		ship_status = -1;
		ship = s;

		if (ship) {
			if (ship->Life() == 0 || ship->IsDying() || ship->IsDead()) {
				ship = 0;
			}
			else {
				Observe(ship);
				new_scale = 1.1 * ship->Radius() / 64;

				if (ship->Design()->hud_icon.Width()) {
					TransferBitmap(ship->Design()->hud_icon, icon_ship, icon_ship_shade);
					ColorizeBitmap(icon_ship, icon_ship_shade, txt_color);
				}
			}
		}

		if (az_ring) {
			az_ring->Rescale(1 / compass_scale);
			az_ring->Rescale(new_scale);
		}

		if (az_pointer) {
			az_pointer->Rescale(1 / compass_scale);
			az_pointer->Rescale(new_scale);
		}

		if (el_ring) {
			el_ring->Rescale(1 / compass_scale);
			el_ring->Rescale(new_scale);
		}

		if (el_pointer) {
			el_pointer->Rescale(1 / compass_scale);
			el_pointer->Rescale(new_scale);
		}

		compass_scale = new_scale;
		inst_page = 0;

		if (ship && ship->GetElement() && ship->GetElement()->NumInstructions() > 0)
			if (!show_inst)
				CycleHUDInst();
	}

	else if (ship && ship->Design()->hud_icon.Width()) {
		bool update = false;
		SimSystem::STATUS s = SimSystem::NOMINAL;
		int integrity = (int)(ship->Integrity() / ship->Design()->integrity * 100);

		if (integrity < 30)        s = SimSystem::CRITICAL;
		else if (integrity < 60)   s = SimSystem::DEGRADED;

		if (s != ship_status) {
			ship_status = s;
			update = true;
		}

		if (update) {
			SetStatusColor((SYSTEM_STATUS)ship_status);
			ColorizeBitmap(icon_ship, icon_ship_shade, status_color);
		}
	}

	if (ship && ship->Cockpit()) {
		Solid* cockpit = (Solid*)ship->Cockpit();

		bool change = false;

		if (cockpit->Hidden()) {
			if (cockpit_hud_texture)
				change = true;

			cockpit_hud_texture = 0;
		}
		else {
			if (!cockpit_hud_texture)
				change = true;

			Model* cockpit_model = cockpit->GetModel();
			Material* hud_material = 0;

			if (cockpit_model) {
				hud_material = (Material*)cockpit_model->FindMaterial("HUD");
				if (hud_material) {
					cockpit_hud_texture = hud_material->tex_emissive;
				}
			}
		}

		if (change) {
			SetHUDColorSet(color);
		}
	}
}

void
UHUDView::SetTarget(SimObject* t)
{
	bool update = false;

	if (target != t) {
		tgt_status = -1;
		target = t;
		if (target) Observe(target);
		update = true;
	}

	if (target && target->Type() == SimObject::SIM_SHIP) {
		SYSTEM_STATUS s = SYSTEM_STATUS::NOMINAL;
		Ship* tship = (Ship*)target;
		int integrity = (int)(tship->Integrity() / tship->Design()->integrity * 100);

		if (integrity < 30)        s = SYSTEM_STATUS::CRITICAL;
		else if (integrity < 60)   s = SYSTEM_STATUS::DEGRADED;

		if ((int) s != tgt_status) {
			tgt_status = (int)s;
			update = true;
		}
	}

	if (update) {
		if (target && target->Type() == SimObject::SIM_SHIP) {
			Ship* tship = (Ship*)target;
			TransferBitmap(tship->Design()->hud_icon, icon_target, icon_target_shade);
		}
		else {
			PrepareBitmap("hud_icon.pcx", icon_target, icon_target_shade);
		}

		SetStatusColor((SYSTEM_STATUS)tgt_status);
		ColorizeBitmap(icon_target, icon_target_shade, status_color);
	}
}

// +--------------------------------------------------------------------+

UMFDView*
UHUDView::GetMFD(int n) const
{
	if (n >= 0 && n < 3)
		return mfd[n];

	return 0;
}

// +--------------------------------------------------------------------+

void
UHUDView::Refresh()
{
	sim = Sim::GetSim();
	mouse_in = false;

	if (!sim || !camview || !projector) {
		return;
	}

	if (Mouse::LButton() == 0) {
		mouse_latch = 0;
		mouse_index = -1;
	}

	int mouse_index_old = mouse_index;

	SetShip(sim->GetPlayerShip());

	if (mode == (int)HUD_MODE::HUD_MODE_OFF) {
		if (cockpit_hud_texture) {
			cockpit_hud_texture->FillRect(0, 0, 512, 256, Color::Black);
		}

		sim->ShowGrid(false);
		return;
	}

	if (cockpit_hud_texture && cockpit_hud_texture->Width() == 512) {
		Bitmap* hud_bmp = 0;

		if (hud_sprite[0]) {
			hud_bmp = hud_sprite[0]->Frame();
			int bmp_w = hud_bmp->Width();
			int bmp_h = hud_bmp->Height();

			cockpit_hud_texture->BitBlt(0, 0, *hud_bmp, 0, 0, bmp_w, bmp_h);
		}

		if (hud_sprite[1]) {
			hud_bmp = hud_sprite[1]->Frame();
			int bmp_w = hud_bmp->Width();
			int bmp_h = hud_bmp->Height();

			cockpit_hud_texture->BitBlt(256, 0, *hud_bmp, 0, 0, bmp_w, bmp_h);
		}

		if (hud_sprite[6]) {
			if (hud_sprite[6]->Hidden()) {
				cockpit_hud_texture->FillRect(0, 384, 128, 512, Color::Black);
			}
			else {
				hud_bmp = hud_sprite[6]->Frame();
				int bmp_w = hud_bmp->Width();
				int bmp_h = hud_bmp->Height();

				cockpit_hud_texture->BitBlt(0, 384, *hud_bmp, 0, 0, bmp_w, bmp_h);
			}
		}

		if (hud_sprite[7]) {
			if (hud_sprite[7]->Hidden()) {
				cockpit_hud_texture->FillRect(128, 384, 256, 512, Color::Black);
			}
			else {
				hud_bmp = hud_sprite[7]->Frame();
				int bmp_w = hud_bmp->Width();
				int bmp_h = hud_bmp->Height();

				cockpit_hud_texture->BitBlt(128, 384, *hud_bmp, 0, 0, bmp_w, bmp_h);
			}
		}

		for (int i = 8; i < 32; i++) {
			if (hud_sprite[i] && !hud_sprite[i]->Hidden()) {
				Sprite* spr = hud_sprite[i];

				int cx = (int)spr->Location().X;
				int cy = (int)spr->Location().Y;
				int w2 = spr->Width() / 2;
				int h2 = spr->Height() / 2;

				window->DrawBitmap(cx - w2, cy - h2, cx + w2, cy + h2, spr->Frame(), Video::BLEND_ALPHA);
			}
		}
	}
	else {
		for (int i = 0; i < 32; i++) {
			if (hud_sprite[i] && !hud_sprite[i]->Hidden()) {
				Sprite* spr = hud_sprite[i];

				int cx = (int)spr->Location().X;
				int cy = (int)spr->Location().Y;
				int w2 = spr->Width() / 2;
				int h2 = spr->Height() / 2;

				window->DrawBitmap(cx - w2, cy - h2, cx + w2, cy + h2, spr->Frame(), Video::BLEND_ALPHA);
			}
		}

		Video* video = Video::GetInstance();

		for (int i = 0; i < 31; i++) {
			Sprite* spr = pitch_ladder[i];

			if (spr && !spr->Hidden()) {
				spr->Render2D(video);
			}
		}
	}

	//DrawStarSystem();
	DrawMessages();

	if (ship) {
		// no hud in transition:
		if (ship->InTransition()) {
			transition = true;
			HideAll();
			return;
		}

		else if (transition) {
			transition = false;
			RestoreHUD();
		}

		CameraManager* cam_dir = CameraManager::GetInstance();

		// everything is off during docking, except the final message:
		if (cam_dir && cam_dir->GetMode() == CameraManager::MODE_DOCKING) {
			docking = true;
			HideAll();

			if (ship->GetFlightPhase() == Ship::DOCKING) {
				Rect dock_rect(width / 2 - 100, height / 6, 200, 20);

				if (ship->IsAirborne())
					DrawHUDText(TXT_AUTO, Game::GetText("HUDView.SUCCESSFUL-LANDING"), dock_rect, DT_CENTER);
				else
					DrawHUDText(TXT_AUTO, Game::GetText("HUDView.DOCKING-COMPLETE"), dock_rect, DT_CENTER);
			}
			return;
		}
		else if (docking) {
			docking = false;
			RestoreHUD();
		}

		// go to NAV mode during autopilot:
		if (ship->GetNavSystem() && ship->GetNavSystem()->AutoNavEngaged() && !arcade)
			mode = (int)HUD_MODE::HUD_MODE_NAV;

		SetTarget(ship->GetTarget());

		// internal view of HUD reticule
		if (CameraManager::GetCameraMode() <= CameraManager::MODE_CHASE)
			SetTacticalMode(0);

		// external view
		else
			SetTacticalMode(!cockpit_hud_texture);

		sim->ShowGrid(tactical &&
			!ship->IsAirborne() &&
			CameraManager::GetCameraMode() != CameraManager::MODE_VIRTUAL);

		// draw HUD bars:
		DrawBars();

		if (missile_lock_sound) {
			if (threat > 1) {
				long max_vol = AudioConfig::WrnVolume();
				long volume = -1500;

				if (volume > max_vol)
					volume = max_vol;

				missile_lock_sound->SetVolume(volume);
				missile_lock_sound->Play();
			}
			else {
				missile_lock_sound->Stop();
			}
		}

		DrawNav();
		DrawILS();

		// FOR DEBUG PURPOSES ONLY:
		// DrawObjective();

		if (!overlay) {
			Rect fov_rect(0, 10, width, 10);
			int  fov_degrees = 180 - 2 * (int)(projector->XAngle() * 180 / PI);

			if (fov_degrees > 90)
				DrawHUDText(TXT_CAM_ANGLE, Game::GetText("HUDView.Wide-Angle"), fov_rect, DT_CENTER);

			fov_rect.y = 20;
			DrawHUDText(TXT_CAM_MODE, CameraManager::GetModeName(), fov_rect, DT_CENTER);
		}

		DrawMFDs();

		instr_left_sprite->Hide();
		instr_right_sprite->Hide();
		warn_left_sprite->Hide();
		warn_right_sprite->Hide();

		if (cockpit_hud_texture)
			cockpit_hud_texture->FillRect(256, 384, 512, 512, Color::Black);

		if (show_inst) {
			DrawInstructions();
		}

		else if (!arcade) {
			if (ship->MasterCaution() && !show_warn)
				ShowHUDWarn();

			if (show_warn)
				DrawWarningPanel();
		}

		if (width > 640 || (!show_inst && !show_warn)) {
			Rect icon_rect(120, height - 24, 128, 16);

			if (ship)
				DrawHUDText(TXT_ICON_SHIP_TYPE, ship->DesignName(), icon_rect, DT_CENTER);

			icon_rect.x = width - 248;

			if (target && target->Type() == SimObject::SIM_SHIP) {
				Ship* tship = (Ship*)target;
				DrawHUDText(TXT_ICON_TARGET_TYPE, tship->DesignName(), icon_rect, DT_CENTER);
			}
		}
	}
	else {
		if (target) {
			SetTarget(0);
		}
	}

	// latch mouse down to prevent dragging into a control:
	if (Mouse::LButton() == 1)
		mouse_latch = 1;

	if (mouse_index > -1 && mouse_index_old != mouse_index)
		MouseFrame();
}

void
UHUDView::DrawMFDs()
{
	for (int i = 0; i < 3; i++) {
		mfd[i]->Show();
		mfd[i]->SetShip(ship);
		mfd[i]->SetCockpitHUDTexture(cockpit_hud_texture);
		mfd[i]->Draw();
	}
}

// +--------------------------------------------------------------------+

void
UHUDView::DrawStarSystem()
{
	if (sim && sim->GetStarSystem()) {
		StarSystem* sys = sim->GetStarSystem();

		ListIter<OrbitalBody> iter = sys->Bodies();
		while (++iter) {
			OrbitalBody* body = iter.value();
			DrawOrbitalBody(body);
		}
	}
}

void
UHUDView::DrawOrbitalBody(OrbitalBody* body)
{
	if (body) {
		FVector p = body->Rep()->Location();

		projector->Transform(p);

		if (p.Z > 100.0f) {
			float r = (float)body->Radius();
			r = projector->ProjectRadius(p, r);
			projector->Project(p, false);

			window->DrawEllipse((int)(p.X - r),
				(int)(p.Y - r),
				(int)(p.X + r),
				(int)(p.Y + r),
				Color::Cyan);
		}

		ListIter<OrbitalBody> iter = body->Satellites();
		while (++iter) {
			OrbitalBody* sat = iter.value();
			DrawOrbitalBody(sat);
		}
	}
}

// +--------------------------------------------------------------------+

void
UHUDView::ExecFrame(float DeltaTime)
{
	// update the position of HUD elements that are
	// part of the 3D scene (like fpm and lcos sprites)
	HideCompass();

	if (ship && !transition && !docking && mode != (int)HUD_MODE::HUD_MODE_OFF) {
		PlayerCharacter* p = PlayerCharacter::GetCurrentPlayer();
		gunsight = p->Gunsight();

		if (ship->IsStarship()) {
			if (tactical) {
				hud_left_sprite->Hide();
				hud_right_sprite->Hide();
			}

			else if (hud_left_sprite->Frame() != &hud_left_starship) {
				hud_left_sprite->SetAnimation(&hud_left_starship);
				hud_right_sprite->SetAnimation(&hud_right_starship);

				hud_left_sprite->MoveTo(FVector((float)(width / 2 - 128), (float)(height / 2), 1.0f));
				hud_right_sprite->MoveTo(FVector((float)(width / 2 + 128), (float)(height / 2), 1.0f));
			}
		}

		else if (!ship->IsStarship()) {
			if (ship->IsAirborne() && hud_left_sprite->Frame() != &hud_left_air) {
				hud_left_sprite->SetAnimation(&hud_left_air);
				hud_right_sprite->SetAnimation(&hud_right_air);
			}

			else if (!ship->IsAirborne() && hud_left_sprite->Frame() != &hud_left_fighter) {
				hud_left_sprite->SetAnimation(&hud_left_fighter);
				hud_right_sprite->SetAnimation(&hud_right_fighter);
			}
		}

		if (!tactical) {
			if (Game::MaxTexSize() > 128) {
				hud_left_sprite->Show();
				hud_right_sprite->Show();
			}

			if (!arcade)
				DrawFPM();

			if (ship->IsStarship() && ship->GetFLCSMode() == Ship::FLCS_HELM)
				DrawHPM();
			else if (!arcade)
				DrawPitchLadder();
		}

		else {
			if (ship->IsStarship() && ship->GetFLCSMode() == Ship::FLCS_HELM)
				DrawCompass();
		}

		if (mode == (int)HUD_MODE::HUD_MODE_TAC) {
			DrawSight();
			DrawDesignators();
		}

		if (width > 640 || (!show_inst && !show_warn)) {
			icon_ship_sprite->Show();
			icon_target_sprite->Show();
		}
		else {
			icon_ship_sprite->Hide();
			icon_target_sprite->Hide();
		}
	}

	// if the hud is off or prohibited,
	// hide all of the sprites:

	else {
		hud_left_sprite->Hide();
		hud_right_sprite->Hide();
		instr_left_sprite->Hide();
		instr_right_sprite->Hide();
		warn_left_sprite->Hide();
		warn_right_sprite->Hide();
		icon_ship_sprite->Hide();
		icon_target_sprite->Hide();
		fpm_sprite->Hide();
		hpm_sprite->Hide();
		lead_sprite->Hide();
		aim_sprite->Hide();
		tgt1_sprite->Hide();
		tgt2_sprite->Hide();
		tgt3_sprite->Hide();
		tgt4_sprite->Hide();
		chase_sprite->Hide();

		for (int i = 0; i < 3; i++)
			mfd[i]->Hide();

		for (int i = 0; i < 31; i++)
			pitch_ladder[i]->Hide();

		DrawILS();
	}
}

// +--------------------------------------------------------------------+

void
UHUDView::CycleMFDMode(int mfd_index)
{
	if (mfd_index < 0 || mfd_index > 2) return;

	int m = (int) mfd[mfd_index]->GetMode();
	m++;

	if (mfd_index == 2) {
		if (m > (int)EMFDMode::SHIP)
			m = (int)EMFDMode::OFF;
	}
	else {
		if (m > (int)EMFDMode::RADAR3D)
			m = (int)EMFDMode::OFF;

		if (m == (int)EMFDMode::GAME)
			m++;

		if (mfd_index != 0 && m == (int)EMFDMode::SHIP)
			m++;
	}

	mfd[mfd_index]->SetMode((int)m);
	HUDSounds::PlaySound(HUDSounds::SND_MFD_MODE);
}

// +--------------------------------------------------------------------+

void
UHUDView::ShowHUDWarn()
{
	if (!show_warn) {
		show_warn = true;

		if (ship && ship->HullStrength() <= 40) {
			// TOO OBNOXIOUS!!
			HUDSounds::PlaySound(HUDSounds::SND_RED_ALERT);
		}
	}
}

void
UHUDView::ShowHUDInst()
{
	show_inst = true;
}

// +--------------------------------------------------------------------+

void
UHUDView::HideHUDWarn()
{
	show_warn = false;

	if (ship) {
		ship->ClearCaution();
		HUDSounds::StopSound(HUDSounds::SND_RED_ALERT);
	}
}

void
UHUDView::HideHUDInst()
{
	show_inst = false;
}

// +--------------------------------------------------------------------+

void
UHUDView::CycleHUDWarn()
{
	HUDSounds::PlaySound(HUDSounds::SND_HUD_WIDGET);
	show_warn = !show_warn;

	if (ship && !show_warn) {
		ship->ClearCaution();
		HUDSounds::StopSound(HUDSounds::SND_RED_ALERT);
	}
}

void
UHUDView::CycleHUDInst()
{
	show_inst = !show_inst;
	HUDSounds::PlaySound(HUDSounds::SND_HUD_WIDGET);
}

// +--------------------------------------------------------------------+

void
UHUDView::SetHUDMode(int m)
{
	if (mode != m) {
		mode = m;

		if (mode > (int) HUD_MODE::HUD_MODE_ILS || mode < (int)HUD_MODE::HUD_MODE_OFF)
			mode = (int) HUD_MODE::HUD_MODE_OFF;

		if (ship && !ship->IsDropship() && mode == (int)HUD_MODE::HUD_MODE_ILS)
			mode = (int)HUD_MODE::HUD_MODE_OFF;

		RestoreHUD();
	}
}

void
UHUDView::CycleHUDMode()
{
	mode++;

	if (arcade && mode != (int)HUD_MODE::HUD_MODE_TAC)
		mode = (int)HUD_MODE::HUD_MODE_OFF;

	else if (mode > (int)HUD_MODE::HUD_MODE_ILS || mode < (int)HUD_MODE::HUD_MODE_OFF)
		mode = (int)HUD_MODE::HUD_MODE_OFF;

	else if (!ship->IsDropship() && mode == (int)HUD_MODE::HUD_MODE_ILS)
		mode = (int)HUD_MODE::HUD_MODE_OFF;

	RestoreHUD();
	HUDSounds::PlaySound(HUDSounds::SND_HUD_MODE);
}

void
UHUDView::RestoreHUD()
{
	if (mode == (int)HUD_MODE::HUD_MODE_OFF) {
		HideAll();
	}
	else {
		for (int i = 0; i < 3; i++)
			mfd[i]->Show();

		if (width > 640 || (!show_inst && !show_warn)) {
			icon_ship_sprite->Show();
			icon_target_sprite->Show();
		}
		else {
			icon_ship_sprite->Hide();
			icon_target_sprite->Hide();
		}

		if (!tactical && Game::MaxTexSize() > 128) {
			hud_left_sprite->Show();
			hud_right_sprite->Show();
		}

		fpm_sprite->Show();

		if (ship && ship->IsStarship())
			hpm_sprite->Show();

		if (gunsight == 0)
			aim_sprite->Show();
		else
			lead_sprite->Show();
	}
}

void
UHUDView::HideAll()
{
	for (int i = 0; i < 3; i++)
		mfd[i]->Hide();

	hud_left_sprite->Hide();
	hud_right_sprite->Hide();
	instr_left_sprite->Hide();
	instr_right_sprite->Hide();
	warn_left_sprite->Hide();
	warn_right_sprite->Hide();
	icon_ship_sprite->Hide();
	icon_target_sprite->Hide();
	fpm_sprite->Hide();
	hpm_sprite->Hide();
	lead_sprite->Hide();
	aim_sprite->Hide();
	tgt1_sprite->Hide();
	tgt2_sprite->Hide();
	tgt3_sprite->Hide();
	tgt4_sprite->Hide();
	chase_sprite->Hide();

	sim->ShowGrid(false);

	for (int i = 0; i < 31; i++)
		pitch_ladder[i]->Hide();

	if (missile_lock_sound)
		missile_lock_sound->Stop();

	HideCompass();
	DrawILS();
	Mouse::Show(false);
}

// +--------------------------------------------------------------------+

FColor
UHUDView::Ambient() const
{
	if (!sim || !ship || mode == (int)HUD_MODE::HUD_MODE_OFF)
		return FColor::Black;

	SimRegion* rgn = sim->GetActiveRegion();

	if (!rgn || !rgn->IsAirSpace())
		return FColor::Black;

	FColor c = sim->GetStarSystem()->Ambient();

	if (c.R > 32 || c.G > 32 || c.B > 32)
		return FColor::Black;

	// if we get this far, the night-vision aid is on
	return night_vision_colors[color];
}

FColor
UHUDView::CycleHUDColor()
{
	HUDSounds::PlaySound(HUDSounds::SND_HUD_MODE);
	SetHUDColorSet(color + 1);
	return hud_color;
}

void
UHUDView::SetHUDColorSet(int c)
{
	color = c;
	if (color > NUM_HUD_COLORS - 1) color = 0;
	hud_color = standard_hud_colors[color];
	txt_color = standard_txt_colors[color];

	ColorizeBitmap(fpm, fpm_shade, hud_color, true);
	ColorizeBitmap(hpm, hpm_shade, hud_color, true);
	ColorizeBitmap(lead, lead_shade, txt_color * 1.25, true);
	ColorizeBitmap(cross, cross_shade, hud_color, true);
	ColorizeBitmap(cross1, cross1_shade, hud_color, true);
	ColorizeBitmap(cross2, cross2_shade, hud_color, true);
	ColorizeBitmap(cross3, cross3_shade, hud_color, true);
	ColorizeBitmap(cross4, cross4_shade, hud_color, true);

	if (Game::MaxTexSize() > 128) {
		ColorizeBitmap(hud_left_air, hud_left_shade_air, hud_color);
		ColorizeBitmap(hud_right_air, hud_right_shade_air, hud_color);
		ColorizeBitmap(hud_left_fighter, hud_left_shade_fighter, hud_color);
		ColorizeBitmap(hud_right_fighter, hud_right_shade_fighter, hud_color);
		ColorizeBitmap(hud_left_starship, hud_left_shade_starship, hud_color);
		ColorizeBitmap(hud_right_starship, hud_right_shade_starship, hud_color);

		ColorizeBitmap(instr_left, instr_left_shade, hud_color);
		ColorizeBitmap(instr_right, instr_right_shade, hud_color);
		ColorizeBitmap(warn_left, warn_left_shade, hud_color);
		ColorizeBitmap(warn_right, warn_right_shade, hud_color);

		ColorizeBitmap(pitch_ladder_pos, pitch_ladder_pos_shade, hud_color);
		ColorizeBitmap(pitch_ladder_neg, pitch_ladder_neg_shade, hud_color);
	}

	ColorizeBitmap(icon_ship, icon_ship_shade, txt_color);
	ColorizeBitmap(icon_target, icon_target_shade, txt_color);

	ColorizeBitmap(chase_left, chase_left_shade, hud_color, true);
	ColorizeBitmap(chase_right, chase_right_shade, hud_color, true);
	ColorizeBitmap(chase_top, chase_top_shade, hud_color, true);
	ColorizeBitmap(chase_bottom, chase_bottom_shade, hud_color, true);

	UMFDView::SetColor(hud_color);
	Hoop::SetColor(hud_color);

	for (int i = 0; i < 3; i++)
		mfd[i]->SetText3DColor(txt_color);

	SystemFont* font = FontManager::Find("HUD");
	if (font)
		font->SetColor(txt_color);

	for (int i = 0; i < TXT_LAST; i++)
		hud_text[i].color = txt_color;
}

// +--------------------------------------------------------------------+

void
UHUDView::Message(const char* fmt, ...)
{
	if (fmt) {
		char msg[512];

		va_list args;
		va_start(args, fmt);
		vsnprintf(msg, sizeof(msg), fmt, args);
		va_end(args);

		char* newline = strchr(msg, '\n');
		if (newline)
			*newline = 0;

		UE_LOG(LogTemp, Log, TEXT("%hs"), msg);

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
}

// +--------------------------------------------------------------------+

void
UHUDView::ClearMessages()
{
	if (hud_view) {
		for (int i = 0; i < MAX_MSG - 1; i++) {
			hud_view->msg_text[i] = Text();
			hud_view->msg_time[i] = 0;
		}
	}
}

// +--------------------------------------------------------------------+

void
UHUDView::PrepareBitmap(const char* name, Bitmap& img, BYTE*& shades)
{
	delete[] shades;
	shades = 0;

	DataLoader* loader = DataLoader::GetLoader();

	loader->SetDataPath("HUD/");
	int loaded = loader->LoadBitmap(name, img, Bitmap::BMP_TRANSPARENT);
	loader->SetDataPath(0);

	if (!loaded)
		return;

	int w = img.Width();
	int h = img.Height();

	shades = new BYTE[w * h];

	for (int y = 0; y < h; y++)
		for (int x = 0; x < w; x++)
			shades[y * w + x] = (BYTE)(img.GetColor(x, y).Red() * 0.66);
}

void
UHUDView::TransferBitmap(const Bitmap& src, Bitmap& img, BYTE*& shades)
{
	delete[] shades;
	shades = 0;

	if (src.Width() != img.Width() || src.Height() != img.Height())
		return;

	img.CopyBitmap(src);
	img.SetType(Bitmap::BMP_TRANSLUCENT);

	int w = img.Width();
	int h = img.Height();

	shades = new BYTE[w * h];

	for (int y = 0; y < h; y++)
		for (int x = 0; x < w; x++)
			shades[y * w + x] = (BYTE)(img.GetColor(x, y).Red() * 0.5);
}

void UHUDView::ColorizeBitmap(Bitmap& img, BYTE* shades, FColor color, bool force_alpha)
{
	if (!shades) return;

	int max_tex_size = Game::MaxTexSize();

	if (max_tex_size < 128)
		Game::SetMaxTexSize(128);

	if (hud_view && hud_view->cockpit_hud_texture && !force_alpha) {
		img.FillColor(FColor::Black);
		FColor* dst = img.HiPixels();
		BYTE* src = shades;

		for (int y = 0; y < img.Height(); y++) {
			for (int x = 0; x < img.Width(); x++) {
				if (*src)
					*dst = DimFColor(color, (float)(*src) / 200.0f);
				else
					*dst = FColor::Black;

				dst++;
				src++;
			}
		}

		img.MakeTexture();
	}
	else {
		img.FillColor(color);
		img.CopyAlphaImage(img.Width(), img.Height(), shades);
		img.MakeTexture();
	}

	if (max_tex_size < 128)
		Game::SetMaxTexSize(max_tex_size);
}

// +--------------------------------------------------------------------+

void
UHUDView::MouseFrame()
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
	if (mouse_index == TXT_PAUSED)
		stars->Pause(!Game::Paused());

	if (mouse_index == TXT_GEAR_DOWN)
		ship->ToggleGear();

	if (mouse_index == TXT_HUD_MODE) {
		CycleHUDMode();

		if (mode == (int)HUD_MODE::HUD_MODE_OFF)
			CycleHUDMode();
	}

	if (mouse_index == TXT_PRIMARY_WEP) {
		HUDSounds::PlaySound(HUDSounds::SND_WEP_MODE);
		ship->CyclePrimary();
	}

	if (mouse_index == TXT_SECONDARY_WEP) {
		HUDSounds::PlaySound(HUDSounds::SND_WEP_MODE);
		ship->CycleSecondary();
	}

	if (mouse_index == TXT_DECOY)
		ship->FireDecoy();

	if (mouse_index == TXT_SHIELD) {
		Shield* shield = ship->GetShield();

		if (shield) {
			double level = shield->GetPowerLevel();

			const Rect& r = hud_text[TXT_SHIELD].rect;
			if (Mouse::X() < r.x + r.w * 0.75)
				shield->SetPowerLevel(level - 10);
			else
				shield->SetPowerLevel(level + 10);

			HUDSounds::PlaySound(HUDSounds::SND_SHIELD_LEVEL);
		}
	}

	if (mouse_index == TXT_AUTO)
		ship->TimeSkip();

	if (mouse_index >= TXT_NAV_INDEX && mouse_index <= TXT_NAV_ETR) {
		ship->SetAutoNav(!ship->IsAutoNavEngaged());
		SetHUDMode((int)HUD_MODE::HUD_MODE_TAC);
	}
}

// +--------------------------------------------------------------------+

bool
UHUDView::IsMouseLatched()
{
	bool result = mouse_in;

	if (!result) {
		UHUDView* hud = UHUDView::GetInstance();

		for (int i = 0; i < 3; i++)
			result = result || hud->mfd[i]->IsMouseLatched();
	}

	return result;
}

// +--------------------------------------------------------------------+

bool
UHUDView::IsNameCrowded(int x, int y)
{
	for (int i = 0; i < MAX_CONTACT; i++) {
		HUDText& test = hud_text[TXT_CONTACT_NAME + i];

		if (!test.hidden) {
			Rect r = test.rect;

			int dx = r.x - x;
			int dy = r.y - y;
			int d = dx * dx + dy * dy;

			if (d <= 400)
				return true;
		}

		test = hud_text[TXT_CONTACT_INFO + i];

		if (!test.hidden) {
			Rect r = test.rect;

			int dx = r.x - x;
			int dy = r.y - y;
			int d = dx * dx + dy * dy;

			if (d <= 400)
				return true;
		}
	}

	return false;
}

void
UHUDView::DrawDiamond(int x, int y, int r, FColor c)
{
	FVector diamond[4];

	diamond[0].X = x;
	diamond[0].Y = y - r;

	diamond[1].X = x + r;
	diamond[1].Y = y;

	diamond[2].X = x;
	diamond[2].Y = y + r;

	diamond[3].X = x - r;
	diamond[3].Y = y;

	window->DrawPoly(4, diamond, c);
}

