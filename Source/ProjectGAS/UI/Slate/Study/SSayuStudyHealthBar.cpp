#include "SSayuStudyHealthBar.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystem/Attributes/SayuAttributeSet_Combat.h"
#include "Rendering/DrawElements.h"
#include "Brushes/SlateColorBrush.h"
#include "SayuStudyStyle.h"

void SSayuStudyHealthBar::Construct(const FArguments& InArgs)
{
}

void SSayuStudyHealthBar::BindToASC(UAbilitySystemComponent* InASC)
{
	if (!InASC) { return; }

	// [Push 담당] 구독 이후의 모든 변화 — 위젯은 TSharedPtr 주민이므로 AddSP (1-3)
	InASC->GetGameplayAttributeValueChangeDelegate(USayuAttributeSet_Combat::GetHealthAttribute())
		 .AddSP(this, &SSayuStudyHealthBar::HandleHealthChanged);

	// [Pull 담당] 구독 이전의 세계 따라잡기 — 딱 1회 (1-4)
	CurrentHealth = InASC->GetNumericAttribute(USayuAttributeSet_Combat::GetHealthAttribute());
	MaxHealth     = InASC->GetNumericAttribute(USayuAttributeSet_Combat::GetMaxHealthAttribute());

	// ⚠️ 단순화 2건을 정직하게 적어둡니다:
	// (1) MaxHealth는 1회 Pull만 하고 구독은 안 함 — 버프로 최대체력이 변하는 게임이면
	//     틀린 바가 됨. 진짜 위젯에선 MaxHealth도 구독해야 함.
	// (2) 1-3에서 예고한 '명시적 해제 규율'을 생략하고 AddSP의 자동 무효화에만 기댐.
	//     둘 다 결과물 단계의 진짜 SHealthBar에서 제대로 처리합니다.
}

void SSayuStudyHealthBar::HandleHealthChanged(const FOnAttributeChangeData& Data)
{
	// 직접 발견하신 그 48->48 무변화 방송을 여기서 걸러냅니다
	if (Data.NewValue == Data.OldValue) { return; }

	CurrentHealth = Data.NewValue;

	// "내 그림이 낡았다"고 Slate에 신고. 사실 지금은 이 줄을 지워도
	// 화면이 갱신될 겁니다 — 왜 그런지, 그런데도 왜 이 줄이 필요한지가
	// 바로 다음 단원(1-8)의 주제입니다.
	//Invalidate(EInvalidateWidgetReason::Paint);
}

int32 SSayuStudyHealthBar::OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry,
	const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements,
	int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const
{
	const FSlateBrush* BackBrush = FSayuStudyStyle::Get().GetBrush("Sayu.Study.HealthBar.Back");
	const FSlateBrush* FillBrush = FSayuStudyStyle::Get().GetBrush("Sayu.Study.HealthBar.Fill");
 	
	const FLinearColor ParentTint = InWidgetStyle.GetColorAndOpacityTint();

	// 명령서 1: 배경 — 내 영역 전체
	FSlateDrawElement::MakeBox(
		OutDrawElements, LayerId,
		AllottedGeometry.ToPaintGeometry(),
		BackBrush, ESlateDrawEffect::None,
		BackBrush->GetTint(InWidgetStyle) * ParentTint);   // 하얀 상자 사건의 교훈

	// 명령서 2: 채움 — 가로폭만 체력 비율로 축소, 배경 '위'니까 층 +1
	const float Percent = (MaxHealth > 0.f) ? FMath::Clamp(CurrentHealth / MaxHealth, 0.f, 1.f) : 0.f;
	const FVector2D LocalSize = AllottedGeometry.GetLocalSize();

	FSlateDrawElement::MakeBox(
		OutDrawElements, LayerId + 1,
		AllottedGeometry.ToPaintGeometry(
			FVector2D(LocalSize.X * Percent, LocalSize.Y),   // 크기: 비율만큼
			FSlateLayoutTransform()),                        // 위치: 왼쪽 위 (0,0) 그대로
			FillBrush, ESlateDrawEffect::None,
			FillBrush->GetTint(InWidgetStyle) * ParentTint);

	return LayerId + 1;   // 규약: 내가 쓴 최고 층 보고
}

FVector2D SSayuStudyHealthBar::ComputeDesiredSize(float LayoutScaleMultiplier) const
{
	return FVector2D(400.f, 32.f);
}

// ─── 뷰포트 토글 커맨드 ───────────────────────────────────────────
#include "EngineUtils.h"
#include "Character/SayuNPCCharacter.h"
#include "Widgets/Layout/SBox.h"

static TSharedPtr<SWidget> GStudyHealthBarRoot;

static FAutoConsoleCommandWithWorld GStudyHealthBarCmd(
	TEXT("Sayu.Study.HealthBar"),
	TEXT("첫 번째 NPC의 체력에 연결된 학습용 체력바를 토글"),
	FConsoleCommandWithWorldDelegate::CreateLambda([](UWorld* World)
	{
		UGameViewportClient* Viewport = World ? World->GetGameViewport() : nullptr;
		if (!Viewport) { return; }

		if (GStudyHealthBarRoot.IsValid())
		{
			Viewport->RemoveViewportWidgetContent(GStudyHealthBarRoot.ToSharedRef());
			GStudyHealthBarRoot.Reset();
			return;
		}

		// 월드에서 첫 NPC를 찾아 연결 대상으로 삼음 (학습용 최단 경로)
		ASayuNPCCharacter* TargetNPC = nullptr;
		for (TActorIterator<ASayuNPCCharacter> It(World); It; ++It) { TargetNPC = *It; break; }
		if (!TargetNPC) { UE_LOG(LogTemp, Warning, TEXT("NPC 없음")); return; }

		TSharedPtr<SSayuStudyHealthBar> Bar;

		GStudyHealthBarRoot =
			SNew(SBox).HAlign(HAlign_Center).VAlign(VAlign_Bottom).Padding(0.f, 0.f, 0.f, 80.f)
			[
				SNew(SBox).WidthOverride(400.f).HeightOverride(32.f)
				[
					SAssignNew(Bar, SSayuStudyHealthBar)   // SNew + 변수에 대입을 한 번에
				]
			];

		Viewport->AddViewportWidgetContent(GStudyHealthBarRoot.ToSharedRef());
		Bar->BindToASC(TargetNPC->GetAbilitySystemComponent());
	})
);

static FDelegateHandle GStudyHealthBarCleanup = FWorldDelegates::OnWorldCleanup.AddLambda(
	[](UWorld*, bool, bool) { GStudyHealthBarRoot.Reset(); });