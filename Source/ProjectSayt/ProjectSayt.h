// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

class FProjectSaytModule : public FDefaultGameModuleImpl
{
public:
	// 모듈이 메모리에 올라오는 순간 / 내려가는 순간의 훅
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};
