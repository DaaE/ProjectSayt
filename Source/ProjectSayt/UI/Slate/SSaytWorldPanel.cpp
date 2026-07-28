// Copyright (c) 2026 Sungmin JI. All Rights Reserved.

#include "SSaytWorldPanel.h"
#include "Engine/GameViewportClient.h"
#include "Engine/LocalPlayer.h"
#include "SaytLogChannels.h"
#include "GameFramework/Actor.h"
#include "GameFramework/PlayerController.h"
#include "Layout/ArrangedChildren.h"
#include "SceneView.h"

// 거리 기반 축소 튜닝 값 — PIE 중 콘솔에서 바로 조절 가능
static TAutoConsoleVariable<float> CVarMobBarNearDistance(
	TEXT("Sayt.MobBar.NearDistance"), 600.f,
	TEXT("이 거리 이내에서는 바가 원래 크기(배율 1.0)"));

static TAutoConsoleVariable<float> CVarMobBarFarDistance(
	TEXT("Sayt.MobBar.FarDistance"), 4000.f,
	TEXT("이 거리 이상에서는 바가 최소 배율"));

static TAutoConsoleVariable<float> CVarMobBarMinScale(
	TEXT("Sayt.MobBar.MinScale"), 0.45f,
	TEXT("먼 거리에서의 최소 배율"));

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
		
		// 정렬 계산은 화면에 실제로 차지할 크기(= 원래 크기 × 배율) 기준이어야 한다
		const FVector2D ScaledSize = ChildSize * CurSlot.CachedScale;
		const FVector2D ChildOffset(LocalPos.X - ScaledSize.X * 0.5f, LocalPos.Y - ScaledSize.Y);
				
		// 배치 변환에 배율을 함께 실으면 자식 내부 크기는 그대로 두고 최종 표시만 축소된다
		ArrangedChildren.AddWidget(AllottedGeometry.MakeChild(Widget, ChildSize, FSlateLayoutTransform(CurSlot.CachedScale, ChildOffset)));
	}
}

int32 SSaytWorldPanel::OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry,
	const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements,
	int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const
{
	// 커스텀 Panel의 표준 그리기 형태: 배치 명단을 만들고 그 명단대로 자식을 그린다
	FArrangedChildren ArrangedChildren(EVisibility::Visible);
	ArrangeChildren(AllottedGeometry, ArrangedChildren);
	
	return PaintArrangedChildren(Args, ArrangedChildren, AllottedGeometry, MyCullingRect,
		OutDrawElements, LayerId, InWidgetStyle, bParentEnabled);
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
	const FVector ViewOrigin = ProjectionData.ViewOrigin;
	
	// 튜닝 값은 매 프레임 한 번만 읽는다 (슬롯마다 읽을 이유가 없다)
	const float NearDistance = CVarMobBarNearDistance.GetValueOnGameThread();
	const float FarDistance = CVarMobBarFarDistance.GetValueOnGameThread();
	const float MinScale = CVarMobBarMinScale.GetValueOnGameThread();

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

		// 카메라에서 대상까지의 거리 → 배율. 가까우면 1.0, 멀면 하한까지 선형 축소
		const float Distance = FVector::Dist(ViewOrigin, TrackedLocation);
		const float Alpha = FMath::GetRangePct(NearDistance, FarDistance, Distance);
		const float NewScale = FMath::Lerp(1.f, MinScale, FMath::Clamp(Alpha, 0.f, 1.f));
		
		if (bInFront != CurSlot.bCachedInFront
			|| !PixelPos.Equals(CurSlot.CachedPixelPosition, 0.01f)
			|| !FMath::IsNearlyEqual(NewScale, CurSlot.CachedScale, 0.001f))
		{
			CurSlot.bCachedInFront = bInFront;
			CurSlot.CachedPixelPosition = PixelPos;
			CurSlot.CachedScale = NewScale;
			bAnyChanged = true;
		}
	}

	// 실제로 달라진 슬롯이 있을 때만 배치 재계산을 요청합니다.
	// 주의: 이 신고가 실제 비용 절감으로 이어지는지는 별개 문제다.
	// 실측(2026-07) 결과, PIE에서는 Slate.EnableGlobalInvalidation을 켜도
	// 뷰포트에 얹힌 이 패널은 매 프레임 Prepass/Arrange/Paint가 모두 돈다.
	// 신고 자체는 (1) 캐싱이 적용되는 환경에서의 정확성 보장 (2) 비용 0
	// 이라는 이유로 유지하되, 실효 여부는 Phase 9 프로파일링에서 재측정할 것.
	if (bAnyChanged)
	{
		Invalidate(EInvalidateWidgetReason::Layout);
	}

	return EActiveTimerReturnType::Continue;
}