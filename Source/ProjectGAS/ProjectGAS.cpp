// Copyright Epic Games, Inc. All Rights Reserved.

#include "ProjectGAS.h"
#include "Modules/ModuleManager.h"
#include "UI/Slate/Study/SayuStudyStyle.h"

void FProjectGASModule::StartupModule()
{
	FSayuStudyStyle::Initialize();
}

void FProjectGASModule::ShutdownModule()
{
	FSayuStudyStyle::Shutdown();
}

IMPLEMENT_PRIMARY_GAME_MODULE( FProjectGASModule, ProjectGAS, "ProjectGAS" );
