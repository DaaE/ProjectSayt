// Fill out your copyright notice in the Description page of Project Settings.


#include "SSayuHealthBar.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystem/Attributes/SayuAttributeSet_Combat.h"
#include "Rendering/DrawElements.h"
#include "UI/SayuStyle.h"

SLATE_IMPLEMENT_WIDGET(SSayuHealthBar)
void SSayuHealthBar::PrivateRegisterAttributes(FSlateAttributeInitializer& AttributeInitializer)
{
	// 계약: 틴트가 바뀌면 Paint만 낡는다 (크기 불변이므로 Layout 불필요)
	SLATE_ADD_MEMBER_ATTRIBUTE_DEFINITION_WITH_NAME(AttributeInitializer, "FillTint", FillTintAttribute, EInvalidateWidgetReason::Paint);
	SLATE_ADD_MEMBER_ATTRIBUTE_DEFINITION_WITH_NAME(AttributeInitializer, "BackgroundTint", BackgroundTintAttribute, EInvalidateWidgetReason::Paint);
}

// TSlateAttribute는 기본 생성이 불가 — 소유 위젯(*this)을 받아야 태어남.
// 그래서 Construct()가 아닌 진짜 생성자의 초기화 목록이 필요해짐
SSayuHealthBar::SSayuHealthBar() : FillTintAttribute(*this), BackgroundTintAttribute(*this)
{
}

void SSayuHealthBar::Construct(const FArguments& InArgs)
{
	// 스타일 폴백: 인자로 안 오면 프로젝트 등록소에서 조회
	Style = InArgs._Style
		? InArgs._Style
		: &FSayuStyle::Get().GetWidgetStyle<FSayuHealthBarStyle>("Sayu.HealthBar");

	DesiredBarSize = InArgs._DesiredBarSize;

	// 런타임 틴트의 시작값은 스타일의 기본 룩
	FillTintAttribute.Set(*this, Style->DefaultFillTint);
	BackgroundTintAttribute.Set(*this, Style->DefaultBackgroundTint);

	// 이 위젯의 갱신 동력은 이벤트(Push)+Invalidate뿐 — Tick은 명시적으로 봉인
	SetCanTick(false);
}

SSayuHealthBar::~SSayuHealthBar()
{
	UnbindFromASC();   // 대칭 해제 규율 (1-5 대화의 그 RAII 스타일)
}

void SSayuHealthBar::BindToASC(UAbilitySystemComponent* InASC)
{
	UnbindFromASC();   // 재바인딩 안전 — 이미 묶여 있어도 이 함수만 부르면 됨
	if (!InASC) { return; }

	BoundASC = InASC;

	// [Push] 구독 이후의 모든 변화 — 위젯은 TSharedPtr 주민이므로 AddSP
	HealthChangedHandle = InASC->GetGameplayAttributeValueChangeDelegate(
			USayuAttributeSet_Combat::GetHealthAttribute())
		.AddSP(this, &SSayuHealthBar::HandleHealthChanged);

	MaxHealthChangedHandle = InASC->GetGameplayAttributeValueChangeDelegate(
			USayuAttributeSet_Combat::GetMaxHealthAttribute())
		.AddSP(this, &SSayuHealthBar::HandleMaxHealthChanged);

	// [Pull] 구독 이전의 세계 따라잡기 — 1회
	CurrentHealth = InASC->GetNumericAttribute(USayuAttributeSet_Combat::GetHealthAttribute());
	MaxHealth     = InASC->GetNumericAttribute(USayuAttributeSet_Combat::GetMaxHealthAttribute());

	Invalidate(EInvalidateWidgetReason::Paint);
}

void SSayuHealthBar::UnbindFromASC()
{
	if (UAbilitySystemComponent* ASC = BoundASC.Get())   // ASC가 먼저 죽었으면 풀 것도 없음
	{
		if (HealthChangedHandle.IsValid())
		{
			ASC->GetGameplayAttributeValueChangeDelegate(
				USayuAttributeSet_Combat::GetHealthAttribute()).Remove(HealthChangedHandle);
		}
		if (MaxHealthChangedHandle.IsValid())
		{
			ASC->GetGameplayAttributeValueChangeDelegate(
				USayuAttributeSet_Combat::GetMaxHealthAttribute()).Remove(MaxHealthChangedHandle);
		}
	}
	HealthChangedHandle.Reset();
	MaxHealthChangedHandle.Reset();
	BoundASC.Reset();
}

void SSayuHealthBar::SetFillTint(const FSlateColor& InTint)
{
	FillTintAttribute.Set(*this, InTint);   // 비교→대입→달라졌을 때만 Paint 신고 (전부 자동)
}

void SSayuHealthBar::SetBackgroundTint(const FSlateColor& InTint)
{
	BackgroundTintAttribute.Set(*this, InTint);
}

void SSayuHealthBar::HandleHealthChanged(const FOnAttributeChangeData& Data)
{
	if (Data.NewValue == Data.OldValue) { return; }   // 무변화 방송 필터
	CurrentHealth = Data.NewValue;
	Invalidate(EInvalidateWidgetReason::Paint);
}

void SSayuHealthBar::HandleMaxHealthChanged(const FOnAttributeChangeData& Data)
{
	if (Data.NewValue == Data.OldValue) { return; }
	MaxHealth = Data.NewValue;
	Invalidate(EInvalidateWidgetReason::Paint);
}

float SSayuHealthBar::GetHealthPercent() const
{
	return (MaxHealth > 0.f) ? FMath::Clamp(CurrentHealth / MaxHealth, 0.f, 1.f) : 0.f;
}

int32 SSayuHealthBar::OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry,
	const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements,
	int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const
{
	// 층 번호는 후위 증가 방식 — ④에서 고스트 층이 배경과 채움 '사이'에 끼어들 예정
	int32 RetLayerId = LayerId;
	const FLinearColor ParentTint = InWidgetStyle.GetColorAndOpacityTint();

	// 명령서 1: 배경 = '다음 줄' 색
	FSlateDrawElement::MakeBox(
		OutDrawElements, RetLayerId++,
		AllottedGeometry.ToPaintGeometry(),
		&Style->BackgroundBrush, ESlateDrawEffect::None,
		Style->BackgroundBrush.GetTint(InWidgetStyle) * ParentTint * BackgroundTintAttribute.Get().GetColor(InWidgetStyle));

	// (④ 예약석: 고스트 명령서가 여기 들어옴)

	// 명령서 2: 채움 = '현재 줄' 색. 0%면 폭 0짜리 명령서를 아예 안 만듦
	const float Percent = GetHealthPercent();
	if (Percent > 0.f)
	{
		const FVector2D LocalSize = AllottedGeometry.GetLocalSize();
		FSlateDrawElement::MakeBox(
			OutDrawElements, RetLayerId++,
			AllottedGeometry.ToPaintGeometry(
				FVector2D(LocalSize.X * Percent, LocalSize.Y), FSlateLayoutTransform()),
			&Style->FillBrush, ESlateDrawEffect::None,
			Style->FillBrush.GetTint(InWidgetStyle) * ParentTint * FillTintAttribute.Get().GetColor(InWidgetStyle));
	}

	return RetLayerId - 1;   // 후위 증가는 '다음에 쓸 번호'를 가리키므로 -1이 '실제로 쓴 마지막 번호'
}

FVector2D SSayuHealthBar::ComputeDesiredSize(float LayoutScaleMultiplier) const
{
	return DesiredBarSize;
}

// ─────────────────────────────────────────────────────────────
// [임시 — 3c(HUD 정식 부착)에서 제거 예정] 검증용 콘솔 커맨드
// ─────────────────────────────────────────────────────────────
#if !UE_BUILD_SHIPPING
#include "AbilitySystemGlobals.h"
#include "Widgets/Layout/SBox.h"

static TSharedPtr<SWidget> GSayuHealthBarRoot;
static TWeakPtr<SSayuHealthBar> GSayuHealthBarWidget;

static FAutoConsoleCommandWithWorld GSayuHealthBarShowCmd(
	TEXT("Sayu.HealthBar.Show"),
	TEXT("[임시] 플레이어 ASC에 바인딩된 SSayuHealthBar를 좌하단에 토글"),
	FConsoleCommandWithWorldDelegate::CreateLambda([](UWorld* World)
	{
		UGameViewportClient* Viewport = World ? World->GetGameViewport() : nullptr;
		if (!Viewport) { return; }

		if (GSayuHealthBarRoot.IsValid())
		{
			Viewport->RemoveViewportWidgetContent(GSayuHealthBarRoot.ToSharedRef());
			GSayuHealthBarRoot.Reset();
			return;
		}

		APlayerController* PC = World->GetFirstPlayerController();
		APawn* Pawn = PC ? PC->GetPawn() : nullptr;
		// 캐릭터 클래스명에 의존하지 않는 표준 ASC 조회 경로 (IAbilitySystemInterface 경유)
		UAbilitySystemComponent* ASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(Pawn);
		if (!ASC) { UE_LOG(LogTemp, Warning, TEXT("[HealthBar.Show] 플레이어 ASC를 찾지 못함")); return; }

		TSharedPtr<SSayuHealthBar> Bar;
		GSayuHealthBarRoot =
			SNew(SBox).HAlign(HAlign_Left).VAlign(VAlign_Bottom).Padding(60.f, 0.f, 0.f, 60.f)
			[
				SAssignNew(Bar, SSayuHealthBar)
			];
		Viewport->AddViewportWidgetContent(GSayuHealthBarRoot.ToSharedRef());
		Bar->BindToASC(ASC);
		GSayuHealthBarWidget = Bar;
	})
);

// 신규 — [임시] 줄 색 스왑 시뮬레이션: Sayu.HealthBar.Tint <R> <G> <B> (0~1)
static FAutoConsoleCommandWithWorldAndArgs GSayuHealthBarTintCmd(
	TEXT("Sayu.HealthBar.Tint"),
	TEXT("[임시] 표시 중인 체력바의 채움 틴트를 변경 (TSlateAttribute 검증용)"),
	FConsoleCommandWithWorldAndArgsDelegate::CreateLambda([](const TArray<FString>& Args, UWorld*)
	{
		TSharedPtr<SSayuHealthBar> Bar = GSayuHealthBarWidget.Pin();
		if (!Bar.IsValid() || Args.Num() < 3) { return; }
		Bar->SetFillTint(FLinearColor(
			FCString::Atof(*Args[0]), FCString::Atof(*Args[1]), FCString::Atof(*Args[2])));
	})
);

static FDelegateHandle GSayuHealthBarCleanup = FWorldDelegates::OnWorldCleanup.AddLambda(
	[](UWorld*, bool, bool) { GSayuHealthBarRoot.Reset(); });
#endif
