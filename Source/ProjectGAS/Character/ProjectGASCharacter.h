// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
// #pragma once : 이 헤더가 여러 번 include 되어도 한 번만 처리하라는 지시어
// C#에는 이런 게 필요 없었는데, C++은 헤더를 여러 파일에서 include하다보면
// 중복 선언 오류가 날 수 있어서 반드시 필요함

#include "CoreMinimal.h"
// UE의 가장 기본적인 타입들 (FString, TArray 등) 을 포함하는 헤더
// 거의 모든 UE C++ 파일 맨 위에 있어야 함

#include "GameFramework/Character.h"
// ACharacter 클래스 - 이동, 점프, 중력 등 기본 캐릭터 기능 내장
// Unity의 CharacterController 컴포넌트랑 비슷한 개념

#include "ProjectGASCharacter.generated.h"
// UE 리플렉션 시스템이 자동 생성하는 코드를 포함
// 반드시 마지막 #include 여야 함 - 순서 틀리면 컴파일 오류

// 전방 선언 (Forward Declaration)
// 헤더에서 포인터로만 쓸 클래스는 전방 선언으로 처리
// #include 대신 쓰면 컴파일 속도가 빨라짐
// C#에는 없는 개념 - C++에서는 헤더 include가 컴파일 시간에 직접 영향을 줌
class USpringArmComponent;   // 카메라 붐 (카메라와 캐릭터 사이 거리 조절)
class UCameraComponent;      // 실제 카메라
class UInputMappingContext;  // Enhanced Input - 키 배열표
class UInputAction;          // Enhanced Input - 개별 입력 행동

UCLASS()
// UCLASS() : 이 클래스를 UE 리플렉션 시스템에 등록
// 에디터에서 보이게 하고, GC 관리 받고, 블루프린트 상속 가능하게 함

class PROJECTGAS_API AProjectGASCharacter : public ACharacter
// PROJECTGAS_API : DLL export 매크로, 다른 모듈에서 이 클래스를 쓸 수 있게 함
// 프로젝트명_API 형태로 자동 생성됨
{
	GENERATED_BODY()
	// 리플렉션 코드 자동 삽입 - 클래스 안에 반드시 첫 줄에 있어야 함

public:
	// Sets default values for this character's properties
	AProjectGASCharacter();


public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	// C#의 Update() 와 같음. DeltaTime = Time.deltaTime
		

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	// 입력 바인딩 설정 함수 - Enhanced Input 연결 여기서 함
	
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	// Unity Start() 와 같음
	
	// === 카메라 컴포넌트 ===
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	TObjectPtr<USpringArmComponent> CameraBoom;
	// UPROPERTY() : UE GC에 이 포인터를 알려줌 + 에디터에 노출
	// VisibleAnywhere : 에디터에서 보이지만 수정 불가
	// BlueprintReadOnly : 블루프린트에서 읽기만 가능
	// Category : 에디터 디테일 패널에서 그룹 이름
	// TObjectPtr<T> : UE5에서 권장하는 UObject 포인터 타입
	//                 C#의 일반 참조형처럼 쓰면 됨, 내부적으로 안전성 강화됨
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	TObjectPtr<UCameraComponent> FollowCamera;
	
	// === Enhanced Input 에셋 참조 ===
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputMappingContext> DefaultMappingContext;
	// EditAnywhere : 에디터에서 읽기/쓰기 모두 가능 (에셋 드래그앤드롭 가능)
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> MoveAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> LookAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> JumpAction;
	
private:
	// === 입력 처리 함수 ===
	void Move(const struct FInputActionValue& Value);
	void Look(const struct FInputActionValue& Value);
	// FInputActionValue : Enhanced Input이 넘겨주는 입력 값 컨테이너
	// bool / float / FVector2D / FVector 중 하나를 담고 있음
};
