// Fill out your copyright notice in the Description page of Project Settings.


#include "SayuStudyLabel.h"
#include "SSayuStudyBox.h"
#include "SlateOptMacros.h"

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION

void SayuStudyLabel::Construct(const FArguments& InArgs)
{
	// FSlateColorBrush: 텍스처 없이 단색만 칠하는 가장 단순한 브러시.
	// (브러시가 정확히 무엇인지는 1-9에서 제대로 다룹니다 — 지금은 '물감' 정도로만)
	// static인 이유: 위젯은 브러시를 '빌려 쓸' 뿐이라, 위젯보다 오래 사는 보관처가 필요.
	static FSlateColorBrush RedBrush(FLinearColor(0.5f, 0.08f, 0.08f));
	
	ChildSlot	// ← Compound의 상징: 자식 자리가 '정확히 1개'
	[
		SNew(SOverlay)	// 1개 자리에 2개(배경+글자)를 넣기 위한 우회로

		+ SOverlay::Slot().HAlign(HAlign_Fill).VAlign(VAlign_Fill)	// Stage 0의 그 Fill 교훈
		[
			SNew(SImage).Image(&RedBrush)
		]

		+ SOverlay::Slot().HAlign(HAlign_Fill).VAlign(VAlign_Fill)
		[
			SNew(STextBlock).Text(NSLOCTEXT("SayuStudy", "CompoundLabel", "COMPOUND: 조립으로 만든 상자"))
		]
		
	];
}

END_SLATE_FUNCTION_BUILD_OPTIMIZATION

static TSharedPtr<SWidget> GStudyLabelRoot;

static FAutoConsoleCommandWithWorld GRoleStudyCmd(
	TEXT("Sayu.Study.RoleLabel"),
	TEXT("Compound 학습 위젯을 뷰포트에 토글"),
	FConsoleCommandWithWorldDelegate::CreateLambda([](UWorld* World)
	{
		UGameViewportClient* Viewport = World ? World->GetGameViewport() : nullptr;
		if (!Viewport) { return; }

		if (GStudyLabelRoot.IsValid())   // 이미 떠 있으면 내리기
		{
			Viewport->RemoveViewportWidgetContent(GStudyLabelRoot.ToSharedRef());
			GStudyLabelRoot.Reset();
			return;
		}

		GStudyLabelRoot =
			SNew(SBox).HAlign(HAlign_Center).VAlign(VAlign_Center)   // 화면 정중앙에
			[
				SNew(SVerticalBox)   // Panel 진영의 새 얼굴: 자식들을 세로로 나열

				+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 20.f)
				[
					SNew(SBox).WidthOverride(400.f).HeightOverride(120.f)
					[
						SNew(SayuStudyLabel)   // 1-5: 조립으로 만든 상자
					]
				]

				+ SVerticalBox::Slot().AutoHeight()
				[
					SNew(SBox).WidthOverride(400.f).HeightOverride(120.f)
					[
						SNew(SSayuStudyBox)     // 1-6: 직접 그린 상자
					]
				]
			];
		Viewport->AddViewportWidgetContent(GStudyLabelRoot.ToSharedRef());
	})
);

// PIE 세션 경계 방어 (Stage 0에서 겪은 버그 재발 방지)
static FDelegateHandle GStudyLabelCleanup = FWorldDelegates::OnWorldCleanup.AddLambda(
	[](UWorld*, bool, bool)
	{
		UE_LOG(LogTemp, Warning, TEXT("PIE 세션 경계 STATIC 잔존 방어"));
		GStudyLabelRoot.Reset();
	});