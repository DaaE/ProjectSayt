// Copyright (c) 2026 Sungmin JI. All Rights Reserved.

#include "SaytGameplayEffectUIData.h"

#if WITH_EDITOR

#include "Misc/DataValidation.h"

#define LOCTEXT_NAMESPACE "SaytGameplayEffectUIData"

EDataValidationResult USaytGameplayEffectUIData::IsDataValid(FDataValidationContext& Context) const
{
	// 부모 구현이 '같은 클래스의 컴포넌트가 중복 부착되지 않았는가'를 검사한다.
	EDataValidationResult Result = Super::IsDataValid(Context);

	// 표시 데이터가 반쯤 채워진 애셋을 런타임에 발견하는 대신 저장 시점에 잡는다.
	// 경고이므로 Result는 건드리지 않는다 — 저장을 막을 사안은 아니다.
	if (Icon == nullptr)
	{
		Context.AddWarning(LOCTEXT("SaytUIData_NoIcon", "트레이 아이콘이 비어 있습니다."));
	}

	if (DisplayName.IsEmpty())
	{
		Context.AddWarning(LOCTEXT("SaytUIData_NoDisplayName", "표시명이 비어 있습니다."));
	}

	return Result;
}

#undef LOCTEXT_NAMESPACE

#endif