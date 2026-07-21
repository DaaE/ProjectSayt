// Copyright (c) 2026 Sungmin JI. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Styling/ISlateStyle.h"            // SLATE_STYLE_ARGUMENT 전개에 필요
#include "Styling/SlateWidgetStyleAsset.h"  // USlateWidgetStyleAsset (상동)
#include "GameplayEffectTypes.h"        // FOnAttributeChangeData
#include "UI/SaytSegmentedHealth.h"
#include "UI/Slate/SaytHealthBarWidgetStyle.h"   // SLATE_STYLE_ARGUMENT 전개용

class UAbilitySystemComponent;
class SSaytHealthBar;
class SSaytOrbTray;

// Stage 2-3 결과물: 범용 체력 표시 단위 (Compound).
// 데이터(SegmentCount)가 형태를 결정 — 1이면 단일 바(플레이어/일반몹),
// N이면 바 + 구슬 N-1 슬롯(세그먼트 보스).
// 책임은 'ASC 구독 + 매핑 + 분배'가 전부이고 그리기는 자식이 담당한다.
// 새 표시 요소가 필요해지면 여기에 그리기를 넣지 말고 부품 위젯을 만들어 꽂을 것.
class PROJECTSAYT_API SSaytHealthDisplay : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SSaytHealthDisplay)
		: _SegmentCount(1)
		, _BarStyle(nullptr)
		, _DesiredBarSize(400.f, 32.f)
		{
		}
		/** 세그먼트 수. 1 = 트레이 미생성, N = 구슬 N-1 슬롯 생성 */
		SLATE_ARGUMENT(int32, SegmentCount)
		/** 자식 바에 전달할 스타일 — 보스/몹 룩 프리셋의 분리 지점 (2-4에서 확장) */
		SLATE_STYLE_ARGUMENT(FSaytHealthBarStyle, BarStyle)
		SLATE_ARGUMENT(FVector2D, DesiredBarSize)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);
	virtual ~SSaytHealthDisplay() override;
	
	// ── 데이터 입구 ──
	void BindToASC(UAbilitySystemComponent* InASC);
	void UnbindFromASC();
	/** ASC 없는 데이터 소스용 직접 주입 (검증 커맨드가 첫 고객) */
	void SetHealthDirect(float InHealth, float InMaxHealth);
	
private:
	void HandleHealthChanged(const FOnAttributeChangeData& Data);
	void HandleMaxHealthChanged(const FOnAttributeChangeData& Data);
	void RecomputeAndDistribute();
	
	int32 SegmentCount = 1;

	TSharedPtr<SSaytHealthBar> HealthBar;
	TSharedPtr<SSaytOrbTray> OrbTray;   // SegmentCount 1이면 미생성 (nullptr)

	FSaytSegmentedHealthState LastState;

	TWeakObjectPtr<UAbilitySystemComponent> BoundASC;
	FDelegateHandle HealthChangedHandle;
	FDelegateHandle MaxHealthChangedHandle;

	float CurrentHealth = 0.f;
	float MaxHealth = 1.f;
};
