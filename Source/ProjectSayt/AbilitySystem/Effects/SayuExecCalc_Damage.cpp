#include "SayuExecCalc_Damage.h"
#include "AbilitySystem/Attributes/SayuAttributeSet_Combat.h"
#include "AbilitySystemComponent.h"
#include "SayuLogChannels.h"

// Execution Calculation은 캡처 정의를 static 구조체로 한 번만 만들어두고
// 재사용하는 게 관례예요 (MMC처럼 멤버 변수로 들고 있는 방식과는 다름,
// Execution은 여러 속성을 한꺼번에 다루다 보니 이렇게 모아두는 게 표준 패턴)
struct FSayuDamageStatics
{
	DECLARE_ATTRIBUTE_CAPTUREDEF(AttackPower);
	DECLARE_ATTRIBUTE_CAPTUREDEF(Defense);

	FSayuDamageStatics()
	{
		// 공격자(Source)의 AttackPower
		DEFINE_ATTRIBUTE_CAPTUREDEF(USayuAttributeSet_Combat, AttackPower, Source, false);
		// 피격자(Target)의 Defense - MMC로는 못 했던 부분
		DEFINE_ATTRIBUTE_CAPTUREDEF(USayuAttributeSet_Combat, Defense, Target, false);
	}
};

static const FSayuDamageStatics& DamageStatics()
{
	static FSayuDamageStatics Statics;
	return Statics;
}

USayuExecCalc_Damage::USayuExecCalc_Damage()
{
	RelevantAttributesToCapture.Add(DamageStatics().AttackPowerDef);
	RelevantAttributesToCapture.Add(DamageStatics().DefenseDef);
}

void USayuExecCalc_Damage::Execute_Implementation(
	const FGameplayEffectCustomExecutionParameters& ExecutionParams,
	FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const
{
	const FGameplayEffectSpec& Spec = ExecutionParams.GetOwningSpec();
	
	FAggregatorEvaluateParameters EvaluationParameters;

	float AttackPower = 0.f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(
		DamageStatics().AttackPowerDef, EvaluationParameters, AttackPower);

	float Defense = 0.f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(
		DamageStatics().DefenseDef, EvaluationParameters, Defense);
	
	// SetByCaller로 안 들어왔으면(WarnIfNotFound=false) 기본값 1.0 사용
	const float Multiplier = Spec.GetSetByCallerMagnitude(
		FGameplayTag::RequestGameplayTag(FName("Data.DamageMultiplier")),
		false, 1.0f);

	// 데미지 공식 - 지금은 단순 차감. Defense가 더 크면 0으로 클램프.
	const float FinalDamage = FMath::Max((AttackPower - Defense) * Multiplier, 0.f);
	
	UE_LOG(LogSayuCombat, Verbose,
	TEXT("ExecCalc - AttackPower: %.1f, Defense: %.1f, Multiplier: %.2f, FinalDamage: %.1f"),
	AttackPower, Defense, Multiplier, FinalDamage);

	// Health에 음수로 Additive 적용 = 데미지
	OutExecutionOutput.AddOutputModifier(
		FGameplayModifierEvaluatedData(
			USayuAttributeSet_Combat::GetHealthAttribute(),
			EGameplayModOp::Additive,
			-FinalDamage));
}