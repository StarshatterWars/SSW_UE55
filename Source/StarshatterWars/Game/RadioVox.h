// /*  Project nGenEx	Fractal Dev Games	Copyright (C) 2024. All Rights Reserved.	SUBSYSTEM:    SSW	FILE:         Game.cpp	AUTHOR:       Carlos Bott*/

#pragma once

#include "CoreMinimal.h"
#include "../Foundation/Types.h"
#include "../Foundation/List.h"
#include "../Foundation/Text.h"

// +--------------------------------------------------------------------+

class Element;
class UShip;
class RadioMessage;
class Sound;

// +--------------------------------------------------------------------+

class STARSHATTERWARS_API RadioVox
{
	friend class RadioVoxController;

public:
	static const char* TYPENAME() { return "RadioVox"; }

	RadioVox(int channel, const char* path, const char* message = 0);
	RadioVox();
	virtual ~RadioVox();

	// Operations:
	virtual bool      AddPhrase(const char* key);
	virtual bool      Start();

	static void       Initialize();
	static void       Close();

protected:
	virtual bool      Update();

	Text              path;
	Text              message;
	List<Sound>       sounds;
	int               index;
	int               channel;
};
