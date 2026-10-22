// Copyright (c) 2026 Sungmin JI. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Containers/ArrayView.h"

class UClass;

/**
 * 트레이 항목 대조 (Phase 8 Stage 3 파트 B).
 * 실행 위치: 양쪽 (월드·UObject 상태에 의존하지 않는 순수 함수).
 *
 * 이전 키 순서열과 새 키 순서열을 비교해 '이전 항목 중 누구를 재사용할 수 있는가'와
 * '구성이 바뀌었는가'만 답한다. ViewModel 객체를 모르므로 월드 없이 입출력만으로 검증할 수 있다.
 */
namespace SaytEffectTray
{
	struct FReconcileResult
	{
		/** NewKeys[i]에 대응하는 이전 항목의 인덱스. 새 항목이면 INDEX_NONE. */
		TArray<int32> OldIndexForNew;

		/**
		 * 키 순서열(항목 추가·제거·순서 변화)이 이전과 다르면 true.
		 * 개수나 이름 같은 항목 내부 값의 변화는 여기에 해당하지 않는다.
		 */
		bool bCompositionChanged = false;
	};

	/** 같은 키가 중복되어도 이전 항목 하나는 한 번만 재사용된다. */
	PROJECTSAYT_API FReconcileResult Reconcile(TConstArrayView<const UClass*> OldKeys, TConstArrayView<const UClass*> NewKeys);
}