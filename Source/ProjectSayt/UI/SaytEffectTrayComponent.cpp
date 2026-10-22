// Copyright (c) 2026 Sungmin JI. All Rights Reserved.

#include "UI/SaytEffectTrayComponent.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "GameplayEffect.h"
#include "SaytLogChannels.h"

USaytEffectTrayComponent::USaytEffectTrayComponent()
{
	// 이 컴포넌트는 통지 구동이다 — 매 프레임 할 일이 없다.
	PrimaryComponentTick.bCanEverTick = false;
}

USaytEffectTrayComponent* USaytEffectTrayComponent::FindOrAdd(AActor* InOwner)
{
	if (!InOwner)
	{
		return nullptr;
	}

	if (USaytEffectTrayComponent* Existing = InOwner->FindComponentByClass<USaytEffectTrayComponent>())
	{
		return Existing;
	}

	// RegisterComponent를 빼면 엔진이 모르는 객체가 되어 BeginPlay가 불리지 않는다.
	USaytEffectTrayComponent* NewComp = NewObject<USaytEffectTrayComponent>(InOwner);
	NewComp->RegisterComponent();

	UE_LOG(LogSaytUI, Verbose, TEXT("[트레이] 컴포넌트 생성 및 등록 (%s)"), *InOwner->GetName());
	return NewComp;
}

UAbilitySystemComponent* USaytEffectTrayComponent::ResolveOwnerASC() const
{
	// 캐릭터 클래스명에 의존하지 않는 표준 경로 (IAbilitySystemInterface 경유).
	return UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(GetOwner());
}

// ─────────────────────────────────────────────────────────────
// 필터 + 표시 데이터 획득 (A-2)
// ─────────────────────────────────────────────────────────────

const USaytGameplayEffectUIData* USaytEffectTrayComponent::FindTrayUIData(const UGameplayEffect* Def)
{
	// 우리 파생 타입으로 직접 질의한다 — 통과 판정과 데이터 획득이 한 번에 끝난다.
	// 기반 타입으로 질의 후 캐스트하는 2단 구조는 _TextOnly가 붙은 경우
	// '필터는 통과하는데 데이터는 못 읽는' 틈을 만든다.
	return Def ? Def->FindComponent<USaytGameplayEffectUIData>() : nullptr;
}

// ─────────────────────────────────────────────────────────────
// 생명주기
// ─────────────────────────────────────────────────────────────

void USaytEffectTrayComponent::BeginPlay()
{
	Super::BeginPlay();

	UAbilitySystemComponent* ASC = ResolveOwnerASC();
	if (!ASC)
	{
		UE_LOG(LogSaytUI, Warning, TEXT("[트레이] 소유자 ASC를 찾지 못해 구독하지 못했습니다 (%s)"),
			*GetNameSafe(GetOwner()));
		return;
	}

	SubscribeToASC(ASC);
}

void USaytEffectTrayComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	UnsubscribeFromASC();
	Super::EndPlay(EndPlayReason);
}

// ─────────────────────────────────────────────────────────────
// 목록 층 구독 (A-3)
// ─────────────────────────────────────────────────────────────

void USaytEffectTrayComponent::SubscribeToASC(UAbilitySystemComponent* ASC)
{
	// 재진입 방지 — 이미 구독 중이면 먼저 대칭 해제한다.
	UnsubscribeFromASC();

	AddedHandle = ASC->OnActiveGameplayEffectAddedDelegateToSelf.AddUObject(
		this, &USaytEffectTrayComponent::HandleEffectAdded);
	RemovedHandle = ASC->OnAnyGameplayEffectRemovedDelegate().AddUObject(
		this, &USaytEffectTrayComponent::HandleEffectRemoved);
	WatchedASC = ASC;

	// 순서 주의: 구독을 먼저 걸고 목록을 읽는다. 반대로 하면 그 틈에 추가된 효과가
	// 통지도 초기 순회도 없이 영구 누락된다.
	//
	// 델리게이트는 구독 이후의 변화만 통지하므로, 이미 걸려 있는 효과를 항목 층에
	// 따라잡아야 한다. 냉기가 스택 4로 미리 걸려 있는 상태로 위젯을 열어도 스택
	// 통지를 받으려면 지금 구독해 둬야 한다.
	for (const FActiveGameplayEffectHandle& Handle : ASC->GetActiveEffects(FGameplayEffectQuery()))
	{
		if (FindTrayUIData(ASC->GetGameplayEffectCDO(Handle)))
		{
			SubscribeToEffect(ASC, Handle);
		}
	}

	UE_LOG(LogSaytUI, Log, TEXT("[트레이] ASC 구독 — 현재 목록 따라잡기"));
	RebuildEntries();
}

void USaytEffectTrayComponent::UnsubscribeFromASC()
{
	// 항목 층부터 정리한다. 맵을 수정하므로 키를 먼저 떠서 순회한다.
	TArray<FActiveGameplayEffectHandle> Handles;
	WatchedEffects.GetKeys(Handles);
	for (const FActiveGameplayEffectHandle& Handle : Handles)
	{
		UnsubscribeFromEffect(Handle);
	}
	WatchedEffects.Empty();

	// ASC가 이미 사라졌다면 델리게이트 목록도 함께 사라졌으므로 해제할 대상이 없다.
	if (UAbilitySystemComponent* ASC = WatchedASC.Get())
	{
		ASC->OnActiveGameplayEffectAddedDelegateToSelf.Remove(AddedHandle);
		ASC->OnAnyGameplayEffectRemovedDelegate().Remove(RemovedHandle);
		UE_LOG(LogSaytUI, Log, TEXT("[트레이] ASC 구독 해제"));
	}

	WatchedASC.Reset();
	AddedHandle.Reset();
	RemovedHandle.Reset();
}

void USaytEffectTrayComponent::HandleEffectAdded(UAbilitySystemComponent* Target,
	const FGameplayEffectSpec& SpecApplied,
	FActiveGameplayEffectHandle ActiveHandle)
{
	// 핸들로 컨테이너에 되묻지 않는다 — 통지가 스펙을 이미 건네줬다.
	const USaytGameplayEffectUIData* UIData = FindTrayUIData(SpecApplied.Def);
	if (!UIData)
	{
		return;
	}

	// 목록 층 추가 통지가 항목 층 구독의 트리거다. 표시 대상만 건다.
	SubscribeToEffect(Target, ActiveHandle);
	RebuildEntries();
}

void USaytEffectTrayComponent::HandleEffectRemoved(const FActiveGameplayEffect& EffectRemoved)
{
	// 이 시점에 효과는 이미 컨테이너에서 빠지는 중이라 핸들로는 되찾을 수 없다.
	// 건네받은 참조 안에서 다 읽고 끝낸다 — 이 참조를 보관하면 안 된다.
	const USaytGameplayEffectUIData* UIData = FindTrayUIData(EffectRemoved.Spec.Def);
	if (!UIData)
	{
		return;
	}

	UnsubscribeFromEffect(EffectRemoved.Handle);
	RebuildEntries();
}

// ─────────────────────────────────────────────────────────────
// 항목 층 구독 생명주기 (A-4)
// ─────────────────────────────────────────────────────────────

void USaytEffectTrayComponent::SubscribeToEffect(UAbilitySystemComponent* ASC, FActiveGameplayEffectHandle Handle)
{
	// 초기 따라잡기와 추가 통지가 같은 항목에서 겹칠 수 있다.
	if (WatchedEffects.Contains(Handle))
	{
		return;
	}

	// 묶음 포인터를 한 번 받아 둘에 바인드한다. 개별 접근자를 쓰면 해제 지점이 그만큼 늘어난다.
	FActiveGameplayEffectEvents* Events = ASC->GetActiveEffectEventSet(Handle);
	if (!Events)
	{
		UE_LOG(LogSaytUI, Warning, TEXT("[트레이] 이벤트 묶음 없음 — 항목 구독 실패"));
		return;
	}

	FWatchedEffect& Watched = WatchedEffects.Add(Handle);
	Watched.StackChangedHandle = Events->OnStackChanged.AddUObject(this, &USaytEffectTrayComponent::HandleStackChanged);
	Watched.TimeChangedHandle = Events->OnTimeChanged.AddUObject(this, &USaytEffectTrayComponent::HandleTimeChanged);
}

void USaytEffectTrayComponent::UnsubscribeFromEffect(FActiveGameplayEffectHandle Handle)
{
	const FWatchedEffect* Watched = WatchedEffects.Find(Handle);
	if (!Watched)
	{
		return;
	}

	// 제거 통지 시점에 이벤트 묶음은 대개 이미 소멸해 있다(노트 10절, 5.7 PIE 실측).
	// 그래서 명시적 해제가 필수는 아니지만, 효과가 살아 있는 상태에서 푸는 경로
	// (UnsubscribeFromASC)가 따로 있으므로 null 검사 후 살아 있을 때만 해제한다.
	UAbilitySystemComponent* ASC = Handle.GetOwningAbilitySystemComponent();
	FActiveGameplayEffectEvents* Events = ASC ? ASC->GetActiveEffectEventSet(Handle) : nullptr;
	if (Events)
	{
		Events->OnStackChanged.Remove(Watched->StackChangedHandle);
		Events->OnTimeChanged.Remove(Watched->TimeChangedHandle);
	}

	WatchedEffects.Remove(Handle);
}

void USaytEffectTrayComponent::HandleStackChanged(FActiveGameplayEffectHandle Handle, int32 NewCount, int32 PrevCount)
{
	// 상한 도달 후 재적용이면 이전 값과 새 값이 같은 채로 불린다.
	// 값이 안 바뀌었으면 표시도 안 바뀌므로 재구축을 건너뛴다.
	if (NewCount == PrevCount)
	{
		return;
	}

	RebuildEntries();
}

void USaytEffectTrayComponent::HandleTimeChanged(FActiveGameplayEffectHandle Handle, float NewStartTime, float NewDuration)
{
	// '시간이 흐른다'가 아니라 '지속시간이 새로 설정됐다'는 사건이다(노트 3절).
	// 잔여 시간은 항목에 담지 않으므로 표시 모델 자체는 바뀌지 않는다.
	// 라디얼 와이프는 파트 C에서 ActiveTimer + 조회로 그린다 — 여기서 재구축하지 않는다.
}

// ─────────────────────────────────────────────────────────────
// 표시 모델 재구축 (A-6 핵심)
// ─────────────────────────────────────────────────────────────

void USaytEffectTrayComponent::RebuildEntries()
{
	Entries.Reset();

	UAbilitySystemComponent* ASC = WatchedASC.Get();
	if (!ASC)
	{
		OnTrayChanged.Broadcast();
		return;
	}

	// 같은 GE는 한 항목으로 묶는다. 개수 산출은 GAS에 위임하므로(아래) 여기서는
	// '이 GE를 이미 항목으로 만들었는가'만 추적하면 된다. 순서는 활성 목록 순.
	TSet<const UGameplayEffect*> SeenDefs;

	// 빈 쿼리 = 모든 활성 효과(노트 4절). 즉발 GE는 애초에 여기 없다(노트 5절).
	for (const FActiveGameplayEffectHandle& Handle : ASC->GetActiveEffects(FGameplayEffectQuery()))
	{
		const UGameplayEffect* Def = ASC->GetGameplayEffectCDO(Handle);
		const USaytGameplayEffectUIData* UIData = FindTrayUIData(Def);
		if (!UIData)
		{
			continue;
		}

		// 같은 GE의 두 번째 이후 인스턴스는 건너뛴다. 개수는 GetGameplayEffectCount가
		// 이 GE의 모든 인스턴스를 이미 합산하므로 여기서 셀 필요가 없다.
		if (SeenDefs.Contains(Def))
		{
			continue;
		}
		SeenDefs.Add(Def);

		// 화면에 찍을 최종 숫자를 GAS가 직접 계산해 준다.
		// 비스택형은 활성 인스턴스의 합, 스택형은 유효 스택 수의 합(5.7 헤더 계약).
		// 이 위임으로 GetCurrentStackCount의 스택/비스택 반환값에 의존하지 않는다.
		// 인스티게이터 필터 nullptr = 모든 소스 집계(PIE 검증 항목).
		const int32 Count = ASC->GetGameplayEffectCount(Def->GetClass(), nullptr);

		// 무한 지속이 아니면 라디얼 와이프 대상이다. 센티넬 비교는 엔진과 같은 방식(==).
		const bool bHasDuration = (Def->DurationPolicy == EGameplayEffectDurationType::HasDuration);

		FSaytEffectTrayEntry& NewEntry = Entries.AddDefaulted_GetRef();
		// 위의 SeenDefs 덕분에 GE당 한 항목이므로 이 키는 Entries 안에서 유일하다.
		NewEntry.EffectClass = Def->GetClass();
		// UI Data는 CDO 공유 불변 데이터라 매번 읽어도 같은 값이다(노트 6절).
		// 값 3개를 복사해 담아 Entries가 GAS 타입을 참조하지 않게 한다.
		NewEntry.DisplayName = UIData->DisplayName;
		NewEntry.Icon = UIData->Icon;
		NewEntry.Category = UIData->Category;
		NewEntry.Count = Count;
		NewEntry.bHasDuration = bHasDuration;
		// 대표 핸들은 첫 등장 핸들. 파트 C가 이 핸들로 잔여 시간을 조회한다.
		NewEntry.RepresentativeHandle = Handle;
	}

	// 로그는 표시명만 쓴다 — 내부 GE 이름(Default__..._C)은 노출하지 않는다.
	// 통지마다 자동으로 도는 흐름 추적이라 Verbose — 콘솔 log LogSaytUI Verbose로 점등.
	for (const FSaytEffectTrayEntry& Entry : Entries)
	{
		UE_LOG(LogSaytUI, Verbose, TEXT("[트레이]   %s / %s / 개수 %d / 지속 %s"),
			*Entry.DisplayName.ToString(),
			Entry.Category == ESaytEffectCategory::Buff ? TEXT("버프") : TEXT("디버프"),
			Entry.Count,
			Entry.bHasDuration ? TEXT("있음") : TEXT("없음"));
	}
	UE_LOG(LogSaytUI, Verbose, TEXT("[트레이] 표시 항목 %d개"), Entries.Num());

	OnTrayChanged.Broadcast();
}