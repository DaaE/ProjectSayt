// Fill out your copyright notice in the Description page of Project Settings.


#include "SayuAttributeSet_Combat.h"
#include "Net/UnrealNetwork.h"
#include "GameplayEffectExtension.h"	// 전방클래스 선언만으로는 내부 멤버에 접근할 수 없다.
#include "GameFramework/GameplayMessageSubsystem.h"
#include "Messages/SayuCombatMessages.h"

USayuAttributeSet_Combat::USayuAttributeSet_Combat()
{
	InitHealth(100.f);
	InitMaxHealth(100.f);
	InitMana(50.f);
	InitMaxMana(50.f);
	InitAttackPower(10.f);
	InitDefense(0.f);
	InitComboCount(0.f);
}

void USayuAttributeSet_Combat::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(USayuAttributeSet_Combat, Health, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(USayuAttributeSet_Combat, MaxHealth, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(USayuAttributeSet_Combat, Mana, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(USayuAttributeSet_Combat, MaxMana, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(USayuAttributeSet_Combat, AttackPower, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(USayuAttributeSet_Combat, Defense, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(USayuAttributeSet_Combat, ComboCount, COND_None, REPNOTIFY_Always);
}

void USayuAttributeSet_Combat::OnRep_Health(const FGameplayAttributeData& OldHealth)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(USayuAttributeSet_Combat, Health, OldHealth);
}

void USayuAttributeSet_Combat::OnRep_MaxHealth(const FGameplayAttributeData& OldMaxHealth)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(USayuAttributeSet_Combat, MaxHealth, OldMaxHealth);
}

void USayuAttributeSet_Combat::OnRep_Mana(const FGameplayAttributeData& OldMana)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(USayuAttributeSet_Combat, Mana, OldMana);
}

void USayuAttributeSet_Combat::OnRep_MaxMana(const FGameplayAttributeData& OldMaxMana)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(USayuAttributeSet_Combat, MaxMana, OldMaxMana);
}

void USayuAttributeSet_Combat::OnRep_AttackPower(const FGameplayAttributeData& OldAttackPower)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(USayuAttributeSet_Combat, AttackPower, OldAttackPower);
}

void USayuAttributeSet_Combat::OnRep_Defense(const FGameplayAttributeData& OldDefense)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(USayuAttributeSet_Combat, Defense, OldDefense);
}

void USayuAttributeSet_Combat::OnRep_ComboCount(const FGameplayAttributeData& OldComboCount)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(USayuAttributeSet_Combat, ComboCount, OldComboCount);
}

void USayuAttributeSet_Combat::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);

	if (Attribute == GetHealthAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.f, GetMaxHealth());
	}
	else if (Attribute == GetManaAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.f, GetMaxMana());
	}
}

void USayuAttributeSet_Combat::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);

	if (Data.EvaluatedData.Attribute == GetHealthAttribute())
	{
		UE_LOG(LogTemp, Warning, TEXT("%s Health changed: %.1f / %.1f"),
			*GetOwningActor()->GetName(), GetHealth(), GetMaxHealth());
		SetHealth(FMath::Clamp(GetHealth(), 0.f, GetMaxHealth()));
		
		// === GMS Broadcast: 데미지 적용 사실을 다른 시스템들에게 알림 ===
		// ExecCalc_Damage에서 Additive로 음수 적용했으니, 부호를 뒤집어야 "깎인 양"이 됩니다.
		const float DamageAmount = -Data.EvaluatedData.Magnitude;
		if (DamageAmount > 0.f) // 힐(양수 적용)일 땐 데미지 메시지를 안 보냄
		{
			FSayuDamageMessage Message;
			Message.Instigator = Data.EffectSpec.GetContext().GetOriginalInstigator();
			Message.Target = GetOwningActor();
			Message.DamageAmount = DamageAmount;
			Message.NewHealth = GetHealth();
			Message.MaxHealth = GetMaxHealth();
			// TODO(임시): 아직 실제 크리티컬 스탯이 없어서, 페이로드 분기 데모용 15% 고정값입니다.
			// 나중에 크리티컬 관련 스탯이 생기면 ExecCalc_Damage 쪽에서 계산해서 넘기는 방식으로 교체할 것.
			Message.bIsCritical = FMath::FRand() < 0.15f;
		
			UGameplayMessageSubsystem::Get(this).BroadcastMessage(
				TAG_Message_Combat_Damage_Dealt, Message);
		}
	}
}
