// Copyright (c) 2026 Sungmin JI. All Rights Reserved.

#include "UI/ViewModel/SaytEffectTrayReconcile.h"

namespace SaytEffectTray
{
	FReconcileResult Reconcile(TConstArrayView<const UClass*> OldKeys, TConstArrayView<const UClass*> NewKeys)
	{
		FReconcileResult Result;

		// 키 → 이전 인덱스. 중복 키는 첫 등장만 둔다.
		TMap<const UClass*, int32> OldIndexByKey;
		OldIndexByKey.Reserve(OldKeys.Num());
		for (int32 Index = 0; Index < OldKeys.Num(); ++Index)
		{
			OldIndexByKey.FindOrAdd(OldKeys[Index], Index);
		}

		Result.OldIndexForNew.Reserve(NewKeys.Num());
		for (const UClass* Key : NewKeys)
		{
			int32 OldIndex = INDEX_NONE;
			if (const int32* Found = OldIndexByKey.Find(Key))
			{
				OldIndex = *Found;
				// 한 번 쓴 이전 항목은 다시 쓰지 않는다 — 새 쪽에 같은 키가 중복돼도 객체가 공유되지 않는다.
				OldIndexByKey.Remove(Key);
			}
			Result.OldIndexForNew.Add(OldIndex);
		}

		Result.bCompositionChanged = (OldKeys.Num() != NewKeys.Num());
		if (!Result.bCompositionChanged)
		{
			for (int32 Index = 0; Index < OldKeys.Num(); ++Index)
			{
				if (OldKeys[Index] != NewKeys[Index])
				{
					Result.bCompositionChanged = true;
					break;
				}
			}
		}

		return Result;
	}
}