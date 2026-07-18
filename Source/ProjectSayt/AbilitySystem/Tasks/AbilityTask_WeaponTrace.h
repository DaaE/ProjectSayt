// Copyright (c) 2026 Sungmin JI. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/Tasks/AbilityTask.h"
#include "DrawDebugHelpers.h"
#include "AbilityTask_WeaponTrace.generated.h"

// 검에 새로 맞은 대상이 생길 때마다 이 델리게이트로 알려줌
// (TwoParams = 매개변수 2개. 정확한 충돌 지점(FHitResult)까지 같이 넘겨줌)
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FWeaponTraceHitDelegate, AActor*, HitActor, FHitResult, Hit);

// 무기에 달린 소켓 여러 개(검 손잡이~검 끝)를 매 프레임 스윕 트레이스해서,
// 한 번의 휘두름 동안 새로 맞은 대상만 델리게이트로 알려주는 Task.
// 한 시점만 검사하면 빠르게 휘두를 때 얇은 대상을 그냥 통과(터널링)할 수 있어서,
// "이전 프레임 위치 -> 현재 프레임 위치"를 스윕해서 그 구간 전체를 검사한다.
UCLASS()
class PROJECTSAYT_API UAbilityTask_WeaponTrace : public UAbilityTask
{
	GENERATED_BODY()

public:
	// 새로 맞은 대상이 생기면 이 델리게이트가 발사됨.
	// 어빌리티 쪽에서 여기에 함수를 바인딩해서 데미지를 적용할 예정.
	UPROPERTY(BlueprintAssignable)
	FWeaponTraceHitDelegate OnHit;

	// 이 Task를 생성하는 정적 팩토리 함수.
	// AbilityTask는 항상 new 대신 이런 정적 함수로 생성해야 함
	// (GAS가 내부적으로 Task 생명주기를 관리하기 때문)
	UFUNCTION(BlueprintCallable, meta = (
		HidePin = "OwningAbility",
		DefaultToSelf = "OwningAbility",
		BlueprintInternalUseOnly = "true"))
	static UAbilityTask_WeaponTrace* WeaponTrace(
		UGameplayAbility* OwningAbility,
		const TArray<FName>& InTraceSocketNames,
		float InTraceRadius);

	// Task가 실제로 시작될 때 호출 (ReadyForActivation() 호출 시점)
	virtual void Activate() override;
	
	// bTickingTask가 true인 동안 매 프레임 호출됨
	virtual void TickTask(float DeltaTime) override;

private:
	// 검사할 소켓 이름들 (예: sword_base, sword_mid, sword_tip)
	TArray<FName> TraceSocketNames;
	
	// 트레이스에 쓸 구체 반지름
	float TraceRadius = 6.f;

	// 소켓별로 "지난 프레임에 어디 있었는지" 기억해두는 배열
	// (스윕 트레이스의 시작점으로 씀)
	TArray<FVector> PreviousLocations;
	
	// 소켓별로 "한 번이라도 위치를 기록한 적 있는지" 체크
	// (첫 프레임에는 비교할 이전 위치가 없으니까 트레이스를 건너뛰기 위함)
	TArray<bool> bHasPreviousLocation;

	// 이번 Task가 살아있는 동안(한 번의 휘두름 동안) 이미 맞춘 대상들.
	// 같은 대상을 중복으로 맞추지 않기 위한 장치.
	UPROPERTY()
	TSet<TObjectPtr<AActor>> AlreadyHitActors;
};