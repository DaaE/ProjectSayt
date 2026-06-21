// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "AbilitySystemComponent.h"
#include "ProjectGASAttributeSetMacros.h"
#include "ProjectGASAttributeSet.generated.h"

UCLASS()
class PROJECTGAS_API UProjectGASAttributeSet : public UAttributeSet
{
	GENERATED_BODY()
	
public:
	UProjectGASAttributeSet();
	
	// === 체력 ===
	UPROPERTY(BlueprintReadOnly, Category = "Health", ReplicatedUsing = OnRep_Health)
	FGameplayAttributeData Health;
	// FGameplayAttributeData : GAS 전용 수치 타입
	// 단순 float이 아니라 "BaseValue(기본값)"와 "CurrentValue(현재값)"를
	// 따로 들고 있어요. 버프가 잠깐 올렸다가 풀리는 경우를 추적하려고요.
	// Unity의 float health 한 줄과 가장 큰 차이점입니다.
	
	ATTRIBUTE_ACCESSORS(UProjectGASAttributeSet, Health)
	
	UPROPERTY(BlueprintReadOnly, Category = "Health", ReplicatedUsing = OnRep_MaxHealth)
	FGameplayAttributeData MaxHealth;
	ATTRIBUTE_ACCESSORS(UProjectGASAttributeSet, MaxHealth)
	
	// === 마나 ===
	UPROPERTY(BlueprintReadOnly, Category = "Mana",	ReplicatedUsing = OnRep_Mana)
	FGameplayAttributeData Mana;
	ATTRIBUTE_ACCESSORS(UProjectGASAttributeSet, Mana)

	UPROPERTY(BlueprintReadOnly, Category = "Mana",	ReplicatedUsing = OnRep_MaxMana)
	FGameplayAttributeData MaxMana;
	ATTRIBUTE_ACCESSORS(UProjectGASAttributeSet, MaxMana)
	
	UPROPERTY(BlueprintReadOnly, Category = "Combat", ReplicatedUsing = OnRep_AttackPower)
	FGameplayAttributeData AttackPower;
	ATTRIBUTE_ACCESSORS(UProjectGASAttributeSet, AttackPower)

	UFUNCTION()
	virtual void OnRep_AttackPower(const FGameplayAttributeData& OldAttackPower);
	
	UPROPERTY(BlueprintReadOnly, Category = "Combat", ReplicatedUsing = OnRep_ComboCount)
	FGameplayAttributeData ComboCount;
	ATTRIBUTE_ACCESSORS(UProjectGASAttributeSet, ComboCount)

	UFUNCTION()
	virtual void OnRep_ComboCount(const FGameplayAttributeData& OldComboCount);
	
	// === 리플리케이션 설정 ===
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	// C#에는 이런 개념이 없었죠. 멀티플레이어에서 "이 변수를 서버에서 클라이언트로 자동 동기화하라"고 등록하는 함수예요.
	
	// 값이 변경되어 클라이언트에 복제됐을 때 호출되는 콜백들
	UFUNCTION()
	virtual void OnRep_Health(const FGameplayAttributeData& OldHealth);
	
	UFUNCTION()
	virtual void OnRep_MaxHealth(const FGameplayAttributeData& OldMaxHealth);

	UFUNCTION()
	virtual void OnRep_Mana(const FGameplayAttributeData& OldMana);

	UFUNCTION()
	virtual void OnRep_MaxMana(const FGameplayAttributeData& OldMaxMana);
		
protected:
	// GameplayEffect가 적용되기 직전에 가로채서 클램핑(범위 제한) 하는 함수
	// Unity에서 health = Mathf.Clamp(health, 0, maxHealth) 했던 로직이
	// 여기로 이동합니다
	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;
	
};
