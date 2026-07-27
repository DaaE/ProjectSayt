// Copyright (c) 2026 Sungmin JI. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SPanel.h"
#include "Layout/Children.h"

class APlayerController;

/**
 * 월드 액터 추적 오버레이 패널 (Phase 8 Stage 2 파트 B)
 * - Panel Role: 배치 규칙("Slot의 월드 위치를 투영한 화면 지점") 자체가 이 클래스의 존재 이유
 * - 배치만 담당 — ASC 구독은 자식(SSaytHealthDisplay), 액터 등록/해제는 2-8 소관
 */
class PROJECTSAYT_API SSaytWorldPanel : public SPanel
{
public:
	// ── 이론의 실체화 ①: "자식의 위치는 자식이 아니라 부모의 Slot에 산다" ──
	// Unity라면 RectTransform으로 자식 자신에게 붙어 있었을 데이터(월드 위치)가
	// 여기, 부모가 자식마다 하나씩 들고 있는 메타데이터 주머니에 들어갑니다.
	class FSlot : public TSlotBase<FSlot>
	{
	public:
		SLATE_SLOT_BEGIN_ARGS(FSlot, TSlotBase<FSlot>)
			SLATE_ARGUMENT(TWeakObjectPtr<AActor>, TrackedActor)
		SLATE_SLOT_END_ARGS()
		
		void Construct(const FChildren& SlotOwner, FSlotArguments&& InArgs);
		
		/** 추적 대상. 약참조 — 대상이 사라져도 안전하게 무효로 감지된다 */
		TWeakObjectPtr<AActor> TrackedActor;
		
		// 갱신 단계가 기록하고, OnArrangeChildren이 읽는 캐시
		FVector2D CachedPixelPosition = FVector2D::ZeroVector;
		bool bCachedInFront = false;
	};
	
	SLATE_BEGIN_ARGS(SSaytWorldPanel)
		: _HeadMargin(30.f)
		{
		}
		/** 액터 충돌 상단에서 위로 띄울 여유 (월드 단위) */
		SLATE_ARGUMENT(float, HeadMargin)
	SLATE_END_ARGS()
	
	SSaytWorldPanel();

	/** PC는 투영(월드→스크린)의 시점 소유자 — 패널이 약참조로 보관 */
	void Construct(const FArguments& InArgs, APlayerController* InPlayerController);
	
	using FScopedWidgetSlotArguments = TPanelChildren<FSlot>::FScopedWidgetSlotArguments;
	FScopedWidgetSlotArguments AddSlot();
	int32 RemoveSlot(const TSharedRef<SWidget>& SlotWidget);
	/** 추적 액터로 슬롯 제거. 이미 파괴된 액터도 약참조 비교로 정확히 찾는다 */
	int32 RemoveSlotForActor(const TWeakObjectPtr<AActor>& InActor);
	void ClearChildren();
	
	// ── 이론의 실체화 ②: 2패스 레이아웃의 두 축 ──
	virtual FVector2D ComputeDesiredSize(float) const override;              // Measure 패스 (상향식)
	virtual void OnArrangeChildren(const FGeometry& AllottedGeometry,
								   FArrangedChildren& ArrangedChildren) const override; // Arrange 패스 (하향식)
	virtual FChildren* GetChildren() override;
	
private:
	/** 갱신 타이머가 없으면 등록 (첫 자식 추가 시점에 점등) */
	void EnsureUpdateTimer();
	
	/** 카메라 표본을 뜨고 Slot 캐시를 갱신 — 변화가 있을 때만 Layout invalidation 발생 */
	EActiveTimerReturnType UpdateProjections(double InCurrentTime, float InDeltaTime);
		
	TPanelChildren<FSlot> Children;
	TWeakObjectPtr<APlayerController> PlayerController;
	TSharedPtr<FActiveTimerHandle> UpdateTimerHandle;
	float HeadMargin = 30.f;
};
