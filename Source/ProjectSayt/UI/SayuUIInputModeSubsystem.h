// Copyright (c) 2026 Sungmin JI. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "SayuUIInputModeSubsystem.generated.h"

// 임시 중앙화 — 진짜 CommonUI Activatable Stack을 세우기 전까지의 가교입니다.
// 나중에 GetDesiredInputConfig 기반으로 교체될 예정이라 외부 호출부(PushUIRequest/PopUIRequest)
// 이름만 유지하면 내부 구현을 갈아끼우기 쉽게 만들어뒀습니다.
UCLASS()
class PROJECTSAYT_API USayuUIInputModeSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
	
public:
	// RequesterId — "누가" UI 모드를 원하는지 식별하는 이름입니다 (예: "Inventory", "AltHold").
	// 같은 ID로 두 번 Push해도 집합 특성상 중복 없이 한 번만 들어갑니다.
	void PushUIRequest(FName RequesterId);
	void PopUIRequest(FName RequesterId);

private:
	void ApplyCurrentMode();

	TSet<FName> ActiveRequesters;
};
