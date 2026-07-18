// Copyright Epic Games, Inc. All Rights Reserved.

#include "ProjectSayt.h"
#include "Modules/ModuleManager.h"
#include "UI/SayuStyle.h"

void FProjectSaytModule::StartupModule()
{
	FSayuStyle::Initialize();
}

void FProjectSaytModule::ShutdownModule()
{
	FSayuStyle::Shutdown();
}

IMPLEMENT_PRIMARY_GAME_MODULE( FProjectSaytModule, ProjectSayt, "ProjectSayt" );
