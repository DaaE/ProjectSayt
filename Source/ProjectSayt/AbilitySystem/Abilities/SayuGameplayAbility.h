// Copyright (c) 2026 Sungmin JI. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "SayuGameplayAbility.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTSAYT_API USayuGameplayAbility : public UGameplayAbility
{
	GENERATED_BODY()
	
public:
	USayuGameplayAbility();
	
	// 모든 스킬이 공통으로 가질 데미지 Effect 클래스 참조
	// 자식 클래스(블루프린트)에서 다른 데미지 값으로 오버라이드 가능
	UPROPERTY(EditDefaultsOnly, Category = "Effects")
	TSubclassOf<UGameplayEffect> DamageEffectClass;
};

