// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "SayuNotifyState_GASEvent.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTGAS_API USayuNotifyState_GASEvent : public UAnimNotifyState
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere, Category = "GAS")
	FGameplayTag BeginEventTag;

	UPROPERTY(EditAnywhere, Category = "GAS")
	FGameplayTag EndEventTag;

	virtual void NotifyBegin(
		USkeletalMeshComponent* MeshComp,
		UAnimSequenceBase* Animation,
		float TotalDuration,
		const FAnimNotifyEventReference& EventReference) override;

	virtual void NotifyEnd(
		USkeletalMeshComponent* MeshComp,
		UAnimSequenceBase* Animation,
		const FAnimNotifyEventReference& EventReference) override;
};
