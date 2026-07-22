// Copyright (c) 2026 Sungmin JI. All Rights Reserved.


#include "SSaytHealthDisplay.h"
#include "AbilitySystemComponent.h"
#include "SaytLogChannels.h"
#include "AbilitySystem/Attributes/SaytAttributeSet_Combat.h"
#include "SSaytHealthBar.h"
#include "SSaytOrbTray.h"
#include "Widgets/SBoxPanel.h"   // SVerticalBox


void SSaytHealthDisplay::Construct(const FArguments& InArgs)
{
	SegmentCount = FMath::Max(InArgs._SegmentCount, 1);

	TSharedRef<SVerticalBox> Root = SNew(SVerticalBox);
	
	// 슬롯 1: 트레이 — 바 '위' 좌측 정렬 (좌상단 배치 결정, 2-4)
	if (SegmentCount > 1)
	{
		Root->AddSlot()
			.AutoHeight()
			.HAlign(HAlign_Left)
			.Padding(2.f, 0.f, 0.f, 4.f)
			[
				SAssignNew(OrbTray, SSaytOrbTray)
				.OrbCount(SegmentCount - 1)
			];
	}

	// 슬롯 2: 체력바 — 크기는 배치 맥락 소관이라 인자 전달 (Stage 1 계약 승계)
	Root->AddSlot()
		.AutoHeight()
		[
			SAssignNew(HealthBar, SSaytHealthBar)
			.Style(InArgs._BarStyle)
			.DesiredBarSize(InArgs._DesiredBarSize)
		];

	ChildSlot
	[
		Root
	];

	// 이 위젯 자신은 Invalidate를 부르지 않는다 — 갱신은 자식 세터가
	// 각자 판단(값 비교 후 자기 Paint 신고)한다. 조립자는 분배만.
	SetCanTick(false);
}

SSaytHealthDisplay::~SSaytHealthDisplay()
{
	UnbindFromASC();   // 대칭 해제 규율 (SSaytHealthBar와 동일)
}

void SSaytHealthDisplay::BindToASC(UAbilitySystemComponent* InASC)
{
	UnbindFromASC();   // 재바인딩 안전
	if (!InASC) { return; }

	BoundASC = InASC;

	// [Push] 구독 이후의 모든 변화
	HealthChangedHandle = InASC->GetGameplayAttributeValueChangeDelegate(
			USaytAttributeSet_Combat::GetHealthAttribute())
		.AddSP(this, &SSaytHealthDisplay::HandleHealthChanged);

	MaxHealthChangedHandle = InASC->GetGameplayAttributeValueChangeDelegate(
			USaytAttributeSet_Combat::GetMaxHealthAttribute())
		.AddSP(this, &SSaytHealthDisplay::HandleMaxHealthChanged);

	// [Pull] 구독 이전의 세계 따라잡기 — 초기값은 '변화'가 아니므로
	// 파괴 신호 없는 스냅숏으로 분배 (Previous 없는 오버로드)
	CurrentHealth = InASC->GetNumericAttribute(USaytAttributeSet_Combat::GetHealthAttribute());
	MaxHealth     = InASC->GetNumericAttribute(USaytAttributeSet_Combat::GetMaxHealthAttribute());

	LastState = FSaytSegmentedHealth::Calculate(CurrentHealth, MaxHealth, SegmentCount);
	if (HealthBar.IsValid()) { HealthBar->SetPercentDirect(LastState.SegmentPercent); }
	if (OrbTray.IsValid())   { OrbTray->SetRemainingOrbs(LastState.RemainingOrbs); }
}

void SSaytHealthDisplay::UnbindFromASC()
{
	if (UAbilitySystemComponent* ASC = BoundASC.Get())
	{
		if (HealthChangedHandle.IsValid())
		{
			ASC->GetGameplayAttributeValueChangeDelegate(
				USaytAttributeSet_Combat::GetHealthAttribute()).Remove(HealthChangedHandle);
		}
		if (MaxHealthChangedHandle.IsValid())
		{
			ASC->GetGameplayAttributeValueChangeDelegate(
				USaytAttributeSet_Combat::GetMaxHealthAttribute()).Remove(MaxHealthChangedHandle);
		}
	}
	HealthChangedHandle.Reset();
	MaxHealthChangedHandle.Reset();
	BoundASC.Reset();
}

void SSaytHealthDisplay::SetHealthDirect(float InHealth, float InMaxHealth)
{
	CurrentHealth = InHealth;
	MaxHealth = InMaxHealth;
	RecomputeAndDistribute();
}

void SSaytHealthDisplay::HandleHealthChanged(const FOnAttributeChangeData& Data)
{
	if (Data.NewValue == Data.OldValue) { return; }   // 무변화 방송 필터
	CurrentHealth = Data.NewValue;
	RecomputeAndDistribute();
}

void SSaytHealthDisplay::HandleMaxHealthChanged(const FOnAttributeChangeData& Data)
{
	if (Data.NewValue == Data.OldValue) { return; }
	MaxHealth = Data.NewValue;
	RecomputeAndDistribute();
}

void SSaytHealthDisplay::RecomputeAndDistribute()
{
	const FSaytSegmentedHealthState NewState =
		FSaytSegmentedHealth::Calculate(CurrentHealth, MaxHealth, SegmentCount, LastState);

	if (HealthBar.IsValid()) { HealthBar->SetPercentDirect(NewState.SegmentPercent); }
	if (OrbTray.IsValid())   { OrbTray->SetRemainingOrbs(NewState.RemainingOrbs); }

	if (NewState.bSegmentBroken)
	{
		// 2-4 연출 훅 예정지 — 지금은 신호 도착 확인만
		UE_LOG(LogSaytUI, Log, TEXT("[HealthDisplay] 세그먼트 파괴 — 남은 구슬 %d"), NewState.RemainingOrbs);
	}

	LastState = NewState;
}
