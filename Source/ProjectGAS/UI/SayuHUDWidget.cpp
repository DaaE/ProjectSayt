// Fill out your copyright notice in the Description page of Project Settings.


#include "SayuHUDWidget.h"
#include "AbilitySystemComponent.h"
#include "../AbilitySystem/Attributes/SayuAttributeSet_Combat.h"
#include "AbilitySystemInterface.h"
#include "Messages/SayuCombatMessages.h"

void USayuHUDWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	
	// 위젯 생성 시점엔 아직 데미지 이벤트가 없었으니 최초 1회는 직접 읽어서 초기화
	SyncInitialHealthFromASC();
		
	DamageListenerHandle = UGameplayMessageSubsystem::Get(this).RegisterListener(
		TAG_Message_Combat_Damage_Dealt, this, &USayuHUDWidget::OnDamageMessage);
}

void USayuHUDWidget::NativeDestruct()
{
	// 위젯이 사라질 때 리스너 해제 필수 - 안 하면 죽은 위젯을 향한 댕글링 콜백이 남음
	DamageListenerHandle.Unregister();
	
	Super::NativeDestruct();
}

void USayuHUDWidget::SyncInitialHealthFromASC()
{
	
	if (APlayerController* PC = GetOwningPlayer())
	{
		if (APawn* Pawn = PC->GetPawn())
		{
			if (IAbilitySystemInterface* ASCInterface =
				Cast<IAbilitySystemInterface>(Pawn))
			{
				if (UAbilitySystemComponent* ASC =
					ASCInterface->GetAbilitySystemComponent())
				{
					if (const USayuAttributeSet_Combat* AttrSet =
						ASC->GetSet<USayuAttributeSet_Combat>())
						// GetSet<T> : ASC가 들고 있는 여러 AttributeSet 중에
							// 원하는 타입을 찾아오는 함수
					{
						CurrentHealth = AttrSet->GetHealth();
						MaxHealth = AttrSet->GetMaxHealth();
					}
				}
			}
		}
	}
}

void USayuHUDWidget::OnDamageMessage(FGameplayTag Channel, const FSayuDamageMessage& Message)
{
	// 채널은 전역이라 모든 액터의 데미지 이벤트가 다 들어옵니다.
	// 내 캐릭터가 맞은 경우만 골라서 반응해야 해요 - 안 하면 npc가 맞아도 내 HUD가 깎임
	if (Message.Target != GetOwningPlayerPawn())
	{
		return;
	}
	
	CurrentHealth = Message.NewHealth;
	MaxHealth = Message.MaxHealth;
}
