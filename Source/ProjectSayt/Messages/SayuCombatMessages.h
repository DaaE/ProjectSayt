// Copyright (c) 2026 Sungmin JI. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "NativeGameplayTags.h"
#include "SayuCombatMessages.generated.h"

// Gameplay Message Subsystem 채널 태그.
// "Message." 네임스페이스로 시작 — Ability/State 태그(Phase 0~5에서 쓰던 것)와
// 한눈에 구분되게 하려는 컨벤션입니다. 실제 정의는 .cpp에서.
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Message_Combat_Damage_Dealt);

/**
 * 데미지가 실제로 적용된 직후 GameplayMessageSubsystem으로 브로드캐스트되는 메시지예요.
 * AttributeSet(영구 스탯)과는 분리된, "그 순간에만 존재하는 이벤트 데이터"라는 게 핵심입니다.
 */
USTRUCT(BlueprintType)
struct FSayuDamageMessage
{
	GENERATED_BODY()

	// 공격자. 없을 수도 있어요 (예: 환경 데미지) - 사용할 때 nullptr 체크 필요
	UPROPERTY(BlueprintReadOnly, Category = "Sayu|Combat")
	TObjectPtr<AActor> Instigator = nullptr;

	// 피격자
	UPROPERTY(BlueprintReadOnly, Category = "Sayu|Combat")
	TObjectPtr<AActor> Target = nullptr;

	// 이번에 실제로 깎인 데미지량
	UPROPERTY(BlueprintReadOnly, Category = "Sayu|Combat")
	float DamageAmount = 0.f;

	// 데미지 적용 후 체력 — 리스너가 ASC를 또 뒤지지 않아도 되게 같이 실어줍니다
	UPROPERTY(BlueprintReadOnly, Category = "Sayu|Combat")
	float NewHealth = 0.f;

	UPROPERTY(BlueprintReadOnly, Category = "Sayu|Combat")
	float MaxHealth = 0.f;
	
	// 실제로 검이 닿은 정확한 위치 (타격 이펙트 스폰용).
	// 히트 정보가 없는 경우(디버그 자가데미지 등)엔 Target의 액터 위치로 대체됩니다.
	UPROPERTY(BlueprintReadOnly, Category = "Sayu|Combat")
	FVector HitLocation = FVector::ZeroVector;

	// 크리티컬 여부 - AttributeSet엔 없는, 이 순간에만 계산되는 파생 데이터
	UPROPERTY(BlueprintReadOnly, Category = "Sayu|Combat")
	bool bIsCritical = false;
};
