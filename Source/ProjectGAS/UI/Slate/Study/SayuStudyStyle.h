#pragma once
#include "CoreMinimal.h"

class FSlateStyleSet;
class ISlateStyle;

// 학습용 스타일 등록소. Initialize/Shutdown은 모듈 시작/종료에서 한 번씩 (대칭 규율)
class PROJECTGAS_API FSayuStudyStyle
{
public:
	static void Initialize();
	static void Shutdown();
	static const ISlateStyle& Get();
	
private:
	static TSharedPtr<FSlateStyleSet> Instance;
	
};
