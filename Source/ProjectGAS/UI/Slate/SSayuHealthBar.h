// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SLeafWidget.h"
#include "Types/SlateAttribute.h"      // TSlateAttribute
#include "Types/WidgetActiveTimerDelegate.h"   // FWidgetActiveTimerDelegate, EActiveTimerReturnType
#include "GameplayEffectTypes.h"        // FOnAttributeChangeData
#include "SayuHealthBarWidgetStyle.h"   // FSayuHealthBarStyle

class UAbilitySystemComponent;

// Stage 1 결과물: GAS Push 델리게이트 기반 플레이어 체력바 (Leaf).
// 이 위젯은 '줄 체력'의 존재를 모름 — 퍼센트 1개와 색 2개만 앎.
// Health→(줄, 줄 내 %, 색 쌍) 매핑은 위젯 밖 게임 로직 소관 (Stage 2/Sayt).
class PROJECTGAS_API SSayuHealthBar : public SLeafWidget
{
	SLATE_DECLARE_WIDGET(SSayuHealthBar, SLeafWidget)
	
public:
	SLATE_BEGIN_ARGS(SSayuHealthBar)
		: _Style(nullptr)
		, _DesiredBarSize(400.f, 32.f)
		{
		}
		/** 비주얼 묶음. 미지정 시 FSayuStyle의 "Sayu.HealthBar"로 폴백 */
		SLATE_STYLE_ARGUMENT(FSayuHealthBarStyle, Style)
		/** 크기는 룩이 아니라 배치 맥락(HUD 대형 바 vs 머리 위 소형 바)의 소관이라 스타일이 아닌 인자 */
		SLATE_ARGUMENT(FVector2D, DesiredBarSize)

	SLATE_END_ARGS()

	/** Constructs this widget with InArgs */
	SSayuHealthBar();   // TSlateAttribute 멤버가 생성자 초기화를 요구함 (아래 cpp 참고)
	void Construct(const FArguments& InArgs);
	virtual ~SSayuHealthBar() override;

	// ── 데이터 입구 ──
	void BindToASC(UAbilitySystemComponent* InASC);
	void UnbindFromASC();   // 공개인 이유: Stage 2 재활용(다른 대상으로 재바인딩)의 전제

	// ── 줄 체력용 런타임 색 (3b에서 TSlateAttribute로 전환 예정) ──
	void SetFillTint(const FSlateColor& InTint);
	void SetBackgroundTint(const FSlateColor& InTint);
	
	// ── SLeafWidget 계약 ──
	virtual int32 OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry,
		const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements,
		int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override;
	virtual FVector2D ComputeDesiredSize(float LayoutScaleMultiplier) const override;
	
private:
	void HandleHealthChanged(const FOnAttributeChangeData& Data);
	void HandleMaxHealthChanged(const FOnAttributeChangeData& Data);
	float GetHealthPercent() const;   // 비율 계산의 단일 지점 — Stage 2가 재사용할 이음새
	
	// ── 고스트 바 (피격 잔상) ──
	void SyncGhostToPercent();   // 비율 변화 후 잔상 방침 결정: 감소면 감쇠 시작, 증가면 즉시 동행
	void StartGhostDrain();
	EActiveTimerReturnType GhostTick(double InCurrentTime, float InDeltaTime);
	
	const FSayuHealthBarStyle* Style = nullptr;
	FVector2D DesiredBarSize = FVector2D(400.f, 32.f);
	
	TSlateAttribute<FSlateColor> FillTintAttribute;         // 현재 줄 색
	TSlateAttribute<FSlateColor> BackgroundTintAttribute;   // 다음 줄 색
	
	float GhostPercent = 0.f;           // 잔상이 현재 가리키는 비율
	float GhostDelayRemaining = 0.f;    // 감쇠 시작까지 남은 대기 시간
	TSharedPtr<FActiveTimerHandle> GhostTimerHandle;   // 중복 등록 방지 + 명시 해제용 손잡이
	
	TWeakObjectPtr<UAbilitySystemComponent> BoundASC;
	FDelegateHandle HealthChangedHandle;
	FDelegateHandle MaxHealthChangedHandle;
	
	float CurrentHealth = 0.f;
	float MaxHealth = 1.f;   // 0 나눗셈 방지 기본값
};
