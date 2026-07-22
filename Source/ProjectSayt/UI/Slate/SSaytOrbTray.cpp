// Copyright (c) 2026 Sungmin JI. All Rights Reserved.


#include "SSaytOrbTray.h"
#include "UI/SaytStyle.h"
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

	// 브러시 = '그리기 명세'를 통째로 받는다. 미지정이면 등록소의 텍스처 브러시.
	// RoundedBox(코드) → 텍스처 전환이 위젯 그리기 코드 무수정으로 끝나는 이유:
	// OnPaint는 처음부터 FSlateBrush라는 추상만 알았기 때문
	FilledBrush = InArgs._FilledBrushOverride ? *InArgs._FilledBrushOverride : *FSaytStyle::Get().GetBrush("Sayt.HealthDisplay.OrbFilled");
	EmptyBrush = InArgs._EmptyBrushOverride ? *InArgs._EmptyBrushOverride : *FSaytStyle::Get().GetBrush("Sayt.HealthDisplay.OrbEmpty");

	RemainingOrbsAttribute.Set(*this, 0);

	// 갱신 동력은 세터+Invalidation뿐 — Tick 봉인 (SSaytHealthBar와 동일 규율)
	SetCanTick(false);
}

void SSaytOrbTray::SetRemainingOrbs(int32 InRemaining)
{
	const int32 OldRemaining = RemainingOrbsAttribute.Get();
	const int32 NewRemaining = FMath::Clamp(InRemaining, 0, OrbCount);
	
	// TSlateAttribute::Set이 비교 후 변화 시에만 Paint 신고 — 중복 호출 무해
	RemainingOrbsAttribute.Set(*this, NewRemaining);
	
	// 감소 = 파괴 → 방금 비워진 [New, Old) 슬롯에 팝 재생
	if (NewRemaining < OldRemaining)
	{
		PopIndexBegin = NewRemaining;
		PopIndexEnd = OldRemaining;
		PopElapsed = 0.f;
		
		if (!PopTimerHandle.IsValid())
		{
			PopTimerHandle = RegisterActiveTimer(0.f,FWidgetActiveTimerDelegate::CreateSP(this, &SSaytOrbTray::PopTick));
		}
	}
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

	// 파괴 팝: 방금 비워진 슬롯 위에 채움 구슬이 커지며 사라진다.
	// 빈 구슬(형제 층)과 '겹치는' 그리기이므로 층 분리 — Stage 1 층 규율 그대로
	if (PopIndexBegin != INDEX_NONE)
	{
		const float T = FMath::Clamp(PopElapsed / PopDuration, 0.f, 1.f);
		const float EaseOut = 1.f - FMath::Square(1.f - T);   // 초반 빠르고 끝에서 감속
		const float Scale = FMath::Lerp(1.f, 1.6f, EaseOut);
		const float Alpha = 1.f - T;
		const float ScaledSize = OrbDiameter * Scale;
		const float CenterShift = (ScaledSize - OrbDiameter) * 0.5f;   // 제자리 중심 확대
		
		for (int32 i = PopIndexBegin; i < PopIndexEnd; ++i)
		{
			const float OffsetX = i * (OrbDiameter + OrbSpacing);
			FSlateDrawElement::MakeBox(
				OutDrawElements, LayerId + 1,
				AllottedGeometry.ToPaintGeometry(
					FVector2D(ScaledSize, ScaledSize),
					FSlateLayoutTransform(FVector2D(OffsetX - CenterShift, OffsetY - CenterShift))),
					&FilledBrush, ESlateDrawEffect::None,
					FilledBrush.GetTint(InWidgetStyle) * ParentTint * FLinearColor(1.f, 1.f, 1.f, Alpha));
		}
		return LayerId + 1;
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

EActiveTimerReturnType SSaytOrbTray::PopTick(double InCurrentTime, float InDeltaTime)
{
	PopElapsed += InDeltaTime;
	Invalidate(EInvalidateWidgetReason::Paint);   // 매 프레임 그림이 변하는 구간 — 정직한 신고

	if (PopElapsed >= PopDuration)
	{
		// 종료 — 장부 정리 후 타이머 소멸, 위젯은 다시 완전 휴면으로 (고스트와 동일 규율)
		PopIndexBegin = INDEX_NONE;
		PopIndexEnd = INDEX_NONE;
		PopTimerHandle.Reset();
		return EActiveTimerReturnType::Stop;
	}
	return EActiveTimerReturnType::Continue;
}