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


List<SystemDesign> SystemDesign::catalog;

SystemDesign::SystemDesign()
{
	static ConstructorHelpers::FObjectFinder<UDataTable> SystemDesignDataTableObject(TEXT("DataTable'/Game/Game/DT_SystemDesign.DT_SystemDesign'"));

	if (SystemDesignDataTableObject.Succeeded())
	{
		SystemDesignDataTable = SystemDesignDataTableObject.Object;
	}
}

SystemDesign::~SystemDesign()
{
	//components.destroy();
}

// +--------------------------------------------------------------------+

void SystemDesign::Initialize()
{
	UE_LOG(LogTemp, Log, TEXT("Loading System Designs from Data Table"));
}

// +--------------------------------------------------------------------+

void
SystemDesign::Close()
{
	catalog.destroy();
}

// +--------------------------------------------------------------------+

SystemDesign*
SystemDesign::Find(const char* name)
{
	SystemDesign  test;
	test.name = name;
	return catalog.find(&test);
}



