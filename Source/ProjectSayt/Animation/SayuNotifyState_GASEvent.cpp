// Copyright (c) 2026 Sungmin JI. All Rights Reserved.


#include "Animation/SayuNotifyState_GASEvent.h"
#include "AbilitySystemBlueprintLibrary.h"


void USayuNotifyState_GASEvent::NotifyBegin(
	USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);
	
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
	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(Owner, BeginEventTag, EventData);
}

void USayuNotifyState_GASEvent::NotifyEnd(
	USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);
	
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
	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(Owner, EndEventTag, EventData);
}
