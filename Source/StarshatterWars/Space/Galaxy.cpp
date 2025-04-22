/*  Project Starshatter Wars
	Fractal Dev Games
	Copyright (C) 2024. All Rights Reserved.

	SUBSYSTEM:    Space
	FILE:         Galaxy.cpp
	AUTHOR:       Carlos Bott
*/



#include "Galaxy.h"
#include "StarSystem.h"
#include "../System/SSWGameInstance.h"
#include "../Foundation/ParseUtil.h"
#include "../Foundation/DataLoader.h"
#include "Engine/DataTable.h"

// Called when the game starts or when spawned
void AGalaxy::BeginPlay()
{
	Super::BeginPlay();
	UE_LOG(LogTemp, Log, TEXT("Loading Galaxy Game Data"));

	LoadGalaxyFromDT();
}

void AGalaxy::LoadGalaxyFromDT() // Test
{
	UE_LOG(LogTemp, Log, TEXT("AGalaxy::LoadGalaxyFromDT()"));
	
	TArray<FName> RowNames = GalaxyDataTable->GetRowNames();

	for (FName Item : RowNames) {
		FS_Galaxy* SystemName = GalaxyDataTable->FindRow<FS_Galaxy>(Item, "");

		FString Name = SystemName->Name;

		UE_LOG(LogTemp, Log, TEXT("System Name: %s"), *Name);
		SpawnSystem(FString(Name));
	}
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
	Root = CreateDefaultSubobject<USceneComponent>(TEXT("StarSystem Scene Component"));
	RootComponent = Root;

	static ConstructorHelpers::FObjectFinder<UDataTable> GalaxyDataTableObject(TEXT("DataTable'/Game/Game/DT_Galaxy.DT_Galaxy'"));

	if (GalaxyDataTableObject.Succeeded())
	{
		GalaxyDataTable = GalaxyDataTableObject.Object;
		//GalaxyDataTable->EmptyTable();
	}

	static ConstructorHelpers::FClassFinder<AStarSystem> StarSystemObj(TEXT("/Script/Engine.Blueprint'/Game/Game/BP_StarSystem.BP_StarSystem_C'"));
	if (StarSystemObj.Succeeded())
	{
		StarSystemObject = StarSystemObj.Class;
	}

	PrimaryActorTick.bCanEverTick = true;
}

AGalaxy::AGalaxy(const char* n)	
{ 
	name = n;
	radius = 10;
}

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
											star_class = (int8)				  ESPECTRAL_CLASS::RED_GIANT;
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

							// define our data table struct
							FS_Galaxy NewGalaxyData;
							NewGalaxyData.Name = FString(sys_name);
							NewGalaxyData.Class = star_class;
							NewGalaxyData.Iff = sys_iff;
							NewGalaxyData.Location = fv;
							NewGalaxyData.Empire = GetEmpireName(sys_iff);
							FName RowName = FName(FString(sys_name));
							
							// call AddRow to insert the record
							GalaxyDataTable->AddRow(RowName, NewGalaxyData);
							
							GalaxyData = NewGalaxyData;

							if (sys_name[0]) {

								SpawnSystem(FString(sys_name));

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



void AGalaxy::Initialize()
{

}

void AGalaxy::SpawnSystem(FString sysName)
{
	UWorld* World = GetWorld();

	FVector location = FVector::ZeroVector;
	FRotator rotate = FRotator::ZeroRotator;

	FTransform ReturnTransform(rotate, location, FVector(1, 1, 1));
	FActorSpawnParameters SpawnInfo;

	AStarSystem* System = GetWorld()->SpawnActorDeferred<AStarSystem>(StarSystemObject, ReturnTransform);


	if (System)
	{
		UE_LOG(LogTemp, Log, TEXT("System Spawned"));
		System->Initialize(sysName);
		System->FinishSpawning(ReturnTransform, true);
	}
	else {
		UE_LOG(LogTemp, Log, TEXT("Failed to Spawn System"));
	}
}

// +--------------------------------------------------------------------+

void AGalaxy::SpawnSystem(FString sysName, FVector sysLoc, int sysIFF, int starClass)
{
	UWorld* World = GetWorld();

	FVector location = FVector::ZeroVector;
	FRotator rotate = FRotator::ZeroRotator;

	FTransform ReturnTransform(rotate, location, FVector(1, 1, 1));
	FActorSpawnParameters SpawnInfo;

	AStarSystem* System = GetWorld()->SpawnActorDeferred<AStarSystem>(StarSystemObject, ReturnTransform);
		
	if (System)
	{
		System->AttachToActor(this, FAttachmentTransformRules::KeepWorldTransform); System->AttachToActor(this, FAttachmentTransformRules::KeepWorldTransform);
		UE_LOG(LogTemp, Log, TEXT("System Spawned: '%s'"), *sysName);
		System->SystemName = sysName;
		System->Initialize(TCHAR_TO_ANSI(*sysName));
		System->FinishSpawning(ReturnTransform, true);
	}
	else {
		UE_LOG(LogTemp, Log, TEXT("Failed to Spawn System"));
	}	
}

void
AGalaxy::ExecFrame()
{
	ListIter<AStarSystem> sys = systems;
	while (++sys) {
		sys->ExecFrame();
	}
}

// +--------------------------------------------------------------------+

AStarSystem*
AGalaxy::GetSystem(const char* Name)
{
	ListIter<AStarSystem> sys = systems;
	while (++sys) {
		if (!strcmp(sys->Name(), Name))
			return sys.value();
	}

	return 0;
}

// +--------------------------------------------------------------------+

AStarSystem*
AGalaxy::FindSystemByRegion(const char* rgn_name)
{
	ListIter<AStarSystem> iter = systems;
	while (++iter) {
		AStarSystem* sys = iter.value();
		//if (sys->FindRegion(rgn_name))
		//	return sys;
	}

	return 0;
}

EEMPIRE_NAME
AGalaxy::GetEmpireName(int32 emp)
{
	EEMPIRE_NAME empire_name;

	switch (emp)
	{
	case 0:
		empire_name = EEMPIRE_NAME::Terellian;
		break;
	case 1:
		empire_name = EEMPIRE_NAME::Marakan;
		break;
	case 2:
		empire_name = EEMPIRE_NAME::Independent;
		break;
	case 3:
		empire_name = EEMPIRE_NAME::Dantari;
		break;
	case 4:
		empire_name = EEMPIRE_NAME::Zolon;
		break;
	case 5:
		empire_name = EEMPIRE_NAME::Other;
		break;
	case 6:
		empire_name = EEMPIRE_NAME::Pirate;
		break;
	case 7:
		empire_name = EEMPIRE_NAME::Neutral;
		break;
	case 8:
		empire_name = EEMPIRE_NAME::Unknown;
		break;
	case 9:
		empire_name = EEMPIRE_NAME::Silessian;
		break;
	case 10:
		empire_name = EEMPIRE_NAME::Solus;
		break;
	case 11:
		empire_name = EEMPIRE_NAME::Haiche;
		break;
	default:
		empire_name = EEMPIRE_NAME::Unknown;
		break;
	}
	return empire_name;
}


