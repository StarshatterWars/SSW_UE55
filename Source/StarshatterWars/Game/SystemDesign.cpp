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


List<USystemDesign> USystemDesign::catalog;

USystemDesign::USystemDesign()
{
}

USystemDesign::~USystemDesign()
{
	//components.destroy();
}

// +--------------------------------------------------------------------+

void USystemDesign::Initialize(UDataTable* SystemDT)
{
	UE_LOG(LogTemp, Log, TEXT("Loading System Designs from Data Table"));

	TArray<FS_SystemDesign> Systems;
	TArray<FName> RowNames = SystemDT->GetRowNames();

	for (int index = 0; index < RowNames.Num(); index++) {
		FName RowName = RowNames[index];
		FS_SystemDesign* Item = SystemDT->FindRow<FS_SystemDesign>(RowName, "");
		FString Name = Item->Name;
		UE_LOG(LogTemp, Log, TEXT("System Design from DT: %s"), *Name);
	}
}

// +--------------------------------------------------------------------+

void
USystemDesign::Close()
{
	catalog.destroy();
}

// +--------------------------------------------------------------------+

USystemDesign*
USystemDesign::Find(const char* name)
{
	USystemDesign  test;
	test.name = name;
	return catalog.find(&test);
}



