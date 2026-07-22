// Copyright (c) 2026 Sungmin JI. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SLeafWidget.h"
#include "Types/SlateAttribute.h"   // TSlateAttribute
#include "Styling/SlateBrush.h"

// Stage 2-2 결과물: 보스 세그먼트 구슬 트레이 (Leaf).
// 이 위젯은 '체력'을 모름 — 구슬 슬롯 수와 채워진 수만 앎.
// Health→구슬 매핑은 FSaytSegmentedHealth(위젯 밖) 소관.
//
// OrbCount 의미 주의: 매핑의 RemainingOrbs는 '현재 진행 중인 세그먼트를
// 제외한' 여분이므로 최대값이 SegmentCount-1. 따라서 호출부는
// OrbCount = SegmentCount - 1로 생성해야 만피에서 전 슬롯이 찬다.
class PROJECTSAYT_API SSaytOrbTray : public SLeafWidget
{
	SLATE_DECLARE_WIDGET(SSaytOrbTray, SLeafWidget)
	
public:
	SLATE_BEGIN_ARGS(SSaytOrbTray)
		: _OrbCount(2)
		, _OrbDiameter(18.f)
		, _OrbSpacing(10.f)
		, _FilledBrushOverride(nullptr)   // 미지정 시 스타일 등록소 조회
		, _EmptyBrushOverride(nullptr)
		{
		}
		/** 구슬 슬롯 수 — 생성 시 고정. DesiredSize의 유일한 변수 */
		SLATE_ARGUMENT(int32, OrbCount)
		SLATE_ARGUMENT(float, OrbDiameter)
		SLATE_ARGUMENT(float, OrbSpacing)
		SLATE_ARGUMENT(const FSlateBrush*, FilledBrushOverride)
		SLATE_ARGUMENT(const FSlateBrush*, EmptyBrushOverride)
	SLATE_END_ARGS()

	SSaytOrbTray();   // TSlateAttribute 멤버가 생성자 초기화를 요구 (SSaytHealthBar와 동일 사정)
	void Construct(const FArguments& InArgs);
	
	/** 채워진 구슬 수 갱신. 값이 실제로 바뀔 때만 Paint Invalidation 발생 */
	void SetRemainingOrbs(int32 InRemaining);
	
	// ── SLeafWidget 계약 ──
	virtual int32 OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry,
		const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements,
		int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override;
	virtual FVector2D ComputeDesiredSize(float LayoutScaleMultiplier) const override;
	
private:
	int32 OrbCount = 2;
	float OrbDiameter = 18.f;
	float OrbSpacing = 10.f;
	
	// 색이 아니라 '그리기 명세' 두 벌을 통째로 보관 — 2-4에서 이 두 벌이
	// 스타일(FSaytBossBarStyle)로 이사하면 텍스처 브러시로도 교체 가능
	FSlateBrush FilledBrush;
	FSlateBrush EmptyBrush;

	TSlateAttribute<int32> RemainingOrbsAttribute;
	
	// ── 파괴 팝 연출: 스케일↑ + 알파↓, 0.35초 (ActiveTimer 구동) ──
	// FCurveSequence가 정석 도구지만 Stage 4(데미지 폰트) 커리큘럼 소재라
	// 여기선 Stage 1에서 배운 ActiveTimer + 수동 보간으로 처리 (의도된 선택)
	float PopDuration = 0.35f;
	float PopElapsed = 0.f;
	int32 PopIndexBegin = INDEX_NONE;   // [Begin, End) 슬롯이 팝 재생 중
	int32 PopIndexEnd = INDEX_NONE;
	TSharedPtr<FActiveTimerHandle> PopTimerHandle;
	EActiveTimerReturnType PopTick(double InCurrentTime, float InDeltaTime);
};
