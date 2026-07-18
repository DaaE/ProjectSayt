// Copyright (c) 2026 Sungmin JI. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
//#include "Engine/DataTable.h"
#include "SayuCombatStatsRow.h"
#include "SayuGameDataSubsystem.generated.h"

class USayuAttributeSet_Combat;

/**
 * 
 */
UCLASS()
class PROJECTSAYT_API USayuGameDataSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
	
public:
	// GameInstance가 생성된 직후, 이 Subsystem이 탄생하는 시점에 자동 호출됨.
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	// GameInstance가 종료되기 직전, 이 Subsystem이 파괴되는 시점에 자동 호출됨.
	virtual void Deinitialize() override;
	
	// 캐릭터 종류별 전투 기본 스탯을 RowID로 조회. 찾으면 true + OutStats에 채워줌.
	UFUNCTION(BlueprintCallable, Category = "Combat Data")
	bool GetCombatStats(FName RowID, FSayuCombatStatsRow& OutStats) const;
	
	UFUNCTION(BlueprintCallable, Category = "Save")
	void SaveCombatState(USayuAttributeSet_Combat* AttributeSet, const FString& SlotName);

	UFUNCTION(BlueprintCallable, Category = "Save")
	bool LoadCombatState(USayuAttributeSet_Combat* AttributeSet, const FString& SlotName);
	
private:
	// Project Settings(USayuGameDataSettings)에서 로드해온 실제 테이블 캐시.
	UPROPERTY()
	TObjectPtr<UDataTable> CombatStatsTable;
};
