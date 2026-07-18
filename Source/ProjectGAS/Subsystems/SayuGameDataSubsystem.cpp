// Fill out your copyright notice in the Description page of Project Settings.


#include "SayuGameDataSubsystem.h"
#include "SayuGameDataSettings.h"
#include "SayuLogChannels.h"
#include "AbilitySystem/Attributes/SayuAttributeSet_Combat.h"
#include "Kismet/GameplayStatics.h"
#include "SaveSystem/SayuSaveGame.h"

void USayuGameDataSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	if (const USayuGameDataSettings* Settings = GetDefault<USayuGameDataSettings>())
	{
		CombatStatsTable = Settings->CombatStatsTable.LoadSynchronous();
	}

	if (CombatStatsTable)
	{
		UE_LOG(LogSayuData, Log, TEXT("[SayuGameDataSubsystem] CombatStatsTable 로드 성공: %s"), *CombatStatsTable->GetName());
	}
	else
	{
		UE_LOG(LogSayuData, Error, TEXT("[SayuGameDataSubsystem] CombatStatsTable 로드 실패 - Project Settings에서 할당됐는지 확인 필요"));
	}
}

void USayuGameDataSubsystem::Deinitialize()
{
	Super::Deinitialize();
}

bool USayuGameDataSubsystem::GetCombatStats(FName RowID, FSayuCombatStatsRow& OutStats) const
{
	if (!CombatStatsTable)
	{
		UE_LOG(LogSayuData, Warning, TEXT("[SayuGameDataSubsystem] GetCombatStats 실패 - CombatStatsTable이 비어있음"));
		return false;
	}

	if (const FSayuCombatStatsRow* FoundRow = CombatStatsTable->FindRow<FSayuCombatStatsRow>(RowID, TEXT("GetCombatStats")))
	{
		OutStats = *FoundRow;
		return true;
	}

	UE_LOG(LogSayuData, Warning, TEXT("[SayuGameDataSubsystem] Row '%s'를 CombatStatsTable에서 찾지 못함"), *RowID.ToString());
	return false;
}

void USayuGameDataSubsystem::SaveCombatState(USayuAttributeSet_Combat* AttributeSet, const FString& SlotName)
{
	if (!AttributeSet)
	{
		return;
	}

	USayuSaveGame* SaveGameObject = Cast<USayuSaveGame>(UGameplayStatics::CreateSaveGameObject(USayuSaveGame::StaticClass()));
	if (!SaveGameObject)
	{
		return;
	}

	SaveGameObject->SavedHealth = AttributeSet->GetHealth();
	SaveGameObject->SavedMana = AttributeSet->GetMana();

	UGameplayStatics::SaveGameToSlot(SaveGameObject, SlotName, 0);

	UE_LOG(LogSayuData, Log, TEXT("[SayuGameDataSubsystem] 저장됨 (Slot: %s, Health: %.1f, Mana: %.1f)"),
		*SlotName, SaveGameObject->SavedHealth, SaveGameObject->SavedMana);
}

bool USayuGameDataSubsystem::LoadCombatState(USayuAttributeSet_Combat* AttributeSet, const FString& SlotName)
{
	if (!AttributeSet || !UGameplayStatics::DoesSaveGameExist(SlotName, 0))
	{
		UE_LOG(LogSayuData, Warning, TEXT("[SayuGameDataSubsystem] 로드 실패 - 슬롯 '%s' 없음"), *SlotName);
		return false;
	}

	const USayuSaveGame* SaveGameObject = Cast<USayuSaveGame>(UGameplayStatics::LoadGameFromSlot(SlotName, 0));
	if (!SaveGameObject)
	{
		return false;
	}

	AttributeSet->InitHealth(SaveGameObject->SavedHealth);
	AttributeSet->InitMana(SaveGameObject->SavedMana);

	UE_LOG(LogSayuData, Log, TEXT("[SayuGameDataSubsystem] 로드됨 (Slot: %s, Health: %.1f, Mana: %.1f)"),
		*SlotName, SaveGameObject->SavedHealth, SaveGameObject->SavedMana);

	return true;
}
