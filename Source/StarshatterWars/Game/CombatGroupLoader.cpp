/*  Project Starshatter Wars
	Fractal Dev Games
	Copyright (C) 2024. All Rights Reserved.

	SUBSYSTEM:    Game
	FILE:         CombatGroupLoader.cpp
	AUTHOR:       Carlos Bott

	OVERVIEW
	========
	Loader and Parser class for initial generation of Combat Group Data Table
	Will not be used after Dable Table is Generated.
*/


#include "CombatGroupLoader.h"


// Sets default values
ACombatGroupLoader::ACombatGroupLoader()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	static ConstructorHelpers::FObjectFinder<UDataTable> CombatGroupDataTableObject(TEXT("DataTable'/Game/Game/DT_CombatGroup.DT_CombatGroup'"));

	if (CombatGroupDataTableObject.Succeeded())
	{
		CombatGroupDataTable = CombatGroupDataTableObject.Object;
		//GalaxyDataTable->EmptyTable();
	}

}

// Called when the game starts or when spawned
void ACombatGroupLoader::BeginPlay()
{
	Super::BeginPlay();
	GetSSWInstance();
	LoadCombatRoster();
}

// Called every frame
void ACombatGroupLoader::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

//TArray<FString> Filelist = {
//	"Alliance.def", "Dantari.def", "Haiche.def", "Hegemony.def", "Pirates.def"
//}
// +-------------------------------------------------------------------+

void ACombatGroupLoader::LoadCombatRoster()
{
	UE_LOG(LogTemp, Log, TEXT("ACombatGroupLoader::LoadCombatRoster()"));
	FString ProjectPath = FPaths::ProjectDir();
	ProjectPath.Append(TEXT("GameData/Campaigns/"));
	FString PathName = ProjectPath;

	TArray<FString> output;
	output.Empty();

	FString path = PathName + "*.def";
	FFileManagerGeneric::Get().FindFiles(output, *path, true, false);

	for (int i = 0; i < output.Num(); i++) {
		
		FString FileName = ProjectPath;
		FileName.Append(output[i]);
		
		char* filename = TCHAR_TO_ANSI(*FileName);

		LoadOrderOfBattle(filename, -1);
	}	
}

void ACombatGroupLoader::LoadOrderOfBattle(const char* filename, int team)
{
	UE_LOG(LogTemp, Log, TEXT("Loading Order of Battle Data: %s"), *FString(filename));

	SSWInstance->loader->GetLoader();
	SSWInstance->loader->SetDataPath(filename);


	BYTE* block = 0;
	SSWInstance->loader->LoadBuffer(filename, block, true);

	Parser parser(new BlockReader((const char*)block));
	Term* term = parser.ParseTerm();

	if (!term) {
		return;
	}
	else {
		TermText* file_type = term->isText();
		if (!file_type || file_type->value() != "ORDER_OF_BATTLE") 
		{
			UE_LOG(LogTemp, Log, TEXT("Invalid Order of Battle File: %s"), *FString(filename)); 
			return;
		}
	}

	do {
		delete term; 
		term = parser.ParseTerm();

		if (term) {
			TermDef* def = term->isDef();
			if (def) {
				if (def->name()->value() == "group") {
					if (!def->term() || !def->term()->isStruct()) {
						Print("WARNING: group struct missing in '%s'\n", filename);
					}
					else {
						TermStruct* val = def->term()->isStruct();

						FS_CombatGroup NewCombatGroup;

						Name = "";
						Type = "";
						Intel = "KNOWN";
						Region = "";
						System = "";
						ParentType = "";
						UnitIndex = 0;
						ParentId = 0;
						Id = 0;
						Iff = -1;
						Loc = Vec3(1.0e9f, 0.0f, 0.0f);

						//List<CombatUnit>  unit_list;
						char              unit_name[64];
						char              unit_regnum[16];
						char              unit_design[64];
						char              unit_skin[64];
						int               unit_class = 0;
						int               unit_count = 1;
						int               unit_dead = 0;
						int               unit_damage = 0;
						int               unit_heading = 0;
						int               unit_index = 0;

						
						*unit_name = 0;
						*unit_regnum = 0;
						*unit_design = 0;
						*unit_skin = 0;

						// all groups in this OOB default to the IFF of the main force
						//if (force)
						//	iff = force->GetIFF();

						for (int i = 0; i < val->elements()->size(); i++) 
						{
							TermDef* pdef = val->elements()->at(i)->isDef();
							//if (pdef && (Iff < 0 || team < 0 || Iff == team)) 
							//{
							if (pdef->name()->value() == ("name")) 
							{
								GetDefText(Name, pdef, filename);
								NewCombatGroup.Name = FString(Name);
							}
							else if (pdef->name()->value() == ("type"))
							{
								GetDefText(Type, pdef, filename);
								NewCombatGroup.Type = FString(Type);
							}
							else if (pdef->name()->value() == ("intel"))
							{
								GetDefText(Intel, pdef, filename);
								NewCombatGroup.Intel = FString(Intel);
							}
							else if (pdef->name()->value() == ("region"))
							{
								GetDefText(Region, pdef, filename);
								NewCombatGroup.Region = FString(Region);
							}
							else if (pdef->name()->value() == ("system"))
							{
								GetDefText(System, pdef, filename);
								NewCombatGroup.System = FString(System);
							}
							else if (pdef->name()->value() == ("loc"))
							{
								GetDefVec(Loc, pdef, filename);
										
								NewCombatGroup.Location.X = Loc.x;
								NewCombatGroup.Location.Y = Loc.y;
								NewCombatGroup.Location.Z = Loc.z;
							}
							else if (pdef->name()->value() == ("parent_type"))
							{
								GetDefText(ParentType, pdef, filename);
								NewCombatGroup.ParentType = FString(ParentType);
							}
							else if (pdef->name()->value() == ("parent_id"))
							{
								GetDefNumber(ParentId, pdef, filename);
								NewCombatGroup.ParentId = ParentId;
							}
							else if (pdef->name()->value() == ("iff"))
							{
								GetDefNumber(Iff, pdef, filename);
								NewCombatGroup.Iff = Iff;
							}
							else if (pdef->name()->value() == ("id"))
							{
								GetDefNumber(Id, pdef, filename);
								NewCombatGroup.Id = Id;
							}
							else if (pdef->name()->value() == ("unit_index"))
							{
								GetDefNumber(UnitIndex, pdef, filename);
								NewCombatGroup.UnitIndex = UnitIndex;
							}

							//}
							FName RowName = FName(FString(Name) + " " + GetOrdinal(Id) + " " + FString(Type));
							// call AddRow to insert the record
							
							if(Iff > 0) 
								CombatGroupDataTable->AddRow(RowName, NewCombatGroup);

							CombatGroupData = NewCombatGroup;
						
							/*GET_DEF_TEXT(name);
							else GET_DEF_TEXT(type);
							else GET_DEF_TEXT(intel);
							else GET_DEF_TEXT(region);
							else GET_DEF_TEXT(system);
							else GET_DEF_VEC(loc);
							else GET_DEF_TEXT(parent_type);
							else GET_DEF_NUM(parent_id);
							else GET_DEF_NUM(iff);
							else GET_DEF_NUM(id);
							else GET_DEF_NUM(unit_index);*/



							/*else if ((iff == team || team < 0) && pdef->name()->value() == "unit") {
									if (!pdef->term() || !pdef->term()->isStruct()) {
										Print("WARNING: unit struct missing for group '%s' in '%s'\n", name, filename);
									}
									else {
										TermStruct* val = pdef->term()->isStruct();

										char unit_region[64];
										char design[256];
										Vec3 unit_loc = Vec3(1.0e9f, 0.0f, 0.0f);
										unit_count = 1;

										for (int i = 0; i < val->elements()->size(); i++) {
											TermDef* pdef = val->elements()->at(i)->isDef();
											if (pdef) {
												if (pdef->name()->value() == "name") {
													GetDefText(unit_name, pdef, filename);
												}
												else if (pdef->name()->value() == "regnum") {
													GetDefText(unit_regnum, pdef, filename);
												}
												else if (pdef->name()->value() == "region") {
													GetDefText(unit_region, pdef, filename);
												}
												else if (pdef->name()->value() == "loc") {
													GetDefVec(unit_loc, pdef, filename);
												}
												else if (pdef->name()->value() == "type") {
													char typestr[32];
													GetDefText(typestr, pdef, filename);
													unit_class = ShipDesign::ClassForName(typestr);
												}
												else if (pdef->name()->value() == "design") {
													GetDefText(unit_design, pdef, filename);
												}
												else if (pdef->name()->value() == "skin") {
													GetDefText(unit_skin, pdef, filename);
												}
												else if (pdef->name()->value() == "count") {
													GetDefNumber(unit_count, pdef, filename);
												}
												else if (pdef->name()->value()=="dead_count") {
													GetDefNumber(unit_dead, pdef, filename);
												}
												else if (pdef->name()->value() == "damage") {
													GetDefNumber(unit_damage, pdef, filename);
												}
												else if (pdef->name()->value() == "heading") {
													GetDefNumber(unit_heading, pdef, filename);
												}
											}
										}

										if (!ShipDesign::CheckName(unit_design)) {
											Print("ERROR: invalid design '%s' for unit '%s' in '%s'\n", unit_design, unit_name, filename);
											return 0;
										}

										CombatUnit* cu = new(__FILE__, __LINE__) CombatUnit(unit_name, unit_regnum, unit_class, unit_design, unit_count, iff);
										cu->SetRegion(unit_region);
										cu->SetSkin(unit_skin);
										cu->MoveTo(unit_loc);
										cu->Kill(unit_dead);
										cu->SetSustainedDamage(unit_damage);
										cu->SetHeading(unit_heading * DEGREES);
										unit_list.append(cu);
									}
									}
							}
						}  // elements

						if (iff >= 0 && (iff == team || team < 0)) {
							CombatGroup* parent_group = 0;

							if (force) {
								parent_group = force->FindGroup(TypeFromName(parent_type), parent_id);
							}

							CombatGroup* g = new
								CombatGroup(TypeFromName(type), id, name, iff, Intel::IntelFromName(intel), parent_group);

							g->region = region;
							g->combatant = combatant;
							g->unit_index = unit_index;

							if (loc.x >= 1e9) {
								if (parent_group)
									g->location = parent_group->location;
								else
									g->location = Vec3(0, 0, 0);
							}
							else {
								g->location = loc;
							}

							if (unit_list.size()) {
								unit_list[0]->SetLeader(true);

								ListIter<CombatUnit> u = unit_list;
								while (++u) {
									u->SetCombatGroup(g);

									if (u->GetRegion().length() < 1) {
										u->SetRegion(g->GetRegion());
										u->MoveTo(g->Location());
									}

									if (parent_group &&
										(u->Type() == Ship::FIGHTER ||
											u->Type() == Ship::ATTACK)) {

										CombatUnit* carrier = 0;
										CombatGroup* p = parent_group;

										while (p && !carrier) {
											if (p->units.size() && p->units[0]->Type() == Ship::CARRIER) {
												carrier = p->units[0];
												u->SetCarrier(carrier);
												u->SetRegion(carrier->GetRegion());
											}

											p = p->parent;
										}
									}
								}

								g->units.append(unit_list);
							} */
					
						}  /// iff == team?
					}    // group-struct
				}          // group
			}           // def
		}             // term
	} while (term);

}

void ACombatGroupLoader::GetSSWInstance()
{
	SSWInstance = (USSWGameInstance*)GetGameInstance();
}

FString
ACombatGroupLoader::GetOrdinal(int id)
{
	FString ordinal;

	int last_two_digits = id % 100;

	if (last_two_digits > 10 && last_two_digits < 20) {
		ordinal = FString::FormatAsNumber(id) + "th";
	}
	else {
		int last_digit = last_two_digits % 10;
		
		if (last_digit == 1)
			ordinal = FString::FormatAsNumber(id) + "st";
		else if (last_digit == 2)
			ordinal = FString::FormatAsNumber(id) + "nd";
		else if (last_digit == 3)
			ordinal = FString::FormatAsNumber(id) + "rd";
		else
			ordinal = FString::FormatAsNumber(id) + "th";
	}

	return ordinal;
}
