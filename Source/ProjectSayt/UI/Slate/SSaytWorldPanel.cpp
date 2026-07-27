// Copyright (c) 2026 Sungmin JI. All Rights Reserved.

#include "SSaytWorldPanel.h"
#include "Engine/GameViewportClient.h"
#include "Engine/LocalPlayer.h"
#include "GameFramework/Actor.h"
#include "GameFramework/PlayerController.h"
#include "Layout/ArrangedChildren.h"
#include "SceneView.h"

void SSaytWorldPanel::FSlot::Construct(const FChildren& SlotOwner, FSlotArguments&& InArgs)
{
	TSlotBase<FSlot>::Construct(SlotOwner, MoveTemp(InArgs));
	TrackedActor = InArgs._TrackedActor;
}

SSaytWorldPanel::SSaytWorldPanel() : Children(this)
{
	SetCanTick(false);
	// 게임 화면을 덮는 오버레이가 클릭을 가로채면 안 됨 —
	// 패널 자신은 히트테스트 투명, 자식은 각자 판단
	SetVisibility(EVisibility::SelfHitTestInvisible);
}

void SSaytWorldPanel::Construct(const FArguments& InArgs, APlayerController* InPlayerController)
{
	PlayerController = InPlayerController;
	HeadMargin = InArgs._HeadMargin;
}

SSaytWorldPanel::FScopedWidgetSlotArguments SSaytWorldPanel::AddSlot()
{
	EnsureUpdateTimer();
	return FScopedWidgetSlotArguments{ MakeUnique<FSlot>(), Children, INDEX_NONE };
}

int32 SSaytWorldPanel::RemoveSlot(const TSharedRef<SWidget>& SlotWidget)
{
	return Children.Remove(SlotWidget);
}

int32 SSaytWorldPanel::RemoveSlotForActor(const TWeakObjectPtr<AActor>& InActor)
{
	for (int32 Index = 0; Index < Children.Num(); ++Index)
	{
		// 약참조끼리의 비교는 대상이 이미 파괴된 뒤에도 동일 객체를 정확히 식별한다
		if (Children[Index].TrackedActor == InActor)
		{
			Children.RemoveAt(Index);
			return Index;
		}
	}
	return INDEX_NONE;
}

void SSaytWorldPanel::ClearChildren()
{
	Children.Empty();
}

FVector2D SSaytWorldPanel::ComputeDesiredSize(float) const
{
	// Measure 패스: "얼마나 크고 싶니?"에 대한 이 패널의 답은 "주장 없음".
	// 뷰포트 전체를 오버레이로 받는 전제라, 스스로 크기를 요구하지 않습니다.
	return FVector2D::ZeroVector;
}

void SSaytWorldPanel::OnArrangeChildren(const FGeometry& AllottedGeometry, FArrangedChildren& ArrangedChildren) const
{
	// Arrange 패스: 부모가 받은 영역(AllottedGeometry) 안에서
	// 자식마다 위치+크기를 "선언"해 명단(ArrangedChildren)에 올립니다.
	// 명단에 안 올라간 자식은 이 프레임에 존재하지 않는 것과 같습니다(그리기/입력 모두).
	for (int32 Index = 0; Index < Children.Num(); ++Index)
	{
		const FSlot& CurSlot = Children[Index];
		const TSharedRef<SWidget>& Widget = CurSlot.GetWidget();

		if (!ArrangedChildren.Accepts(Widget->GetVisibility()))
		{
			continue;
		}

		// 2-7 개편: 여기서는 외부 세계(카메라)를 읽지 않습니다.
		// Slot에 캐시된 값 → 기하 번역만 수행 (입력이 같으면 결과도 같은 순수 계산)
		if (!CurSlot.bCachedInFront)
		{
			continue; // 카메라 뒤 → 명단 제외 (2-5 문답 Q1의 '유령 체력바' 방지 분기)
		}

		// 2-5 예고분: 투영 결과(뷰포트 픽셀) → Slate 로컬 단위.
		// Geometry의 누적 레이아웃 변환에서 스케일을 조회 (.Scale 멤버는 legacy)
		const FVector2D LocalPos = CurSlot.CachedPixelPosition / AllottedGeometry.GetAccumulatedLayoutTransform().GetScale();
		
		// 바 가로 중앙 + 기준점 아래 정렬 (머리 위치가 바의 '발밑'이 되도록)
		const FVector2D ChildSize = Widget->GetDesiredSize();
		const FVector2D ChildOffset(LocalPos.X - ChildSize.X * 0.5f, LocalPos.Y - ChildSize.Y);

		// 자식 배치 = "이 크기로, 이 변환(평행이동)을 부여한다" — 개편 후 FGeometry의 본래 모델
		ArrangedChildren.AddWidget(AllottedGeometry.MakeChild(Widget, ChildSize, FSlateLayoutTransform(ChildOffset)));
	}
}

FChildren* SSaytWorldPanel::GetChildren()
{
	return &Children;
}

void SSaytWorldPanel::EnsureUpdateTimer()
{
	if (!UpdateTimerHandle.IsValid())
	{
		// 주기 0 = 매 프레임. 필요 시 여기만 0.033f 등으로 바꾸면 30Hz 갱신으로 조절됩니다.
		UpdateTimerHandle = RegisterActiveTimer(0.f,FWidgetActiveTimerDelegate::CreateSP(this, &SSaytWorldPanel::UpdateProjections));
	}
}

EActiveTimerReturnType SSaytWorldPanel::UpdateProjections(double InCurrentTime, float InDeltaTime)
{
	// 자동 소등: 추적 대상이 없으면 타이머 자체를 내립니다 (빈 패널 = 완전 휴면)
	if (Children.Num() == 0)
	{
		UpdateTimerHandle.Reset();
		return EActiveTimerReturnType::Stop;
	}

	APlayerController* PC = PlayerController.Get();
	if (!PC)
	{
		return EActiveTimerReturnType::Continue;
	}

	ULocalPlayer* LocalPlayer = PC->GetLocalPlayer();
	if (!LocalPlayer || !LocalPlayer->ViewportClient)
	{
		return EActiveTimerReturnType::Continue;
	}

	// ── 2-5 문답에서 다뤘던 loop-invariant hoisting의 실전 적용 ──
	// (반복 전체에서 변하지 않는 계산은 반복 밖에서 1회만 — Lyra SActorCanvas와 같은 구조)
	// 투영 재료(뷰·투영 행렬, 뷰 사각형)를 프레임당 1회만 조립하고,
	// 아래 루프에서는 행렬 곱만 N번 수행합니다.
	FSceneViewProjectionData ProjectionData;
	if (!LocalPlayer->GetProjectionData(LocalPlayer->ViewportClient->Viewport, ProjectionData))
	{
		return EActiveTimerReturnType::Continue;
	}

	const FMatrix ViewProjectionMatrix = ProjectionData.ComputeViewProjectionMatrix();
	const FIntRect ViewRect = ProjectionData.GetConstrainedViewRect();

	bool bAnyChanged = false;

	for (int32 Index = 0; Index < Children.Num(); ++Index)
	{
		FSlot& CurSlot = Children[Index];
		
		// 대상이 사라졌으면 숨김 처리만 하고 넘어간다 (슬롯 제거는 소유자 책임)
		AActor* TargetActor = CurSlot.TrackedActor.Get();
		if (!TargetActor)
		{
			if (CurSlot.bCachedInFront)
			{
				CurSlot.bCachedInFront = false;
				bAnyChanged = true;
			}
			
			continue;
		}
		
		// 추적 지점 = 액터 충돌 상단 + 여유. 이 규칙이 이 패널의 정체다
		const FVector TrackedLocation = TargetActor->GetActorLocation()	+ FVector(0.f, 0.f, TargetActor->GetSimpleCollisionHalfHeight() + HeadMargin);

		FVector2D PixelPos = FVector2D::ZeroVector;
		const bool bInFront = FSceneView::ProjectWorldToScreen(TrackedLocation, ViewRect, ViewProjectionMatrix, PixelPos);

		if (bInFront != CurSlot.bCachedInFront || !PixelPos.Equals(CurSlot.CachedPixelPosition, 0.01f))
		{
			CurSlot.bCachedInFront = bInFront;
			CurSlot.CachedPixelPosition = PixelPos;
			bAnyChanged = true;
		}
	}

	// 실제로 달라진 슬롯이 있을 때만 배치 재계산을 요청합니다.
	// 카메라·몬스터가 모두 정지해 있으면 이 요청이 발생하지 않으므로,
	// invalidation 캐싱 아래에서 '정지 화면 = Arrange 생략'이라는 진짜 휴면이 성립합니다.
	if (bAnyChanged)
	{
		Invalidate(EInvalidateWidgetReason::Layout);
	}

	return EActiveTimerReturnType::Continue;
}