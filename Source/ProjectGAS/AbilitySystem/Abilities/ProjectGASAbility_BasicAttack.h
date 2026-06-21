// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ProjectGASGameplayAbility.h"
#include "ProjectGASAbility_BasicAttack.generated.h"

class UAnimMontage;
class UAbilityTask_PlayMontageAndWait;
class UAbilityTask_WaitGameplayEvent;

/**
 * 
 */
UCLASS()
class PROJECTGAS_API UProjectGASAbility_BasicAttack : public UProjectGASGameplayAbility
{
	GENERATED_BODY()
	
public:
	UProjectGASAbility_BasicAttack();

protected:
	// ActivateAbility : 어빌리티가 실제로 "발동"될 때 호출되는 함수
	// CanActivateAbility 체크를 통과한 후에만 호출됨 (자동)
	virtual void ActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData) override;
	// 매개변수 4개가 다 낯설 수 있는데, 지금은 패턴으로 외우셔도 돼요.
	// Handle      : 이 어빌리티 인스턴스를 가리키는 식별자
	// ActorInfo   : 누가 이 어빌리티를 쓰는지 (Avatar, Owner 등 정보)
	// ActivationInfo : 예측 관련 정보 (Phase 2-3에서 얘기했던 PredictionKey 포함)
	// TriggerEventData : GameplayEvent로 트리거된 경우의 데이터 (지금은 안 씀)

	virtual void EndAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		bool bReplicateEndAbility,
		bool bWasCancelled) override;
	
	// 쿨타임으로 쓸 GameplayEffect 클래스
	UPROPERTY(EditDefaultsOnly, Category = "Cooldown")
	TSubclassOf<UGameplayEffect> CooldownEffectClass;

	// UGameplayAbility가 CanActivateAbility 체크 시 자동으로 호출하는 함수
	virtual UGameplayEffect* GetCooldownGameplayEffect() const override;
	
	UPROPERTY(EditDefaultsOnly, Category = "Combo")
	TSubclassOf<UGameplayEffect> ComboIncrementEffectClass;
	
	// 콤보 윈도우 유지용 Effect (에디터에서 GE_ComboWindow 연결)
	UPROPERTY(EditDefaultsOnly, Category = "Combo")
	TSubclassOf<UGameplayEffect> ComboWindowEffectClass;

	// 콤보 단계별 데미지 배율 (1타, 2타, 3타)
	UPROPERTY(EditDefaultsOnly, Category = "Combo")
	TArray<float> ComboDamageMultipliers = { 1.0f, 1.3f, 1.8f };
	// 1타는 기본 데미지, 2타는 30% 증가, 3타는 80% 증가
	
	// 콤보별 몽타주 (1타/2타/3타 - 지금은 비어있어도 됨)
	UPROPERTY(EditDefaultsOnly, Category = "Animation")
	TArray<TObjectPtr<UAnimMontage>> ComboMontages;

	// 현재 재생 중인 Task에 대한 참조
	// (UPROPERTY 없이는 GC가 수거해버려서 콜백을 못 받음 - 중요!)
	UPROPERTY()
	TObjectPtr<UAbilityTask_PlayMontageAndWait> MontageTask;

	// Task의 콜백들 - UFUNCTION이어야 델리게이트 바인딩 가능
	UFUNCTION()
	void OnMontageCompleted();

	UFUNCTION()
	void OnMontageInterrupted();

	UFUNCTION()
	void OnMontageCancelled();
	
	// 콤보 윈도우 이벤트 대기용 Task
	UPROPERTY()
	TObjectPtr<UAbilityTask_WaitGameplayEvent> WaitWindowOpenTask;

	UPROPERTY()
	TObjectPtr<UAbilityTask_WaitGameplayEvent> WaitWindowCloseTask;

	UFUNCTION()
	void OnComboWindowOpenEvent(FGameplayEventData Payload);

	UFUNCTION()
	void OnComboWindowCloseEvent(FGameplayEventData Payload);
	
	UPROPERTY()
	TObjectPtr<UAbilityTask_WaitGameplayEvent> WaitComboInputTask;

	UFUNCTION()
	void OnComboInputEvent(FGameplayEventData Payload);

	void PlayComboStep(int32 Index); // 몬타주 전환 + 콤보 증가를 전담하는 헬퍼

	int32 CurrentComboIndex = -1; // 인스턴스가 콤보 생애 동안 직접 들고 있는 상태
};
