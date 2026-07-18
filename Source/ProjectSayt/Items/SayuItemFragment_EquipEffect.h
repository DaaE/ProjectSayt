// Copyright (c) 2026 Sungmin JI. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "SayuItemFragment.h"
#include "SayuItemFragment_EquipEffect.generated.h"

class UGameplayEffect;

/**
 * 장착 시 지정된 GameplayEffect를 캐릭터에게 부여하는 Fragment.
 * 예: 공격력 버프, 방어력 버프 등 — 어떤 효과인지는 EffectToApply에 에디터에서 지정.
 */
UCLASS()
class PROJECTSAYT_API USayuItemFragment_EquipEffect : public USayuItemFragment
{
	GENERATED_BODY()
	
public:
	virtual void OnEquipped(ASayuCharacterBase* OwningCharacter) const override;

protected:
	// 장착 시 부여할 GameplayEffect. 어떤 구체적인 GE인지(버프 종류, 수치 등)는
	// 에디터에서 GE 에셋을 골라 지정 — 이 클래스는 "장착하면 GE를 준다"는 동작만 책임짐.
	UPROPERTY(EditDefaultsOnly, Category = "Sayu|Item")
	TSubclassOf<UGameplayEffect> EffectToApply;
};
