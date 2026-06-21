// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/Tasks/AbilityTask.h"
#include "AbilityTask_WeaponTrace.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FWeaponTraceHitDelegate, AActor*, HitActor);

// 무기에 달린 소켓 여러 개를 매 프레임 스윕 트레이스해서,
// 한 번의 휘두름 동안 새로 맞은 대상만 델리게이트로 알려준다.
UCLASS()
class PROJECTGAS_API UAbilityTask_WeaponTrace : public UAbilityTask
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintAssignable)
	FWeaponTraceHitDelegate OnHit;

	UFUNCTION(BlueprintCallable, meta = (
		HidePin = "OwningAbility",
		DefaultToSelf = "OwningAbility",
		BlueprintInternalUseOnly = "true"))
	static UAbilityTask_WeaponTrace* WeaponTrace(
		UGameplayAbility* OwningAbility,
		const TArray<FName>& InTraceSocketNames,
		float InTraceRadius);

	virtual void Activate() override;
	virtual void TickTask(float DeltaTime) override;

private:
	TArray<FName> TraceSocketNames;
	float TraceRadius = 6.f;

	TArray<FVector> PreviousLocations;
	TArray<bool> bHasPreviousLocation;

	UPROPERTY()
	TSet<TObjectPtr<AActor>> AlreadyHitActors;
};