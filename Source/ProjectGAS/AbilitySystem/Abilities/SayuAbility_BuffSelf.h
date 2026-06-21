// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/SayuGameplayAbility.h"
#include "SayuAbility_BuffSelf.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTGAS_API USayuAbility_BuffSelf : public USayuGameplayAbility
{
	GENERATED_BODY()
	
public:
	USayuAbility_BuffSelf();

protected:
	// 버프로 사용할 Effect (에디터에서 GE_Buff_AttackUp 연결할 거예요)
	UPROPERTY(EditDefaultsOnly, Category = "Effects")
	TSubclassOf<UGameplayEffect> BuffEffectClass;

	virtual void ActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData) override;

	virtual void EndAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		bool bReplicateEndAbility,
		bool bWasCancelled) override;
};
