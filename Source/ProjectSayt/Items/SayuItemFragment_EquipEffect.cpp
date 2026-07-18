// Copyright (c) 2026 Sungmin JI. All Rights Reserved.


#include "SayuItemFragment_EquipEffect.h"
#include "AbilitySystemComponent.h"
#include "Character/SayuCharacterBase.h"
#include "GameplayEffect.h"

void USayuItemFragment_EquipEffect::OnEquipped(ASayuCharacterBase* OwningCharacter) const
{
	if (!OwningCharacter || !EffectToApply)
	{
		return;
	}

	UAbilitySystemComponent* ASC = OwningCharacter->GetAbilitySystemComponent();
	if (!ASC)
	{
		return;
	}

	const FGameplayEffectContextHandle ContextHandle = ASC->MakeEffectContext();
	const FGameplayEffectSpecHandle SpecHandle =
		ASC->MakeOutgoingSpec(EffectToApply, 1.f, ContextHandle);

	if (SpecHandle.IsValid())
	{
		ASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
	}
}
