// Copyright (c) 2026 Sungmin JI. All Rights Reserved.


#include "SayuGameplayAbility.h"

USayuGameplayAbility::USayuGameplayAbility()
{
	// InstancingPolicy : 이 어빌리티가 발동될 때마다 새 인스턴스를 만들지,
	// 캐릭터당 하나만 만들지를 결정해요.
	// Unity에는 없는 개념인데, GAS는 어빌리티 자체가 "객체"라서
	// 메모리/상태 관리 방식을 선택해야 해요.
	
	//InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	// InstancedPerActor : 캐릭터 한 명당 어빌리티 인스턴스 하나
	// (다른 옵션: InstancedPerExecution - 발동마다 새로 생성, NonInstanced - 공유)
	
	// 변경
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerExecution;
	// 매 발동(Activate)마다 완전히 새로운 어빌리티 인스턴스를 생성
	// 콤보 1타, 2타, 3타가 각각 독립된 인스턴스로 처리되어
	// Task 충돌이나 상태 공유 문제가 원천적으로 사라짐
	
	// 이미 활성 중인 같은 어빌리티에 재진입을 허용할지 여부
	bRetriggerInstancedAbility = false;
	// false면 이미 실행 중인 인스턴스를 취소하지 않고 무시
	// (지금 기본 동작이 true였을 가능성이 있어서 이게 원인일 수 있음)
}
