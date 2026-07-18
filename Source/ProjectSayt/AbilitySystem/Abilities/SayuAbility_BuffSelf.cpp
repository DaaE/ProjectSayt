// Copyright (c) 2026 Sungmin JI. All Rights Reserved.


#include "SayuAbility_BuffSelf.h"
#include "AbilitySystemComponent.h"
#include "GameplayEffect.h"

USayuAbility_BuffSelf::USayuAbility_BuffSelf()
{
	FGameplayTagContainer NewTags;
	NewTags.AddTag(FGameplayTag::RequestGameplayTag(FName("Ability.Skill.Dash")));
	// 마침 Phase 0에 등록된 태그 중 비어있는 걸 재활용
	// 나중에 진짜 "대시" 어빌리티를 만들면 이 태그명을 바꿔주면 됩니다
	SetAssetTags(NewTags);
}

void USayuAbility_BuffSelf::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	if (BuffEffectClass)
	{
		FGameplayEffectSpecHandle EffectSpec =
			MakeOutgoingGameplayEffectSpec(BuffEffectClass);

		if (EffectSpec.IsValid())
		{
			ApplyGameplayEffectSpecToOwner(Handle, ActorInfo, ActivationInfo, EffectSpec);
		}
	}

	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}

void USayuAbility_BuffSelf::EndAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility, bool bWasCancelled)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}
