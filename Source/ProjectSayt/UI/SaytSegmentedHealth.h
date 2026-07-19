// Copyright (c) 2026 Sungmin JI. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

// ─────────────────────────────────────────────────────────────
// 세그먼트 체력 매핑 (Phase 8 Stage 2-1)
//
// GAS의 연속적인 Health 하나를 "구슬 N개 + 현재 세그먼트 내 %"라는
// 표시 계층 상태로 변환하는 순수 로직입니다. 상태를 갖지 않으며
// UObject/위젯 어디에도 의존하지 않습니다 — 방법 (i): 표시 계층 분할.
// 게임플레이(실제 리필/페이즈 전환)로의 승격은 Phase 11에서
// bSegmentBroken 신호를 트리거로 물려받아 처리합니다.
// ─────────────────────────────────────────────────────────────

struct FSaytSegmentedHealthState
{
	// 현재 진행 중인 세그먼트를 제외한, 남아 있는 구슬 수 [0, SegmentCount-1]
	int32 RemainingOrbs = 0;

	// 현재 세그먼트 내 체력 비율 [0, 1]
	float SegmentPercent = 0.f;

	// 직전 상태 대비 세그먼트가 하나 이상 깨졌는가 (Previous를 준 오버로드에서만 설정)
	bool bSegmentBroken = false;
};

class FSaytSegmentedHealth
{
public:
	// 순수 스냅숏 계산. 경계값(Health가 정확히 세그먼트 경계)은
	// '구슬 유지 + 바 100%'로 판정한다 — 바가 0%로 보이는 순간은
	// 깨지는 순간뿐이어야 한다는 게임 UI 관례.
	static FSaytSegmentedHealthState Calculate(float Health, float MaxHealth, int32 SegmentCount);

	// 직전 상태를 받아 bSegmentBroken(구슬 수 감소 감지)까지 채우는 오버로드.
	static FSaytSegmentedHealthState Calculate(float Health, float MaxHealth, int32 SegmentCount,
		const FSaytSegmentedHealthState& Previous);
};