// Copyright (c) 2026 Sungmin JI. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "UI/SaytHealthDisplayTypes.h"
#include "SaytIndicatorManagerComponent.generated.h"

class APlayerController;

/** 화면에 표시할 대상 하나. 액터는 약참조 — 대상이 사라지면 스스로 무효가 된다 */
struct FSaytIndicatorEntry
{
	TWeakObjectPtr<AActor> Actor;
	ESaytHealthDisplayType DisplayType = ESaytHealthDisplayType::Mob;
};

/**
 * 화면 표시 대상 목록의 소유자 (Phase 8 Stage 2 파트 B).
 * 게임 코드는 여기에 '이 액터를 표시해 달라'고만 말하고 위젯을 모른다.
 * UI는 이 목록의 변경 알림만 구독하고 몬스터를 모른다.
 *
 * PlayerController 소속인 이유: 화면 표시는 로컬 플레이어마다 다른 것이라
 * 월드에 하나뿐인 Subsystem이 아니라 시점 소유자에 붙는 것이 맞다.
 */
UCLASS()
class PROJECTSAYT_API USaytIndicatorManagerComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	/** 없으면 만들어서 붙인다 — 호출부가 생성 시점을 신경 쓰지 않도록 */
	static USaytIndicatorManagerComponent* FindOrAdd(APlayerController* InPC);

	void AddIndicator(AActor* InActor, ESaytHealthDisplayType InType);
	void RemoveIndicator(AActor* InActor);

	/** 늦게 참여한 구독자가 현재 상태를 따라잡는 경로 */
	const TArray<FSaytIndicatorEntry>& GetIndicators() const { return Indicators; }

	// 우리가 직접 선언하는 알림 채널 (구독자 여럿 가능)
	DECLARE_MULTICAST_DELEGATE_OneParam(FOnIndicatorChanged, const FSaytIndicatorEntry&);
	FOnIndicatorChanged OnIndicatorAdded;
	FOnIndicatorChanged OnIndicatorRemoved;

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	/** AActor::OnDestroyed는 dynamic 델리게이트라 UFUNCTION 필수 */
	UFUNCTION()
	void HandleTrackedActorDestroyed(AActor* DestroyedActor);

	TArray<FSaytIndicatorEntry> Indicators;
};