// Copyright (c) 2026 Sungmin JI. All Rights Reserved.


#include "SSaytOrbTray.h"
#include "Brushes/SlateRoundedBoxBrush.h"
#include "Rendering/DrawElements.h"

SLATE_IMPLEMENT_WIDGET(SSaytOrbTray)

void SSaytOrbTray::PrivateRegisterAttributes(FSlateAttributeInitializer& AttributeInitializer)
{
	// 계약: 채워진 구슬 수가 바뀌면 Paint만 낡는다.
	// 슬롯 수(OrbCount)가 고정이라 DesiredSize 불변 → Layout Invalidation 불필요.
	SLATE_ADD_MEMBER_ATTRIBUTE_DEFINITION_WITH_NAME(AttributeInitializer, "RemainingOrbs", RemainingOrbsAttribute, EInvalidateWidgetReason::Paint);
}

SSaytOrbTray::SSaytOrbTray() : RemainingOrbsAttribute(*this)
{
}

void SSaytOrbTray::Construct(const FArguments& InArgs)
{
	OrbCount = FMath::Max(InArgs._OrbCount, 0);
	OrbDiameter = InArgs._OrbDiameter;
	OrbSpacing = InArgs._OrbSpacing;

	// 원 = 코너 반경을 한 변의 절반으로 준 RoundedBox.
	// 찬 구슬 = 단색 채움 / 빈 구슬 = 투명 채움 + 외곽선 (FrameBrush와 같은 4인자 생성자)
	const float Radius = OrbDiameter * 0.5f;
	FilledBrush = FSlateRoundedBoxBrush(InArgs._FilledColor, Radius);
	EmptyBrush = FSlateRoundedBoxBrush(FLinearColor::Transparent, Radius, InArgs._EmptyOutlineColor, 1.5f);

	RemainingOrbsAttribute.Set(*this, 0);

	// 갱신 동력은 세터+Invalidation뿐 — Tick 봉인 (SSaytHealthBar와 동일 규율)
	SetCanTick(false);
}

void SSaytOrbTray::SetRemainingOrbs(int32 InRemaining)
{
	// TSlateAttribute::Set이 이전 값과 비교해 실제 변화 시에만
	// 등록된 사유(Paint)로 Invalidation을 걸어준다 — 중복 호출 무해
	RemainingOrbsAttribute.Set(*this, FMath::Clamp(InRemaining, 0, OrbCount));
}

int32 SSaytOrbTray::OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry,
	const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements,
	int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const
{
	const FLinearColor ParentTint = InWidgetStyle.GetColorAndOpacityTint();
	const FVector2D LocalSize = AllottedGeometry.GetLocalSize();
	const int32 Remaining = RemainingOrbsAttribute.Get();

	// 부모가 신고 크기보다 크게 줬을 때 세로 중앙 정렬
	const float OffsetY = (LocalSize.Y - OrbDiameter) * 0.5f;

	// 구슬 N개는 서로 겹치지 않는 형제 → 전부 '같은 층'에 제출 (눈금 3개와 같은 사례).
	// 왼쪽부터 채움 = 가장 오른쪽 구슬부터 깨짐.
	for (int32 i = 0; i < OrbCount; ++i)
	{
		const FSlateBrush* Brush = (i < Remaining) ? &FilledBrush : &EmptyBrush;
		const float OffsetX = i * (OrbDiameter + OrbSpacing);

		FSlateDrawElement::MakeBox(
			OutDrawElements, LayerId,
			AllottedGeometry.ToPaintGeometry(
				FVector2D(OrbDiameter, OrbDiameter),
				FSlateLayoutTransform(FVector2D(OffsetX, OffsetY))),
			Brush, ESlateDrawEffect::None,
			Brush->GetTint(InWidgetStyle) * ParentTint);
	}

	return LayerId;
}

FVector2D SSaytOrbTray::ComputeDesiredSize(float LayoutScaleMultiplier) const
{
	if (OrbCount <= 0)
	{
		return FVector2D::ZeroVector;
	}
	return FVector2D(OrbCount * OrbDiameter + (OrbCount - 1) * OrbSpacing, OrbDiameter);
}
