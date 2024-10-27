// /*  Project nGenEx	Fractal Dev Games	Copyright (C) 2024. All Rights Reserved.	SUBSYSTEM:    SSW	FILE:         Game.cpp	AUTHOR:       Carlos Bott*/


#include "Galaxy.h"
#include "StarSystem.h"
#include "../System/SSWGameInstance.h"
#include "../Foundation/ParseUtil.h"
#include "../Foundation/DataLoader.h"

// Called when the game starts or when spawned
void AGalaxy::BeginPlay()
{
	Super::BeginPlay();
	
	ProjectPath = FPaths::ProjectDir();
	ProjectPath.Append(TEXT("GameData/Galaxy/"));
	FilePath = "Galaxy.def";
	
	UE_LOG(LogTemp, Log, TEXT("Setting Galaxy Game Data Directory %s"), *ProjectPath);

	Load();
}

// Called every frame
void AGalaxy::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	ExecFrame();
}

static AGalaxy* galaxy = 0;

// +--------------------------------------------------------------------+

AGalaxy::AGalaxy()
{

}

AGalaxy::AGalaxy(const char* n)
	: name(n), radius(10)
{ }

// +--------------------------------------------------------------------+

AGalaxy::~AGalaxy()
{
	Print("  Destroying Galaxy %s\n", (const char*)name);
	//systems.destroy();
	//stars.destroy();
}

// +--------------------------------------------------------------------+

void
AGalaxy::Close()
{
	delete galaxy;
	galaxy = 0;
}

AGalaxy*
AGalaxy::GetInstance()
{
	return galaxy;
}

// +--------------------------------------------------------------------+

void
AGalaxy::Load()
{
	USSWGameInstance* SSWInstance = (USSWGameInstance*)GetGameInstance();
	SSWInstance->loader->GetLoader();
	
	ProjectPath = FPaths::ProjectDir();
	ProjectPath.Append(TEXT("GameData/Galaxy/"));
	FString FileName = ProjectPath;
	FileName.Append(FilePath);
	SSWInstance->loader->SetDataPath(FileName);
	const char* result = TCHAR_TO_ANSI(*FileName);
	Load(result);
}

void
AGalaxy::Load(const char* FileName)
{
	USSWGameInstance* SSWInstance = (USSWGameInstance*)GetGameInstance();
	SSWInstance->loader->GetLoader(); 

	FString fs = FString(ANSI_TO_TCHAR(FileName));
	FString FileString;
	BYTE* block = 0;
	
	SSWInstance->loader->LoadBuffer(FileName, block, true);

	UE_LOG(LogTemp, Log, TEXT("Loading Galaxy: %s"), *fs);

	if (FFileHelper::LoadFileToString(FileString, *fs, FFileHelper::EHashOptions::None))
	{
		UE_LOG(LogTemp, Log, TEXT("%s"), *FileString);

		const char* result = TCHAR_TO_ANSI(*FileString);
	}
		
		Parser parser(new BlockReader((const char*)block));

		Term* term = parser.ParseTerm();

		if (!term) {
			UE_LOG(LogTemp, Log, TEXT("WARNING: could not parse '%s'"), *FilePath);
			return;
		}
		else {
			TermText* file_type = term->isText();
			if (!file_type || file_type->value() != "GALAXY") {
				UE_LOG(LogTemp, Log, TEXT("WARNING: invalid galaxy file '%s'"), *FilePath);
				return;
			}
			else {
				UE_LOG(LogTemp, Log, TEXT("Galaxy file '%s'"), *FilePath);
			}
		}

		// parse the galaxy:
		do {
			delete term;
			term = parser.ParseTerm();
			FVector fv;

			if (term) {
				TermDef* def = term->isDef();
				if (def) {
					if (def->name()->value() == "radius") {
						GetDefNumber(radius, def, filename);
					}

					else if (def->name()->value() == "system") {
						if (!def->term() || !def->term()->isStruct()) {
							UE_LOG(LogTemp, Log, TEXT("WARNING: system struct missing in '%s'"), *FString(filename));
						}
						else {
							TermStruct* val = def->term()->isStruct();

							UE_LOG(LogTemp, Log, TEXT("%s"), *FString(def->name()->value()));
							char  sys_name[32];
							char  classname[32];
							Vec3  sys_loc;
							int   sys_iff = 0;
							int   star_class = (int8) ESPECTRAL_CLASS::G;

							sys_name[0] = 0;

							for (int i = 0; i < val->elements()->size(); i++) {
								TermDef* pdef = val->elements()->at(i)->isDef();
								if (pdef) {
									if (pdef->name()->value() == "name") {
										GetDefText(sys_name, pdef, filename);
									}
									else if (pdef->name()->value() == "loc") {
										
										GetDefVec(sys_loc, pdef, filename);
										fv = FVector(sys_loc.x, sys_loc.y, sys_loc.z);
									}
									else if (pdef->name()->value() == "iff") {
										GetDefNumber(sys_iff, pdef, filename);
									}
									else if (pdef->name()->value() == "class") {
										GetDefText(classname, pdef, filename);

										switch (classname[0]) {
										case 'B':   
											star_class = (int8)ESPECTRAL_CLASS::B;	
											break;
										case 'A':   
											star_class = (int8)ESPECTRAL_CLASS::A;
											break;
										case 'F':   
											star_class = (int8)ESPECTRAL_CLASS::F;
											break;
										case 'G': 
											star_class = (int8)ESPECTRAL_CLASS::G;
											break;
										case 'K':   
											star_class = (int8)ESPECTRAL_CLASS::K; 
											break;
										case 'M':   
											star_class = (int8)ESPECTRAL_CLASS::M;
											break;
										case 'R':   
											star_class = (int8)ESPECTRAL_CLASS::RED_GIANT;
											break;
										case 'W':   
											star_class = (int8)ESPECTRAL_CLASS::WHITE_DWARF;
											break;
										case 'Z':   
											star_class = (int8)ESPECTRAL_CLASS::BLACK_HOLE; 
											break;
										}
									}
								}
							}

							if (sys_name[0]) {
								StarSystem* star_system = new StarSystem(sys_name, sys_loc, sys_iff, star_class);

								//SpawnSystem(FString(sys_name));

								SpawnSystem(FString(sys_name), fv, sys_iff, star_class);

								//star_system->Load();
								//systems.append(star_system);

								//Star* star = new Star(sys_name, sys_loc, star_class);
								//stars.append(star);
							}
						}
					}

					else if (def->name()->value() == "star") {
						if (!def->term() || !def->term()->isStruct()) {
							UE_LOG(LogTemp, Log, TEXT("WARNING: star struct missing in '%s'"), *FString(filename));
						}
						else {
							TermStruct* val = def->term()->isStruct();
							UE_LOG(LogTemp, Log, TEXT("%s"), *FString(def->name()->value()));
							char  star_name[32];
							char  classname[32];
							Vec3  star_loc;
							int   star_class = (int8)ESPECTRAL_CLASS::G;

							star_name[0] = 0;

							for (int i = 0; i < val->elements()->size(); i++) {
								TermDef* pdef = val->elements()->at(i)->isDef();
								if (pdef) {
									if (pdef->name()->value() == "name")
										GetDefText(star_name, pdef, filename);

									else if (pdef->name()->value() == "loc")
										GetDefVec(star_loc, pdef, filename);

									else if (pdef->name()->value() == "class") {
										GetDefText(classname, pdef, filename);

										switch (classname[0]) {
										case 'O':   
											star_class = (int8)ESPECTRAL_CLASS::O;  
											break;
										case 'B':   
											star_class = (int8)ESPECTRAL_CLASS::B;
											break;
										case 'A':   
											star_class = (int8)ESPECTRAL_CLASS::A; 
											break;
										case 'F':   
											star_class = (int8)ESPECTRAL_CLASS::F;  
											break;
										case 'G':   
											star_class = (int8)ESPECTRAL_CLASS::G; 
											break;
										case 'K':   
											star_class = (int8)ESPECTRAL_CLASS::K;  
											break;
										case 'M':   
											star_class = (int8)ESPECTRAL_CLASS::M;
											break;
										case 'R':   
											star_class = (int8)ESPECTRAL_CLASS::RED_GIANT;
											break;
										case 'W':   
											star_class = (int8)ESPECTRAL_CLASS::WHITE_DWARF;
											break;
										case 'Z':   
											star_class = (int8)ESPECTRAL_CLASS::BLACK_HOLE; 
											break;
										}
									}
								}
							}

							
							//if (star_name[0]) {
							//	Star* star = new Star(star_name, star_loc, star_class);
							//	stars.append(star);
							//}
						}
					}
				}
				UE_LOG(LogTemp, Log, TEXT("------------------------------------------------------------"));
			}
		} while (term);
}

void AGalaxy::SpawnSystem(FString sysName)
{
	UWorld* World = GetWorld();

	FVector location = FVector::ZeroVector;
	FRotator rotate = FRotator::ZeroRotator;

	FActorSpawnParameters SpawnInfo;

	AStarSystem* System = GetWorld()->SpawnActor<AStarSystem>(AStarSystem::StaticClass(), location, rotate, SpawnInfo);


	if (System)
	{
		UE_LOG(LogTemp, Log, TEXT("System Spawned"));
		System->Initialize(TCHAR_TO_ANSI(*sysName));
	}
	else {
		UE_LOG(LogTemp, Log, TEXT("Failed to Spawn System"));
	}
}

// +--------------------------------------------------------------------+

void AGalaxy::SpawnSystem(FString sysName, FVector sysLoc, int sysIFF, int starClass)
{
	UWorld* World = GetWorld();

	FRotator rotate = FRotator::ZeroRotator;

	FActorSpawnParameters SpawnInfo;

		AStarSystem* System = GetWorld()->SpawnActor<AStarSystem>(AStarSystem::StaticClass(), sysLoc, rotate, SpawnInfo);


	if (System)
	{
		UE_LOG(LogTemp, Log, TEXT("System Spawned"));
		System->Initialize(TCHAR_TO_ANSI(*sysName));
	}
	else {
		UE_LOG(LogTemp, Log, TEXT("Failed to Spawn System"));
	}	
}

void
AGalaxy::ExecFrame()
{
	//ListIter<StarSystem> sys = systems;
	//while (++sys) {
	//	sys->ExecFrame();
	//}
}

// +--------------------------------------------------------------------+

/*StarSystem*
AGalaxy::GetSystem(const char* name)
{
	ListIter<StarSystem> sys = systems;
	while (++sys) {
		if (!strcmp(sys->Name(), name))
			return sys.value();
	}

	return 0;
}

// +--------------------------------------------------------------------+

StarSystem*
AGalaxy::FindSystemByRegion(const char* rgn_name)
{
	ListIter<StarSystem> iter = systems;
	while (++iter) {
		StarSystem* sys = iter.value();
		if (sys->FindRegion(rgn_name))
			return sys;
	}

	return 0;
}*/


