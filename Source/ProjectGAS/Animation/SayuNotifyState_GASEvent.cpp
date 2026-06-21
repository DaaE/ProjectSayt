// Fill out your copyright notice in the Description page of Project Settings.


#include "Animation/SayuNotifyState_GASEvent.h"
#include "AbilitySystemBlueprintLibrary.h"


void USayuNotifyState_GASEvent::NotifyBegin(
	USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);
	
	if (AActor* Owner = MeshComp->GetOwner())
	{
		FGameplayEventData EventData;
		UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(
			Owner, BeginEventTag, EventData);
	}
}

void USayuNotifyState_GASEvent::NotifyEnd(
	USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);
	
	if (AActor* Owner = MeshComp->GetOwner())
	{
		FGameplayEventData EventData;
		UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(
			Owner, EndEventTag, EventData);
	}
}
