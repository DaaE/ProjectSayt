#include "SaytStyle.h"
#include "Styling/SlateStyle.h"
#include "Styling/SlateStyleRegistry.h"
#include "Brushes/SlateRoundedBoxBrush.h"
#include "Misc/Paths.h"
#include "Slate/SaytHealthBarWidgetStyle.h"
#include "Brushes/SlateImageBrush.h"

// 엔진 스타일 파일들의 표준 관용구 — 콘텐츠 루트 기준 상대 경로로 이미지 브러시 정의
#define IMAGE_BRUSH(RelativePath, ...) FSlateImageBrush(Instance->RootToContentDir(RelativePath, TEXT(".png")), __VA_ARGS__)

TSharedPtr<FSlateStyleSet> FSaytStyle::Instance = nullptr;

void FSaytStyle::Initialize()
{
	if (Instance.IsValid()) { return; }

	Instance = MakeShared<FSlateStyleSet>("SaytStyle");

	// ⑤ 폴리시 패스에서 텍스처를 상대 경로로 불러올 루트 (지금은 예약만)
	Instance->SetContentRoot(FPaths::ProjectContentDir() / TEXT("UI"));

	// 1차 브러시: 무텍스처 RoundedBox (⑤에서 텍스처로 교체 예정)
	// 주의: 채움의 원색은 흰색 — 런타임 틴트(③)가 '곱해지는' 구조라
	//       원색에 색이 있으면 틴트가 오염됨 (하얀 상자 사건의 곱셈, 이번엔 역방향 활용)
	Instance->Set("Sayt.HealthBar", FSaytHealthBarStyle()
		.SetBackgroundBrush(FSlateRoundedBoxBrush(FLinearColor::White, 4.f))		// 배경도 '다음 줄 색' 틴트를 입어야 하므로 원색 흰색
		.SetFillBrush(FSlateRoundedBoxBrush(FLinearColor::White, 4.f))
		.SetGhostBrush(FSlateRoundedBoxBrush(FLinearColor(1.f, 0.45f, 0.1f, 0.6f), 4.f))
		// 프레임: 내부 투명 + 외곽선만. ⚠️ 4인자 오버로드(색, 반경, 외곽선색, 외곽선두께)는
		// 5.7 실물 대조 필요 — 에러 시 F12로 생성자 가족 확인
		.SetFrameBrush(FSlateRoundedBoxBrush(FLinearColor::Transparent, 4.f, FLinearColor(0.85f, 0.9f, 1.f, 0.55f), 1.5f))
		.SetHighlightBrush(FSlateColorBrush(FLinearColor(1.f, 1.f, 1.f, 0.10f)))
		.SetDefaultFillTint(FLinearColor(0.7f, 0.1f, 0.1f))
		.SetDefaultBackgroundTint(FLinearColor(0.02f, 0.02f, 0.025f, 0.9f))
		.SetShadeTopAlpha(0.f)
		.SetShadeBottomAlpha(0.35f)
		.SetShowTicks(true)
		.SetTickColor(FLinearColor(0.f, 0.f, 0.f, 0.25f))
		.SetGhostDelay(0.25f)
		.SetGhostDrainSpeed(1.2f));
	
	// ── 타입 프리셋 (2-4): 키가 곧 타입별 룩. 조회는 GetSaytHealthDisplayPreset 경유 ──
	Instance->Set("Sayt.HealthBar.Boss", FSaytHealthBarStyle()
		.SetBackgroundBrush(FSlateRoundedBoxBrush(FLinearColor::White, 5.f))
		.SetFillBrush(FSlateRoundedBoxBrush(FLinearColor::White, 5.f))
		.SetGhostBrush(FSlateRoundedBoxBrush(FLinearColor(1.f, 0.45f, 0.1f, 0.6f), 5.f))
		.SetFrameBrush(FSlateRoundedBoxBrush(FLinearColor::Transparent, 5.f, FLinearColor(0.9f, 0.95f, 1.f, 0.7f), 2.5f))
		.SetHighlightBrush(FSlateColorBrush(FLinearColor(1.f, 1.f, 1.f, 0.10f)))
		.SetDefaultFillTint(FLinearColor(0.7f, 0.1f, 0.1f))
		.SetDefaultBackgroundTint(FLinearColor(0.02f, 0.02f, 0.025f, 0.92f))
		.SetShadeTopAlpha(0.f)
		.SetShadeBottomAlpha(0.4f)
		.SetShowTicks(true)
		.SetTickColor(FLinearColor(0.f, 0.f, 0.f, 0.25f))
		.SetEnableGhost(true)
		.SetGhostDelay(0.3f)
		.SetGhostDrainSpeed(1.0f));
	
	Instance->Set("Sayt.HealthBar.Mob", FSaytHealthBarStyle()
		.SetBackgroundBrush(FSlateRoundedBoxBrush(FLinearColor::White, 3.f))
		.SetFillBrush(FSlateRoundedBoxBrush(FLinearColor::White, 3.f))
		.SetGhostBrush(FSlateRoundedBoxBrush(FLinearColor::Transparent, 3.f))   // 미사용이지만 명시
		.SetFrameBrush(FSlateRoundedBoxBrush(FLinearColor::Transparent, 3.f, FLinearColor(1.f, 1.f, 1.f, 0.3f), 1.f))
		.SetHighlightBrush(FSlateColorBrush(FLinearColor::Transparent))   // 소형 바에 유리띠는 노이즈
		.SetDefaultFillTint(FLinearColor(0.7f, 0.1f, 0.1f))
		.SetDefaultBackgroundTint(FLinearColor(0.02f, 0.02f, 0.025f, 0.85f))
		.SetShadeTopAlpha(0.f)
		.SetShadeBottomAlpha(0.2f)
		.SetShowTicks(false)   // 90px 폭에 눈금은 노이즈
		.SetTickColor(FLinearColor::Transparent)
		.SetEnableGhost(false));   // 일반몹: 잔상 없음
	
	// ── 구슬 텍스처 브러시 (느슨한 파일 — Content/UI/HealthDisplay/*.png) ──
	// 텍스처는 백색 계조, 색은 여기 틴트가 전담 (원색 흰색 + 틴트 곱셈 관례)
	// ⚠️ 패키징 부채: 느슨한 파일은 스테이징 설정 필요 — Phase 12 체크리스트 참조
	Instance->Set("Sayt.HealthDisplay.OrbFilled",
		new IMAGE_BRUSH("HealthDisplay/T_UI_Orb_Filled_32", FVector2D(32.f, 32.f),
		FLinearColor(0.66f, 0.545f, 0.98f)));                      // 바이올렛 #A88BFA
	
	Instance->Set("Sayt.HealthDisplay.OrbEmpty",
		new IMAGE_BRUSH("HealthDisplay/T_UI_Orb_Empty_32", FVector2D(32.f, 32.f),
		FLinearColor(1.f, 1.f, 1.f, 0.35f)));                      // 빈 슬롯: 백색 35%

	FSlateStyleRegistry::RegisterSlateStyle(*Instance);
}

void FSaytStyle::Shutdown()
{
	if (Instance.IsValid())
	{
		FSlateStyleRegistry::UnRegisterSlateStyle(*Instance);
		Instance.Reset();
	}
}

const ISlateStyle& FSaytStyle::Get()
{
	check(Instance.IsValid());
	return *Instance;
}