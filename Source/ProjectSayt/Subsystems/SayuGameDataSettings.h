// Copyright (c) 2026 Sungmin JI. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "Engine/DataTable.h"
#include "SayuGameDataSettings.generated.h"

/**
 * 
 */
UCLASS(Config = Game, DefaultConfig, DisplayName = "Sayu Game Data")
class PROJECTSAYT_API USayuGameDataSettings : public UDeveloperSettings
{
	GENERATED_BODY()
	
public:
	USayuGameDataSettings();

	// Project Settings 메뉴에서 디자이너가 직접 DataTable 에셋을 골라 넣는 슬롯.
	UPROPERTY(EditAnywhere, Config, Category = "Combat")
	TSoftObjectPtr<UDataTable> CombatStatsTable;
};
