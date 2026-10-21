// Copyright (c) 2026 Sungmin JI. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ActiveGameplayEffectHandle.h"
#include "AbilitySystem/Effects/SaytGameplayEffectUIData.h"   // ESaytEffectCategory
#include "SaytEffectTrayComponent.generated.h"

class UAbilitySystemComponent;
class UGameplayEffect;
class UTexture2D;
struct FActiveGameplayEffect;
struct FGameplayEffectSpec;

/**
 * 트레이에 그릴 항목 하나. GE 클래스 단위로 묶인 결과물이다.
 *
 * GAS 타입을 값으로 복사해 담는다(대표 핸들 하나만 예외). 이렇게 하면 위젯이
 * 이 배열만 받아 GAS 없이 더미 데이터로 렌더 테스트가 된다 — Stage 2
 * FSaytSegmentedHealth와 같은 형태다.
 *
 * 잔여 시간은 넣지 않는다. 시간 통지는 '갱신 사건'이라 넣는 즉시 낡는다.
 * 라디얼 와이프는 파트 C에서 bHasDuration을 보고 대표 핸들로 직접 조회해 그린다.
 */
USTRUCT()
struct FSaytEffectTrayEntry
{
	GENERATED_BODY()

	/** 툴팁·접근성 텍스트가 쓴다. 로그도 이 이름만 쓴다(내부 GE 이름은 노출하지 않는다). */
	UPROPERTY()
	FText DisplayName;

	/** 파트 C까지 비어 있다. */
	UPROPERTY()
	TObjectPtr<UTexture2D> Icon = nullptr;

	/** 버프/디버프 — 테두리 색만 가른다. */
	UPROPERTY()
	ESaytEffectCategory Category = ESaytEffectCategory::Buff;

	/** 화면에 찍을 숫자. GetGameplayEffectCount가 스택/인스턴스를 합산해 준 최종값. */
	UPROPERTY()
	int32 Count = 0;

	/** 라디얼 와이프 대상 여부. 잔여 시간 자체는 담지 않는다. */
	UPROPERTY()
	bool bHasDuration = false;

	/**
	 * 파트 C에서 이 핸들로 잔여 시간을 조회한다. 그룹 대표(첫 등장) 하나만 보관.
	 * UPROPERTY를 붙이지 않는다 — 핸들은 로컬 ASC 컨테이너의 인덱스라 복제해도
	 * 의미가 없고, UObject 강참조도 아니라 GC 대상이 아니다.
	 */
	FActiveGameplayEffectHandle RepresentativeHandle;
};

/**
 * 버프/디버프 트레이의 표시 모델 (Phase 8 Stage 3 파트 A).
 *
 * A-2~A-5가 SaytDebugCommands.cpp의 파일 스코프 정적 함수로 흩어져 있던 것을
 * 클래스 하나로 모으고, 로그로 흘려보내던 것을 상태(표시 항목 배열)로 붙든다.
 * 게임 코드는 GE를 적용할 뿐이고, UI는 이 컴포넌트의 방송만 구독한다.
 *
 * 플레이어 캐릭터 소속인 이유: 구독 대상 ASC가 그 캐릭터의 것이고, 표시는
 * 로컬 플레이어의 화면에 붙는 것이라 월드에 하나뿐인 Subsystem이 아니라
 * 시점 소유자에 붙는 것이 맞다. Stage 2 USaytIndicatorManagerComponent와 일관.
 */
UCLASS()
class PROJECTSAYT_API USaytEffectTrayComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	USaytEffectTrayComponent();

	/** 없으면 만들어 붙인다 — 검증 커맨드가 생성 시점을 신경 쓰지 않도록. */
	static USaytEffectTrayComponent* FindOrAdd(AActor* InOwner);

	/** 늦게 참여한 구독자(위젯)가 현재 상태를 따라잡는 경로. */
	const TArray<FSaytEffectTrayEntry>& GetEntries() const { return Entries; }

	/** 표시 항목 목록이 바뀔 때마다 1회 방송. 통지 종류를 가리지 않는다. */
	DECLARE_MULTICAST_DELEGATE(FOnEffectTrayChanged);
	FOnEffectTrayChanged OnTrayChanged;

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	// ── 소유자 ASC 조회 ──
	UAbilitySystemComponent* ResolveOwnerASC() const;

	// ── 필터 + 표시 데이터 획득 (A-2) ──
	/** 트레이 표시 자격 판정과 표시 데이터 획득이 한 번에 끝난다. 자격 없으면 nullptr. */
	static const USaytGameplayEffectUIData* FindTrayUIData(const UGameplayEffect* Def);

	// ── 목록 층 구독 (A-3) ──
	void SubscribeToASC(UAbilitySystemComponent* ASC);
	void UnsubscribeFromASC();

	void HandleEffectAdded(UAbilitySystemComponent* Target,
		const FGameplayEffectSpec& SpecApplied,
		FActiveGameplayEffectHandle ActiveHandle);
	void HandleEffectRemoved(const FActiveGameplayEffect& EffectRemoved);

	// ── 항목 층 구독 생명주기 (A-4) ──
	void SubscribeToEffect(UAbilitySystemComponent* ASC, FActiveGameplayEffectHandle Handle);
	void UnsubscribeFromEffect(FActiveGameplayEffectHandle Handle);

	void HandleStackChanged(FActiveGameplayEffectHandle Handle, int32 NewCount, int32 PrevCount);
	void HandleTimeChanged(FActiveGameplayEffectHandle Handle, float NewStartTime, float NewDuration);

	// ── 표시 모델 재구축 (A-6 핵심) ──
	/**
	 * 통지가 무엇이든 목록 전체를 다시 순회해 GE 클래스로 묶고 1회 방송한다.
	 * 화면에 찍을 개수는 GetGameplayEffectCount에 위임한다 — 비스택형은 인스턴스 합,
	 * 스택형은 스택 수 합을 GAS가 직접 돌려주므로 두 경로를 손으로 합치지 않는다.
	 */
	void RebuildEntries();

private:
	// 항목 하나당 델리게이트 핸들 둘. 걸 때와 풀 때가 대칭이어야 한다.
	struct FWatchedEffect
	{
		FDelegateHandle StackChangedHandle;
		FDelegateHandle TimeChangedHandle;
	};

	/** 구독 대상 ASC. 컴포넌트가 ASC보다 오래 살 수 있어 약참조. */
	TWeakObjectPtr<UAbilitySystemComponent> WatchedASC;

	FDelegateHandle AddedHandle;
	FDelegateHandle RemovedHandle;

	/** 항목 층 구독 중인 효과들. 핸들 → 그 효과에 건 델리게이트 핸들 쌍. */
	TMap<FActiveGameplayEffectHandle, FWatchedEffect> WatchedEffects;

	/** 위젯에 넘길 표시 모델. RebuildEntries가 통째로 다시 채운다. */
	TArray<FSaytEffectTrayEntry> Entries;
};