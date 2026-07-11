// Fill out your copyright notice in the Description page of Project Settings.


#include "SSayuStudyBox.h"
#include "Fonts/FontMeasure.h"


void SSayuStudyBox::Construct(const FArguments& InArgs)
{
	// 조립할 자식이 없으니 할 일이 없음 — Compound와의 첫 대비점
}

int32 SSayuStudyBox::OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry,
	const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements,
	int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const
{
	static FSlateColorBrush BlueBrush(FLinearColor(0.08f, 0.15f, 0.5f));

	// 명령서 1: "배정받은 내 영역 전체에 파란 브러시를 칠해라"
	FSlateDrawElement::MakeBox(
		OutDrawElements,
		LayerId,
		AllottedGeometry.ToPaintGeometry(),   // 인자 없이 호출 = 내 로컬 영역 전체
		&BlueBrush,
		ESlateDrawEffect::None,
		// 브러시의 색(TintColor)을 '우리가 직접' 꺼내서 틴트 인자로 전달.
		// 부모에게서 물려받은 틴트(InWidgetStyle)까지 곱하는 것이 엔진 위젯들의 관례
		// (부모가 반투명해지면 나도 같이 반투명해지는 연쇄가 이 곱셈에서 나옴)
		BlueBrush.GetTint(InWidgetStyle) * InWidgetStyle.GetColorAndOpacityTint());

	// 명령서 2: 글자 — 1-5에서 STextBlock이 공짜로 해주던 일을 전부 수동으로
	const FText Text = NSLOCTEXT("SayuStudy", "LeafLabel", "LEAF: OnPaint로 직접 그린 상자");
	const FSlateFontInfo Font = FCoreStyle::GetDefaultFontStyle("Regular", 14);

	// 가운데 정렬'조차' 수동: 글자의 픽셀 크기를 재서 좌표를 직접 계산
	const TSharedRef<FSlateFontMeasure> FontMeasure =
		FSlateApplication::Get().GetRenderer()->GetFontMeasureService();
	const FVector2D TextSize = FontMeasure->Measure(Text, Font);
	const FVector2D TextPos = (AllottedGeometry.GetLocalSize() - TextSize) * 0.5f;

	FSlateDrawElement::MakeText(
		OutDrawElements,
		LayerId + 1,   // 상자 '위에' 보여야 하므로 층 번호를 올림
		AllottedGeometry.ToPaintGeometry(TextSize, FSlateLayoutTransform(TextPos)),
		Text.ToString(),
		Font,
		ESlateDrawEffect::None,
		FLinearColor::White);

	return LayerId + 1;   // 규약: 내가 쓴 가장 높은 층 번호를 부모에게 보고
}

FVector2D SSayuStudyBox::ComputeDesiredSize(float LayoutScaleMultiplier) const
{
	// 계약 이행: 자식이 없으니 "원하는 크기"를 스스로 답함
	return FVector2D(400.f, 120.f);
}