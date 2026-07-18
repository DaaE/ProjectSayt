#include "AbilityTask_WeaponTrace.h"
#include "GameFramework/Character.h"
#include "Components/SkeletalMeshComponent.h"
#include "SayuLogChannels.h"

UAbilityTask_WeaponTrace* UAbilityTask_WeaponTrace::WeaponTrace(
	UGameplayAbility* OwningAbility,
	const TArray<FName>& InTraceSocketNames,
	float InTraceRadius)
{
	// NewAbilityTask : AbilityTask 전용 생성 함수.
	// 일반 NewObject 대신 이걸 써야 GAS가 이 Task를
	// 소유 어빌리티의 생명주기에 맞춰 관리해줌
	UAbilityTask_WeaponTrace* Task =
		NewAbilityTask<UAbilityTask_WeaponTrace>(OwningAbility);
	Task->TraceSocketNames = InTraceSocketNames;
	Task->TraceRadius = InTraceRadius;
	return Task;
}

void UAbilityTask_WeaponTrace::Activate()
{
	// 소켓 개수만큼 "이전 위치 기록용" 배열을 미리 크기 맞춰서 준비
	const int32 NumSockets = TraceSocketNames.Num();
	PreviousLocations.SetNum(NumSockets);
	bHasPreviousLocation.Init(false, NumSockets);
	
	// 활성화되는 그 순간 소켓 위치를 바로 기록해서,
	// 첫 틱부터 곧바로 검사가 가능하게 함 (기준점 잡기로 틱 낭비 안 하게)
	if (ACharacter* AvatarCharacter = Cast<ACharacter>(GetAvatarActor()))
	{
		if (USkeletalMeshComponent* Mesh = AvatarCharacter->GetMesh())
		{
			for (int32 i = 0; i < TraceSocketNames.Num(); ++i)
			{
				PreviousLocations[i] = Mesh->GetSocketLocation(TraceSocketNames[i]);
				bHasPreviousLocation[i] = true;
			}
		}
	}

	// 이 플래그가 있어야 매 프레임 TickTask가 호출됨
	bTickingTask = true;
}

void UAbilityTask_WeaponTrace::TickTask(float DeltaTime)
{
	Super::TickTask(DeltaTime);

	// 이 Task를 소유한 캐릭터(검을 든 본인)를 가져옴
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

	// 소켓 하나씩 돌면서, 지난 프레임 위치 -> 현재 프레임 위치로 스윕 트레이스
	for (int32 i = 0; i < TraceSocketNames.Num(); ++i)
	{
		const FVector CurrentLocation =
			Mesh->GetSocketLocation(TraceSocketNames[i]);

		if (bHasPreviousLocation[i])
		{
			TArray<FHitResult> HitResults;
			FCollisionQueryParams QueryParams;
			QueryParams.AddIgnoredActor(AvatarCharacter);	// 자기 자신은 제외

			// SweepMultiByChannel : 시작점~끝점 사이 "구간 전체"를 검사.
			// 단일 시점 OverlapMulti와 다르게, 그 사이를 빠르게 지나간
			// 대상도 잡아낼 수 있음 (터널링 방지)
			GetWorld()->SweepMultiByChannel(
				HitResults,
				PreviousLocations[i],
				CurrentLocation,
				FQuat::Identity,
				ECC_Pawn,
				FCollisionShape::MakeSphere(TraceRadius),
				QueryParams);
			
#if !UE_BUILD_SHIPPING
			// 디버그용 - 트레이스가 실제로 지나간 경로를 화면에 그림
			DrawDebugLine(GetWorld(), PreviousLocations[i], CurrentLocation,
				FColor::Yellow, false, 1.5f, 0, 1.5f);
			DrawDebugSphere(GetWorld(), CurrentLocation, TraceRadius, 8,
				FColor::Red, false, 1.5f);
#endif

			for (const FHitResult& Hit : HitResults)
			{
				AActor* HitActor = Hit.GetActor();
				if (HitActor && !AlreadyHitActors.Contains(HitActor))
				{
					UE_LOG(LogSayuCombat, Verbose, TEXT("스윕 충돌 발견: %s"), *HitActor->GetName());
					AlreadyHitActors.Add(HitActor);
					OnHit.Broadcast(HitActor, Hit);	// 어빌리티 쪽에 새 타격 알림
				}
			}
		}

		PreviousLocations[i] = CurrentLocation;
		bHasPreviousLocation[i] = true;
	}
}