// Copyright (c) 2026 Sungmin JI. All Rights Reserved.

#include "SaytSegmentedHealth.h"

FSaytSegmentedHealthState FSaytSegmentedHealth::Calculate(float Health, float MaxHealth, int32 SegmentCount)
{
	FSaytSegmentedHealthState Result;

	// 방어: 잘못된 입력은 전부 '빈 바'로 수렴시킨다.
	if (MaxHealth <= 0.f || SegmentCount <= 0)
	{
		return Result;
	}

	const float SegmentSize = MaxHealth / SegmentCount;

	// Ratio = "지금 몇 번째 세그먼트의 어디쯤인가"를 하나의 실수로 표현한 값.
	// 예: 3000/3구슬에서 Health 2100 → Ratio 2.1 (2세그먼트 완충 + 현재 10%)
	float Ratio = FMath::Clamp(Health, 0.f, MaxHealth) / SegmentSize;

	// 스냅을 하지 않는다. 경계 관례(정확히 경계 = 구슬 유지 + 100%)는 ceil이
	// 이미 보장하고(경계값은 float으로 정확히 표현됨), 경계보다 엡실론 위의
	// 값을 아래로 스냅하면 이산 상태(구슬 수)를 왜곡하고 파괴 신호가 실제
	// 경계 통과보다 먼저 발화한다. 구슬 수는 항상 실제 Health에 정직해야 한다.

	// 경계값 관례: Ratio 2.0은 ceil로 2가 되어 '구슬 1개 + 바 100%'.
	// Ratio 2.1은 ceil로 3이 되어 '구슬 2개 + 바 10%'.
	Result.RemainingOrbs = FMath::Clamp(FMath::CeilToInt32(Ratio) - 1, 0, SegmentCount - 1);
	Result.SegmentPercent = FMath::Clamp(Ratio - Result.RemainingOrbs, 0.f, 1.f);

	return Result;
}

FSaytSegmentedHealthState FSaytSegmentedHealth::Calculate(float Health, float MaxHealth, int32 SegmentCount,
	const FSaytSegmentedHealthState& Previous)
{
	FSaytSegmentedHealthState Result = Calculate(Health, MaxHealth, SegmentCount);
	Result.bSegmentBroken = Result.RemainingOrbs < Previous.RemainingOrbs;
	return Result;
}