// Copyright (c) 2026 Sungmin JI. All Rights Reserved.


#include "SayuAbility_BasicAttack.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "GameplayEffect.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "Effects/SayuEffectPoolManager.h"
#include "Kismet/GameplayStatics.h"
#include "SayuLogChannels.h"

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
	
	// 무기 판정 시작 리스너 - 몬타주 NotifyState의 Begin에서 신호를 보냄
	FGameplayTag TraceStartTag =
		FGameplayTag::RequestGameplayTag(FName("Event.Combat.TraceStart"));
	WaitTraceStartTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, TraceStartTag, nullptr, false, false);
	WaitTraceStartTask->EventReceived.AddDynamic(
		this, &USayuAbility_BasicAttack::OnTraceStartEvent);
	WaitTraceStartTask->ReadyForActivation();

	// 무기 판정 종료 리스너 - 몬타주 NotifyState의 End에서 신호를 보냄
	FGameplayTag TraceEndTag = FGameplayTag::RequestGameplayTag(FName("Event.Combat.TraceEnd"));
	WaitTraceEndTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, TraceEndTag, nullptr, false, false);
	WaitTraceEndTask->EventReceived.AddDynamic(
		this, &USayuAbility_BasicAttack::OnTraceEndEvent);
	WaitTraceEndTask->ReadyForActivation();
	
	FGameplayTag AttackVoiceTag = FGameplayTag::RequestGameplayTag(FName("Event.Combat.AttackVoice"));
	WaitAttackVoiceTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, AttackVoiceTag, nullptr, false, false);
	WaitAttackVoiceTask->EventReceived.AddDynamic(this, &USayuAbility_BasicAttack::OnAttackVoiceEvent);
	WaitAttackVoiceTask->ReadyForActivation();

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
	
	if (WaitTraceStartTask)
	{
		WaitTraceStartTask->EndTask(); 
		WaitTraceStartTask = nullptr;
	}
	
	if (WaitTraceEndTask)
	{
		WaitTraceEndTask->EndTask(); 
		WaitTraceEndTask = nullptr;
	}
	
	if (WeaponTraceTask)
	{
		WeaponTraceTask->EndTask(); 
		WeaponTraceTask = nullptr;
	}
	
	if (WaitAttackVoiceTask)
	{
		WaitAttackVoiceTask->EndTask(); 
		WaitAttackVoiceTask = nullptr;
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
	UE_LOG(LogSayuCombat, Verbose, TEXT("Montage Completed - Normal End"));
	// "정상 종료" - 여기서 쿨타임을 정식으로 시작해도 됨
	EndAbility(GetCurrentAbilitySpecHandle(), CurrentActorInfo, 
		GetCurrentActivationInfo(), true, false);
}

void USayuAbility_BasicAttack::OnMontageInterrupted()
{
	UE_LOG(LogSayuCombat, Verbose, TEXT("Montage Interrupted - Cancelled"));
	// "비정상 캔슬" - 다른 어빌리티가 끼어들어 중단된 경우
	EndAbility(GetCurrentAbilitySpecHandle(), CurrentActorInfo,
		GetCurrentActivationInfo(), true, true);
}

void USayuAbility_BasicAttack::OnMontageCancelled()
{
	UE_LOG(LogSayuCombat, Verbose, TEXT("Montage Cancelled"));
	EndAbility(GetCurrentAbilitySpecHandle(), CurrentActorInfo,
		GetCurrentActivationInfo(), true, true);
}

void USayuAbility_BasicAttack::OnComboWindowOpenEvent(FGameplayEventData Payload)
{
	UE_LOG(LogSayuCombat, Verbose, TEXT("Combo Window OPEN event received"));

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
	UE_LOG(LogSayuCombat, Verbose, TEXT("Combo Window CLOSE event received"));

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
	UE_LOG(LogSayuCombat, Verbose, TEXT("Combo Hit: %d"), Index + 1);

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

void USayuAbility_BasicAttack::OnTraceStartEvent(FGameplayEventData Payload)
{
	UE_LOG(LogSayuCombat, Verbose, TEXT("[Combo %d] TraceStart - Task 생성"), CurrentComboIndex);
	
	// 혹시 직전 Task가 안 끝나고 남아있으면 정리 (안전장치)
	if (WeaponTraceTask)
	{
		UE_LOG(LogSayuCombat, Verbose, TEXT("[Combo %d] 이전 Task 정리"), CurrentComboIndex);
		WeaponTraceTask->EndTask();
	}

	WeaponTraceTask = UAbilityTask_WeaponTrace::WeaponTrace(
		this, WeaponTraceSockets, WeaponTraceRadius);
	WeaponTraceTask->OnHit.AddDynamic(
		this, &USayuAbility_BasicAttack::OnWeaponHitActor);
	WeaponTraceTask->ReadyForActivation();
}

void USayuAbility_BasicAttack::OnTraceEndEvent(FGameplayEventData Payload)
{
	UE_LOG(LogSayuCombat, Verbose, TEXT("[Combo %d] TraceEnd - Task 종료"), CurrentComboIndex);
	
	if (WeaponTraceTask)
	{
		WeaponTraceTask->EndTask();
		WeaponTraceTask = nullptr;
	}
}

void USayuAbility_BasicAttack::OnWeaponHitActor(AActor* HitActor, FHitResult Hit)
{
	UE_LOG(LogSayuCombat, Verbose, TEXT("[Combo %d] OnWeaponHitActor 호출: %s"),
		CurrentComboIndex, HitActor ? *HitActor->GetName() : TEXT("null"));
	
	if (!HitActor || !DamageEffectClass)
	{
		return;
	}

	// 자기 자신이 아니라, 실제로 맞은 대상의 ASC를 찾아서 거기에 적용
	UAbilitySystemComponent* TargetASC =
		UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(HitActor);
	if (!TargetASC)
	{
		return;
	}

	FGameplayEffectSpecHandle EffectSpec = MakeOutgoingGameplayEffectSpec(DamageEffectClass);
	if (EffectSpec.IsValid())
	{
		// 검이 실제로 닿은 정확한 지점을 EffectContext에 같이 실어줍니다.
		// GAS가 기본 제공하는 자리라 새로 만들 필요 없이 호출만 하면 됩니다.
		EffectSpec.Data->GetContext().AddHitResult(Hit);
		
		// 콤보 타수별 데미지 배율 - 배열 범위 밖이면 기본값 1.0
		float Multiplier = 1.0f;
		if (ComboDamageMultipliers.IsValidIndex(CurrentComboIndex))
		{
			Multiplier = ComboDamageMultipliers[CurrentComboIndex];
		}
		
		UE_LOG(LogSayuCombat, Verbose, TEXT("[Combo %d] Multiplier: %.2f, ComboDamageMultipliers.Num(): %d"),
		CurrentComboIndex, Multiplier, ComboDamageMultipliers.Num());

		EffectSpec.Data->SetSetByCallerMagnitude(FGameplayTag::RequestGameplayTag(FName("Data.DamageMultiplier")),
			Multiplier);
		
		// ApplyGameplayEffectSpecToTarget : ToOwner와 달리, 지정한
		// 다른 ASC(여기선 맞은 대상)에게 이펙트를 적용함
		GetAbilitySystemComponentFromActorInfo()->ApplyGameplayEffectSpecToTarget(*EffectSpec.Data.Get(), TargetASC);
	}
}

void USayuAbility_BasicAttack::OnAttackVoiceEvent(FGameplayEventData Payload)
{
	AActor* Avatar = GetAvatarActorFromActorInfo();
	if (!Avatar)
	{
		return;
	}

	// 타격 위치가 아니라 본인(공격자) 위치에서 재생 - 기합 소리니까
	if (ASayuEffectPoolManager* PoolManager = Cast<ASayuEffectPoolManager>(
		UGameplayStatics::GetActorOfClass(
			GetWorld(), ASayuEffectPoolManager::StaticClass())))
	{
		PoolManager->PlayEffectAtLocation(
			Avatar->GetActorLocation(), FRotator::ZeroRotator);
	}
}
