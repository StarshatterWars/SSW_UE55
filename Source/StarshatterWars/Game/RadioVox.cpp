// /*  Project nGenEx	Fractal Dev Games	Copyright (C) 2024. All Rights Reserved.	SUBSYSTEM:    SSW	FILE:         Game.cpp	AUTHOR:       Carlos Bott*/


#include "RadioVox.h"
#include "RadioVoxController.h"

#include "W_RadioView.h"
//#include "AudioConfig.h"

//#include "DataLoader.h"
#include "Game.h"
//#include "Sound.h"
#include "ThreadSync.h"

// +====================================================================+
//
// RADIO VOX MESSAGE:
//

static RadioVoxController* controller = 0;

RadioVox::RadioVox()
{
}

RadioVox::~RadioVox()
{
	//sounds.destroy();
}

void
RadioVox::Initialize()
{
	if (!controller) {
		controller = new RadioVoxController();
	}
}

void
RadioVox::Close()
{
	delete controller;
	controller = 0;
}

// +--------------------------------------------------------------------+

RadioVox::RadioVox(int n, const char* p, const char* m)
{
	path = p;
	message = m;
	index = 0;
	channel = n;
}

// +--------------------------------------------------------------------+

bool
RadioVox::AddPhrase(const char* key)
{
	/*if (AudioConfig::VoxVolume() <= AudioConfig::Silence())
		return false;

	DataLoader* loader = DataLoader::GetLoader();
	if (!loader)
		return false;

	if (key && *key) {
		char datapath[256];
		char filename[256];

		sprintf_s(datapath, "Vox/%s/", path.data());
		sprintf_s(filename, "%s.wav", key);

		bool        use_fs = loader->IsFileSystemEnabled();
		Sound* sound = 0;

		loader->UseFileSystem(true);
		loader->SetDataPath(datapath);
		loader->LoadSound(filename, sound, Sound::LOCALIZED, true); // optional sound
		loader->SetDataPath(0);
		loader->UseFileSystem(use_fs);

		if (sound) {
			sound->SetVolume(AudioConfig::VoxVolume());
			sound->SetFlags(Sound::LOCALIZED | Sound::LOCKED);
			sound->SetFilename(filename);
			sounds.append(sound);

			return true;
		}
	}*/

	return false;
}

// +--------------------------------------------------------------------+

bool
RadioVox::Start()
{
	if (controller)
		return controller->Add(this);

	return false;
}

bool
RadioVox::Update()
{
	bool active = false;
	
	/*if (message.length()) {
		RadioView::Message(message);
		message = "";
	}
	
	while (!active && index < sounds.size()) {
		Sound* s = sounds[index];

		if (s->IsReady()) {
			if (channel & 1)
				s->SetPan(channel * -3000);
			else
				s->SetPan(channel * 3000);

			s->Play();
			active = true;
		}

		else if (s->IsPlaying()) {
			s->Update();
			active = true;
		}

		else {
			index++;
		}
	}*/

	return active;
}


