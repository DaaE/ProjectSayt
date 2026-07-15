#include "SayuStyle.h"
#include "Styling/SlateStyle.h"
#include "Styling/SlateStyleRegistry.h"
#include "Brushes/SlateRoundedBoxBrush.h"
#include "Misc/Paths.h"
#include "Slate/SayuHealthBarWidgetStyle.h"

TSharedPtr<FSlateStyleSet> FSayuStyle::Instance = nullptr;

void FSayuStyle::Initialize()
{
	if (Instance.IsValid()) { return; }

	Instance = MakeShared<FSlateStyleSet>("SayuStyle");

	// ⑤ 폴리시 패스에서 텍스처를 상대 경로로 불러올 루트 (지금은 예약만)
	Instance->SetContentRoot(FPaths::ProjectContentDir() / TEXT("UI"));

	// 1차 브러시: 무텍스처 RoundedBox (⑤에서 텍스처로 교체 예정)
	// 주의: 채움의 원색은 흰색 — 런타임 틴트(③)가 '곱해지는' 구조라
	//       원색에 색이 있으면 틴트가 오염됨 (하얀 상자 사건의 곱셈, 이번엔 역방향 활용)
	Instance->Set("Sayu.HealthBar", FSayuHealthBarStyle()
		.SetBackgroundBrush(FSlateRoundedBoxBrush(FLinearColor(0.02f, 0.02f, 0.025f, 0.9f), 4.f))
		.SetFillBrush(FSlateRoundedBoxBrush(FLinearColor::White, 4.f))
		.SetGhostBrush(FSlateRoundedBoxBrush(FLinearColor(1.f, 0.45f, 0.1f, 0.6f), 4.f))
		.SetGhostDelay(0.25f)
		.SetGhostDrainSpeed(1.2f));

	FSlateStyleRegistry::RegisterSlateStyle(*Instance);
}

void FSayuStyle::Shutdown()
{
	if (Instance.IsValid())
	{
		FSlateStyleRegistry::UnRegisterSlateStyle(*Instance);
		Instance.Reset();
	}
}

const ISlateStyle& FSayuStyle::Get()
{
	check(Instance.IsValid());
	return *Instance;
}