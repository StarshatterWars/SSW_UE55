// /*  Project nGenEx	Fractal Dev Games	Copyright (C) 2024. All Rights Reserved.	SUBSYSTEM:    SSW	FILE:         Game.cpp	AUTHOR:       Carlos Bott*/

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "../Foundation/Types.h"
//#include "View.h"
#include "../Foundation/Color.h"
#include "../Game/SimObject.h"
#include "../Foundation/Text.h"
#include "W_RadioView.generated.h"



// +--------------------------------------------------------------------+

class Font;
class Element;
class UShip;
class RadioMessage;
class CameraView;
class HUDView;
class Menu;
class MenuItem;
class USim;
class USimObect;

// +--------------------------------------------------------------------+


/**
 * 
 */
UCLASS()
class STARSHATTERWARS_API UW_RadioView : public UUserWidget, public SimObserver
{
	GENERATED_BODY()
	
	//UW_RadioView();
	//UW_RadioView(Window* c);
	virtual ~UW_RadioView();

	// Operations:
	virtual void      Refresh();
	virtual void      OnWindowMove();
	virtual void      ExecFrame();

	virtual Menu*	  GetRadioMenu(UShip* ship);
	virtual bool      IsMenuShown();
	virtual void      ShowMenu();
	virtual void      CloseMenu();

	static void       Message(const char* msg);
	static void       ClearMessages();

	virtual bool        Update(USimObject* obj);
	virtual const char* GetObserverName() const;

	static void       SetColor(Color c);

	static void       Initialize();
	static void       Close();

	static UW_RadioView* GetInstance() { return radio_view; }

protected:
	void              SendRadioMessage(UShip* ship, MenuItem* item);

	int         width, height;
	double      xcenter, ycenter;

	Font*		font;
	USim*		sim;
	UShip*		ship;
	Element*	dst_elem;

	enum { MAX_MSG = 6 };
	Text        msg_text[MAX_MSG];
	double      msg_time[MAX_MSG];

	static UW_RadioView* radio_view;
	static ThreadSync sync;
	
	
};
