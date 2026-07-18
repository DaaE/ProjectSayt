// Copyright (c) 2026 Sungmin JI. All Rights Reserved.


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
		// Base(원장) 교정: 막타 오버킬로 Base가 음수로 남는 것 방지.
		// (PreAttributeChange는 Current 쪽 관문이라 Base까지 지켜주지 않음 —
		//  Base가 -6.8로 남으면 이후 +10 힐이 3.2부터 시작하는 잠식 버그가 됨)
		// 단, 실제로 교정이 필요할 때만 Set — 정상 타격마다 나가던
		// 무변화 2차 방송(48->48)을 여기서 제거.
		const float BaseHealth = GetOwningAbilitySystemComponent()->GetNumericAttributeBase(GetHealthAttribute());
		const float ClampedBase = FMath::Clamp(BaseHealth, 0.f, GetMaxHealth());
		if (!FMath::IsNearlyEqual(BaseHealth, ClampedBase))
		{
			SetHealth(ClampedBase);
		}
		
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
			
			// 실제 충돌 지점이 있으면 그걸 쓰고, 없으면(디버그 자가데미지 등) 액터 위치로 대체
			if (const FHitResult* HitResult = Data.EffectSpec.GetContext().GetHitResult())
			{
				Message.HitLocation = HitResult->ImpactPoint;
			}
			else if (Message.Target)
			{
				Message.HitLocation = Message.Target->GetActorLocation();
			}
			
			// TODO(임시): 아직 실제 크리티컬 스탯이 없어서, 페이로드 분기 데모용 15% 고정값입니다.
			// 나중에 크리티컬 관련 스탯이 생기면 ExecCalc_Damage 쪽에서 계산해서 넘기는 방식으로 교체할 것.
			Message.bIsCritical = FMath::FRand() < 0.15f;
		
			UGameplayMessageSubsystem::Get(this).BroadcastMessage(
				TAG_Message_Combat_Damage_Dealt, Message);
		}
	}
}
