// Copyright 2018-2023, Athian Games. All Rights Reserved. 

#pragma once

#include "Modules/ModuleManager.h"

class FTreeViewControlModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};