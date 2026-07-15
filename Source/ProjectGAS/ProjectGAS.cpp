// Copyright Epic Games, Inc. All Rights Reserved.

#include "ProjectGAS.h"
#include "Modules/ModuleManager.h"
#include "UI/Slate/Study/SayuStudyStyle.h"
#include "UI/SayuStyle.h"

void FProjectGASModule::StartupModule()
{
	FSayuStudyStyle::Initialize();
	FSayuStyle::Initialize();
}

void FProjectGASModule::ShutdownModule()
{
	FSayuStyle::Shutdown();
	FSayuStudyStyle::Shutdown();
}

IMPLEMENT_PRIMARY_GAME_MODULE( FProjectGASModule, ProjectGAS, "ProjectGAS" );
