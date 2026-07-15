// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SLeafWidget.h"
#include "GameplayEffectTypes.h"        // FOnAttributeChangeData
#include "SayuHealthBarWidgetStyle.h"   // FSayuHealthBarStyle

class UAbilitySystemComponent;

// Stage 1 결과물: GAS Push 델리게이트 기반 플레이어 체력바 (Leaf).
// 이 위젯은 '줄 체력'의 존재를 모름 — 퍼센트 1개와 색 2개만 앎.
// Health→(줄, 줄 내 %, 색 쌍) 매핑은 위젯 밖 게임 로직 소관 (Stage 2/Sayt).
class PROJECTGAS_API SSayuHealthBar : public SLeafWidget
{
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
	
	const FSayuHealthBarStyle* Style = nullptr;
	FVector2D DesiredBarSize = FVector2D(400.f, 32.f);
	
	FSlateColor FillTint;         // 현재 줄 색
	FSlateColor BackgroundTint;   // 다음 줄 색
	
	TWeakObjectPtr<UAbilitySystemComponent> BoundASC;
	FDelegateHandle HealthChangedHandle;
	FDelegateHandle MaxHealthChangedHandle;
	
	float CurrentHealth = 0.f;
	float MaxHealth = 1.f;   // 0 나눗셈 방지 기본값
};
