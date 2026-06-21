// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "SayuNotify_GASEvent.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTGAS_API USayuNotify_GASEvent : public UAnimNotify
{
	GENERATED_BODY()
	
public:
	// 에디터에서 어떤 이벤트인지 태그로 지정
	// SaveAttack Notify에는 Combo.Window.Open
	// ResetCombo Notify에는 별도 종료용 태그를 지정할 거예요
	UPROPERTY(EditAnywhere, Category = "GAS")
	FGameplayTag EventTag;

	virtual void Notify(USkeletalMeshComponent* MeshComp,
		UAnimSequenceBase* Animation,
		const FAnimNotifyEventReference& EventReference) override;
};
