#pragma once

#include "CoreMinimal.h"

class FSlateStyleSet;
class ISlateStyle;

// 프로젝트 정식 스타일 등록소. 위젯 스타일들을 Slate 스타일 레지스트리에 등록/해제한다.
class PROJECTSAYT_API FSayuStyle
{
public:
	static void Initialize();
	static void Shutdown();
	static const ISlateStyle& Get();
	
private:
	static TSharedPtr<FSlateStyleSet> Instance;
};
