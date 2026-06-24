// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "SayuItemIdSubsystem.generated.h"

/**
 * 아이템 인스턴스에 고유 ID를 발급하는 책임만 갖는 Subsystem.
 * 실무에서는 서버가 발급 권한을 갖지만, 지금은 싱글플레이 스코프라
 * 순번 증가 방식으로 단순화함.
 * (참고: Phase 9 멀티플레이어 작업 시, 이 발급 권한이 서버에만 있어야
 * 한다는 점을 다시 다뤄야 할 수 있음 — ASC의 PlayerState 이전과 같은 결의 이슈.)
 */
UCLASS()
class PROJECTGAS_API USayuItemIdSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	int32 IssueNextItemInstanceId();

private:
	int32 NextItemInstanceId = 0;
};