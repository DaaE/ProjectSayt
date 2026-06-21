// Fill out your copyright notice in the Description page of Project Settings.


#include "SayuAbility_BasicAttack.h"
#include "AbilitySystemComponent.h"
#include "GameplayEffect.h"
#include "GameplayTagsManager.h"
#include "../Attributes/SayuAttributeSet.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"

USayuAbility_BasicAttack::USayuAbility_BasicAttack()
{
	// InstancingPolicy는 부모 생성자에서 이미 설정됨

	// 이 어빌리티가 활성화되는 동안 부여할 태그
	// "지금 공격 중이다" 라는 상태를 GameplayTag로 표현
	// Unity에서 bool isAttacking 변수로 했을 일을
	// GAS는 태그 시스템으로 표준화함
	// (Phase 0에서 만든 DefaultGameplayTags.ini의 Ability.Attack.Light 사용)
	
	// AbilityTags : 이 어빌리티 자신을 식별하는 태그
	// TryActivateAbilitiesByTag가 바로 이 태그를 보고 어빌리티를 찾아냄
	FGameplayTagContainer NewTags;
	NewTags.AddTag(FGameplayTag::RequestGameplayTag(FName("Ability.Attack.Light")));

	SetAssetTags(NewTags);
	// SetAssetTags() : UE5.3+ 에서 권장하는 방식
	// (예전 버전은 AbilityTags 멤버에 직접 접근했는데, 최신 API로 변경됨)
	
	ActivationOwnedTags.AddTag(FGameplayTag::RequestGameplayTag(FName("Ability.Attack.Light")));
}

void USayuAbility_BasicAttack::ActivateAbility(
	const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, 
	const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	// 아직 Super를 호출하지 않음 - 먼저 조건 재확인

	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
		// CommitAbility : 코스트(마나 소모)와 쿨타임을 "실제로 소비"하는 함수
		// CanActivateAbility는 "가능한지 확인"만 하고, CommitAbility가
		// 실제 차감을 실행해요. 이 둘이 분리된 이유는 멀티플레이어에서
		// "확인"과 "실행" 사이에 다른 어빌리티가 끼어들 수 있어서예요.
	{
		//UE_LOG(LogTemp, Warning, TEXT("BasicAttack Commit Failed - On Cooldown"));
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}
	// 콤보 윈도우 Open 리스너
	FGameplayTag WindowOpenTag =
		FGameplayTag::RequestGameplayTag(FName("Event.Combo.WindowOpen"));
	WaitWindowOpenTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, WindowOpenTag, nullptr, false, false);
	WaitWindowOpenTask->EventReceived.AddDynamic(this, &USayuAbility_BasicAttack::OnComboWindowOpenEvent);
	WaitWindowOpenTask->ReadyForActivation();

	// 콤보 윈도우 Close 리스너
	FGameplayTag WindowCloseTag =
		FGameplayTag::RequestGameplayTag(FName("Event.Combo.WindowClose"));
	WaitWindowCloseTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, WindowCloseTag, nullptr, false, false);
	WaitWindowCloseTask->EventReceived.AddDynamic(this, &USayuAbility_BasicAttack::OnComboWindowCloseEvent);
	WaitWindowCloseTask->ReadyForActivation();

	// 콤보 입력 리스너 (콤보 생애 동안 계속 수신)
	FGameplayTag ComboInputTag =
		FGameplayTag::RequestGameplayTag(FName("Event.Combo.Input"));
	WaitComboInputTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, ComboInputTag, nullptr, false, false);
	WaitComboInputTask->EventReceived.AddDynamic(this, &USayuAbility_BasicAttack::OnComboInputEvent);
	WaitComboInputTask->ReadyForActivation();

	// 1타 시작 - 이후 콤보 진행 전부 PlayComboStep이 전담
	PlayComboStep(0);
}

void USayuAbility_BasicAttack::EndAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility, bool bWasCancelled)
{
	// Task 정리 - 어빌리티가 끝날 때 미완료 Task들을 확실히 종료
	if (WaitWindowOpenTask)
	{
		WaitWindowOpenTask->EndTask();
		WaitWindowOpenTask = nullptr;
	}

	if (WaitWindowCloseTask)
	{
		WaitWindowCloseTask->EndTask();
		WaitWindowCloseTask = nullptr;
	}
	
	if (WaitComboInputTask)
	{
		WaitComboInputTask->EndTask();
		WaitComboInputTask = nullptr;
	}
	
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
	// Super 호출이 중요해요. 부모(UGameplayAbility)의 EndAbility가
	// 내부적으로 정리 작업(태그 제거, 인스턴스 정리 등)을 하기 때문이에요.
}

UGameplayEffect* USayuAbility_BasicAttack::GetCooldownGameplayEffect() const
{
	if (CooldownEffectClass)
	{
		// GetDefaultObject : 이 클래스의 CDO를 가져옴
		// GameplayEffect는 인스턴스화 없이 CDO 자체를 "명세"로 사용해요
		return CooldownEffectClass->GetDefaultObject<UGameplayEffect>();
	}
	return nullptr;
}

void USayuAbility_BasicAttack::OnMontageCompleted()
{
	UE_LOG(LogTemp, Warning, TEXT("Montage Completed - Normal End"));
	// "정상 종료" - 여기서 쿨타임을 정식으로 시작해도 됨
	EndAbility(GetCurrentAbilitySpecHandle(), CurrentActorInfo, 
		GetCurrentActivationInfo(), true, false);
}

void USayuAbility_BasicAttack::OnMontageInterrupted()
{
	UE_LOG(LogTemp, Warning, TEXT("Montage Interrupted - Cancelled"));
	// "비정상 캔슬" - 다른 어빌리티가 끼어들어 중단된 경우
	EndAbility(GetCurrentAbilitySpecHandle(), CurrentActorInfo,
		GetCurrentActivationInfo(), true, true);
}

void USayuAbility_BasicAttack::OnMontageCancelled()
{
	UE_LOG(LogTemp, Warning, TEXT("Montage Cancelled"));
	EndAbility(GetCurrentAbilitySpecHandle(), CurrentActorInfo,
		GetCurrentActivationInfo(), true, true);
}

void USayuAbility_BasicAttack::OnComboWindowOpenEvent(FGameplayEventData Payload)
{
	UE_LOG(LogTemp, Warning, TEXT("Combo Window OPEN event received"));

	if (ComboWindowEffectClass)
	{
		FGameplayEffectSpecHandle WindowSpec =
			MakeOutgoingGameplayEffectSpec(ComboWindowEffectClass);
		// MakeOutgoingGameplayEffectSpec : GameplayAbility 안에서 쓰는 헬퍼 함수
		// Character에서 직접 했던 MakeEffectContext + MakeOutgoingSpec을
		// 한 줄로 줄여줌 (어빌리티 컨텍스트 안에서는 더 간결하게 쓸 수 있음)

		if (WindowSpec.IsValid())
		{
			ApplyGameplayEffectSpecToOwner(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo,WindowSpec);
			// ActivateAbility 매개변수 대신 Current* 멤버를 씀
			// (콜백 함수는 ActivateAbility의 지역 변수에 접근할 수 없어서
			//  UGameplayAbility가 제공하는 현재 상태 멤버를 사용해요)
		}
	}
}

void USayuAbility_BasicAttack::OnComboWindowCloseEvent(FGameplayEventData Payload)
{
	UE_LOG(LogTemp, Warning, TEXT("Combo Window CLOSE event received"));

	if (UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo())
	{
		FGameplayTagContainer TagsToRemove;
		TagsToRemove.AddTag(FGameplayTag::RequestGameplayTag(FName("Combo.Window.Open")));

		ASC->RemoveActiveEffectsWithGrantedTags(TagsToRemove);
		// RemoveActiveEffectsWithGrantedTags : 특정 태그를 부여하는
		// 모든 활성 Effect를 찾아서 제거. GE_ComboWindow가 정확히
		// Combo.Window.Open 태그를 부여하고 있으니, 이 호출로
		// 그 Effect가 제거되면서 태그도 같이 사라져요
	}
}

void USayuAbility_BasicAttack::OnComboInputEvent(FGameplayEventData Payload)
{
	UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
	if (!ASC) return;

	const FGameplayTag ComboWindowTag = FGameplayTag::RequestGameplayTag(FName("Combo.Window.Open"));
	if (!ASC->HasMatchingGameplayTag(ComboWindowTag))
	{
		return; // 윈도우 밖 입력은 드롭 (추후 QTE형 버퍼링 확장 지점)
	}

	const int32 NextIndex = CurrentComboIndex + 1;
	if (ComboMontages.IsValidIndex(NextIndex))
	{
		PlayComboStep(NextIndex);
	}
}

void USayuAbility_BasicAttack::PlayComboStep(int32 Index)
{
	CurrentComboIndex = Index;
	UE_LOG(LogTemp, Warning, TEXT("Combo Hit: %d"), Index + 1);

	if (ComboIncrementEffectClass)
	{
		// 여기서 "Data.ComboCount 태그로 표시된 슬롯에 CurrentCombo 값을 넣어라"
		// 라고 동적으로 채워주는 거예요. 에디터에서 만든 GE_ComboIncrement가
		// 이 값을 가져다가 ComboCount Attribute에 Override 연산을 적용해요
		FGameplayEffectSpecHandle ComboSpec = MakeOutgoingGameplayEffectSpec(ComboIncrementEffectClass);
		if (ComboSpec.IsValid())
		{
			ComboSpec.Data->SetSetByCallerMagnitude(
				FGameplayTag::RequestGameplayTag(FName("Data.ComboCount")), static_cast<float>(Index + 1));
			ApplyGameplayEffectSpecToOwner(GetCurrentAbilitySpecHandle(), CurrentActorInfo, GetCurrentActivationInfo(), ComboSpec);
		}
	}

	// MakeOutgoingGameplayEffectSpec : GameplayAbility 안에서 쓰는 헬퍼 함수
	// Character에서 직접 했던 MakeEffectContext + MakeOutgoingSpec을
	// 한 줄로 줄여줌 (어빌리티 컨텍스트 안에서는 더 간결하게 쓸 수 있음)
	if (DamageEffectClass) // 배율 SetByCaller 연동은 여전히 미완성 (기존 메모 유지)
	{
		FGameplayEffectSpecHandle EffectSpec = MakeOutgoingGameplayEffectSpec(DamageEffectClass);
		if (EffectSpec.IsValid())
		{
			// SetSetByCallerMagnitude : Effect 적용 시점에 외부에서
			// 수치를 동적으로 주입하는 방법
			// 우리 MMC는 AttackPower를 읽지만, 거기에 배율을 곱해주려면
			// Spec 단계에서 추가 파라미터를 넘겨야 해요.
			// 이건 다음 단계에서 MMC를 살짝 손볼 때 같이 설명할게요.
			// 지금은 일단 Spec까지만 만들고 적용
			ApplyGameplayEffectSpecToOwner(GetCurrentAbilitySpecHandle(), CurrentActorInfo, GetCurrentActivationInfo(), EffectSpec);
			// ApplyGameplayEffectSpecToOwner : 자기 자신에게 적용
			// (테스트 단계라 Self 적용. 나중에 적 타겟팅으로 바꿀 예정)
		}
	}

	// === 애니메이션 재생 (Montage Task) ===
	if (MontageTask) // 핵심: 전환 직전 이전 Task를 명시적으로 정리해야 false Interrupt 콜백을 막음
	{
		MontageTask->EndTask();
		MontageTask = nullptr;
	}

	UAnimMontage* MontageToPlay = ComboMontages.IsValidIndex(Index) ? ComboMontages[Index] : nullptr;
	if (!MontageToPlay)
	{
		EndAbility(GetCurrentAbilitySpecHandle(), CurrentActorInfo, GetCurrentActivationInfo(), true, false);
		return;
	}

	MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, NAME_None, MontageToPlay);
	MontageTask->OnCompleted.AddDynamic(this, &USayuAbility_BasicAttack::OnMontageCompleted);
	MontageTask->OnInterrupted.AddDynamic(this, &USayuAbility_BasicAttack::OnMontageInterrupted);
	MontageTask->OnCancelled.AddDynamic(this, &USayuAbility_BasicAttack::OnMontageCancelled);
	MontageTask->ReadyForActivation();
}
