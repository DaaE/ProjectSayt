#include "AbilityTask_WeaponTrace.h"
#include "GameFramework/Character.h"
#include "Components/SkeletalMeshComponent.h"

UAbilityTask_WeaponTrace* UAbilityTask_WeaponTrace::WeaponTrace(
	UGameplayAbility* OwningAbility,
	const TArray<FName>& InTraceSocketNames,
	float InTraceRadius)
{
	UAbilityTask_WeaponTrace* Task =
		NewAbilityTask<UAbilityTask_WeaponTrace>(OwningAbility);
	Task->TraceSocketNames = InTraceSocketNames;
	Task->TraceRadius = InTraceRadius;
	return Task;
}

void UAbilityTask_WeaponTrace::Activate()
{
	const int32 NumSockets = TraceSocketNames.Num();
	PreviousLocations.SetNum(NumSockets);
	bHasPreviousLocation.Init(false, NumSockets);

	bTickingTask = true;
	// 이 플래그가 있어야 매 프레임 TickTask가 호출됨
}

void UAbilityTask_WeaponTrace::TickTask(float DeltaTime)
{
	Super::TickTask(DeltaTime);

	ACharacter* AvatarCharacter = Cast<ACharacter>(GetAvatarActor());
	if (!AvatarCharacter)
	{
		return;
	}

	USkeletalMeshComponent* Mesh = AvatarCharacter->GetMesh();
	if (!Mesh)
	{
		return;
	}

	for (int32 i = 0; i < TraceSocketNames.Num(); ++i)
	{
		const FVector CurrentLocation =
			Mesh->GetSocketLocation(TraceSocketNames[i]);

		if (bHasPreviousLocation[i])
		{
			TArray<FHitResult> HitResults;
			FCollisionQueryParams QueryParams;
			QueryParams.AddIgnoredActor(AvatarCharacter);

			GetWorld()->SweepMultiByChannel(
				HitResults,
				PreviousLocations[i],
				CurrentLocation,
				FQuat::Identity,
				ECC_Pawn,
				FCollisionShape::MakeSphere(TraceRadius),
				QueryParams);

			for (const FHitResult& Hit : HitResults)
			{
				AActor* HitActor = Hit.GetActor();
				if (HitActor && !AlreadyHitActors.Contains(HitActor))
				{
					AlreadyHitActors.Add(HitActor);
					OnHit.Broadcast(HitActor);
				}
			}
		}

		PreviousLocations[i] = CurrentLocation;
		bHasPreviousLocation[i] = true;
	}
}