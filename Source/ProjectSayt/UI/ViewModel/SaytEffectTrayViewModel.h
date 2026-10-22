// Copyright (c) 2026 Sungmin JI. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "MVVMViewModelBase.h"
#include "UI/ViewModel/SaytEffectTrayEntryViewModel.h"
#include "SaytEffectTrayViewModel.generated.h"

class USaytEffectTrayComponent;

/**
 * 버프/디버프 트레이 전체의 ViewModel (Phase 8 Stage 3 파트 B).
 * 실행 위치: 클라이언트 (로컬 표시 계층).
 *
 * 컴포넌트의 OnTrayChanged(인자 없는 네이티브 멀티캐스트)를 구독해 FieldNotify로 바꾼다
 * (Observer → Presentation Model). 블루프린트는 네이티브 델리게이트를 직접 받을 수 없으므로
 * 이 변환이 C++ 쪽 책임이다.
 *
 * 방송마다 컴포넌트의 Entries 전체를 읽되, 키(효과 클래스)로 대조해서
 * - 항목 내부 값 변화는 해당 항목 ViewModel의 필드 통지로,
 * - 항목 추가·제거·순서 변화일 때만 Items 통지로 내보낸다.
 * Items 통지는 위젯 쪽에서 Set Items 호출(전체 배열 전달)로 이어지므로 구성이 바뀔 때만 낸다.
 *
 * 관전 대상을 바꿀 때는 BindToComponent를 다시 호출한다. 같은 효과 클래스의 항목은 재사용된다.
 */
UCLASS(meta = (MVVMAllowedContextCreationType = "Manual"))
class PROJECTSAYT_API USaytEffectTrayViewModel : public UMVVMViewModelBase
{
	GENERATED_BODY()

public:
	/**
	 * 대상 캐릭터의 트레이 컴포넌트를 구독하고 현재 상태를 따라잡는다.
	 * 이미 다른 컴포넌트에 붙어 있으면 먼저 해제하므로 몇 번 불러도 안전하다(멱등).
	 * nullptr을 주면 해제하고 목록을 비운다.
	 */
	void BindToComponent(USaytEffectTrayComponent* InComponent);

	/** 구독만 끊는다. 목록은 그대로 둔다(소유자 소멸 시 정리용). */
	void UnbindFromComponent();

	TArray<TObjectPtr<USaytEffectTrayEntryViewModel>> GetItems() const { return Items; }

private:
	void HandleTrayChanged();

	/** 컴포넌트의 현재 Entries와 대조해 Items를 맞춘다. */
	void Refresh();

private:
	UPROPERTY(BlueprintReadOnly, FieldNotify, Getter, Category = "Sayt|Tray", meta = (AllowPrivateAccess = "true"))
	TArray<TObjectPtr<USaytEffectTrayEntryViewModel>> Items;

	TWeakObjectPtr<USaytEffectTrayComponent> WatchedComponent;
	FDelegateHandle ChangedHandle;
};