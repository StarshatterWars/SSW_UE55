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
#include "GameStructs.h"
#include "ShipDesign.h"


// Sets default values
ACombatGroupLoader::ACombatGroupLoader()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	static ConstructorHelpers::FObjectFinder<UDataTable> CombatGroupDataTableObject(TEXT("DataTable'/Game/Game/DT_CombatGroup.DT_CombatGroup'"));

	if (CombatGroupDataTableObject.Succeeded())
	{
		CombatGroupDataTable = CombatGroupDataTableObject.Object;
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
						UE_LOG(LogTemp, Log, TEXT("WARNING: group struct missing in '%s'"), *FString(filename));
					}
					else {
						TermStruct* val = def->term()->isStruct();

						FS_CombatGroup NewCombatGroup;
						NewCombatUnitArray.Empty();

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

						for (int i = 0; i < val->elements()->size(); i++) 
						{
							NewCombatGroup.UnitIndex = 0;
							TermDef* pdef = val->elements()->at(i)->isDef();

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

							else if (pdef->name()->value() == ("unit"))
							{
								TermStruct* UnitTerm = pdef->term()->isStruct();

								NewCombatGroup.UnitIndex = UnitTerm->elements()->size();
							
								if(NewCombatGroup.UnitIndex > 0 )
								{
									// Add Unit Stuff Here
									
									FS_CombatGroupUnit NewCombatUnit; 
									
									UnitName = "";
									UnitRegnum = "";
									UnitRegion = "";
									UnitClass = "";
									UnitDesign = "";
									UnitSkin = "";

									UnitCount = 1;
									UnitDamage = 0;
									UnitDead = 0;
									UnitHeading = 0;
									UnitLoc = Vec3(1.0e9f, 0.0f, 0.0f);

									for (int UnitIdx= 0; UnitIdx < NewCombatGroup.UnitIndex; UnitIdx++)
									{
										pdef = UnitTerm->elements()->at(UnitIdx)->isDef();
										
									
										if (pdef->name()->value() == "name") {
											GetDefText(UnitName, pdef, filename);
											UE_LOG(LogTemp, Log, TEXT("unit name '%s'"), *FString(UnitName));
											NewCombatUnit.UnitName = FString(UnitName);
										}
										else if (pdef->name()->value() == "regnum") {
											GetDefText(UnitRegnum, pdef, filename);
										}
										else if (pdef->name()->value() == "region") {
											GetDefText(UnitRegion, pdef, filename);
										}
										else if (pdef->name()->value() == "loc") {
											GetDefVec(UnitLoc, pdef, filename);
										}
										else if (pdef->name()->value() == "type") {
											//char typestr[32];
											GetDefText(UnitClass, pdef, filename);
											//UnitClass = ShipDesign::ClassForName(typestr);
										}
										else if (pdef->name()->value() == "design") {
											GetDefText(UnitDesign, pdef, filename);
										}
										else if (pdef->name()->value() == "skin") {
											GetDefText(UnitSkin, pdef, filename);
															}
										else if (pdef->name()->value() == "count") {
											GetDefNumber(UnitCount, pdef, filename);
															}
										else if (pdef->name()->value() == "dead_count") {
											GetDefNumber(UnitDead, pdef, filename);
																}
										else if (pdef->name()->value() == "damage") {
											GetDefNumber(UnitDamage, pdef, filename);																}
										else if (pdef->name()->value() == "heading") {
											GetDefNumber(UnitHeading, pdef, filename);
										}		
									
									
										NewCombatUnit.UnitRegnum = FString(UnitRegnum);
										NewCombatUnit.UnitRegion = FString(UnitRegion);
										NewCombatUnit.UnitLoc.X = UnitLoc.x;
										NewCombatUnit.UnitLoc.Y = UnitLoc.y;
										NewCombatUnit.UnitLoc.Z = UnitLoc.z;
										NewCombatUnit.UnitClass = FString(UnitClass);
										NewCombatUnit.UnitDesign = FString(UnitDesign);
										NewCombatUnit.UnitSkin = FString(UnitSkin);
										NewCombatUnit.UnitCount = UnitCount;
										NewCombatUnit.UnitDead = UnitDead;
										NewCombatUnit.UnitDamage = UnitDamage;
										NewCombatUnit.UnitHeading = UnitHeading;
									}
									NewCombatUnitArray.Add(NewCombatUnit);				
								}
								NewCombatGroup.Unit = NewCombatUnitArray;
							}
							
							FName RowName = FName(GetOrdinal(Id) + " "+ FString(Name) + " " + +" " + FString(GetNameFromType(FString(Type))));
							// call AddRow to insert the record
							
							if(Iff > 0) {
								CombatGroupDataTable->AddRow(RowName, NewCombatGroup);
							}
							CombatGroupData = NewCombatGroup;
									
						}  /// iff == team?
					}    // group-struct
				}          // group
			}           // def
		}             // term
	} while (term);
	SSWInstance->loader->ReleaseBuffer(block);
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

FString ACombatGroupLoader::GetNameFromType(FString name)
{
	FString TypeName;

	if (name == "force") {
		TypeName = "Force";
	}
	else if (name == "wing") {
		TypeName = "Wing";
	}
	else if (name == "intercept_squadron") {
		TypeName = "Intercept Squadron";
	}
	else if (name == "fighter_squadron") {
		TypeName = "Fighter Squadron";
	}
	else if (name == "attack_squadron") {
		TypeName = "Attack Squadron";
	}
	else if (name == "lca_squadron") {
		TypeName = "LCA Squadron";
	}
	else if (name == "fleet") {
		TypeName = "Fleet";
	}
	else if (name == "destroyer_squadron") {
		TypeName = "DESRON";
	}
	else if (name == "battle_group") {
		TypeName = "Battle Group";
	}
	else if (name == "carrier_group") {
		TypeName = "CVBG";
	}
	else 
	{
		TypeName = name;
	}
	return TypeName;
 }
