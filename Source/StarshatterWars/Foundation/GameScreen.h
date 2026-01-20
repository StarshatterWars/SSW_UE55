/*  Project Starshatter Wars
	Fractal Dev Studios
	Copyright 2025-2026. All Rights Reserved.

	SUBSYSTEM:    Stars.exe
	FILE:         GameScreen.h
	AUTHOR:       Carlos Bott

	ORIGINAL AUTHOR AND STUDIO:
	John DiCamillo, Destroyer Studios LLC

	OVERVIEW
	========
	GameScreen UI controller for the in-game HUD, dialogs, and overlays.
*/

#pragma once

#include "Types.h"
#include "BaseUIScreen.h"

// Minimal Unreal forward declaration for render assets:
class UTexture2D;

// +--------------------------------------------------------------------+

class Screen;
class Sim;
class Window;
class Font;

class NavDlg;
class EngDlg;
class FltDlg;
class CtlDlg;
class JoyDlg;
class KeyDlg;
class ModDlg;
class ModInfoDlg;

class AudDlg;
class VidDlg;
class OptDlg;

class CameraDirector;
class DisplayView;
class HUDView;
class WepView;
class QuantumView;
class QuitView;
class RadioView;
class TacticalView;
class CameraView;
class PolyRender;

class DataLoader;
class Video;
class VideoFactory;

// +--------------------------------------------------------------------+

class GameScreen : public BaseUIScreen
{
public:
	GameScreen();
	virtual ~GameScreen();

	virtual void         Setup(Screen* screen);
	virtual void         TearDown();
	virtual bool         CloseTopmost();

	virtual bool         IsShown()         const { return is_shown; }
	virtual void         Show();
	virtual void         Hide();

	virtual bool         IsFormShown()     const;
	virtual void         ShowExternal();

	virtual void         ShowNavDlg();
	virtual void         HideNavDlg();
	virtual bool         IsNavShown();
	virtual NavDlg*		 GetNavDlg() { return navdlg; }

	virtual void         ShowEngDlg();
	virtual void         HideEngDlg();
	virtual bool         IsEngShown();
	virtual EngDlg*		 GetEngDlg() { return engdlg; }

	virtual void         ShowFltDlg();
	virtual void         HideFltDlg();
	virtual bool         IsFltShown();
	virtual FltDlg*		 GetFltDlg() { return fltdlg; }

	virtual void         ShowCtlDlg();
	virtual void         HideCtlDlg();
	virtual bool         IsCtlShown();

	virtual void         ShowJoyDlg();
	virtual bool         IsJoyShown();

	virtual void         ShowKeyDlg();
	virtual bool         IsKeyShown();

	virtual AudDlg* GetAudDlg() const { return auddlg; }
	virtual VidDlg* GetVidDlg() const { return viddlg; }
	virtual OptDlg* GetOptDlg() const { return optdlg; }
	virtual CtlDlg* GetCtlDlg() const { return ctldlg; }
	virtual JoyDlg* GetJoyDlg() const { return joydlg; }
	virtual KeyDlg* GetKeyDlg() const { return keydlg; }
	virtual ModDlg* GetModDlg() const { return moddlg; }
	virtual ModInfoDlg* GetModInfoDlg() const { return mod_info_dlg; }

	virtual void         ShowAudDlg();
	virtual void         HideAudDlg();
	virtual bool         IsAudShown();

	virtual void         ShowVidDlg();
	virtual void         HideVidDlg();
	virtual bool         IsVidShown();

	virtual void         ShowOptDlg();
	virtual void         HideOptDlg();
	virtual bool         IsOptShown();

	virtual void         ShowModDlg();
	virtual void         HideModDlg();
	virtual bool         IsModShown();

	virtual void         ShowModInfoDlg();
	virtual void         HideModInfoDlg();
	virtual bool         IsModInfoShown();

	virtual void         ApplyOptions();
	virtual void         CancelOptions();

	virtual void         ShowWeaponsOverlay();
	virtual void         HideWeaponsOverlay();

	void                 SetFieldOfView(double fov);
	double               GetFieldOfView() const;
	void                 CycleMFDMode(int mfd);
	void                 CycleHUDMode();
	void                 CycleHUDColor();
	void                 CycleHUDWarn();
	void                 FrameRate(double f);
	void                 ExecFrame();

	static GameScreen* GetInstance() { return game_screen; }
	CameraView* GetCameraView()   const { return cam_view; }

	// UE-compatible render assets:
	UTexture2D* GetLensFlare(int index);

private:
	void                 HideAll();

	Sim* sim;
	Screen* screen;

	Window* gamewin;
	NavDlg* navdlg;
	EngDlg* engdlg;
	FltDlg* fltdlg;
	CtlDlg* ctldlg;
	KeyDlg* keydlg;
	JoyDlg* joydlg;
	AudDlg* auddlg;
	VidDlg* viddlg;
	OptDlg* optdlg;
	ModDlg* moddlg;
	ModInfoDlg* mod_info_dlg;

	Font* hud_font;
	Font* gui_font;
	Font* gui_small_font;
	Font* title_font;

	// UE-compatible render assets:
	UTexture2D* flare1;
	UTexture2D* flare2;
	UTexture2D* flare3;
	UTexture2D* flare4;

	CameraDirector* cam_dir;
	DisplayView* disp_view;
	HUDView* hud_view;
	WepView* wep_view;
	QuantumView* quantum_view;
	QuitView* quit_view;
	TacticalView* tac_view;
	RadioView* radio_view;
	CameraView* cam_view;
	DataLoader* loader;

	double               frame_rate;
	bool                 is_shown;

	static GameScreen* game_screen;
};

