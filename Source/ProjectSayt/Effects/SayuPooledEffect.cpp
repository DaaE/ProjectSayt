// Copyright (c) 2026 Sungmin JI. All Rights Reserved.


#include "Effects/SayuPooledEffect.h"
#include "NiagaraComponent.h"
#include "Components/AudioComponent.h"

// Sets default values
ASayuPooledEffect::ASayuPooledEffect()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	// 매 프레임 갱신할 게 없는 단순 이펙트 액터라 틱 꺼둠 (성능)
	
	NiagaraComp = CreateDefaultSubobject<UNiagaraComponent>(TEXT("NiagaraComp"));
	RootComponent = NiagaraComp;
	NiagaraComp->bAutoActivate = false; // PlayEffect에서 직접 재생 시작

	AudioComp = CreateDefaultSubobject<UAudioComponent>(TEXT("AudioComp"));
	AudioComp->SetupAttachment(RootComponent);
	AudioComp->bAutoActivate = false;

	// 풀에서 쉬는 동안은 화면에 안 보이게
	SetActorHiddenInGame(true);
}

void ASayuPooledEffect::PlayEffect(const FVector& Location, const FRotator& Rotation, float PitchMultiplier)
{
	SetActorLocationAndRotation(Location, Rotation);
	SetActorHiddenInGame(false);

	if (NiagaraSystemAsset)
	{
		NiagaraComp->SetAsset(NiagaraSystemAsset);
		NiagaraComp->Activate(true);
	}

	if (SoundAsset)
	{
		AudioComp->SetSound(SoundAsset);
		AudioComp->SetPitchMultiplier(PitchMultiplier);
		AudioComp->Play();
	}

	GetWorld()->GetTimerManager().SetTimer(
		FinishTimerHandle, this, &ASayuPooledEffect::OnFinishTimer,
		EffectDuration, false);
}

void ASayuPooledEffect::OnFinishTimer()
{
	SetActorHiddenInGame(true);
	OnEffectFinished.Broadcast(this);
	// Destroy() 안 함 - 이게 풀링의 핵심. 숨기기만 하고 재사용 대기.
}