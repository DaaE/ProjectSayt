// Copyright (c) 2026 Sungmin JI. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayModMagnitudeCalculation.h"
#include "SayuMMC_Damage.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTSAYT_API USayuMMC_Damage : public UGameplayModMagnitudeCalculation
{
	GENERATED_BODY()
	
public:
	USayuMMC_Damage();

	// CalculateBaseMagnitude : 이 Effect가 적용될 때마다 호출되는 계산 함수
	// 여기서 AttackPower를 읽어서 최종 데미지를 리턴함
	virtual float CalculateBaseMagnitude_Implementation(
		const FGameplayEffectSpec& Spec) const override;

private:
	// CaptureDefinition : "어떤 Attribute를, 누구(가해자/피해자)로부터,
	// 언제(적용 직전 스냅샷 vs 실시간) 가져올지"를 정의하는 명세
	// Unity에는 없는 개념인데, 멀티플레이어에서 "버프가 끝난 후에도
	// 데미지 계산은 버프 걸렸을 때 기준으로 고정해야 하는" 경우가 있어서
	// 이렇게 "캡처 시점"까지 명세하는 구조가 필요해요
	FGameplayEffectAttributeCaptureDefinition AttackPowerDef;
};
