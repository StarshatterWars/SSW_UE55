// /*  Project nGenEx	Fractal Dev Games	Copyright (C) 2024. All Rights Reserved.	SUBSYSTEM:    SSW	FILE:         Game.cpp	AUTHOR:       Carlos Bott*/


#include "RadioVoxController.h"
#include "RadioVox.h"

DWORD WINAPI VoxUpdateProc(LPVOID link);

// +--------------------------------------------------------------------+

RadioVoxController::RadioVoxController()
{
	hthread = 0;
	shutdown = false;
	
	DWORD thread_id = 0;
	hthread = CreateThread(0, 4096, VoxUpdateProc,
		(LPVOID)this, 0, &thread_id);
}

// +--------------------------------------------------------------------+

RadioVoxController::~RadioVoxController()
{
	shutdown = true;

	WaitForSingleObject(hthread, 500);
	CloseHandle(hthread);
	hthread = 0;

	queue.destroy();
}

// +--------------------------------------------------------------------+

DWORD WINAPI VoxUpdateProc(LPVOID link)
{
	RadioVoxController* contrl = (RadioVoxController*)link;

	if (contrl)
		return contrl->UpdateThread();

	return (DWORD)E_POINTER;
}

// +--------------------------------------------------------------------+

DWORD
RadioVoxController::UpdateThread()
{
	while (!shutdown) {
		Update();
		Sleep(50);
	}

	return 0;
}

// +--------------------------------------------------------------------+

void
RadioVoxController::Update()
{
	AutoThreadSync a(sync);

	if (queue.size()) {
		RadioVox* vox = queue.first();

		if (!vox->Update())
			delete queue.removeIndex(0);
	}
}

bool
RadioVoxController::Add(RadioVox* vox)
{
	if (!vox || vox->sounds.isEmpty())
		return false;

	AutoThreadSync a(sync);

	if (queue.size() < MAX_QUEUE) {
		queue.append(vox);
		return true;
	}

	return false;
}
