// Fill out your copyright notice in the Description page of Project Settings.


#include "SayuAttributeSet.h"
#include "Net/UnrealNetwork.h"
// GetLifetimeReplicatedProps에서 쓰는 DOREPLIFETIME 매크로가 여기 있어요

USayuAttributeSet::USayuAttributeSet()
{
	// 초기값 설정
	// FGameplayAttributeData는 생성자에서 명시적으로 초기화 안 해도
	// 기본값 0이지만, 명확성을 위해 Initter 함수로 설정하는 걸 권장
	InitHealth(100.f);
	InitMaxHealth(100.f);
	InitMana(50.f);
	InitMaxMana(50.f);
	// Init~ 함수들은 ATTRIBUTE_ACCESSORS 매크로가 자동 생성해준 거예요
	
	InitAttackPower(10.f);
	// 기본 공격력 10
	
	InitComboCount(0.f);
	// 임시로 count 수치를 사용 이후 Tag stack api를 이용하는 방식으로 리펙토링 필요.
}

void USayuAttributeSet::GetLifetimeReplicatedProps(
	TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// DOREPLIFETIME_CONDITION_NOTIFY 매크로 설명:
	// - 첫 번째 인자: 어떤 클래스의 변수인지
	// - 두 번째 인자: 어떤 변수인지
	// - COND_None: 항상 복제 (조건 없음). 다른 옵션으로 COND_OwnerOnly 등 있음
	// - REPNOTIFY_Always: 값이 같아도 항상 OnRep 콜백 호출
	//   (GAS는 버프 적용/해제가 같은 값으로 끝나도 UI 갱신이 필요한 경우가 있어서
	//    Always를 권장합니다)
	DOREPLIFETIME_CONDITION_NOTIFY(USayuAttributeSet, Health,
		COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(USayuAttributeSet, MaxHealth,
		COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(USayuAttributeSet, Mana,
		COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(USayuAttributeSet, MaxMana,
		COND_None, REPNOTIFY_Always);
	
	DOREPLIFETIME_CONDITION_NOTIFY(USayuAttributeSet, AttackPower,
		COND_None, REPNOTIFY_Always);
	
	DOREPLIFETIME_CONDITION_NOTIFY(USayuAttributeSet, ComboCount,
		COND_None, REPNOTIFY_Always);
}

void USayuAttributeSet::OnRep_Health(const FGameplayAttributeData& OldHealth)
{
	// GAS 전용 매크로. ASC에게 "이 Attribute가 리플리케이션으로 변경됐다"고
	// 알려주는 역할. UI 바인딩이나 GameplayCue 트리거에 필요해요.
	GAMEPLAYATTRIBUTE_REPNOTIFY(USayuAttributeSet, Health, OldHealth);
}

void USayuAttributeSet::OnRep_MaxHealth(const FGameplayAttributeData& OldMaxHealth)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(USayuAttributeSet, MaxHealth, OldMaxHealth);
}

void USayuAttributeSet::OnRep_Mana(const FGameplayAttributeData& OldMana)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(USayuAttributeSet, Mana, OldMana);
}

void USayuAttributeSet::OnRep_MaxMana(const FGameplayAttributeData& OldMaxMana)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(USayuAttributeSet, MaxMana, OldMaxMana);
}

void USayuAttributeSet::OnRep_AttackPower(const FGameplayAttributeData& OldAttackPower)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(USayuAttributeSet, AttackPower, OldAttackPower);
}

void USayuAttributeSet::OnRep_ComboCount(const FGameplayAttributeData& OldComboCount)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(USayuAttributeSet, ComboCount, OldComboCount);
}

void USayuAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);
	
	// Unity로 치면: health = Mathf.Clamp(health, 0, maxHealth) 와 동일한 역할
	// 어떤 Attribute가 바뀌는지 비교해서 그에 맞는 클램핑 적용
	if (Attribute == GetHealthAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.f, GetMaxHealth());
	}
	else if (Attribute == GetManaAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.f, GetMaxMana());
	}
}
