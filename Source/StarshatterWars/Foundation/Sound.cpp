/*  Project Starshatter Wars
	Fractal Dev Studios
	Copyright © 2025-2026. All Rights Reserved.

	ORIGINAL AUTHOR AND STUDIO:
	John DiCamillo / Destroyer Studios LLC

	SUBSYSTEM:    nGenEx.lib
	FILE:         Sound.cpp
	AUTHOR:       Carlos Bott


	OVERVIEW
	========
	Abstract sound class
*/

#include "Sound.h"
#include "SoundCard.h"
#include "Wave.h"

// Unreal logging:
#include "Logging/LogMacros.h"

#include "vorbis/codec.h"
#include "vorbis/vorbisfile.h"

// +--------------------------------------------------------------------+

// If you already have a shared log category, replace this with that category.
DEFINE_LOG_CATEGORY_STATIC(LogStarshatterSound, Log, All);

SoundCard* Sound::creator = 0;

Sound*
Sound::CreateStream(const char* filename)
{
	Sound* sound = 0;

	if (!filename || !filename[0] || !creator)
		return sound;

	int namelen = (int)strlen(filename);

	if (namelen < 5)
		return sound;

	if ((filename[namelen - 3] == 'o' || filename[namelen - 3] == 'O') &&
		(filename[namelen - 2] == 'g' || filename[namelen - 2] == 'G') &&
		(filename[namelen - 1] == 'g' || filename[namelen - 1] == 'G')) {

		return CreateOggStream(filename);
	}

	WAVE_HEADER    head;
	WAVE_FMT       fmt;
	WAVE_FACT      fact;
	WAVE_DATA      data;
	WAVEFORMATEX   wfex;

	ZeroMemory(&head, sizeof(head));
	ZeroMemory(&fmt, sizeof(fmt));
	ZeroMemory(&fact, sizeof(fact));
	ZeroMemory(&data, sizeof(data));

	LPBYTE         buf = 0;
	LPBYTE         p = 0;
	int            len = 0;

	FILE* f;
	::fopen_s(&f, filename, "rb");

	if (f) {
		fseek(f, 0, SEEK_END);
		len = (int)ftell(f);
		fseek(f, 0, SEEK_SET);

		if (len > 4096) {
			len = 4096;
		}

		buf = new BYTE[len];

		if (buf && len)
			fread(buf, len, 1, f);

		fclose(f);
	}

	if (len > (int)sizeof(head)) {
		CopyMemory(&head, buf, sizeof(head));

		if (head.RIFF == MAKEFOURCC('R', 'I', 'F', 'F') &&
			head.WAVE == MAKEFOURCC('W', 'A', 'V', 'E')) {

			p = buf + sizeof(WAVE_HEADER);

			do {
				DWORD chunk_id = *((LPDWORD)p);

				switch (chunk_id) {
				case MAKEFOURCC('f', 'm', 't', ' '):
					CopyMemory(&fmt, p, sizeof(fmt));
					p += fmt.chunk_size + 8;
					break;

				case MAKEFOURCC('f', 'a', 'c', 't'):
					CopyMemory(&fact, p, sizeof(fact));
					p += fact.chunk_size + 8;
					break;

				case MAKEFOURCC('s', 'm', 'p', 'l'):
					CopyMemory(&fact, p, sizeof(fact));
					p += fact.chunk_size + 8;
					break;

				case MAKEFOURCC('d', 'a', 't', 'a'):
					CopyMemory(&data, p, sizeof(data));
					p += 8;
					break;

				default:
					delete[] buf;
					return sound;
				}
			} while (data.chunk_size == 0);

			wfex.wFormatTag = fmt.wFormatTag;
			wfex.nChannels = fmt.nChannels;
			wfex.nSamplesPerSec = fmt.nSamplesPerSec;
			wfex.nAvgBytesPerSec = fmt.nAvgBytesPerSec;
			wfex.nBlockAlign = fmt.nBlockAlign;
			wfex.wBitsPerSample = fmt.wBitsPerSample;
			wfex.cbSize = 0;

			sound = Create(Sound::STREAMED, &wfex);

			if (sound) {
				sound->SetFilename(filename);
				sound->StreamFile(filename, (DWORD)(p - buf));
			}
		}
	}

	delete[] buf;
	return sound;
}

// +--------------------------------------------------------------------+

Sound*
Sound::CreateOggStream(const char* filename)
{
	Sound* sound = 0;

	if (!filename || !filename[0] || !creator)
		return sound;

	int namelen = (int)strlen(filename);

	if (namelen < 5)
		return sound;

	WAVEFORMATEX wfex;
	ZeroMemory(&wfex, sizeof(wfex));

	FILE* f;
	::fopen_s(&f, filename, "rb");

	if (f) {
		OggVorbis_File* povf = new OggVorbis_File;

		if (!povf) {
			UE_LOG(LogStarshatterSound, Error, TEXT("Sound::CreateOggStream(%s) - out of memory!"), ANSI_TO_TCHAR(filename));
			return sound;
		}

		ZeroMemory(povf, sizeof(OggVorbis_File));

		if (ov_open(f, povf, NULL, 0) < 0) {
			UE_LOG(LogStarshatterSound, Warning, TEXT("Sound::CreateOggStream(%s) - not an Ogg bitstream"), ANSI_TO_TCHAR(filename));
			delete povf;
			return sound;
		}

		UE_LOG(LogStarshatterSound, Log, TEXT("Opened Ogg Bitstream '%s'"), ANSI_TO_TCHAR(filename));

		char** ptr = ov_comment(povf, -1)->user_comments;
		vorbis_info* vi = ov_info(povf, -1);

		while (ptr && *ptr) {
			UE_LOG(LogStarshatterSound, Verbose, TEXT("%s"), ANSI_TO_TCHAR(*ptr));
			++ptr;
		}

		if (vi) {
			UE_LOG(LogStarshatterSound, Log, TEXT("Bitstream is %d channel, %ldHz"), vi->channels, (long)vi->rate);
		}

		UE_LOG(LogStarshatterSound, Log, TEXT("Decoded length: %ld samples"), (long)ov_pcm_total(povf, -1));

		const char* vendor = ov_comment(povf, -1) ? ov_comment(povf, -1)->vendor : "";
		UE_LOG(LogStarshatterSound, Log, TEXT("Encoded by: %s"), ANSI_TO_TCHAR(vendor ? vendor : ""));

		wfex.wFormatTag = WAVE_FORMAT_PCM;
		wfex.nChannels = vi ? vi->channels : 0;
		wfex.nSamplesPerSec = vi ? (DWORD)vi->rate : 0;
		wfex.nAvgBytesPerSec = (vi ? (DWORD)(vi->channels * vi->rate * 2) : 0);
		wfex.nBlockAlign = (vi ? (WORD)(vi->channels * 2) : 0);
		wfex.wBitsPerSample = 16;
		wfex.cbSize = 0;

		sound = Create(Sound::STREAMED | Sound::OGGVORBIS,
			&wfex,
			(DWORD)sizeof(OggVorbis_File),
			(LPBYTE)povf);

		if (sound) {
			sound->SetFilename(filename);
		}
	}

	return sound;
}

// +--------------------------------------------------------------------+

Sound*
Sound::Create(DWORD flags, LPWAVEFORMATEX format)
{
	if (creator) return creator->CreateSound(flags, format);
	else         return 0;
}

Sound*
Sound::Create(DWORD flags, LPWAVEFORMATEX format, DWORD len, LPBYTE data)
{
	if (creator) return creator->CreateSound(flags, format, len, data);
	else         return 0;
}

void
Sound::SetListener(const Camera& cam, const FVector& vel)
{
	if (creator)
		creator->SetListener(cam, vel);
}

// +--------------------------------------------------------------------+

Sound::Sound()
	: status(UNINITIALIZED),
	volume(0),
	flags(0),
	looped(0),
	velocity(0, 0, 0),
	location(0, 0, 0),
	sound_check(0)
{
	strcpy_s(filename, "Sound()");
}

// +--------------------------------------------------------------------+

Sound::~Sound()
{
}

// +--------------------------------------------------------------------+

void
Sound::Release()
{
	flags &= ~LOCKED;
}

// +--------------------------------------------------------------------+

void
Sound::AddToSoundCard()
{
	if (creator)
		creator->AddSound(this);
}

// +--------------------------------------------------------------------+

void
Sound::SetFilename(const char* s)
{
	if (s) {
		int n = (int)strlen(s);

		if (n >= 60) {
			ZeroMemory(filename, sizeof(filename));
			strcpy_s(filename, "...");
			strcat_s(filename, s + n - 59);
			filename[63] = 0;
		}
		else {
			strcpy_s(filename, s);
		}
	}
}
