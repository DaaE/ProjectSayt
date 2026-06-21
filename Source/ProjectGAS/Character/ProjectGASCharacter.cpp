// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectGASCharacter.h"
#include "AbilitySystemComponent.h"
#include "../AbilitySystem/Attributes/ProjectGASAttributeSet.h"

// Enhanced Input 관련 헤더
// 구현 파일(.cpp)에서는 전방 선언 대신 실제 헤더를 include 해야 함
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"

// 카메라 관련
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"

// 이동 관련
#include "GameFramework/CharacterMovementComponent.h"


#include "Blueprint/UserWidget.h"
#include "AbilitySystemBlueprintLibrary.h"

// Sets default values
AProjectGASCharacter::AProjectGASCharacter()
// 생성자 - C#의 Awake() 또는 생성자와 같음
// 컴포넌트 생성과 기본값 설정은 여기서
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	// Tick(Update)를 활성화할지 여부
	// C#에서는 Update()가 항상 호출됐지만
	// UE에서는 성능을 위해 필요한 것만 true로 설정
	
	// === 카메라 붐 생성 ===
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	// CreateDefaultSubobject : 생성자에서만 쓰는 컴포넌트 생성 함수
	// C#의 AddComponent<T>() 와 비슷하지만 생성자 전용
	// TEXT() : FString/FName용 문자열 매크로, 유니코드 처리해줌
	
	CameraBoom->SetupAttachment(RootComponent);
	// RootComponent에 붙임 (캡슐 콜라이더가 루트)
	// C#의 transform.SetParent() 와 비슷

	CameraBoom->TargetArmLength = 400.f;
	// 카메라와 캐릭터 사이 거리 (cm 단위, UE는 cm 기준)

	CameraBoom->bUsePawnControlRotation = true;
	// 마우스 회전에 따라 카메라 붐도 회전
	
	// === 카메라 생성 ===
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	// 카메라 붐 끝에 붙임
	
	FollowCamera->bUsePawnControlRotation = false;
	// 카메라 자체는 회전 안 함 (붐이 회전하니까)
	
	// === 캐릭터 회전 설정 ===
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;
	// 컨트롤러(마우스) 회전이 캐릭터 몸통에 직접 영향 안 주게
	
	GetCharacterMovement()->bOrientRotationToMovement = true;
	// 이동 방향으로 캐릭터가 자동으로 회전 (RPG 스타일)
	GetCharacterMovement()->RotationRate = FRotator(0.f, 500.f, 0.f);
	// 회전 속도
	
	AttributeSet = CreateDefaultSubobject<UProjectGASAttributeSet>(TEXT("AttributeSet"));
	// AttributeSet은 Actor 컴포넌트가 아니라 UObject라서 좀 특이하게
	// CreateDefaultSubobject로 만들지만 SetupAttachment는 필요 없어요
	// (계층 구조가 아니라 ASC 내부 데이터로 등록될 거라서)
}

void AProjectGASCharacter::OnBasicAttackInput(const struct FInputActionValue& Value)
{
	if (!AbilitySystemComponent)
	{
		return;;
	}
	
	const FGameplayTag LightAttackTag = FGameplayTag::RequestGameplayTag(FName("Ability.Attack.Light"));
	
	if (AbilitySystemComponent->HasMatchingGameplayTag(LightAttackTag))
	{
		// 어빌리티가 이미 활성 중 → 재발동 시도 안 함, 신호만 전달
		FGameplayEventData EventData;
		UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(
			this, FGameplayTag::RequestGameplayTag(FName("Event.Combo.Input")), EventData);
	}
	else
	{
		FGameplayTagContainer TagContainer;
		// FGameplayTagContainer : 태그를 "여러 개" 담는 컨테이너
		// 지금은 1개만 넣지만, 컨테이너로 받는 이유는 나중에
		// "이 중 하나라도 매칭되면 발동" 같은 OR 조건 확장이 가능해서예요

		TagContainer.AddTag(FGameplayTag::RequestGameplayTag(FName("Ability.Attack.Light")));
				
		// "Try"가 붙은 이유: CanActivateAbility 체크를 통과해야만
		// 실제 발동되기 때문이에요 (쿨타임/코스트 등 자동 체크)
		AbilitySystemComponent->TryActivateAbilitiesByTag(TagContainer);
		// 클래스를 직접 안 적었죠? 태그만 보고 ASC가 보유한 어빌리티 중에서
		// 해당 태그를 가진 걸 찾아서 발동시켜요
	}
}

void AProjectGASCharacter::OnBuffInput(const struct FInputActionValue& Value)
{
	if (AbilitySystemComponent)
	{
		FGameplayTagContainer TagContainer;
		TagContainer.AddTag(FGameplayTag::RequestGameplayTag(FName("Ability.Skill.Dash")));

		AbilitySystemComponent->TryActivateAbilitiesByTag(TagContainer);
	}
}

// Called when the game starts or when spawned
void AProjectGASCharacter::BeginPlay()
{
	Super::BeginPlay();
	// C#의 base.Start() - 부모 클래스 BeginPlay 먼저 호출
	// UE에서는 Super:: 호출을 빠뜨리면 부모 초기화가 안 돼서
	// 예측 불가한 버그가 생길 수 있음. 습관적으로 항상 호출할 것
	
	// === Enhanced Input 컨텍스트 등록 ===
	if (APlayerController* PC = Cast<APlayerController>(GetController()))
		// Cast<T> : C#의 as 연산자와 같음
		// APlayerController* PC = GetController() as APlayerController; 과 동일
		// 실패 시 nullptr 반환 (C#의 null 반환과 같음)
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
			ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(
				PC->GetLocalPlayer()))
			// Subsystem에서 Enhanced Input 가져오는 방법
				// Subsystem 개념은 Phase 4에서 자세히 다룰 예정
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
			// 0 = 우선순위. 숫자 낮을수록 우선순위 높음
			// 여러 컨텍스트를 상황에 따라 교체하는 게 Enhanced Input의 핵심
		}
		
		if (HUDWidgetClass)
		{
			HUDWidgetInstance = CreateWidget<UUserWidget>(PC, HUDWidgetClass);
			if (HUDWidgetInstance)
			{
				HUDWidgetInstance->AddToViewport();
				// AddToViewport : 이 위젯을 실제 화면에 그리기 시작
			}
		}
	}
	
}

// Called to bind functionality to input
void AProjectGASCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	// UInputComponent를 UEnhancedInputComponent로 캐스팅
	if (UEnhancedInputComponent* EIC =
		Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		// IA를 함수에 바인딩
		// ETriggerEvent::Triggered : 키 누르는 동안 매 프레임 호출
		// ETriggerEvent::Started   : 키 누른 순간 한 번
		// ETriggerEvent::Completed : 키 뗀 순간 한 번
		EIC->BindAction(MoveAction, ETriggerEvent::Triggered,
			this, &AProjectGASCharacter::Move);

		EIC->BindAction(LookAction, ETriggerEvent::Triggered,
			this, &AProjectGASCharacter::Look);

		EIC->BindAction(JumpAction, ETriggerEvent::Started,
			this, &ACharacter::Jump);
		// Jump는 ACharacter에 이미 구현되어 있어서 직접 바인딩 가능

		EIC->BindAction(JumpAction, ETriggerEvent::Completed,
			this, &ACharacter::StopJumping);
		
		EIC->BindAction(BasicAttackAction, ETriggerEvent::Started,
			this, &AProjectGASCharacter::OnBasicAttackInput);
		
		EIC->BindAction(BuffAction, ETriggerEvent::Started,
			this, &AProjectGASCharacter::OnBuffInput);
	}
}

void AProjectGASCharacter::Move(const FInputActionValue& Value)
{
	FVector2D MovementVector = Value.Get<FVector2D>();
	// FInputActionValue에서 Vector2D 타입으로 꺼냄
	// IA 에셋에서 Value Type을 Axis2D로 설정했을 때 사용

	if (Controller != nullptr)
	{
		// 컨트롤러(카메라)가 바라보는 방향 기준으로 이동
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);
		// Yaw = 좌우 회전값만 추출 (위아래 기울기 제거)

		const FVector ForwardDirection =
			FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		const FVector RightDirection =
			FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		// 카메라 기준 앞/옆 방향 벡터 계산

		AddMovementInput(ForwardDirection, MovementVector.Y);
		AddMovementInput(RightDirection, MovementVector.X);
		// AddMovementInput : CharacterMovementComponent에 이동 의도를 전달
		// 실제 이동은 CharacterMovementComponent가 처리
	}
}

void AProjectGASCharacter::Look(const FInputActionValue& Value)
{
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		AddControllerYawInput(LookAxisVector.X);    // 좌우 회전
		AddControllerPitchInput(LookAxisVector.Y);  // 상하 회전
	}
}

// Called every frame
void AProjectGASCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	// 지금은 비어있음. 나중에 필요하면 채울 것
}
