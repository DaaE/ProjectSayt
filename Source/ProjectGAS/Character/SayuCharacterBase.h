// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
// UE의 가장 기본적인 타입들 (FString, TArray 등) 을 포함하는 헤더
// 거의 모든 UE C++ 파일 맨 위에 있어야 함
#include "AbilitySystemInterface.h"
#include "GameFramework/Character.h"
// ACharacter 클래스 - 이동, 점프, 중력 등 기본 캐릭터 기능 내장
// Unity의 CharacterController 컴포넌트랑 비슷한 개념
#include "SayuCharacterBase.generated.h"
// UE 리플렉션 시스템이 자동 생성하는 코드를 포함
// 반드시 마지막 #include 여야 함 - 순서 틀리면 컴파일 오류


// 전방 선언 (Forward Declaration)
// 헤더에서 포인터로만 쓸 클래스는 전방 선언으로 처리
// #include 대신 쓰면 컴파일 속도가 빨라짐
// C#에는 없는 개념 - C++에서는 헤더 include가 컴파일 시간에 직접 영향을 줌
class UAbilitySystemComponent;
class USayuGameplayAbility;
class USayuAttributeSet_Combat;

// 플레이어/적 등 GAS를 쓰는 모든 Character가 공유하는 베이스.
// ASC 생성, 어빌리티 부여, 초기화 타이밍까지 여기서 전담한다.
UCLASS()
class PROJECTGAS_API ASayuCharacterBase : public ACharacter, public IAbilitySystemInterface
// PROJECTGAS_API : DLL export 매크로, 다른 모듈에서 이 클래스를 쓸 수 있게 함
// 프로젝트명_API 형태로 자동 생성됨
// IAbilitySystemInterface : GAS가 "이 액터는 ASC를 갖고 있다"는 걸
// 표준화된 방식으로 알 수 있게 하는 인터페이스예요.
// Unity로 치면 ICombatant 같은 커스텀 인터페이스를 직접 만들어서
// GetHealthComponent() 를 표준화했던 것과 비슷한 개념이에요.
// GAS 생태계 전체가 이 인터페이스를 통해 ASC를 찾아가도록 설계되어 있어요.
{
	GENERATED_BODY()
	// 리플렉션 코드 자동 삽입 - 클래스 안에 반드시 첫 줄에 있어야 함
	
public:
	// Sets default values for this character's properties
	ASayuCharacterBase();
	
	virtual void Tick(float DeltaTime) override;
	
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void PossessedBy(AController* NewController) override;
	// 중요한 함수예요. Character가 Controller에게 "점유(소유)"되는 시점.
	// ASC 초기화는 보통 여기서 해요. BeginPlay보다 이 타이밍이
	// 멀티플레이어에서 더 안전한 시점이에요 (Controller 정보가 확정된 후니까)
	
	// === GAS 컴포넌트 추가 ===
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Abilities")
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;
	
	// 추가 - 플레이어/NPC 공통으로 쓰는 전투 속성 (합성 방식)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Abilities")
	TObjectPtr<USayuAttributeSet_Combat> CombatAttributeSet;

	// 캐릭터가 기본으로 보유할 어빌리티 목록
	// 블루프린트에서 배열에 BP_Ability들을 등록할 거예요
	UPROPERTY(EditDefaultsOnly, Category = "Abilities")
	TArray<TSubclassOf<USayuGameplayAbility>> DefaultAbilities;

	// 게임 시작 시 위 목록을 실제로 ASC에 등록하는 함수
	void GiveDefaultAbilities();
	
	// 임시 디버그용 - Phase 7에서 정식 토글 시스템으로 대체될 예정
	UPROPERTY(EditAnywhere, Category = "Debug")
	bool bShowCollisionDebug = false;

private:
	void InitializeAbilitySystem();

	bool bAbilitySystemInitialized = false;
};
