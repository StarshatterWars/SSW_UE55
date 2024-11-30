/*  Project Starshatter Wars
	Fractal Dev Games
	Copyright (C) 2024. All Rights Reserved.

	SUBSYSTEM:    Game
	FILE:         SystemDesign.cpp
	AUTHOR:       Carlos Bott

	OVERVIEW
	========
	Generic Ship Design class
	Note: Loaded from Data Tables
*/



#include "SystemDesign.h"
#include "ComponentDesign.h"

List<SystemDesign> SystemDesign::catalog;

SystemDesign::SystemDesign()
{
	//USystemDesign* design = NewObject<USystemDesign>();
}

SystemDesign::~SystemDesign()
{
	//components.destroy();
}

// +--------------------------------------------------------------------+

void SystemDesign::Initialize(TArray<FS_SystemDesign*> Systems)
{
	//UE_LOG(LogTemp, Log, TEXT("Loading System Designs from Data Table"));

	//for (int index = 0; index < Systems.Num(); index++) {
	//	FS_SystemDesign* NewSystemDesign = Systems[index];
	//	UE_LOG(LogTemp, Log, TEXT("System Design from Struct: %s"), *NewSystemDesign->Name);

		//Systems[index] = Item[index];

		//Systems.Add(Item[index]);

	//	for (int comp_index = 0; comp_index < NewSystemDesign->Component.Num(); comp_index++) {
	//		UE_LOG(LogTemp, Log, TEXT("Component Design from DT: %s"), *NewSystemDesign->Component[comp_index].Name);
		
	//	}		
	//}
}

void SystemDesign::Load(TArray<FS_SystemDesign*> Systems) {

}
// +--------------------------------------------------------------------+

void
SystemDesign::Close()
{
	catalog.destroy();
}

// +--------------------------------------------------------------------+

SystemDesign*
SystemDesign::Find(Text name)
{
	SystemDesign  test;
	test.name = name;
	return catalog.find(&test);
}



