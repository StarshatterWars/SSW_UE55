/*  Project Starshatter Wars
	Fractal Dev Studios
	Copyright (c) 2025-2026. All Rights Reserved.

	ORIGINAL AUTHOR AND STUDIO
	==========================
	John DiCamillo / Destroyer Studios LLC

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
#include "Power.h"
#include "Instruction.h"

#include "CameraView.h"
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
#include "Bitmap.h"

// Minimal Unreal includes:
#include "Logging/LogMacros.h"
#include "Math/Vector.h"
#include "Math/Color.h"
#include "Math/UnrealMathUtility.h"

#include <cctype>
#include <cstring>

DEFINE_LOG_CATEGORY_STATIC(LogStarshatterWarsMFD, Log, All);

// +--------------------------------------------------------------------+

static Bitmap sensor_fov;
static Bitmap sensor_fwd;
static Bitmap sensor_hsd;
static Bitmap sensor_3d;

static BYTE* sensor_fov_shade = nullptr;
static BYTE* sensor_fwd_shade = nullptr;
static BYTE* sensor_hsd_shade = nullptr;
static BYTE* sensor_3d_shade = nullptr;

// Unreal colors (HUDView/Window drawing expects FColor in the UE port):
static FColor hud_color = FColor::Black;
static FColor txt_color = FColor::Black;

// +--------------------------------------------------------------------+

MFD::MFD(Window* c, int n)
	: window(c)
	, rect(0, 0, 0, 0)
	, index(n)
	, mode(MFD_MODE_OFF)
	, lines(0)
	, sprite(nullptr)
	, hidden(true)
	, ship(nullptr)
	, camview(nullptr)
	, cockpit_hud_texture(nullptr)
	, mouse_latch(0)
	, mouse_in(false)
{
	sprite = new Sprite(&sensor_fov);

	sprite->SetBlendMode(2);
	sprite->SetFilter(0);
	sprite->Hide();

	SystemFont* font = FontManager::Find("HUD");

	for (int i = 0; i < TXT_LAST; i++) {
		mfd_text[i].font = font;
		mfd_text[i].color = FColor::White;
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

	HUDView::PrepareBitmap("sensor_fov.pcx", sensor_fov, sensor_fov_shade);
	HUDView::PrepareBitmap("sensor_fwd.pcx", sensor_fwd, sensor_fwd_shade);
	HUDView::PrepareBitmap("sensor_hsd.pcx", sensor_hsd, sensor_hsd_shade);
	HUDView::PrepareBitmap("sensor_3d.pcx", sensor_3d, sensor_3d_shade);

	sensor_fov.SetType(Bitmap::BMP_TRANSLUCENT);
	sensor_fwd.SetType(Bitmap::BMP_TRANSLUCENT);
	sensor_hsd.SetType(Bitmap::BMP_TRANSLUCENT);
	sensor_3d.SetType(Bitmap::BMP_TRANSLUCENT);

	initialized = 1;
}

void
MFD::Close()
{
	sensor_fov.ClearImage();
	sensor_fwd.ClearImage();
	sensor_hsd.ClearImage();
	sensor_3d.ClearImage();

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
MFD::SetColor(FColor c)
{
	HUDView* hud = HUDView::GetInstance();

	if (hud) {
		hud_color = hud->GetHUDColor();
		txt_color = hud->GetTextColor();
	}
	else {
		// Legacy fallback: convert Starshatter Color -> FColor
		hud_color = FColor(c.R, c.G, c.B, 255);
		txt_color = hud_color;
	}

	// HUDView::ColorizeBitmap expects FColor in UE port:
	HUDView::ColorizeBitmap(sensor_fov, sensor_fov_shade, hud_color, true);
	HUDView::ColorizeBitmap(sensor_fwd, sensor_fwd_shade, hud_color, true);
	HUDView::ColorizeBitmap(sensor_hsd, sensor_hsd_shade, hud_color, true);
	HUDView::ColorizeBitmap(sensor_3d, sensor_3d_shade, hud_color, true);
}

void
MFD::SetText3DColor(FColor c)
{
	const FColor fc(c.R, c.G, c.B, 255);
	for (int i = 0; i < TXT_LAST; i++)
		mfd_text[i].color = fc;
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
		const float X = (float)(rect.x + sprite->Width() / 2);
		const float Y = (float)(rect.y + sprite->Height() / 2);
		sprite->MoveTo(FVector(X, Y, 1.0f));
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

	if (sprite)
		sprite->Hide();

	for (int i = 0; i < TXT_LAST; i++)
		HideMFDText(i);

	switch (mode) {
	case MFD_MODE_GAME:
	case MFD_MODE_SHIP:
		lines = 0;
		break;

	case MFD_MODE_FOV:
		sprite->SetAnimation(&sensor_fov);
		sprite->Show();
		sprite->Reshape(sensor_fov.Width() - 8, 16);
		break;

	case MFD_MODE_HSD:
		sprite->SetAnimation(&sensor_hsd);
		sprite->Show();
		sprite->Reshape(sensor_hsd.Width() - 8, 16);
		break;

	case MFD_MODE_3D:
		sprite->SetAnimation(&sensor_3d);
		sprite->Show();
		sprite->Reshape(sensor_3d.Width() - 8, 16);
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
			const int x1 = index * 128;
			const int y1 = 256;
			const int x2 = x1 + 128;
			const int y2 = y1 + 128;

			cockpit_hud_texture->FillRect(x1, y1, x2, y2, FColor::Black);
		}

		if (hidden)
			return;
	}

	if (sprite && !sprite->Hidden()) {
		if (cockpit_hud_texture) {
			const int x1 = index * 128;
			const int y1 = 256;
			const int w = sprite->Width();
			const int h = sprite->Height();

			cockpit_hud_texture->BitBlt(x1, y1, *sprite->Frame(), 0, 0, w, h);
		}
		else {
			const int cx = rect.x + rect.w / 2;
			const int cy = rect.y + rect.h / 2;
			const int w2 = sprite->Width() / 2;
			const int h2 = sprite->Height() / 2;

			window->DrawBitmap(cx - w2, cy - h2, cx + w2, cy + h2, sprite->Frame(), Video::BLEND_ALPHA);
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
	case Sensor::PAS: strcpy_s(mode_buf, Game::GetText("MFD.mode.passive").data());       break;
	case Sensor::STD: strcpy_s(mode_buf, Game::GetText("MFD.mode.standard").data());      break;
	case Sensor::ACM: strcpy_s(mode_buf, Game::GetText("MFD.mode.auto-combat").data());   break;
	case Sensor::GM:  strcpy_s(mode_buf, Game::GetText("MFD.mode.ground").data());        break;
	case Sensor::PST: strcpy_s(mode_buf, Game::GetText("MFD.mode.passive").data());       break;
	case Sensor::CST: strcpy_s(mode_buf, Game::GetText("MFD.mode.combined").data());      break;
	default: break;
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

// AZIMUTH-ELEVATION ANGULAR SCANNER

void
MFD::DrawSensorMFD()
{
	const int scan_r = rect.w;
	const int scan_x = cockpit_hud_texture ? (index * 128) : rect.x;
	const int scan_y = cockpit_hud_texture ? 256 : rect.y;
	const int r = scan_r / 2;

	const double xctr = (scan_r / 2.0) - 0.5;
	const double yctr = (scan_r / 2.0) + 0.5;

	Sensor* sensor = ship ? ship->GetSensor() : nullptr;
	if (!sensor) {
		DrawMFDText(0, Game::GetText("MFD.inactive").data(), rect, DT_CENTER);
		return;
	}

	int w = sprite->Width();
	int h = sprite->Height();

	if (w < sprite->Frame()->Width())
		w += 2;

	if (h < sprite->Frame()->Height())
		h += 16;

	sprite->Reshape(w, h);
	sprite->Show();

	if (h < sprite->Frame()->Height())
		return;

	double sweep_scale = r / (PI / 2);

	if (sensor->GetBeamLimit() > 90 * DEGREES)
		sweep_scale = (double)r / (90 * DEGREES);

	const int az = (int)(sensor->GetBeamLimit() * sweep_scale);
	const int el = az;
	const int xc = (int)(scan_x + xctr);
	const int yc = (int)(scan_y + yctr);

	if (mode == MFD_MODE_FOV) {
		if (sensor->GetMode() < Sensor::GM) {
			if (cockpit_hud_texture)
				cockpit_hud_texture->DrawEllipse(xc - az, yc - el, xc + az, yc + el, hud_color);
			else
				window->DrawEllipse(xc - az, yc - el, xc + az, yc + el, hud_color);
		}
	}
	else {
		char az_txt[8];
		sprintf_s(az_txt, "%d", (int)(sensor->GetBeamLimit() / DEGREES));

		Rect az_rect(scan_x + 2, scan_y + scan_r - 12, 32, 12);
		DrawMFDText(1, az_txt, az_rect, DT_LEFT);

		az_rect.x = scan_x + (scan_r / 2) - (az_rect.w / 2);
		DrawMFDText(2, "0", az_rect, DT_CENTER);

		az_rect.x = scan_x + scan_r - az_rect.w - 2;
		DrawMFDText(3, az_txt, az_rect, DT_RIGHT);
	}

	// draw next nav point:
	Instruction* navpt = ship ? ship->GetNextNavPoint() : nullptr;
	if (navpt && navpt->Region() == ship->GetRegion()) {
		const Camera* cam = &ship->Cam();

		const FVector Pt = navpt->Location() - ship->Location();

		// rotate (camera basis projection):
		const double tx = FVector::DotProduct(Pt, cam->vrt());
		const double ty = FVector::DotProduct(Pt, cam->vup());
		const double tz = FVector::DotProduct(Pt, cam->vpn());

		if (tz > 1.0) {
			// convert to spherical coords:
			const double rng = Pt.Length();
			if (rng > 1e-6) {
				double azp = asin(FMath::Abs(tx) / rng);
				double elp = asin(FMath::Abs(ty) / rng);

				if (tx < 0) azp = -azp;
				if (ty < 0) elp = -elp;

				if (FMath::Abs(azp) < 90 * DEGREES) {
					azp *= sweep_scale;
					elp *= sweep_scale;

					const int x = (int)(r + azp);
					const int y = (int)(r - elp);

					// clip again:
					if (x > 0 && x < scan_r && y > 0 && y < scan_r) {
						const int xcp = scan_x + x;
						const int ycp = scan_y + y;

						if (cockpit_hud_texture) {
							cockpit_hud_texture->DrawLine(xcp - 2, ycp - 2, xcp + 2, ycp + 2, FColor::White);
							cockpit_hud_texture->DrawLine(xcp - 2, ycp + 2, xcp + 2, ycp - 2, FColor::White);
						}
						else {
							window->DrawLine(xcp - 2, ycp - 2, xcp + 2, ycp + 2, FColor::White);
							window->DrawLine(xcp - 2, ycp + 2, xcp + 2, ycp - 2, FColor::White);
						}
					}
				}
			}
		}
	}

	int num_contacts = ship->NumContacts();
	ListIter<SimContact> iter = ship->ContactList();

	while (++iter) {
		SimContact* contact = iter.value();
		Ship* c_ship = contact->GetShip();
		double   azc, elc, rng;
		bool     aft = false;

		if (c_ship == ship) continue;

		contact->GetBearing(ship, azc, elc, rng);

		// clip (is in-front):
		if (FMath::Abs(azc) < 90 * DEGREES) {
			azc *= sweep_scale;
			elc *= sweep_scale;
		}

		// rear anulus:
		else {
			const double len = sqrt(azc * azc + elc * elc);

			if (len > 1e-6) {
				azc = r * azc / len;
				elc = r * elc / len;
			}
			else {
				azc = -r;
				elc = 0;
			}

			aft = true;
		}

		const int x = (int)(r + azc);
		const int y = (int)(r - elc);

		// clip again:
		if (x < 0 || x > scan_r) continue;
		if (y < 0 || y > scan_r) continue;

		// draw:
		FColor mark = HUDView::MarkerColor(contact);

		if (aft) {
			const FLinearColor lm = FLinearColor(mark) * 0.75f;
			mark = lm.ToFColor(true);
		}

		const int xcp = scan_x + x;
		const int ycp = scan_y + y;
		int       size = 1;

		if (c_ship && c_ship == ship->GetTarget())
			size = 2;

		if (cockpit_hud_texture)
			cockpit_hud_texture->FillRect(xcp - size, ycp - size, xcp + size, ycp + size, mark);
		else
			window->FillRect(xcp - size, ycp - size, xcp + size, ycp + size, mark);

		if (contact->Threat(ship)) {
			if (c_ship) {
				if (cockpit_hud_texture)
					cockpit_hud_texture->DrawEllipse(xcp - 4, ycp - 4, xcp + 3, ycp + 3, mark);
				else
					window->DrawEllipse(xcp - 4, ycp - 4, xcp + 3, ycp + 3, mark);
			}
			else {
				if (cockpit_hud_texture) {
					cockpit_hud_texture->DrawLine(xcp, ycp - 5, xcp + 5, ycp, mark);
					cockpit_hud_texture->DrawLine(xcp + 5, ycp, xcp, ycp + 5, mark);
					cockpit_hud_texture->DrawLine(xcp, ycp + 5, xcp - 5, ycp, mark);
					cockpit_hud_texture->DrawLine(xcp - 5, ycp, xcp, ycp - 5, mark);
				}
				else {
					window->DrawLine(xcp, ycp - 5, xcp + 5, ycp, mark);
					window->DrawLine(xcp + 5, ycp, xcp, ycp + 5, mark);
					window->DrawLine(xcp, ycp + 5, xcp - 5, ycp, mark);
					window->DrawLine(xcp - 5, ycp, xcp, ycp - 5, mark);
				}
			}
		}
	}

	DrawSensorLabels(Game::GetText("MFD.mode.field-of-view").data());
}

// +--------------------------------------------------------------------+

// HORIZONTAL SITUATION DISPLAY

void
MFD::DrawHSD()
{
	const int scan_r = rect.w;
	const int scan_x = cockpit_hud_texture ? (index * 128) : rect.x;
	const int scan_y = cockpit_hud_texture ? 256 : rect.y;
	const int r = scan_r / 2 - 4;

	const double xctr = (scan_r / 2.0) - 0.5;
	const double yctr = (scan_r / 2.0) + 0.5;

	const int xc = (int)xctr + scan_x;
	const int yc = (int)yctr + scan_y;

	Sensor* sensor = ship ? ship->GetSensor() : nullptr;
	if (!sensor) {
		DrawMFDText(0, Game::GetText("MFD.inactive").data(), rect, DT_CENTER);
		return;
	}

	int w = sprite->Width();
	int h = sprite->Height();

	if (w < sprite->Frame()->Width())
		w += 2;

	if (h < sprite->Frame()->Height())
		h += 16;

	sprite->Reshape(w, h);
	sprite->Show();

	if (h < sprite->Frame()->Height())
		return;

	if (sensor->GetMode() < Sensor::PST) {
		const double s = sin(sensor->GetBeamLimit());
		const double c = cos(sensor->GetBeamLimit());

		const int x0 = (int)(0.1 * r * s);
		const int y0 = (int)(0.1 * r * c);
		const int x1 = (int)(1.0 * r * s);
		const int y1 = (int)(1.0 * r * c);

		if (cockpit_hud_texture) {
			cockpit_hud_texture->DrawLine(xc - x0, yc - y0, xc - x1, yc - y1, hud_color);
			cockpit_hud_texture->DrawLine(xc + x0, yc - y0, xc + x1, yc - y1, hud_color);
		}
		else {
			window->DrawLine(xc - x0, yc - y0, xc - x1, yc - y1, hud_color);
			window->DrawLine(xc + x0, yc - y0, xc + x1, yc - y1, hud_color);
		}
	}

	const double rscale = (double)r / (sensor->GetBeamRange());

	Camera hsd_cam = ship->Cam();

	// Look-at point in world space, flattened on Y (original behavior):
	FVector look = ship->Location() + ship->Heading() * 1000.0f;
	look.Y = ship->Location().Y;

	hsd_cam.LookAt(look);

	// draw tick marks on range rings:
	for (int dir = 0; dir < 4; dir++) {
		FVector tick;

		switch (dir) {
		case 0:  tick = FVector(0.0f, 0.0f, 1000.0f); break;
		case 1:  tick = FVector(1000.0f, 0.0f, 0.0f); break;
		case 2:  tick = FVector(0.0f, 0.0f, -1000.0f); break;
		case 3:  tick = FVector(-1000.0f, 0.0f, 0.0f); break;
		}

		const double tx = FVector::DotProduct(tick, hsd_cam.vrt());
		const double tz = FVector::DotProduct(tick, hsd_cam.vpn());
		double az = asin(FMath::Abs(tx) / 1000.0);

		if (tx < 0) az = -az;

		if (tz < 0) {
			if (az < 0) az = -PI - az;
			else        az = PI - az;
		}

		for (double range = 0.3; range < 1; range += 0.3) {
			const int x0 = (int)(sin(az) * r * range);
			const int y0 = (int)(cos(az) * r * range);
			const int x1 = (int)(sin(az) * r * (range + 0.1));
			const int y1 = (int)(cos(az) * r * (range + 0.1));

			if (cockpit_hud_texture)
				cockpit_hud_texture->DrawLine(xc + x0, yc - y0, xc + x1, yc - y1, hud_color);
			else
				window->DrawLine(xc + x0, yc - y0, xc + x1, yc - y1, hud_color);
		}
	}

	// draw next nav point:
	Instruction* navpt = ship->GetNextNavPoint();
	if (navpt && navpt->Region() == ship->GetRegion()) {
		const Camera* cam = &hsd_cam;

		const FVector pt = navpt->Location() - ship->Location();

		const double tx = FVector::DotProduct(pt, cam->vrt());
		const double tz = FVector::DotProduct(pt, cam->vpn());

		double rng = pt.Length();
		if (rng < 1e-6)
			rng = 1e-6;

		double az = asin(FMath::Abs(tx) / rng);

		if (rng > sensor->GetBeamRange())
			rng = sensor->GetBeamRange();

		if (tx < 0)
			az = -az;

		if (tz < 0) {
			if (az < 0) az = -PI - az;
			else        az = PI - az;
		}

		const int x = (int)(xc + sin(az) * rng * rscale);
		const int y = (int)(yc - cos(az) * rng * rscale);

		if (cockpit_hud_texture) {
			cockpit_hud_texture->DrawLine(x - 2, y - 2, x + 2, y + 2, FColor::White);
			cockpit_hud_texture->DrawLine(x - 2, y + 2, x + 2, y - 2, FColor::White);
		}
		else {
			window->DrawLine(x - 2, y - 2, x + 2, y + 2, FColor::White);
			window->DrawLine(x - 2, y + 2, x + 2, y - 2, FColor::White);
		}
	}

	// draw contact markers:
	const double limit = sensor->GetBeamRange();
	ListIter<SimContact> contact = ship->ContactList();

	while (++contact) {
		Ship* c_ship = contact->GetShip();
		if (c_ship == ship) continue;

		const FVector targ_pt = contact->Location() - hsd_cam.Pos();

		const double tx = FVector::DotProduct(targ_pt, hsd_cam.vrt());
		const double rg = contact->Range(ship, limit);

		double true_range = targ_pt.Length();
		if (true_range < 1e-6)
			true_range = 1e-6;

		double az = asin(FMath::Abs(tx) / true_range);

		if (rg > limit || rg <= 0)
			continue;

		if (tx < 0)
			az = -az;

		if (!contact->InFront(ship)) {
			if (az < 0) az = -PI - az;
			else        az = PI - az;
		}

		const int x = (int)(xc + sin(az) * rg * rscale);
		const int y = (int)(yc - cos(az) * rg * rscale);
		int size = 2;

		if (x < scan_x || y < scan_y)
			continue;

		if (c_ship && c_ship == ship->GetTarget())
			size = 3;

		FColor mark = HUDView::MarkerColor(contact.value());

		if (cockpit_hud_texture)
			cockpit_hud_texture->FillRect(x - size, y - size, x + size, y + size, mark);
		else
			window->FillRect(x - size, y - size, x + size, y + size, mark);

		if (contact->Threat(ship)) {
			if (c_ship) {
				if (cockpit_hud_texture)
					cockpit_hud_texture->DrawEllipse(x - 4, y - 4, x + 3, y + 3, mark);
				else
					window->DrawEllipse(x - 4, y - 4, x + 3, y + 3, mark);
			}
			else {
				if (cockpit_hud_texture) {
					cockpit_hud_texture->DrawLine(x, y - 5, x + 5, y, mark);
					cockpit_hud_texture->DrawLine(x + 5, y, x, y + 5, mark);
					cockpit_hud_texture->DrawLine(x, y + 5, x - 5, y, mark);
					cockpit_hud_texture->DrawLine(x - 5, y, x, y - 5, mark);
				}
				else {
					window->DrawLine(x, y - 5, x + 5, y, mark);
					window->DrawLine(x + 5, y, x, y + 5, mark);
					window->DrawLine(x, y + 5, x - 5, y, mark);
					window->DrawLine(x - 5, y, x, y - 5, mark);
				}
			}
		}
	}

	DrawSensorLabels(Game::GetText("MFD.mode.horizontal").data());
}

// +--------------------------------------------------------------------+

// ELITE-STYLE 3D RADAR

void
MFD::Draw3D()
{
	const int scan_r = rect.w;
	const int scan_x = cockpit_hud_texture ? (index * 128) : rect.x;
	const int scan_y = cockpit_hud_texture ? 256 : rect.y;
	const int r = scan_r / 2 - 4;

	const double xctr = (scan_r / 2.0) - 0.5;
	const double yctr = (scan_r / 2.0) + 0.5;

	const int xc = (int)xctr + scan_x;
	const int yc = (int)yctr + scan_y;

	Sensor* sensor = ship ? ship->GetSensor() : nullptr;
	if (!sensor) {
		DrawMFDText(0, Game::GetText("MFD.inactive").data(), rect, DT_CENTER);
		return;
	}

	int w = sprite->Width();
	int h = sprite->Height();

	if (w < sprite->Frame()->Width())
		w += 2;

	if (h < sprite->Frame()->Height())
		h += 16;

	sprite->Reshape(w, h);
	sprite->Show();

	if (h < sprite->Frame()->Height())
		return;

	const double rscale = (double)r / (sensor->GetBeamRange());

	Camera hsd_cam = ship->Cam();

	if (ship->IsStarship()) {
		FVector look = ship->Location() + ship->Heading() * 1000.0f;
		look.Y = ship->Location().Y;
		hsd_cam.LookAt(look);
	}

	// draw next nav point:
	Instruction* navpt = ship->GetNextNavPoint();
	if (navpt && navpt->Region() == ship->GetRegion()) {
		const Camera* cam = &hsd_cam;

		const FVector pt = navpt->Location() - ship->Location();

		const double tx = FVector::DotProduct(pt, cam->vrt());
		double ty = FVector::DotProduct(pt, cam->vup());
		const double tz = FVector::DotProduct(pt, cam->vpn());

		double rng = pt.Length();
		if (rng < 1e-6)
			rng = 1e-6;

		double az = asin(FMath::Abs(tx) / rng);

		if (rng > sensor->GetBeamRange())
			rng = sensor->GetBeamRange();

		if (tx < 0)
			az = -az;

		if (tz < 0) {
			if (az < 0) az = -PI - az;
			else        az = PI - az;
		}

		// accentuate vertical:
		if (ty > 10)
			ty = log10(ty - 9) * r / 8;
		else if (ty < -10)
			ty = -log10(9 - ty) * r / 8;
		else
			ty = 0;

		const int x = (int)(sin(az) * rng * rscale);
		const int y = (int)(cos(az) * rng * rscale / 2);
		const int z = (int)(ty);

		const int x0 = xc + x;
		const int y0 = yc - y - z;

		if (cockpit_hud_texture) {
			cockpit_hud_texture->DrawLine(x0 - 2, y0 - 2, x0 + 2, y0 + 2, FColor::White);
			cockpit_hud_texture->DrawLine(x0 - 2, y0 + 2, x0 + 2, y0 - 2, FColor::White);
		}
		else {
			window->DrawLine(x0 - 2, y0 - 2, x0 + 2, y0 + 2, FColor::White);
			window->DrawLine(x0 - 2, y0 + 2, x0 + 2, y0 - 2, FColor::White);
		}

		if (cockpit_hud_texture) {
			if (z > 0)
				cockpit_hud_texture->DrawLine(x0, y0 + 1, x0, y0 + z, FColor::White);
			else if (z < 0)
				cockpit_hud_texture->DrawLine(x0, y0 + z, x0, y0 - 1, FColor::White);
		}
		else {
			if (z > 0)
				window->DrawLine(x0, y0 + 1, x0, y0 + z, FColor::White);
			else if (z < 0)
				window->DrawLine(x0, y0 + z, x0, y0 - 1, FColor::White);
		}
	}

	// draw contact markers:
	const double limit = sensor->GetBeamRange();
	ListIter<SimContact> contact = ship->ContactList();

	while (++contact) {
		Ship* c_ship = contact->GetShip();
		if (c_ship == ship) continue;

		const FVector targ_pt = contact->Location() - hsd_cam.Pos();

		const double tx = FVector::DotProduct(targ_pt, hsd_cam.vrt());
		double ty = FVector::DotProduct(targ_pt, hsd_cam.vup());
		const double rg = contact->Range(ship, limit);

		double true_range = targ_pt.Length();
		if (true_range < 1e-6)
			true_range = 1e-6;

		double az = asin(FMath::Abs(tx) / true_range);

		if (rg > limit || rg <= 0)
			continue;

		if (tx < 0)
			az = -az;

		if (!contact->InFront(ship)) {
			if (az < 0) az = -PI - az;
			else        az = PI - az;
		}

		// accentuate vertical:
		ty *= 4;

		const int x = (int)(sin(az) * rg * rscale);
		const int y = (int)(cos(az) * rg * rscale / 2);
		const int z = (int)(ty * rscale / 2);
		int size = 1;

		const int x0 = xc + x;
		const int y0 = yc - y - z;

		if (c_ship && c_ship == ship->GetTarget())
			size = 2;

		FColor mark = HUDView::MarkerColor(contact.value());

		if (cockpit_hud_texture) {
			cockpit_hud_texture->FillRect(x0 - size, y0 - size, x0 + size, y0 + size, mark);

			if (contact->Threat(ship)) {
				if (c_ship) {
					cockpit_hud_texture->DrawEllipse(x0 - 4, y0 - 4, x0 + 3, y0 + 3, mark);
				}
				else {
					cockpit_hud_texture->DrawLine(x0, y0 - 5, x0 + 5, y0, mark);
					cockpit_hud_texture->DrawLine(x0 + 5, y0, x0, y0 + 5, mark);
					cockpit_hud_texture->DrawLine(x0, y0 + 5, x0 - 5, y0, mark);
					cockpit_hud_texture->DrawLine(x0 - 5, y0, x0, y0 - 5, mark);
				}
			}

			if (z > 0)
				cockpit_hud_texture->FillRect(x0 - 1, y0 + size, x0, y0 + z, mark);
			else if (z < 0)
				cockpit_hud_texture->FillRect(x0 - 1, y0 + z, x0, y0 - size, mark);
		}
		else {
			window->FillRect(x0 - size, y0 - size, x0 + size, y0 + size, mark);

			if (contact->Threat(ship)) {
				if (c_ship) {
					window->DrawEllipse(x0 - 4, y0 - 4, x0 + 3, y0 + 3, mark);
				}
				else {
					window->DrawLine(x0, y0 - 5, x0 + 5, y0, mark);
					window->DrawLine(x0 + 5, y0, x0, y0 + 5, mark);
					window->DrawLine(x0, y0 + 5, x0 - 5, y0, mark);
					window->DrawLine(x0 - 5, y0, x0, y0 - 5, mark);
				}
			}

			if (z > 0)
				window->FillRect(x0 - 1, y0 + size, x0, y0 + z, mark);
			else if (z < 0)
				window->FillRect(x0 - 1, y0 + z, x0, y0 - size, mark);
		}
	}

	DrawSensorLabels(Game::GetText("MFD.mode.3D").data());
}

// +--------------------------------------------------------------------+

// GROUND MAP

void
MFD::DrawMap()
{
	Rect TextRect(rect.x, rect.y, rect.w, 12);
	DrawMFDText(
		0,
		Game::GetText("MFD.mode.ground").data(),
		TextRect,
		DT_CENTER
	);
}

// +--------------------------------------------------------------------+

void
MFD::DrawGauge(int x, int y, int percent)
{
	if (cockpit_hud_texture) {
		x += this->index * 128 - this->rect.x;
		y += 256 - this->rect.y;
		cockpit_hud_texture->DrawRect(x, y, x + 53, y + 8, FColor(64, 64, 64, 255));
	}
	else {
		window->DrawRect(x, y, x + 53, y + 8, FColor(64, 64, 64, 255));
	}

	if (percent < 3) return;
	if (percent > 100) percent = 100;

	percent /= 2;

	if (cockpit_hud_texture)
		cockpit_hud_texture->FillRect(x + 2, y + 2, x + 2 + percent, y + 7, FColor(64, 64, 64, 255));
	else
		window->FillRect(x + 2, y + 2, x + 2 + percent, y + 7, FColor(128, 128, 128, 255));
}

void
MFD::DrawGameMFD()
{
	if (lines < 10) lines++;

	char txt[64];
	Rect txt_rect(rect.x, rect.y, rect.w, 12);

	int t = 0;

	if (!HUDView::IsArcade() && HUDView::ShowFPS()) {
		sprintf_s(txt, "FPS: %6.2f", Game::FrameRate());
		DrawMFDText(t++, txt, txt_rect, DT_LEFT);
		txt_rect.y += 10;

		if (lines <= 1) return;
	}

	if (ship) {
		DrawMFDText(t++, ship->Name(), txt_rect, DT_LEFT);
		txt_rect.y += 10;
	}

	if (lines <= 2) return;

	int hours = (Game::GameTime() / 3600000);
	int minutes = (Game::GameTime() / 60000) % 60;
	int seconds = (Game::GameTime() / 1000) % 60;

	if (ship) {
		DWORD clock = ship->MissionClock();

		hours = (clock / 3600000);
		minutes = (clock / 60000) % 60;
		seconds = (clock / 1000) % 60;
	}

	FString TimeText;

	if (Game::TimeCompression() > 1) {
		TimeText = FString::Printf(
			TEXT("%02d:%02d:%02d x%d"),
			hours,
			minutes,
			seconds,
			Game::TimeCompression()
		);
	}
	else {
		TimeText = FString::Printf(
			TEXT("%02d:%02d:%02d"),
			hours,
			minutes,
			seconds
		);
	}

	DrawMFDText(t++, txt, txt_rect, DT_LEFT);
	txt_rect.y += 10;

	if (HUDView::IsArcade() || lines <= 3) return;

	DrawMFDText(t++, ship->GetRegion()->GetName(), txt_rect, DT_LEFT);
	txt_rect.y += 10;

	if (lines <= 4) return;

	if (ship) {
		switch (ship->GetFlightPhase()) {
		case Ship::DOCKED:   DrawMFDText(t++, Game::GetText("MFD.phase.DOCKED").data(), txt_rect, DT_LEFT); break;
		case Ship::ALERT:    DrawMFDText(t++, Game::GetText("MFD.phase.ALERT").data(), txt_rect, DT_LEFT); break;
		case Ship::LOCKED:   DrawMFDText(t++, Game::GetText("MFD.phase.LOCKED").data(), txt_rect, DT_LEFT); break;
		case Ship::LAUNCH:   DrawMFDText(t++, Game::GetText("MFD.phase.LAUNCH").data(), txt_rect, DT_LEFT); break;
		case Ship::TAKEOFF:  DrawMFDText(t++, Game::GetText("MFD.phase.TAKEOFF").data(), txt_rect, DT_LEFT); break;
		case Ship::ACTIVE:   DrawMFDText(t++, Game::GetText("MFD.phase.ACTIVE").data(), txt_rect, DT_LEFT); break;
		case Ship::APPROACH: DrawMFDText(t++, Game::GetText("MFD.phase.APPROACH").data(), txt_rect, DT_LEFT); break;
		case Ship::RECOVERY: DrawMFDText(t++, Game::GetText("MFD.phase.RECOVERY").data(), txt_rect, DT_LEFT); break;
		case Ship::DOCKING:  DrawMFDText(t++, Game::GetText("MFD.phase.DOCKING").data(), txt_rect, DT_LEFT); break;
		}
	}
}



// +--------------------------------------------------------------------+

void
MFD::SetStatusColor(int status)
{
	FColor status_color;

	switch (status) {
	default:
	case SimSystem::NOMINAL:    
		status_color = txt_color;
		break;
	case SimSystem::DEGRADED: 
		status_color = FColor(255, 255, 0, 255);
		break;
	case SimSystem::CRITICAL:
		status_color = FColor(255, 0, 0, 255);
		break;
	case SimSystem::DESTROYED:
		status_color = FColor(0, 0, 0, 255);
		break;
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
MFD::DrawMFDText(int32 InTextIndex, const char* InText, Rect& InTextRect, int32 InAlign, int32 InStatus)
{
	if (InTextIndex < 0 || InTextIndex >= MFD::TXT_LAST) {
		UE_LOG(LogTemp, Warning,
			TEXT("MFD::DrawMFDText() invalid mfd_text index %d '%hs'"),
			InTextIndex,
			InText ? InText : "");
		return;
	}

	HUDText& TextSlot = mfd_text[InTextIndex];
	FColor   DrawColor = TextSlot.color;

	switch (InStatus) {
	default:
	case SimSystem::NOMINAL:
		DrawColor = txt_color;
		break;

	case SimSystem::DEGRADED:
		DrawColor = FColor(255, 255, 0, 255);
		break;

	case SimSystem::CRITICAL:
		DrawColor = FColor(255, 0, 0, 255);
		break;

	case SimSystem::DESTROYED:
		DrawColor = FColor(0, 0, 0, 255);
		break;
	}

	char UpperText[256] = { 0 };
	int32 TextLen = InText ? (int32)strlen(InText) : 0;
	if (TextLen > 250) {
		TextLen = 250;
	}

	for (int32 CharIndex = 0; CharIndex < TextLen; ++CharIndex) {
		const unsigned char Ch = (unsigned char)InText[CharIndex];
		UpperText[CharIndex] = islower(Ch) ? (char)toupper(Ch) : (char)Ch;
	}
	UpperText[TextLen] = 0;

	if (cockpit_hud_texture) {
		Rect HudRect(InTextRect);

		HudRect.x = InTextRect.x + this->index * 128 - this->rect.x;
		HudRect.y = InTextRect.y + 256 - this->rect.y;

		if (TextSlot.font) {
			TextSlot.font->SetColor(DrawColor);
			TextSlot.font->DrawText(UpperText, 0, HudRect, InAlign | DT_SINGLELINE, cockpit_hud_texture);
		}

		TextSlot.rect = rect;
		TextSlot.hidden = false;
	}
	else {
		// Legacy Mouse interface retained; only variable names + types updated.
		if (InTextRect.Contains(Mouse::X(), Mouse::Y())) {
			DrawColor = FColor::White;
		}

		if (TextSlot.font) {
			TextSlot.font->SetColor(DrawColor);
			TextSlot.font->DrawText(UpperText, 0, InTextRect, InAlign | DT_SINGLELINE);
		}

		TextSlot.rect = rect;
		TextSlot.hidden = false;
	}
}


void
MFD::HideMFDText(int32 InTextIndex)
{
	if (InTextIndex < 0 || InTextIndex >= MFD::TXT_LAST) {
		UE_LOG(LogTemp, Warning,
			TEXT("MFD::HideMFDText() invalid mfd_text index %d"),
			InTextIndex);
		return;
	}

	mfd_text[InTextIndex].hidden = true;
}