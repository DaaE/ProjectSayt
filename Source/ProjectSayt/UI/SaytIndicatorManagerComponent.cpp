// Copyright (c) 2026 Sungmin JI. All Rights Reserved.

#include "UI/SaytIndicatorManagerComponent.h"

#include "GameFramework/PlayerController.h"
#include "SaytLogChannels.h"

USaytIndicatorManagerComponent* USaytIndicatorManagerComponent::FindOrAdd(APlayerController* InPC)
{
	if (!InPC)
	{
		return nullptr;
	}

	if (USaytIndicatorManagerComponent* Existing = InPC->FindComponentByClass<USaytIndicatorManagerComponent>())
	{
		return Existing;
	}

	// Unity의 AddComponent<T>()에 해당 — RegisterComponent를 빼면 엔진이 모르는 객체가 된다
	USaytIndicatorManagerComponent* NewComp = NewObject<USaytIndicatorManagerComponent>(InPC);
	NewComp->RegisterComponent();

	UE_LOG(LogSaytUI, Verbose, TEXT("[인디케이터] 매니저 생성 및 등록 (%s)"), *InPC->GetName());
	return NewComp;
}

void USaytIndicatorManagerComponent::AddIndicator(AActor* InActor, ESaytHealthDisplayType InType)
{
	if (!InActor)
	{
		return;
	}

	const bool bAlready = Indicators.ContainsByPredicate([InActor](const FSaytIndicatorEntry& Entry) { return Entry.Actor.Get() == InActor; });
	if (bAlready)
	{
		return;
	}

	FSaytIndicatorEntry NewEntry;
	NewEntry.Actor = InActor;
	NewEntry.DisplayType = InType;
	Indicators.Add(NewEntry);

	// 대상이 파괴되면 자동으로 빠지도록 — 게임 코드가 해제를 깜빡해도 목록이 썩지 않는다
	InActor->OnDestroyed.AddDynamic(this, &USaytIndicatorManagerComponent::HandleTrackedActorDestroyed);

	UE_LOG(LogSaytUI, Verbose, TEXT("[인디케이터] 등록: %s (총 %d)"), *InActor->GetName(), Indicators.Num());
	OnIndicatorAdded.Broadcast(NewEntry);
}

void USaytIndicatorManagerComponent::RemoveIndicator(AActor* InActor)
{
	const int32 Index = Indicators.IndexOfByPredicate(
		[InActor](const FSaytIndicatorEntry& Entry) { return Entry.Actor.Get() == InActor; });
	if (Index == INDEX_NONE)
	{
		return;
	}

	// 목록에서 빼기 전에 복사 — 알림 수신자가 어떤 항목이 빠졌는지 알아야 한다
	const FSaytIndicatorEntry RemovedEntry = Indicators[Index];
	Indicators.RemoveAt(Index);

	if (AActor* Actor = RemovedEntry.Actor.Get())
	{
		Actor->OnDestroyed.RemoveDynamic(this, &USaytIndicatorManagerComponent::HandleTrackedActorDestroyed);
	}

	UE_LOG(LogSaytUI, Verbose, TEXT("[인디케이터] 해제: %s (남은 %d)"), InActor ? *InActor->GetName() : TEXT("(무효)"), Indicators.Num());
	OnIndicatorRemoved.Broadcast(RemovedEntry);
}

void USaytIndicatorManagerComponent::HandleTrackedActorDestroyed(AActor* DestroyedActor)
{
	UE_LOG(LogSaytUI, Verbose, TEXT("[인디케이터] 대상 파괴 감지 → 자동 해제"));
	RemoveIndicator(DestroyedActor);
}

void USaytIndicatorManagerComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// 남은 구독을 정리하지 않으면 다음 PIE 세션까지 끌고 가는 사고가 난다
	for (const FSaytIndicatorEntry& Entry : Indicators)
	{
		if (AActor* Actor = Entry.Actor.Get())
		{
			Actor->OnDestroyed.RemoveDynamic(this, &USaytIndicatorManagerComponent::HandleTrackedActorDestroyed);
		}
	}
	Indicators.Empty();

	Super::EndPlay(EndPlayReason);
}