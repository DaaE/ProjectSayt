// Copyright (c) 2026 Sungmin JI. All Rights Reserved.


#include "SayuAssetManagerInspectorWidget.h"

#include "SayuAssetAuditRowObject.h"
#include "Engine/AssetManager.h"

TArray<USayuAssetAuditRowObject*> USayuAssetManagerInspectorWidget::GetAssetAuditReport()
{
	TArray<USayuAssetAuditRowObject*> Result;

	if (!UAssetManager::IsInitialized())
	{
		return Result;
	}

	UAssetManager& Manager = UAssetManager::Get();

	// 1) 등록된 모든 Primary Asset Type 조회 — 특정 클래스 하드코딩 없음
	TArray<FPrimaryAssetTypeInfo> TypeInfos;
	Manager.GetPrimaryAssetTypeInfoList(TypeInfos);

	for (const FPrimaryAssetTypeInfo& TypeInfo : TypeInfos)
	{
		// 2) 그 타입에 속한 모든 에셋 Id 조회
		TArray<FPrimaryAssetId> AssetIds;
		Manager.GetPrimaryAssetIdList(TypeInfo.PrimaryAssetType, AssetIds, EAssetManagerFilter::Default);

		for (const FPrimaryAssetId& AssetId : AssetIds)
		{
			USayuAssetAuditRowObject* Row = NewObject<USayuAssetAuditRowObject>(this);
			Row->AssetId = AssetId;
			Row->AssetType = AssetId.PrimaryAssetType.GetName();
			Row->AssetName = AssetId.PrimaryAssetName;

			// 3) 이 에셋이 실제로 어떤 Bundle에 등록돼 있는지 조회
			TArray<FAssetBundleEntry> BundleEntries;
			Manager.GetAssetBundleEntries(AssetId, BundleEntries);

			for (const FAssetBundleEntry& Entry : BundleEntries)
			{
				Row->Bundles.Add(Entry.BundleName);
			}

			Row->bHasNoBundles = (Row->Bundles.Num() == 0);

			Result.Add(Row);
		}
	}

	return Result;
}
