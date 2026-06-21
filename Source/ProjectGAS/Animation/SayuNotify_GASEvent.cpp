// Fill out your copyright notice in the Description page of Project Settings.


#include "SayuNotify_GASEvent.h"
#include "AbilitySystemBlueprintLibrary.h"
// SendGameplayEventToActor가 정의된 헤더

void USayuNotify_GASEvent::Notify(
	USkeletalMeshComponent* MeshComp,
	UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	if (AActor* Owner = MeshComp->GetOwner())
	{
		FGameplayEventData EventData;
		// Payload는 지금 비워둠 - 나중에 추가 데이터 필요하면 여기 채움

		UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(Owner, EventTag, EventData);
		// Owner가 IAbilitySystemInterface를 구현하지 않으면 조용히 실패함
		// 우리 AProjectGASCharacter는 이미 구현했으니 안전
	}
}
