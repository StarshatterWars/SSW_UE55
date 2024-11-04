// /*  Project nGenEx	Fractal Dev Games	Copyright (C) 2024. All Rights Reserved.	SUBSYSTEM:    SSW	FILE:         Game.cpp	AUTHOR:       Carlos Bott*/


#include "StarSystem.h"
#include "Galaxy.h"
#include "OrbitalBody.h"
#include "OrbitalRegion.h"
//#include "Sky.h"
//#include "Starshatter.h"
//#include "TerrainRegion.h"
//#include "TerrainHaze.h"
//#include "Weather.h"
#include "Star.h"
#include "../System/Game.h"
#include "Engine/World.h"
//#include "Sound.h"
//#include "Solid.h"
//#include "Light.h"
//#include "Bitmap.h"
#include "../Foundation/DataLoader.h"
#include "../System/SSWGameInstance.h"
//#include "Scene.h"
#include "GameTime.h"
#include "../Foundation/ParseUtil.h"

const double epoch = 0.5e9;
double AStarSystem::StarDate = 0;
double AStarSystem::RealTimeSeconds = 0;
static const double GRAV = 6.673e-11;
static const int    NAMELEN = 64;

// +====================================================================+

static double base_time = 0;
static WORD   oldcw = 0;
static WORD   fpcw = 0;


// Sets default values
AStarSystem::AStarSystem()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("StarSystem Scene Component"));
	RootComponent = Root;

	static ConstructorHelpers::FObjectFinder<UDataTable> StarsDataTableObject(TEXT("DataTable'/Game/Game/DT_Stars.DT_Stars'"));

	if (StarsDataTableObject.Succeeded())
	{
		StarsDataTable = StarsDataTableObject.Object;
		//StarsDataTable->EmptyTable();
	}
	else {
		UE_LOG(LogTemp, Log, TEXT("Failed to get Stars Data Table"));
	}

	static ConstructorHelpers::FObjectFinder<UDataTable> PlanetsDataTableObject(TEXT("DataTable'/Game/Game/DT_Planets.DT_Planets'"));

	if (PlanetsDataTableObject.Succeeded())
	{
		PlanetsDataTable = PlanetsDataTableObject.Object;
		//StarsDataTable->EmptyTable();
	}
	else {
		UE_LOG(LogTemp, Log, TEXT("Failed to get Planets Data Table"));
	}

	static ConstructorHelpers::FObjectFinder<UDataTable> MoonsDataTableObject(TEXT("DataTable'/Game/Game/DT_Moons.DT_Moons'"));

	if (MoonsDataTableObject.Succeeded())
	{
		MoonsDataTable = MoonsDataTableObject.Object;
		//StarsDataTable->EmptyTable();
	}
	else {
		UE_LOG(LogTemp, Log, TEXT("Failed to get Planets Data Table"));
	}

	static ConstructorHelpers::FObjectFinder<UDataTable> RegionsDataTableObject(TEXT("DataTable'/Game/Game/DT_Regions.DT_Regions'"));

	if (RegionsDataTableObject.Succeeded())
	{
		RegionsDataTable = RegionsDataTableObject.Object;
		//StarsDataTable->EmptyTable();
	}
	else {
		UE_LOG(LogTemp, Log, TEXT("Failed to get Regions Data Table"));
	}

	static ConstructorHelpers::FClassFinder<AOrbitalBody> StarObject(TEXT("/Script/Engine.Blueprint'/Game/Game/BP_Star.BP_Star_C'"));
	if (StarObject.Succeeded())
	{
		CentralStar = StarObject.Class;
	}

	static ConstructorHelpers::FClassFinder<AOrbitalBody> PlanetObj(TEXT("/Script/Engine.Blueprint'/Game/Game/BP_Planet.BP_Planet_C'"));
	if (PlanetObj.Succeeded())
	{
		PlanetObject = PlanetObj.Class;
	}

	static ConstructorHelpers::FClassFinder<AOrbitalBody> MoonObj(TEXT("/Script/Engine.Blueprint'/Game/Game/BP_Moon.BP_Moon_C'"));
	if (MoonObj.Succeeded())
	{
		MoonObject = MoonObj.Class;
	}
}

// Called when the game starts or when spawned
void AStarSystem::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AStarSystem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	ExecFrame();
}

void AStarSystem::ExecFrame()
{
	UWorld* World = GetWorld();
	RealTimeSeconds = UGameplayStatics::GetTimeSeconds(World);
	CalcStardate(RealTimeSeconds);

	ListIter<AOrbitalBody> star = bodies;
	while (++star)
		star->Update();

	ListIter<AOrbitalRegion> region = regions;
	while (++region)
		region->Update();

	// update the graphic reps, relative to the active region:
	/*if (instantiated && active_region) {
		Point active_loc = active_region->Location();
		active_loc = active_loc.OtherHand();

		Scene* scene = 0;
		TerrainRegion* trgn = 0;
		bool           terrain = (active_region->Type() == Orbital::TERRAIN);
		Matrix         terrain_orientation;
		Matrix         terrain_transformation;

		if (terrain) {
			trgn = (TerrainRegion*)active_region;
			Color tmp = trgn->SkyColor();
			Game::SetScreenColor(tmp);

			tvpn = (active_region->Location() - active_region->Primary()->Location());
			tvpn.Normalize();
			tvup = Point(0, 0, -1);
			tvrt = tvpn.cross(tvup);
			tvrt.Normalize();

			terrain_orientation.Rotate(0, PI / 2, 0);
			terrain_transformation = Matrix(tvrt, tvup, tvpn);

			if (point_stars) {
				point_stars->SetOrientation(terrain_transformation);

				Color sky_color = trgn->SkyColor();
				BYTE  sky_red = (BYTE)sky_color.Red();
				BYTE  sky_green = (BYTE)sky_color.Green();
				BYTE  sky_blue = (BYTE)sky_color.Blue();

				BYTE Max = max3(sky_red, sky_green, sky_blue);
				BYTE Min = min3(sky_red, sky_green, sky_blue);

				BYTE lum = (BYTE)(240.0 * (Max + Min) / 510.0);

				if (lum > 50) {
					point_stars->Hide();
				}
				else {
					Stars* pstars = (Stars*)point_stars;

					pstars->Illuminate(1.0 - lum / 50.0);
					pstars->Show();
				}

				scene = point_stars->GetScene();
			}

			if (haze) {
				((TerrainHaze*)haze)->UseTerrainRegion(trgn);
				scene = haze->GetScene();
			}

			if (scene) {
				scene->SetAmbient(ambient);
			}

		}
		else {
			Game::SetScreenColor(Color::Black);
		}*/

		double star_alt = 0;

		ListIter<AOrbitalBody> star_iter = bodies;
		while (++star_iter) {
			AOrbitalBody* Star = star_iter.value();

			if (active_region->Inclination() != 0) {
				double distance = (active_region->Location() - Star->Location()).length();
				star_alt = sin(active_region->Inclination()) * distance;
			}

			/*if (terrain) {
				Point sloc = TerrainTransform(star->Location());

				if (star->rep) {
					star->rep->MoveTo(sloc);

					PlanetRep* pr = (PlanetRep*)star->rep;
					pr->SetDaytime(true);
				}

				if (star->light_rep) {
					star->light_rep->MoveTo(sloc);
					star->light_rep->SetActive(sloc.y > -100);
				}

				if (star->back_light) {
					star->back_light->MoveTo(sloc * -1);
					star->back_light->SetActive(sloc.y > -100);
				}

				if (trgn && star->rep) {
					if (trgn->IsEclipsed())
						star->rep->Hide();
					else
						star->rep->Show();
				}
			}

			else {
				Point sloc = Point(star->Location() - active_region->Location()).OtherHand();
				sloc.y = star_alt;

				if (star->rep) {
					star->rep->MoveTo(sloc);

					PlanetRep* pr = (PlanetRep*)star->rep;
					pr->SetDaytime(false);
				}

				if (star->light_rep) {
					star->light_rep->MoveTo(sloc);
					star->light_rep->SetActive(true);
				}

				if (star->back_light) {
					star->back_light->MoveTo(sloc * -1);
					star->back_light->SetActive(true);
				}
			}

			ListIter<AOrbitalBody> planet_iter = star->Satellites();
			while (++planet_iter) {
				AOrbitalBody* planet = planet_iter.value();

				if (planet->rep) {
					PlanetRep* pr = (PlanetRep*)planet->rep;

					if (terrain) {
						pr->MoveTo(TerrainTransform(planet->Location()));
						pr->SetOrientation(terrain_orientation);

						if (planet == active_region->Primary()) {
							pr->Hide();
						}

						else {
							pr->Show();
							pr->SetDaytime(true);
						}
					}
					else {
						pr->Show();
						pr->TranslateBy(active_loc);
						pr->SetDaytime(false);
					}
				}

				ListIter<AOrbitalBody> moon_iter = planet->Satellites();
				while (++moon_iter) {
					OrbitalBody* moon = moon_iter.value();

					if (moon->rep) {
						PlanetRep* pr = (PlanetRep*)moon->rep;

						if (terrain) {
							pr->MoveTo(TerrainTransform(moon->Location()));
							pr->SetOrientation(terrain_orientation);

							if (moon == active_region->Primary()) {
								pr->Hide();
							}

							else {
								pr->Show();
								pr->SetDaytime(true);
							}
						}
						else {
							pr->Show();
							pr->TranslateBy(active_loc);
							pr->SetDaytime(false);
						}
					}
				}
			}
		}*/
	}
}

void AStarSystem::Load()
{
	CalcStardate(RealTimeSeconds);
	//active_region = 0;
	BYTE* block = 0;

	USSWGameInstance* SSWInstance = (USSWGameInstance*)GetGameInstance();
	SSWInstance->loader->GetLoader();

	ProjectPath = FPaths::ProjectDir();
	ProjectPath.Append(TEXT("GameData/Galaxy/"));
	FString FileName = ProjectPath;

	FileName.Append(FilePath);

	sprintf_s(filename, "%s/%s.def", (const char*)name, (const char*)name);

	FileName.Append(filename);
	SSWInstance->loader->SetDataPath(FileName);

	FString fs = FString(FileName);
	FString FileString;

	UE_LOG(LogTemp, Log, TEXT("Loading StarSystem: '%s'"), *FString(name));
	UE_LOG(LogTemp, Log, TEXT("StarSystem Path '%s'"), *FileName);
	const char* result = TCHAR_TO_ANSI(*FileName);
	SSWInstance->loader->ReleaseBuffer(block);
	Load(result);
}

void AStarSystem::Load(const char* FileName)
{
	USSWGameInstance* SSWInstance = (USSWGameInstance*)GetGameInstance();
	SSWInstance->loader->GetLoader();

	FString fs = FString(ANSI_TO_TCHAR(FileName));
	FString FileString;
	BYTE* block = 0;

	if (FFileHelper::LoadFileToString(FileString, *fs, FFileHelper::EHashOptions::None))
	{
		UE_LOG(LogTemp, Log, TEXT("%s"), *FileString);
	}

	SSWInstance->loader->LoadBuffer(FileName, block, true);

	if (!block) {
		UE_LOG(LogTemp, Log, TEXT("ERROR: invalid star system file '%s'"), *FString(FileName));
		exit(-2);
		return;
	}

	Parser parser(new BlockReader((const char*)block));

	Term* term = parser.ParseTerm();

	if (!term) {
		UE_LOG(LogTemp, Log, TEXT("ERROR: could not parse '%s'"), *FString(FileName));
		exit(-3);
		return;
	}
	else {
		TermText* file_type = term->isText();
		if (!file_type || file_type->value() != "STARSYSTEM") {
			UE_LOG(LogTemp, Log, TEXT("ERROR: invalid star system file '%s'"), *FString(FileName));
			term->print(10);
			exit(-4);
			return;
		}
	}

	// parse the system:
	do {
		delete term;
		term = parser.ParseTerm();

		if (term) {
			TermDef* def = term->isDef();
			if (def) {
				if (def->name()->value() == "name") {
					char namebuf[NAMELEN];
					namebuf[0] = 0;
					GetDefText(namebuf, def, filename);

					if (namebuf[0])
						name = namebuf;
				}

				else if (def->name()->value() == "sky") {
					if (!def->term() || !def->term()->isStruct()) {
						Print("WARNING: sky struct missing in '%s'\n", filename);
					}
					else {
						TermStruct* val = def->term()->isStruct();

						char  imgname[NAMELEN];
						char  magname[NAMELEN];
						char  hazname[NAMELEN];

						imgname[0] = 0;
						magname[0] = 0;
						hazname[0] = 0;

						for (int i = 0; i < val->elements()->size(); i++) {
							TermDef* pdef = val->elements()->at(i)->isDef();
							if (pdef) {
								if (pdef->name()->value() == "poly_stars")
									GetDefText(imgname, pdef, filename);

								else if (pdef->name()->value() == "nebula")
									GetDefText(magname, pdef, filename);

								else if (pdef->name()->value() == "haze")
									GetDefText(hazname, pdef, filename);
							}
						}

						if (imgname[0])
							sky_poly_stars = imgname;

						if (magname[0])
							sky_nebula = magname;

						if (hazname[0])
							sky_haze = hazname;
					}
				}

				else if (def->name()->value() == "stars") {
					GetDefNumber(sky_stars, def, filename);
				}

				else if (def->name()->value() == "ambient") {
					Vec3 a;
					GetDefVec(a, def, filename);

					ambient = Color((BYTE)a.x, (BYTE)a.y, (BYTE)a.z) * 2.5;
				}

				else if (def->name()->value() == "dust") {
					GetDefNumber(sky_dust, def, filename);
				}

				else if (def->name()->value() == "star") {
					if (!def->term() || !def->term()->isStruct()) {
						Print("WARNING: star struct missing in '%s'\n", filename);
					}
					else {
						TermStruct* val = def->term()->isStruct();
						ParseStar(val);
					}
				}

				else if (def->name()->value() == "planet") {
					if (!def->term() || !def->term()->isStruct()) {
						Print("WARNING: planet struct missing in '%s'\n", filename);
					}
					else {
						TermStruct* val = def->term()->isStruct();
						ParsePlanet(val);
					}
				}

				else if (def->name()->value() == "moon") {
					if (!def->term() || !def->term()->isStruct()) {
						Print("WARNING: moon struct missing in '%s'\n", filename);
					}
					else {
						TermStruct* val = def->term()->isStruct();
						ParseMoon(val);
					}
				}

				else if (def->name()->value() == "region") {
					if (!def->term() || !def->term()->isStruct()) {
						Print("WARNING: region struct missing in '%s'\n", filename);
					}
					else {
						TermStruct* val = def->term()->isStruct();
						ParseRegion(val);
					}
				}

				else if (def->name()->value() == "terrain") {
					if (!def->term() || !def->term()->isStruct()) {
						Print("WARNING: terrain struct missing in '%s'\n", filename);
					}
					else {
						TermStruct* val = def->term()->isStruct();
						ParseTerrain(val);
					}
				}

			}
		}
	} while (term);

	SSWInstance->loader->ReleaseBuffer(block);
	//this->SetActorLabel(FString(name).Append(" System"));
}

void AStarSystem::Initialize(const char* Name)
{
	name = Name;
	UE_LOG(LogTemp, Log, TEXT("StarSystem Spawned: '%s'"), *FString(Name));
	this->SetActorLabel(FString(Name));
	Load();
}

void AStarSystem::Create()
{

}

void AStarSystem::Destroy()
{

}

Color AStarSystem::Ambient() const
{
	return Color();
}

AOrbital* AStarSystem::FindOrbital(const char* oname)
{
	if (!name || !name[0])
		return 0;

	ListIter<AOrbitalBody> star = bodies;
	while (++star) {
		if (!_stricmp(star->Name(), name))
			return star.value();

		ListIter<AOrbitalRegion> star_rgn = star->Regions();
		while (++star_rgn) {
			if (!_stricmp(star_rgn->Name(), name))
				return star_rgn.value();
		}

		ListIter<AOrbitalBody> planet = star->Satellites();
		while (++planet) {
			if (!_stricmp(planet->Name(), name))
				return planet.value();

			ListIter<AOrbitalRegion> planet_rgn = planet->Regions();
			while (++planet_rgn) {
				if (!_stricmp(planet_rgn->Name(), name))
					return planet_rgn.value();
			}

			ListIter<AOrbitalBody> moon = planet->Satellites();
			while (++moon) {
				if (!_stricmp(moon->Name(), name))
					return moon.value();

				ListIter<AOrbitalRegion> moon_rgn = moon->Regions();
				while (++moon_rgn) {
					if (!_stricmp(moon_rgn->Name(), name))
						return moon_rgn.value();
				}
			}
		}
	}

	ListIter<AOrbitalRegion> region = regions;
	while (++region) {
		if (!_stricmp(region->Name(), name))
			return region.value();
	}

	return 0;
}

AOrbitalRegion* AStarSystem::FindRegion(const char* regname)
{

	if (!name || !name[0])
		return 0;

	ListIter<AOrbitalRegion> region = all_regions;
	while (++region) {
		if (!_stricmp(region->Name(), name))
			return region.value();
	}

	return 0;
}

void AStarSystem::SetActiveRegion(AOrbitalRegion* rgn)
{
}

void AStarSystem::SetBaseTime(double t, bool absolute)
{
	if (absolute) {
		base_time = t;
		CalcStardate(AStarSystem::RealTimeSeconds);
	}

	else if (t > 0) {
		if (t > epoch) t -= epoch;
		base_time = t;
		CalcStardate(RealTimeSeconds);
	}
}

double AStarSystem::GetBaseTime()
{
	return base_time;
}

void AStarSystem::CalcStardate(double Sec)
{
	
	if (base_time < 1) {
		time_t clock_seconds;
		time(&clock_seconds);

		base_time = clock_seconds;

		while (base_time < 0)
			base_time += epoch;
	}

	double gtime = (double)Game::GameTime();
	
	double sdate = Sec + base_time + epoch;

	StarDate = sdate;
	UE_LOG(LogTemp, Log, TEXT("Stardate: '%f'"), sdate);

}

void AStarSystem::SetSunlight(Color color, double brightness)
{
}

void AStarSystem::SetBacklight(Color color, double brightness)
{
}

void AStarSystem::RestoreTrueSunColor()
{
}

bool AStarSystem::HasLinkTo(AStarSystem* s) const
{
	ListIter<AOrbitalRegion> iter = ((AStarSystem*)this)->all_regions;
	while (++iter) {
		AOrbitalRegion* rgn = iter.value();

		ListIter<Text> lnk_iter = rgn->Links();
		while (++lnk_iter) {
			Text* t = lnk_iter.value();

			if (s->FindRegion(*t))
				return true;
		}
	}

	return false;
}

void AStarSystem::ParseStar(TermStruct* val)
{
	USSWGameInstance* SSWInstance = (USSWGameInstance*)GetGameInstance();

	char  star_name[NAMELEN];
	char  img_name[NAMELEN];
	char  map_name[NAMELEN];
	double light = 0.0;
	double Radius = 0.0;
	double rot = 0.0;
	double mass = 0.0;
	double orbit = 0.0;
	double tscale = 1.0;
	bool   retro = false;
	Color  color;
	Color  back;
	FColor StarColor;
	FColor BackColor;

	for (int i = 0; i < val->elements()->size(); i++) {
		SystemParent = this;

		TermDef* pdef = val->elements()->at(i)->isDef();
		if (pdef) {
			if (pdef->name()->value() == "name")
				GetDefText(star_name, pdef, filename);

			else if (pdef->name()->value() == "map" || pdef->name()->value() == "icon")
				GetDefText(map_name, pdef, filename);

			else if (pdef->name()->value() == "image")
				GetDefText(img_name, pdef, filename);

			else if (pdef->name()->value() == "mass")
				GetDefNumber(mass, pdef, filename);

			else if (pdef->name()->value() == "orbit")
				GetDefNumber(orbit, pdef, filename);

			else if (pdef->name()->value() == "radius")
				GetDefNumber(Radius, pdef, filename);

			else if (pdef->name()->value() == "rotation")
				GetDefNumber(rot, pdef, filename);

			else if (pdef->name()->value() == "tscale")
				GetDefNumber(tscale, pdef, filename);

			else if (pdef->name()->value() == "light")
				GetDefNumber(light, pdef, filename);

			else if (pdef->name()->value() == "retro")
				GetDefBool(retro, pdef, filename);

			else if (pdef->name()->value() == "color") {
				Vec3 a;
				GetDefVec(a, pdef, filename);
				color = Color((BYTE)a.x, (BYTE)a.y, (BYTE)a.z);
				StarColor = FColor(a.x, a.y, a.z, 1);
			}

			else if (pdef->name()->value() == "back" || pdef->name()->value() == "back_color") {
				Vec3 a;
				GetDefVec(a, pdef, filename);
				back = Color((BYTE)a.x, (BYTE)a.y, (BYTE)a.z);
				BackColor = FColor(a.x, a.y, a.z, 1);
			}
		}

		// define our data table struct
		FS_Star NewStarData;
		NewStarData.Name = FString(star_name);
		NewStarData.Map = FString(map_name);
		NewStarData.Image = FString(img_name);
		NewStarData.Light = light;
		NewStarData.Radius = Radius;
		NewStarData.Rot = rot;
		NewStarData.Mass = mass;
		NewStarData.Orbit = orbit;
		NewStarData.Tscale = tscale;
		NewStarData.Retro = retro;
		NewStarData.Color = StarColor;
		NewStarData.Back = BackColor;

		FName RowName = FName(FString(star_name));


		// call AddRow to insert the record
		StarsDataTable->AddRow(RowName, NewStarData);
	}

	SpawnStar(FString(star_name), mass, Radius, orbit, rot);
	
	if(PlanetParent) {
		PlanetParent->map_name = map_name;
		PlanetParent->tex_name = img_name;
		PlanetParent->light = light;
		PlanetParent->tscale = tscale;
		PlanetParent->subtype = ESPECTRAL_CLASS::G;
		PlanetParent->retro = retro;
		PlanetParent->color = color;
		PlanetParent->back = back;

		bodies.append(PlanetParent);
	}

	// map icon:
	//if (*map_name) {
	//	SSWInstance->loader->GetLoader()->LoadBitmap(map_name, star->map_icon, Bitmap::BMP_TRANSLUCENT, true);
	//}

	//primary_star = star;
	//primary_planet = 0;
	//primary_moon = 0;

	if (orbit > AStarSystem::radius) {
		AStarSystem::radius = orbit;
	}
}

void AStarSystem::ParsePlanet(TermStruct* val)
{
	char   pln_name[NAMELEN] = "";
	char   img_name[NAMELEN] = "";
	char   map_name[NAMELEN] = "";
	char   hi_name[NAMELEN] = "";
	char   img_ring[NAMELEN] = "";
	char   glo_name[NAMELEN] = "";
	char   glo_hi_name[NAMELEN] = "";
	char   gloss_name[NAMELEN] = "";

	double Radius = 0.0;
	double mass = 0.0;
	double orbit = 0.0;
	double rot = 0.0;
	double minrad = 0.0;
	double maxrad = 0.0;
	double tscale = 1.0;
	double tilt = 0.0;
	bool   retro = false;
	bool   lumin = false;
	Color  atmos = Color::Black;
	FColor AtmosColor = FColor::Black;

	for (int i = 0; i < val->elements()->size(); i++) {
		TermDef* pdef = val->elements()->at(i)->isDef();
		if (pdef) {
			if (pdef->name()->value() == "name")
				GetDefText(pln_name, pdef, filename);

			else if (pdef->name()->value() == "map" || pdef->name()->value() == "icon")
				GetDefText(map_name, pdef, filename);

			else if (pdef->name()->value() == "image")
				GetDefText(img_name, pdef, filename);

			else if (pdef->name()->value() == "image_west")
				GetDefText(img_name, pdef, filename);

			else if (pdef->name()->value() == "image_east")
				GetDefText(img_name, pdef, filename);

			else if (pdef->name()->value() == "glow")
				GetDefText(glo_name, pdef, filename);

			else if (pdef->name()->value() == "gloss")
				GetDefText(gloss_name, pdef, filename);

			else if (pdef->name()->value() == "high_res")
				GetDefText(hi_name, pdef, filename);

			else if (pdef->name()->value() == "high_res_west")
				GetDefText(hi_name, pdef, filename);

			else if (pdef->name()->value() == "high_res_east")
				GetDefText(hi_name, pdef, filename);

			else if (pdef->name()->value() == "glow_high_res")
				GetDefText(glo_hi_name, pdef, filename);

			else if (pdef->name()->value() == "mass")
				GetDefNumber(mass, pdef, filename);

			else if (pdef->name()->value() == "orbit")
				GetDefNumber(orbit, pdef, filename);

			else if (pdef->name()->value() == "retro")
				GetDefBool(retro, pdef, filename);

			else if (pdef->name()->value() == "luminous")
				GetDefBool(lumin, pdef, filename);

			else if (pdef->name()->value() == "rotation")
				GetDefNumber(rot, pdef, filename);

			else if (pdef->name()->value() == "radius")
				GetDefNumber(Radius, pdef, filename);

			else if (pdef->name()->value() == "ring")
				GetDefText(img_ring, pdef, filename);

			else if (pdef->name()->value() == "minrad")
				GetDefNumber(minrad, pdef, filename);

			else if (pdef->name()->value() == "maxrad")
				GetDefNumber(maxrad, pdef, filename);

			else if (pdef->name()->value() == "tscale")
				GetDefNumber(tscale, pdef, filename);

			else if (pdef->name()->value() == "tilt")
				GetDefNumber(tilt, pdef, filename);

			else if (pdef->name()->value() == "atmosphere") {
				Vec3 a;
				GetDefVec(a, pdef, filename);
				atmos = Color((BYTE)a.x, (BYTE)a.y, (BYTE)a.z);
				AtmosColor = FColor(a.x, a.y, a.z, 1);
			}
		}

		// define our data table struct
		FS_Planet NewPlanetData;
		NewPlanetData.Name = FString(pln_name);
		NewPlanetData.Map = FString(map_name);
		NewPlanetData.Image = FString(img_name);
		NewPlanetData.High = FString(hi_name); 
		NewPlanetData.Rings = FString(img_ring); 
		NewPlanetData.Glow = FString(glo_name);
		NewPlanetData.GlowHigh = FString(glo_hi_name);
		NewPlanetData.Gloss = FString(gloss_name);
		NewPlanetData.Radius = Radius;
		NewPlanetData.Mass = mass;
		NewPlanetData.Orbit = orbit;
		NewPlanetData.Rot = rot * 3600;
		NewPlanetData.Minrad = minrad;
		NewPlanetData.Maxrad = maxrad;	
		NewPlanetData.Tscale = tscale;
		NewPlanetData.Tilt = tilt;
		NewPlanetData.Retro = retro;
		NewPlanetData.Lumin = lumin;
		NewPlanetData.Atmos = AtmosColor;

		FName RowName = FName(FString(pln_name));

		// call AddRow to insert the record
		PlanetsDataTable->AddRow(RowName, NewPlanetData);
	}

	SpawnPlanet(FString(pln_name), mass, Radius, orbit, rot);
	
	//if (primary_star)
	//	primary_star->satellites.append(planet);
	//else
	//	bodies.append(planet);

	//primary_planet = planet;
	//primary_moon = 0;

	if (orbit > AStarSystem::radius)
		AStarSystem::radius = orbit;

	// map icon:
	//if (map_name[0]) {
	//	DataLoader::GetLoader()->LoadBitmap(map_name, planet->map_icon, Bitmap::BMP_TRANSLUCENT, true);
	//}
}

void AStarSystem::ParseMoon(TermStruct* val)
{
	char   map_name[NAMELEN] = "";
	char   pln_name[NAMELEN] = "";
	char   img_name[NAMELEN] = "";
	char   hi_name[NAMELEN] = "";
	char   glo_name[NAMELEN] = "";
	char   glo_hi_name[NAMELEN] = "";
	char   gloss_name[NAMELEN] = "";

	double Radius = 0.0;
	double mass = 0.0;
	double orbit = 0.0;
	double rot = 0.0;
	double tscale = 1.0;
	double tilt = 0.0;
	bool   retro = false;
	Color  atmos = Color::Black;
	FColor AtmosColor = FColor::Black;

	for (int i = 0; i < val->elements()->size(); i++) {
		TermDef* pdef = val->elements()->at(i)->isDef();
		if (pdef) {
			if (pdef->name()->value() == "name")
				GetDefText(pln_name, pdef, filename);

			else if (pdef->name()->value() == "map" || pdef->name()->value() == "icon")
				GetDefText(map_name, pdef, filename);

			else if (pdef->name()->value() == "image")
				GetDefText(img_name, pdef, filename);

			else if (pdef->name()->value() == "glow")
				GetDefText(glo_name, pdef, filename);

			else if (pdef->name()->value() == "high_res")
				GetDefText(hi_name, pdef, filename);

			else if (pdef->name()->value() == "glow_high_res")
				GetDefText(glo_hi_name, pdef, filename);

			else if (pdef->name()->value() == "gloss")
				GetDefText(gloss_name, pdef, filename);

			else if (pdef->name()->value() == "mass")
				GetDefNumber(mass, pdef, filename);

			else if (pdef->name()->value() == "orbit")
				GetDefNumber(orbit, pdef, filename);

			else if (pdef->name()->value() == "rotation")
				GetDefNumber(rot, pdef, filename);

			else if (pdef->name()->value() == "retro")
				GetDefBool(retro, pdef, filename);

			else if (pdef->name()->value() == "radius")
				GetDefNumber(Radius, pdef, filename);

			else if (pdef->name()->value() == "tscale")
				GetDefNumber(tscale, pdef, filename);

			else if (pdef->name()->value() == "inclination")
				GetDefNumber(tilt, pdef, filename);

			else if (pdef->name()->value() == "atmosphere") {
				Vec3 a;
				GetDefVec(a, pdef, filename);
				atmos = Color((BYTE)a.x, (BYTE)a.y, (BYTE)a.z);
			}
		}
		// define our data table struct
		FS_Moon NewMoonData;
		NewMoonData.Name = FString(pln_name);
		NewMoonData.Map = FString(map_name);
		NewMoonData.Image = FString(img_name);
		NewMoonData.High = FString(hi_name);
		NewMoonData.GlowHigh = FString(glo_hi_name);
		NewMoonData.Gloss = FString(gloss_name);
		NewMoonData.Radius = Radius;
		NewMoonData.Mass = mass;
		NewMoonData.Orbit = orbit;
		NewMoonData.Rot = rot * 3600;
		NewMoonData.Tscale = tscale;
		NewMoonData.Tilt = tilt;
		NewMoonData.Retro = retro;
		NewMoonData.Atmos = AtmosColor;

		FName RowName = FName(FString(pln_name));

		// call AddRow to insert the record
		MoonsDataTable->AddRow(RowName, NewMoonData);
	}

	SpawnMoon(FString(pln_name), mass, Radius, orbit, rot);
	
	//if (primary_planet)
	//	primary_planet->satellites.append(moon);
	//else {
	//	Print("WARNING: no planet for moon %s in '%s', deleted.\n", pln_name, filename);
	//	delete moon;
	//	moon = 0;
	//}

	//primary_moon = moon;

	// map icon:
	//if (map_name[0]) {
	//	DataLoader::GetLoader()->LoadBitmap(map_name, moon->map_icon, Bitmap::BMP_TRANSLUCENT, true);
	//}
}

void AStarSystem::ParseRegion(TermStruct* val)
{
	char  rgn_name[NAMELEN] = "";
	char  rgn_parent[NAMELEN] = "";
	char  lnk_name[NAMELEN] = "";
	char  parent_type[32];
	double size = 1.0e6;
	double orbit = 0.0;
	double grid = 25000;
	double inclination = 0.0;
	int    asteroids = 0;
	EOrbitalType parent_class = EOrbitalType::NOTHING;
	TArray<FString> LinksName;
	List<Text> links;

	for (int i = 0; i < val->elements()->size(); i++) {
		TermDef* pdef = val->elements()->at(i)->isDef();
		if (pdef) {
			if (pdef->name()->value() == "name")
				GetDefText(rgn_name, pdef, filename);

			else if (pdef->name()->value() == "parent")
				GetDefText(rgn_parent, pdef, filename);

			else if (pdef->name()->value() == "link") {
				GetDefText(lnk_name, pdef, filename);
				if (lnk_name[0]) {
					links.append(new Text(lnk_name));
					LinksName.Add(FString(lnk_name));
				}
			}

			else if (pdef->name()->value() == "orbit")
				GetDefNumber(orbit, pdef, filename);

			else if (pdef->name()->value() == "size")
				GetDefNumber(size, pdef, filename);

			else if (pdef->name()->value() == "radius")
				GetDefNumber(size, pdef, filename);

			else if (pdef->name()->value() == "grid")
				GetDefNumber(grid, pdef, filename);

			else if (pdef->name()->value() == "inclination")
				GetDefNumber(inclination, pdef, filename);

			else if (pdef->name()->value() == "asteroids")
				GetDefNumber(asteroids, pdef, filename);
			else if (pdef->name()->value() == "type") {
				GetDefText(parent_type, pdef, filename);

				switch (parent_type[0]) {
				case 'S':
					parent_class = EOrbitalType::STAR;
					break;
				case 'P':
					parent_class = EOrbitalType::PLANET;
					break;
				case 'M':
					parent_class = EOrbitalType::MOON;
					break;
				default:
					parent_class = EOrbitalType::NOTHING;
					break;
				}
			}
		}

		// define our data table struct
		FS_Region NewRegionData;
		NewRegionData.Name = FString(rgn_name);
		NewRegionData.Parent = FString(rgn_name);
		NewRegionData.Link = LinksName;	
		NewRegionData.Size = size;
		NewRegionData.Grid = grid;
		NewRegionData.Inclination = inclination;
		NewRegionData.Asteroids = asteroids;
		NewRegionData.Type = parent_class;

		FName RowName = FName(FString(rgn_name));

		// call AddRow to insert the record
		RegionsDataTable->AddRow(RowName, NewRegionData);
	}

	SpawnRegion(FString(rgn_name).Append(" Region"));
	//Orbital* primary = primary_moon;
	//if (!primary) primary = primary_planet;
	//if (!primary) primary = primary_star;

	//OrbitalRegion* region = new OrbitalRegion(this, rgn_name, 0, size, orbit, primary);
	//region->grid = grid;
	//region->inclination = inclination;
	//region->asteroids = asteroids;
	//region->links.append(links);

	//if (primary)
	//	primary->regions.append(region);
	//else
	//	regions.append(region);

	//all_regions.append(region);

	//if (orbit > StarSystem::radius)
	//	StarSystem::radius = orbit;
}

void AStarSystem::ParseTerrain(TermStruct* val)
{
	//Orbital* primary = primary_moon;
	//if (!primary) primary = primary_planet;

	//if (!primary) {
	//	Print("WARNING: Terrain region with no primary ignored in '%s'\n", filename);
	//	return;
	//}

	//TerrainRegion* region = 0;

	Text   rgn_name;
	Text   patch_name;
	Text   patch_texture;
	Text   noise_tex0;
	Text   noise_tex1;
	Text   apron_name;
	Text   apron_texture;
	Text   water_texture;
	Text   env_texture_positive_x;
	Text   env_texture_negative_x;
	Text   env_texture_positive_y;
	Text   env_texture_negative_y;
	Text   env_texture_positive_z;
	Text   env_texture_negative_z;
	Text   haze_name;
	Text   sky_name;
	Text   clouds_high;
	Text   clouds_low;
	Text   shades_high;
	Text   shades_low;

	double size = 1.0e6;
	double grid = 25000;
	double inclination = 0.0;
	double scale = 10e3;
	double mtnscale = 1e3;
	double fog_density = 0;
	double fog_scale = 0;
	double haze_fade = 0;
	double clouds_alt_high = 0;
	double clouds_alt_low = 0;
	double w_period = 0;
	double w_chances[EWEATHER_STATE::NUM_STATES];

	for (int i = 0; i < val->elements()->size(); i++) {
		TermDef* pdef = val->elements()->at(i)->isDef();
		if (pdef) {
			if (pdef->name()->value() == "name")
				GetDefText(rgn_name, pdef, filename);

			else if (pdef->name()->value() == "patch")
				GetDefText(patch_name, pdef, filename);

			else if (pdef->name()->value() == "patch_texture")
				GetDefText(patch_texture, pdef, filename);

			else if (pdef->name()->value() == "detail_texture_0")
				GetDefText(noise_tex0, pdef, filename);

			else if (pdef->name()->value() == "detail_texture_1")
				GetDefText(noise_tex1, pdef, filename);

			else if (pdef->name()->value() == "apron")
				GetDefText(apron_name, pdef, filename);

			else if (pdef->name()->value() == "apron_texture")
				GetDefText(apron_texture, pdef, filename);

			else if (pdef->name()->value() == "water_texture")
				GetDefText(water_texture, pdef, filename);

			else if (pdef->name()->value() == "env_texture_positive_x")
				GetDefText(env_texture_positive_x, pdef, filename);

			else if (pdef->name()->value() == "env_texture_negative_x")
				GetDefText(env_texture_negative_x, pdef, filename);

			else if (pdef->name()->value() == "env_texture_positive_y")
				GetDefText(env_texture_positive_y, pdef, filename);

			else if (pdef->name()->value() == "env_texture_negative_y")
				GetDefText(env_texture_negative_y, pdef, filename);

			else if (pdef->name()->value() == "env_texture_positive_z")
				GetDefText(env_texture_positive_z, pdef, filename);

			else if (pdef->name()->value() == "env_texture_negative_z")
				GetDefText(env_texture_negative_z, pdef, filename);

			else if (pdef->name()->value() == "clouds_high")
				GetDefText(clouds_high, pdef, filename);

			else if (pdef->name()->value() == "shades_high")
				GetDefText(shades_high, pdef, filename);

			else if (pdef->name()->value() == "clouds_low")
				GetDefText(clouds_low, pdef, filename);

			else if (pdef->name()->value() == "shades_low")
				GetDefText(shades_low, pdef, filename);

			else if (pdef->name()->value() == "haze")
				GetDefText(haze_name, pdef, filename);

			else if (pdef->name()->value() == "sky_color")
				GetDefText(sky_name, pdef, filename);

			else if (pdef->name()->value() == "size" ||
				pdef->name()->value() == "radius")
				GetDefNumber(size, pdef, filename);

			else if (pdef->name()->value() == "grid")
				GetDefNumber(grid, pdef, filename);

			else if (pdef->name()->value() == "inclination")
				GetDefNumber(inclination, pdef, filename);

			else if (pdef->name()->value() == "scale")
				GetDefNumber(scale, pdef, filename);

			else if (pdef->name()->value() == "mtnscale" ||
				pdef->name()->value() == "mtn_scale")
				GetDefNumber(mtnscale, pdef, filename);

			else if (pdef->name()->value() == "fog_density")
				GetDefNumber(fog_density, pdef, filename);

			else if (pdef->name()->value() == "fog_scale")
				GetDefNumber(fog_scale, pdef, filename);

			else if (pdef->name()->value() == "haze_fade")
				GetDefNumber(haze_fade, pdef, filename);

			else if (pdef->name()->value() == "clouds_alt_high")
				GetDefNumber(clouds_alt_high, pdef, filename);

			else if (pdef->name()->value() == "clouds_alt_low")
				GetDefNumber(clouds_alt_low, pdef, filename);

			else if (pdef->name()->value() == "weather_period")
				GetDefNumber(w_period, pdef, filename);

			else if (pdef->name()->value() == "weather_clear")
				GetDefNumber(w_chances[0], pdef, filename);
			else if (pdef->name()->value() == "weather_high_clouds")
				GetDefNumber(w_chances[1], pdef, filename);
			else if (pdef->name()->value() == "weather_moderate_clouds")
				GetDefNumber(w_chances[2], pdef, filename);
			else if (pdef->name()->value() == "weather_overcast")
				GetDefNumber(w_chances[3], pdef, filename);
			else if (pdef->name()->value() == "weather_fog")
				GetDefNumber(w_chances[4], pdef, filename);
			else if (pdef->name()->value() == "weather_storm")
				GetDefNumber(w_chances[5], pdef, filename);

			else if (pdef->name()->value() == "layer") {
				if (!pdef->term() || !pdef->term()->isStruct()) {
					Print("WARNING: terrain layer struct missing in '%s'\n", filename);
				}
				else {

					//if (!region)
					//	region = new(__FILE__, __LINE__) TerrainRegion(this, rgn_name, size, primary);

					//TermStruct* val = pdef->term()->isStruct();
					//ParseLayer(region, val);
				}
			}
		}
	}

	/*if (!region)
		region = new TerrainRegion(this, rgn_name, size, primary);

	region->grid = grid;
	region->inclination = inclination;
	region->patch_name = patch_name;
	region->patch_texture = patch_texture;
	region->noise_tex0 = noise_tex0;
	region->noise_tex1 = noise_tex1;
	region->apron_name = apron_name;
	region->apron_texture = apron_texture;
	region->water_texture = water_texture;
	region->haze_name = haze_name;
	region->clouds_high = clouds_high;
	region->shades_high = shades_high;
	region->clouds_low = clouds_low;
	region->shades_low = shades_low;
	region->scale = scale;
	region->mtnscale = mtnscale;
	region->fog_density = fog_density;
	region->fog_scale = fog_scale;
	region->haze_fade = haze_fade;
	region->clouds_alt_high = clouds_alt_high;
	region->clouds_alt_low = clouds_alt_low;

	region->env_texture_positive_x = env_texture_positive_x;
	region->env_texture_negative_x = env_texture_negative_x;
	region->env_texture_positive_y = env_texture_positive_y;
	region->env_texture_negative_y = env_texture_negative_y;
	region->env_texture_positive_z = env_texture_positive_z;
	region->env_texture_negative_z = env_texture_negative_z;

	Weather& weather = region->GetWeather();

	weather.SetPeriod(w_period);

	for (int i = 0; i < Weather::NUM_STATES; i++)
		weather.SetChance(i, w_chances[i]);

	region->LoadSkyColors(sky_name);

	primary->regions.append(region);
	all_regions.append(region);*/
}

void AStarSystem::SpawnStar(FString Name, double m, double rad, double o, double r)
{
	UWorld* World = GetWorld();

	FRotator rotate = FRotator::ZeroRotator;
	FVector SystemLoc = FVector::ZeroVector;

	FActorSpawnParameters StarInfo;
	StarInfo.Name = FName(Name);

	AOrbitalBody* Star = GetWorld()->SpawnActor<AOrbitalBody>(CentralStar, SystemLoc, rotate, StarInfo);


	if (Star)
	{
		PlanetParent = Star;
		RegionParent = Star;
		Star->AttachToActor(SystemParent, FAttachmentTransformRules::KeepWorldTransform);
		Star->SetActorLabel(FString(Name));
		UE_LOG(LogTemp, Log, TEXT("Spawned Star '%s'"), *Name);
		
		Star->type = EOrbitalType::STAR;

		Star->InitializeStar(this, Name, m, rad, o, (r*3600), nullptr);
		
	}
	else {
		UE_LOG(LogTemp, Log, TEXT("Failed to Spawn Star"));
	}
}

void AStarSystem::SpawnPlanet(FString Name, double m, double rad, double o, double r)
{
	UWorld* World = GetWorld();

	FRotator rotate = FRotator::ZeroRotator;
	FVector SystemLoc = FVector::ZeroVector;

	FActorSpawnParameters Info;
	Info.Name = FName(Name);

	AOrbitalBody* Planet = GetWorld()->SpawnActor<AOrbitalBody>(PlanetObject, SystemLoc, rotate, Info);

	if (Planet)
	{
		if (PlanetParent) {
			Planet->AttachToActor(PlanetParent, FAttachmentTransformRules::KeepWorldTransform);
		}

		Planet->SetActorLabel(FString(Name));
		UE_LOG(LogTemp, Log, TEXT("Spawned Planet '%s'"), *Name);
		Planet->type = EOrbitalType::PLANET;

		Planet->InitializePlanet(this, Name, m, rad, o, (r * 3600), PlanetParent);
		MoonParent = Planet;
		RegionParent = Planet;
	}
	else {
		UE_LOG(LogTemp, Log, TEXT("Failed to Spawn Planet"));
	}
}

void AStarSystem::SpawnMoon(FString Name, double m, double rad, double o, double r)
{
	UWorld* World = GetWorld();

	FRotator rotate = FRotator::ZeroRotator;
	FVector SystemLoc = FVector::ZeroVector;

	FActorSpawnParameters Info;
	Info.Name = FName(Name);

	AOrbitalBody* Moon = GetWorld()->SpawnActor<AOrbitalBody>(MoonObject, SystemLoc, rotate, Info);

	if (Moon)
	{
		RegionParent = Moon;
		if (MoonParent) {
			Moon->AttachToActor(MoonParent, FAttachmentTransformRules::KeepWorldTransform);
		}

		Moon->SetActorLabel(FString(Name));
		UE_LOG(LogTemp, Log, TEXT("Spawned Moon '%s'"), *Name);
		Moon->type = EOrbitalType::MOON;

		Moon->InitializeMoon(this, Name, m, rad, o, (r * 3600), MoonParent);
	}
	else {
		UE_LOG(LogTemp, Log, TEXT("Failed to Spawn Moon"));
	}
}

void AStarSystem::SpawnRegion(FString Name)
{
	UWorld* World = GetWorld();

	FRotator rotate = FRotator::ZeroRotator;
	FVector SystemLoc = FVector::ZeroVector;

	FActorSpawnParameters Info;
	Info.Name = FName(Name);

	AOrbitalRegion* Region = GetWorld()->SpawnActor<AOrbitalRegion>(AOrbitalRegion::StaticClass(), SystemLoc, rotate, Info);

	if (Region)
	{
		Region->AttachToActor(RegionParent, FAttachmentTransformRules::KeepWorldTransform);

		Region->SetActorLabel(FString(Name));
		UE_LOG(LogTemp, Log, TEXT("Spawned Region '%s'"), *Name);
		Region->type = EOrbitalType::REGION;

		//Moon->InitializeMoon(this, Name, m, rad, o, (r * 3600), MoonParent);
	}
	else {
		UE_LOG(LogTemp, Log, TEXT("Failed to Region"));
	}
}
