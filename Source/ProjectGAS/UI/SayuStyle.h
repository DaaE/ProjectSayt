#pragma once

#include "CoreMinimal.h"

class FSlateStyleSet;
class ISlateStyle;

// 프로젝트 정식 스타일 등록소. 학습용 FSayuStudyStyle과 별개로 공존 (Study 처분 시 함께 정리).
class PROJECTGAS_API FSayuStyle
{
public:
	static void Initialize();
	static void Shutdown();
	static const ISlateStyle& Get();
	
private:
	static TSharedPtr<FSlateStyleSet> Instance;
};
