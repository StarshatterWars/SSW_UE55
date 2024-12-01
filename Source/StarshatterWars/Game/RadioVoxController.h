// /*  Project nGenEx	Fractal Dev Games	Copyright (C) 2024. All Rights Reserved.	SUBSYSTEM:    SSW	FILE:         Game.cpp	AUTHOR:       Carlos Bott*/

#pragma once

#include "CoreMinimal.h"
#include "../Foundation/Types.h"
#include "../Foundation/List.h"
#include "../Foundation/Text.h"

/**
 * 
 */

class RadioVox;

class STARSHATTERWARS_API RadioVoxController
{
public:
	RadioVoxController();
	~RadioVoxController();

	enum { MAX_QUEUE = 5 };

	bool  Add(RadioVox* vox);
	void  Update();
	DWORD UpdateThread();

	bool           shutdown;
	HANDLE         hthread;
	List<RadioVox> queue;
	ThreadSync     sync;

};
