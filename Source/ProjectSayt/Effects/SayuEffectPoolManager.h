// Copyright (c) 2026 Sungmin JI. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SayuEffectPoolManager.generated.h"

class ASayuPooledEffect;

// 타격 이펙트 액터를 미리 여러 개 만들어두고 빌려주는 풀 매니저.
// Phase 4에서 Subsystem으로 전환할 걸 염두에 두고, 풀링 로직을
// 이 클래스 안에 최대한 캡슐화해서 나중에 그대로 옮기기 쉽게 함.
UCLASS()
class PROJECTSAYT_API ASayuEffectPoolManager : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ASayuEffectPoolManager();
	
	// 지정한 위치/방향에서 타격 이펙트 재생.
	// 풀에 빈 인스턴스가 없으면 새로 하나 더 만들어서 씀(경고 로그 남김).
	void PlayEffectAtLocation(const FVector& Location, const FRotator& Rotation, float PitchMultiplier = 1.f);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	// 풀에 쓸 이펙트 클래스 (블루프린트 자식으로 설정할 것)
	UPROPERTY(EditDefaultsOnly, Category = "Pool")
	TSubclassOf<ASayuPooledEffect> EffectClass;

	// 시작할 때 미리 만들어둘 개수
	UPROPERTY(EditDefaultsOnly, Category = "Pool")
	int32 InitialPoolSize = 5;

private:
	UPROPERTY()
	TArray<TObjectPtr<ASayuPooledEffect>> Pool;

	ASayuPooledEffect* SpawnPooledEffect();
};
