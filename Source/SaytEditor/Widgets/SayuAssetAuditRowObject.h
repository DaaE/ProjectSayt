// Copyright (c) 2026 Sungmin JI. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "SayuAssetAuditRowObject.generated.h"

struct FStreamableHandle;

/**
 * AssetManager에 등록된 Primary Asset 하나를 점검한 결과를 담는 UObject.
 * List View는 UObject*만 데이터로 받기 때문에 USTRUCT 대신 이 형태로 둔다.
 * 점검(읍기)뿐 아니라 실제 Bundle Load/Unload까지 이 객체 스스로 수행한다.
 */
UCLASS(BlueprintType)
class SAYTEDITOR_API USayuAssetAuditRowObject : public UObject
{
	GENERATED_BODY()
	
public:
	UPROPERTY(BlueprintReadOnly, Category = "Sayu|AssetManager")
	FName AssetType;

	UPROPERTY(BlueprintReadOnly, Category = "Sayu|AssetManager")
	FName AssetName;

	UPROPERTY(BlueprintReadOnly, Category = "Sayu|AssetManager")
	TArray<FName> Bundles;

	/** Bundle이 하나도 없으면 true — 태깅 누락 신호 */
	UPROPERTY(BlueprintReadOnly, Category = "Sayu|AssetManager")
	bool bHasNoBundles = false;

	/** 실제 Load/Unload 호출에 쓰는 식별자 */
	UPROPERTY(BlueprintReadOnly, Category = "Sayu|AssetManager")
	FPrimaryAssetId AssetId;

	/** 이 에셋이 가진 Bundle 전부를 비동기로 로드 시작 ("UI" 같은 이름을 코드에 하드코딩하지 않음) */
	UFUNCTION(BlueprintCallable, Category = "Sayu|AssetManager")
	void LoadAllBundles();

	/** 로드 요청 해제 */
	UFUNCTION(BlueprintCallable, Category = "Sayu|AssetManager")
	void UnloadAsset();
	
	/** 로드 요청이 완료됐는지 (버튼/텍스트 표시용) */
	UFUNCTION(BlueprintCallable, Category = "Sayu|AssetManager")
	bool IsLoaded() const;

private:
	TSharedPtr<FStreamableHandle> LoadHandle;
};
