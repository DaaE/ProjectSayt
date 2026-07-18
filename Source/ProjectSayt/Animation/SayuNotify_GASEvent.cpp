// Copyright (c) 2026 Sungmin JI. All Rights Reserved.


#include "SayuNotify_GASEvent.h"
#include "AbilitySystemBlueprintLibrary.h"
// SendGameplayEventToActor가 정의된 헤더

void USayuNotify_GASEvent::Notify(
	USkeletalMeshComponent* MeshComp,
	UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);
	
	AActor* Owner = MeshComp->GetOwner();
	if (!Owner)
	{
		return;
	}
	
	// 에디터에서 몬타주 미리보기할 때 생기는 AnimationEditorPreviewActor 등은
	// ASC가 없는 게 정상 - 그런 경우는 조용히 무시 (에러 로그 스팸 방지)
	if (!UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Owner))
	{
		return;
	}

	FGameplayEventData EventData;
	// Payload는 지금 비워둠 - 나중에 추가 데이터 필요하면 여기 채움

	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(Owner, EventTag, EventData);
	// Owner가 IAbilitySystemInterface를 구현하지 않으면 조용히 실패함
	// 우리 AProjectGASCharacter는 이미 구현했으니 안전
}
