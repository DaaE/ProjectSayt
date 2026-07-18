// Copyright (c) 2026 Sungmin JI. All Rights Reserved.


#include "SayuMMC_Damage.h"
#include "../Attributes/SayuAttributeSet_Combat.h"


USayuMMC_Damage::USayuMMC_Damage()
{
	// 가해자(Source)의 AttackPower를 캡처하겠다는 명세
	AttackPowerDef = FGameplayEffectAttributeCaptureDefinition(
		USayuAttributeSet_Combat::GetAttackPowerAttribute(),
		EGameplayEffectAttributeCaptureSource::Source,
		// Source : 이 Effect를 발생시킨 쪽 (공격자)
		// Target이었다면 맞는 쪽(피격자) 기준이 됨
		false
		// false : 스냅샷 안 함 = 적용되는 "그 순간"의 실시간 값을 읽음
	);

	RelevantAttributesToCapture.Add(AttackPowerDef);
	// 이 배열에 등록해야 실제로 캡처가 작동함
}

float USayuMMC_Damage::CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const
{
	float AttackPower = 0.f;

	FAggregatorEvaluateParameters EvaluationParameters;
	// 캡처할 때 추가 조건(태그 필터 등)을 넣을 수 있는 파라미터.
	// 지금은 기본값 그대로 사용

	GetCapturedAttributeMagnitude(
		AttackPowerDef, Spec, EvaluationParameters, AttackPower);
	// 실제로 캡처된 AttackPower 값을 꺼내서 변수에 담음

	// 데미지 공식 - 지금은 단순하게 AttackPower 그대로 반환
	// 나중에 크리티컬, 방어력 등 추가하고 싶으면 여기서 확장
	return -AttackPower;
}
