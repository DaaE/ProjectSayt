// Fill out your copyright notice in the Description page of Project Settings.


#include "SayuHUDWidget.h"
#include "AbilitySystemComponent.h"
#include "../AbilitySystem/Attributes/SayuAttributeSet_Combat.h"
#include "AbilitySystemInterface.h"

void USayuHUDWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	// 매 프레임 플레이어 캐릭터의 ASC에서 값을 직접 읽어옴
	// (지금은 가장 단순한 방식. 나중에 델리게이트로 이벤트 기반으로 바꿀 거예요)
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