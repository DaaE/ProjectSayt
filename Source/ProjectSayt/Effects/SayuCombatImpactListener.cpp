// Copyright (c) 2026 Sungmin JI. All Rights Reserved.


#include "SayuCombatImpactListener.h"

#include "SayuEffectPoolManager.h"
#include "Messages/SayuCombatMessages.h"


// Sets default values
ASayuCombatImpactListener::ASayuCombatImpactListener()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
}

// Called when the game starts or when spawned
void ASayuCombatImpactListener::BeginPlay()
{
	Super::BeginPlay();
	
	DamageListenerHandle = UGameplayMessageSubsystem::Get(this).RegisterListener(
		TAG_Message_Combat_Damage_Dealt, this, &ASayuCombatImpactListener::OnDamageMessage);
}

void ASayuCombatImpactListener::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	DamageListenerHandle.Unregister();
	Super::EndPlay(EndPlayReason);
}

void ASayuCombatImpactListener::OnDamageMessage(FGameplayTag Channel, const FSayuDamageMessage& Message)
{
	if (!HitEffectPool || !Message.Target)
	{
		return;
	}

	const FVector Location = Message.HitLocation;
	const FRotator Rotation = Message.Target->GetActorRotation();
	const float Pitch = Message.bIsCritical ? CriticalPitchMultiplier : 1.f;

	HitEffectPool->PlayEffectAtLocation(Location, Rotation, Pitch);
}