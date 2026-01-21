/*  Project Starshatter Wars
	Fractal Dev Studios
	Copyright (C) 2025-2026. All Rights Reserved.

	Original Author and Studio:
	John DiCamillo, Destroyer Studios LLC

	SUBSYSTEM:    Stars.exe
	FILE:         MFD.cpp
	AUTHOR:       Carlos Bott


	OVERVIEW
	========
	Class for all Multi Function Displays
*/

#include "MFD.h"
#include "HUDView.h"
#include "Ship.h"
#include "NavSystem.h"
#include "Power.h"
#include "Shield.h"
#include "Sensor.h"
#include "SimContact.h"
#include "ShipDesign.h"
#include "SimShot.h"
#include "Weapon.h"
#include "WeaponGroup.h"
#include "Sim.h"
#include "StarSystem.h"
#include "Starshatter.h"
#include "Drive.h"
#include "QuantumDrive.h"
#include "Instruction.h"

#include "CameraView.h"
#include "Color.h"
#include "SystemFont.h"
#include "FontManager.h"
#include "Window.h"
#include "Video.h"
#include "Screen.h"
#include "DataLoader.h"
#include "SimScene.h"
#include "Graphic.h"
#include "Sprite.h"
#include "Keyboard.h"
#include "Mouse.h"
#include "Game.h"

#include "Engine/Texture2D.h"
#include "Math/Vector.h"
#include "Math/UnrealMathUtility.h"
#include "Logging/LogMacros.h"

DEFINE_LOG_CATEGORY_STATIC(LogMFD, Log, All);

static UTexture2D* sensor_fov = nullptr;
static UTexture2D* sensor_fwd = nullptr;
static UTexture2D* sensor_hsd = nullptr;
static UTexture2D* sensor_3d = nullptr;

static BYTE* sensor_fov_shade = nullptr;
static BYTE* sensor_fwd_shade = nullptr;
static BYTE* sensor_hsd_shade = nullptr;
static BYTE* sensor_3d_shade = nullptr;

static Color hud_color = Color::Black;
static Color txt_color = Color::Black;

// +--------------------------------------------------------------------+

MFD::MFD(Window* c, int n)
	: window(c), rect(0, 0, 0, 0), index(n), mode(MFD_MODE_OFF), sprite(0),
	ship(0), hidden(true), camview(0), lines(0), mouse_latch(0), mouse_in(false),
	cockpit_hud_texture(0)
{
	sprite = new Sprite(sensor_fov);

	sprite->SetBlendMode(2);
	sprite->SetFilter(0);
	sprite->Hide();

	SystemFont* font = FontManager::Find("HUD");

	for (int i = 0; i < TXT_LAST; i++) {
		mfd_text[i].font = font;
		mfd_text[i].color = Color::White;
		mfd_text[i].hidden = true;
	}
}

MFD::~MFD()
{
	GRAPHIC_DESTROY(sprite);
}

// +--------------------------------------------------------------------+

void
MFD::Initialize()
{
	static int initialized = 0;
	if (initialized) return;

	// NOTE: Expected Unreal-port signatures:
	//   PrepareTexture(const char* name, UTexture2D*& out_tex, BYTE*& out_shade)
	//   ColorizeTexture(UTexture2D* tex, BYTE* shade, Color c)
	HUDView::PrepareTexture("sensor_fov.pcx", sensor_fov, sensor_fov_shade);
	HUDView::PrepareTexture("sensor_fwd.pcx", sensor_fwd, sensor_fwd_shade);
	HUDView::PrepareTexture("sensor_hsd.pcx", sensor_hsd, sensor_hsd_shade);
	HUDView::PrepareTexture("sensor_3d.pcx", sensor_3d, sensor_3d_shade);

	initialized = 1;
}

void
MFD::Close()
{
	sensor_fov = nullptr;
	sensor_fwd = nullptr;
	sensor_hsd = nullptr;
	sensor_3d = nullptr;

	delete[] sensor_fov_shade; sensor_fov_shade = nullptr;
	delete[] sensor_fwd_shade; sensor_fwd_shade = nullptr;
	delete[] sensor_hsd_shade; sensor_hsd_shade = nullptr;
	delete[] sensor_3d_shade;  sensor_3d_shade = nullptr;
}

// +--------------------------------------------------------------------+

void
MFD::UseCameraView(CameraView* v)
{
	if (v && !camview) {
		camview = v;
	}
}

void
MFD::SetColor(Color c)
{
	HUDView* hud = HUDView::GetInstance();

	if (hud) {
		hud_color = hud->GetHUDColor();
		txt_color = hud->GetTextColor();
	}
	else {
		hud_color = c;
		txt_color = c;
	}

	HUDView::ColorizeTexture(sensor_fov, sensor_fov_shade, c);
	HUDView::ColorizeTexture(sensor_fwd, sensor_fwd_shade, c);
	HUDView::ColorizeTexture(sensor_hsd, sensor_hsd_shade, c);
	HUDView::ColorizeTexture(sensor_3d, sensor_3d_shade, c);
}

void
MFD::SetText3DColor(Color c)
{
	for (int i = 0; i < TXT_LAST; i++)
		mfd_text[i].color = c;
}

// +--------------------------------------------------------------------+

void
MFD::Show()
{
	switch (mode) {
	case MFD_MODE_FOV:
	case MFD_MODE_HSD:
	case MFD_MODE_3D:
		if (sprite)
			sprite->Show();
		break;
	}

	hidden = false;
}

void
MFD::Hide()
{
	if (sprite)
		sprite->Hide();

	for (int i = 0; i < TXT_LAST; i++)
		HideMFDText(i);

	hidden = true;
}

// +--------------------------------------------------------------------+

void
MFD::SetRect(const Rect& r)
{
	rect = r;

	if (sprite) {
		// Point -> FVector (Z used as depth in legacy UI)
		const FVector NewPos(
			(float)(rect.x + sprite->Width() / 2),
			(float)(rect.y + sprite->Height() / 2),
			1.0f
		);
		sprite->MoveTo(NewPos);
	}
}

// +--------------------------------------------------------------------+

void
MFD::SetMode(int m)
{
	if (m < MFD_MODE_OFF || m > MFD_MODE_3D)
		mode = MFD_MODE_OFF;
	else
		mode = m;

	sprite->Hide();

	for (int i = 0; i < TXT_LAST; i++)
		HideMFDText(i);

	switch (mode) {
	case MFD_MODE_GAME:
	case MFD_MODE_SHIP:
		lines = 0;
		break;

	case MFD_MODE_FOV:
		sprite->SetAnimation(sensor_fov);
		sprite->Show();
		sprite->Reshape(HUDView::TextureWidth(sensor_fov) - 8, 16);
		break;

	case MFD_MODE_HSD:
		sprite->SetAnimation(sensor_hsd);
		sprite->Show();
		sprite->Reshape(HUDView::TextureWidth(sensor_hsd) - 8, 16);
		break;

	case MFD_MODE_3D:
		sprite->SetAnimation(sensor_3d);
		sprite->Show();
		sprite->Reshape(HUDView::TextureWidth(sensor_3d) - 8, 16);
		break;
	}
}

// +--------------------------------------------------------------------+

void
MFD::Draw()
{
	mouse_in = false;

	if (Mouse::LButton() == 0)
		mouse_latch = 0;

	if (rect.Contains(Mouse::X(), Mouse::Y()))
		mouse_in = true;

	// click to turn on MFD when off:
	if (mode < MFD_MODE_FOV && Mouse::LButton() && !mouse_latch) {
		mouse_latch = 1;
		if (mouse_in) {
			HUDView* hud = HUDView::GetInstance();
			if (hud)
				hud->CycleMFDMode(index);
		}
	}

	for (int i = 0; i < TXT_LAST; i++)
		HideMFDText(i);

	if (hidden || mode < MFD_MODE_FOV) {
		if (cockpit_hud_texture) {
			int x1 = index * 128;
			int y1 = 256;
			int x2 = x1 + 128;
			int y2 = y1 + 128;

			cockpit_hud_texture->FillRect(x1, y1, x2, y2, Color::Black);
		}

		if (hidden)
			return;
	}

	if (sprite && !sprite->Hidden()) {
		if (cockpit_hud_texture) {
			int x1 = index * 128;
			int y1 = 256;
			int w = sprite->Width();
			int h = sprite->Height();

			cockpit_hud_texture->BitBlt(x1, y1, sprite->Frame(), 0, 0, w, h);
		}
		else {
			int cx = rect.x + rect.w / 2;
			int cy = rect.y + rect.h / 2;
			int w2 = sprite->Width() / 2;
			int h2 = sprite->Height() / 2;

			window->DrawTexture(cx - w2, cy - h2, cx + w2, cy + h2, sprite->Frame(), Video::BLEND_ALPHA);
		}
	}

	switch (mode) {
	default:
	case MFD_MODE_OFF:                        break;
	case MFD_MODE_GAME:     DrawGameMFD();    break;
	case MFD_MODE_SHIP:     DrawStatusMFD();  break;

		// sensor sub-modes:
	case MFD_MODE_FOV:      DrawSensorMFD();  break;
	case MFD_MODE_HSD:      DrawHSD();        break;
	case MFD_MODE_3D:       Draw3D();         break;
	}
}

// +--------------------------------------------------------------------+

void
MFD::DrawSensorLabels(const char* mfd_mode)
{
	Sensor* sensor = ship->GetSensor();
	char    mode_buf[8] = "       ";
	int     scan_r = rect.w;
	int     scan_x = rect.x;
	int     scan_y = rect.y;

	switch (sensor->GetMode()) {
	case Sensor::PAS: strcpy_s(mode_buf, Game::GetText("MFD.mode.passive").data());      break;
	case Sensor::STD: strcpy_s(mode_buf, Game::GetText("MFD.mode.standard").data());     break;
	case Sensor::ACM: strcpy_s(mode_buf, Game::GetText("MFD.mode.auto-combat").data());  break;
	case Sensor::GM:  strcpy_s(mode_buf, Game::GetText("MFD.mode.ground").data());       break;
	case Sensor::PST: strcpy_s(mode_buf, Game::GetText("MFD.mode.passive").data());      break;
	case Sensor::CST: strcpy_s(mode_buf, Game::GetText("MFD.mode.combined").data());     break;
	default:          break;
	}

	Rect mode_rect(scan_x + 2, scan_y + 2, 40, 12);
	DrawMFDText(0, mode_buf, mode_rect, DT_LEFT);

	char   range_txt[12];
	double beam_range = sensor->GetBeamRange() + 1;

	if (beam_range >= 1e6)
		sprintf_s(range_txt, "-%dM+", (int)(beam_range / 1e6));
	else
		sprintf_s(range_txt, "-%3d+", (int)(beam_range / 1e3));

	Rect range_rect(scan_x + 2, scan_y + scan_r - 12, 40, 12);
	DrawMFDText(1, range_txt, range_rect, DT_LEFT);

	Rect disp_rect(scan_x + scan_r - 41, scan_y + 2, 40, 12);
	DrawMFDText(2, mfd_mode, disp_rect, DT_RIGHT);

	Rect probe_rect(scan_x + scan_r - 41, scan_y + scan_r - 12, 40, 12);

	if (ship->GetProbeLauncher()) {
		char probes[32];
		sprintf_s(probes, "%s %02d", Game::GetText("MFD.probe").data(), ship->GetProbeLauncher()->Ammo());
		DrawMFDText(3, probes, probe_rect, DT_RIGHT);
	}
	else {
		HideMFDText(3);
	}

	if (Mouse::LButton() && !mouse_latch) {
		mouse_latch = 1;

		if (mode_rect.Contains(Mouse::X(), Mouse::Y())) {
			if (sensor->GetMode() < Sensor::PST) {
				int sensor_mode = sensor->GetMode() + 1;
				if (sensor_mode > Sensor::GM)
					sensor_mode = Sensor::PAS;

				sensor->SetMode((Sensor::Mode)sensor_mode);
			}
		}

		else if (range_rect.Contains(Mouse::X(), Mouse::Y())) {
			if (Mouse::X() > range_rect.x + range_rect.w / 2)
				sensor->IncreaseRange();
			else
				sensor->DecreaseRange();
		}

		else if (disp_rect.Contains(Mouse::X(), Mouse::Y())) {
			HUDView* hud = HUDView::GetInstance();
			if (hud)
				hud->CycleMFDMode(index);
		}

		else if (probe_rect.Contains(Mouse::X(), Mouse::Y())) {
			ship->LaunchProbe();
		}
	}
}

// +--------------------------------------------------------------------+

bool
MFD::IsMouseLatched() const
{
	return mouse_in;
}

// +--------------------------------------------------------------------+

void
MFD::DrawMFDText(int InIndex, const char* txt, Rect& txt_rect, int align, int status)
{
	if (InIndex >= MFD::TXT_LAST) {
		UE_LOG(LogMFD, Warning, TEXT("MFD DrawMFDText() invalid mfd_text index %d"), InIndex);
		return;
	}

	HUDText& mt = mfd_text[InIndex];
	Color    mc = mt.color;

	switch (status) {
	default:
	case SimSystem::NOMINAL:   mc = txt_color;          break;
	case SimSystem::DEGRADED:  mc = Color(255, 255, 0); break;
	case SimSystem::CRITICAL:  mc = Color(255, 0, 0); break;
	case SimSystem::DESTROYED: mc = Color(0, 0, 0); break;
	}

	char txt_buf[256];
	int  n = (int)strlen(txt);

	if (n > 250) n = 250;

	int i = 0;
	for (i = 0; i < n; i++) {
		if (islower((unsigned char)txt[i]))
			txt_buf[i] = (char)toupper((unsigned char)txt[i]);
		else
			txt_buf[i] = txt[i];
	}

	txt_buf[i] = 0;

	if (cockpit_hud_texture) {
		Rect hud_rect(txt_rect);

		hud_rect.x = txt_rect.x + this->index * 128 - this->rect.x;
		hud_rect.y = txt_rect.y + 256 - this->rect.y;

		mt.font->SetColor(mc);
		mt.font->DrawText(txt_buf, 0, hud_rect, align | DT_SINGLELINE, cockpit_hud_texture);
		mt.rect = rect;
		mt.hidden = false;
	}
	else {
		if (txt_rect.Contains(Mouse::X(), Mouse::Y()))
			mc = Color::White;

		mt.font->SetColor(mc);
		mt.font->DrawText(txt_buf, 0, txt_rect, align | DT_SINGLELINE);
		mt.rect = rect;
		mt.hidden = false;
	}
}

void
MFD::HideMFDText(int InIndex)
{
	if (InIndex >= MFD::TXT_LAST) {
		UE_LOG(LogMFD, Warning, TEXT("MFD HideMFDText() invalid mfd_text index %d"), InIndex);
		return;
	}

	mfd_text[InIndex].hidden = true;
}
