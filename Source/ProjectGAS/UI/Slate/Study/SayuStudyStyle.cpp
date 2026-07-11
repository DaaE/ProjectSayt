#include "SayuStudyStyle.h"
#include "Styling/SlateStyle.h"
#include "Styling/SlateStyleRegistry.h"
#include "Brushes/SlateColorBrush.h"

TSharedPtr<FSlateStyleSet> FSayuStudyStyle::Instance = nullptr;

void FSayuStudyStyle::Initialize()
{
	if (Instance.IsValid()) { return; }   // 중복 초기화 방어

	Instance = MakeShared<FSlateStyleSet>("SayuStudyStyle");

	// 이름표를 붙여 등록. 'new'로 넘기는 순간 소유권이 스타일셋으로 이전됩니다
	// (스타일셋이 소멸할 때 자기가 delete — 위젯은 앞으로 빌려 쓰기만 함)
	Instance->Set("Sayu.Study.HealthBar.Back", new FSlateColorBrush(FLinearColor(0.05f, 0.05f, 0.05f)));
	Instance->Set("Sayu.Study.HealthBar.Fill", new FSlateColorBrush(FLinearColor(0.7f, 0.1f, 0.1f)));

	// 전역 전화번호부에 등록 — 이름으로 어디서든 찾을 수 있게
	FSlateStyleRegistry::RegisterSlateStyle(*Instance);
}

void FSayuStudyStyle::Shutdown()
{
	if (Instance.IsValid())
	{
		FSlateStyleRegistry::UnRegisterSlateStyle(*Instance);
		Instance.Reset();
	}
}

const ISlateStyle& FSayuStudyStyle::Get()
{
	check(Instance.IsValid());   // Initialize 전에 부르면 즉시 알려달라는 안전핀
	return *Instance;
}