// Copyright (c) 2026 Sungmin JI. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SayuPooledEffect.generated.h"

class UNiagaraComponent;
class UAudioComponent;
class UNiagaraSystem;
class USoundBase;

// 이 액터가 재생을 끝내고 풀로 돌아갈 준비가 됐음을 알리는 델리게이트
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FEffectFinishedDelegate, AActor*, FinishedEffect);

// 타격 이펙트(Niagara)+사운드를 묶어서 재생하는 풀링 전용 액터.
// Spawn/Destroy 대신, 미리 만들어둔 인스턴스를 빌려서 재생하고 다시 숨긴다.
UCLASS()
class PROJECTSAYT_API ASayuPooledEffect : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ASayuPooledEffect();
	
	UPROPERTY(BlueprintAssignable)
	FEffectFinishedDelegate OnEffectFinished;

	// 풀 매니저가 이 액터를 빌려갈 때 호출
	void PlayEffect(const FVector& Location, const FRotator& Rotation, float PitchMultiplier = 1.f);

protected:
	UPROPERTY(VisibleAnywhere, Category = "Effect")
	TObjectPtr<UNiagaraComponent> NiagaraComp;

	UPROPERTY(VisibleAnywhere, Category = "Effect")
	TObjectPtr<UAudioComponent> AudioComp;

	// Aurora FX/Audio 폴더에서 골라서 여기 채워넣을 것 (블루프린트에서)
	UPROPERTY(EditDefaultsOnly, Category = "Effect")
	TObjectPtr<UNiagaraSystem> NiagaraSystemAsset;

	UPROPERTY(EditDefaultsOnly, Category = "Effect")
	TObjectPtr<USoundBase> SoundAsset;

	// 이 시간이 지나면 자동으로 숨겨지고 풀에 반납됨
	UPROPERTY(EditDefaultsOnly, Category = "Effect")
	float EffectDuration = 2.f;

public:	
	FTimerHandle FinishTimerHandle;

	void OnFinishTimer();
};
