// Copyright (c) 2026 Sungmin JI. All Rights Reserved.

// ─────────────────────────────────────────────────────────────
// 프로젝트 디버그 콘솔 커맨드 집합소
//
// 이 파일은 헤더가 없습니다 — 아무도 이 파일을 include할 수 없으므로
// 의존 방향이 항상 [디버그 → 도메인] 단방향으로 강제됩니다.
// FAutoConsoleCommand 계열은 파일 스코프 정적 객체의 생성자에서
// 스스로 콘솔에 등록되고 소멸자에서 해제되는 RAII 객체라, 별도의
// 초기화 호출 없이 이 파일이 링크되는 것만으로 커맨드가 살아납니다.
//
// [Phase 12 정리 대상] 아래 커맨드들은 개발 검증용입니다.
// ─────────────────────────────────────────────────────────────

#include "CoreMinimal.h"

#if !UE_BUILD_SHIPPING

#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "Engine/Engine.h"
#include "Engine/GameViewportClient.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "HAL/IConsoleManager.h"
#include "SaytLogChannels.h"
#include "Character/SaytNPCCharacter.h"
#include "Character/SaytCharacter.h"
#include "EngineUtils.h"
#include "UI/Slate/SSaytWorldPanel.h"
#include "UI/SaytIndicatorManagerComponent.h"
#include "Widgets/Colors/SColorBlock.h"
#include "UI/SaytSegmentedHealth.h"
#include "UI/Slate/SSaytHealthBar.h"
#include "UI/Slate/SSaytHealthDisplay.h"
#include "UI/Slate/SSaytOrbTray.h"
#include "UI/SaytHealthDisplayTypes.h"
#include "UI/Slate/SSaytTuningPanel.h"
#include "Widgets/Layout/SBox.h"
#include "GameplayEffect.h"
#include "UI/SaytEffectTrayComponent.h"
#include "UObject/UObjectGlobals.h"


// ═════════════════════════════════════════════════════════════
// Phase 8 Stage 0 — Slate 라이브 튜닝 데모 패널
// ═════════════════════════════════════════════════════════════
namespace SaytTuningDemo
{
	static TSharedPtr<SSaytTuningPanel> ActivePanel;
	static FDelegateHandle WorldCleanupHandle;

	static void OnWorldCleanup(UWorld* World, bool bSessionEnded, bool bCleanupResources)
	{
		// PIE가 끝나면(뷰포트가 사라지면) 우리 포인터도 같이 비워서,
		// 다음 PIE 세션이 '이전 세션의 유령'을 진짜로 살아있다고 착각하지 않게 합니다.
		ActivePanel.Reset();
	}

	static void ToggleTuningDemo()
	{
		if (!GEngine || !GEngine->GameViewport)
		{
			return;
		}

		if (!WorldCleanupHandle.IsValid())
		{
			WorldCleanupHandle = FWorldDelegates::OnWorldCleanup.AddStatic(&OnWorldCleanup);
		}

		if (ActivePanel.IsValid())
		{
			GEngine->GameViewport->RemoveViewportWidgetContent(ActivePanel.ToSharedRef());
			ActivePanel.Reset();
		}
		else
		{
			GEngine->GameViewport->AddViewportWidgetContent(SAssignNew(ActivePanel, SSaytTuningPanel));
		}
	}

	static FAutoConsoleCommand ToggleTuningDemoCommand(
		TEXT("Sayt.ToggleTuningDemo"),
		TEXT("Phase 8 Stage 0 Slate 라이브 튜닝 데모 패널을 켜고 끕니다."),
		FConsoleCommandDelegate::CreateStatic(&ToggleTuningDemo)
	);
}

// ═════════════════════════════════════════════════════════════
// Phase 8 Stage 1 — 체력바 검증 (Stage 2~5 검증에도 계속 사용)
// ═════════════════════════════════════════════════════════════
namespace SaytHealthBarDebug
{
	static TSharedPtr<SWidget> BarRoot;
	static TWeakPtr<SSaytHealthBar> BarWidget;

	static FAutoConsoleCommandWithWorld ShowCmd(
		TEXT("Sayt.HealthBar.Show"),
		TEXT("플레이어 ASC에 바인딩된 SSaytHealthBar를 좌하단에 토글"),
		FConsoleCommandWithWorldDelegate::CreateLambda([](UWorld* World)
		{
			UGameViewportClient* Viewport = World ? World->GetGameViewport() : nullptr;
			if (!Viewport) { return; }

			if (BarRoot.IsValid())
			{
				Viewport->RemoveViewportWidgetContent(BarRoot.ToSharedRef());
				BarRoot.Reset();
				return;
			}

			APlayerController* PC = World->GetFirstPlayerController();
			APawn* Pawn = PC ? PC->GetPawn() : nullptr;
			// 캐릭터 클래스명에 의존하지 않는 표준 ASC 조회 경로 (IAbilitySystemInterface 경유)
			UAbilitySystemComponent* ASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(Pawn);
			if (!ASC) { UE_LOG(LogSaytUI, Warning, TEXT("[HealthBar.Show] 플레이어 ASC를 찾지 못함")); return; }

			TSharedPtr<SSaytHealthBar> Bar;
			BarRoot =
				SNew(SBox).HAlign(HAlign_Left).VAlign(VAlign_Bottom).Padding(60.f, 0.f, 0.f, 60.f)
				[
					SAssignNew(Bar, SSaytHealthBar)
				];
			Viewport->AddViewportWidgetContent(BarRoot.ToSharedRef());
			Bar->BindToASC(ASC);
			BarWidget = Bar;
		})
	);

	// 줄 색 스왑 시뮬레이션: Sayt.HealthBar.Tint <R> <G> <B> (0~1)
	static FAutoConsoleCommandWithWorldAndArgs TintCmd(
		TEXT("Sayt.HealthBar.Tint"),
		TEXT("표시 중인 체력바의 채움 틴트를 변경 (TSlateAttribute 검증용)"),
		FConsoleCommandWithWorldAndArgsDelegate::CreateLambda([](const TArray<FString>& Args, UWorld*)
		{
			TSharedPtr<SSaytHealthBar> Bar = BarWidget.Pin();
			if (!Bar.IsValid() || Args.Num() < 3) { return; }
			Bar->SetFillTint(FLinearColor(
				FCString::Atof(*Args[0]), FCString::Atof(*Args[1]), FCString::Atof(*Args[2])));
		})
	);

	static FDelegateHandle CleanupHandle = FWorldDelegates::OnWorldCleanup.AddLambda(
		[](UWorld*, bool, bool) { BarRoot.Reset(); });
}

// ═════════════════════════════════════════════════════════════
// Phase 8 Stage 2 — 세그먼트 체력 매핑 검증
// ═════════════════════════════════════════════════════════════
namespace SaytSegmentedHealthDebug
{
	static void LogOne(float Health, float MaxHealth, int32 SegmentCount,
		const FSaytSegmentedHealthState* Previous)
	{
		const FSaytSegmentedHealthState S = Previous
			? FSaytSegmentedHealth::Calculate(Health, MaxHealth, SegmentCount, *Previous)
			: FSaytSegmentedHealth::Calculate(Health, MaxHealth, SegmentCount);

		UE_LOG(LogSaytUI, Log, TEXT("[SegHealth] HP %8.2f / %.0f (구슬 %d) → 남은 구슬 %d, 바 %6.2f%%%s"),
			Health, MaxHealth, SegmentCount,
			S.RemainingOrbs, S.SegmentPercent * 100.f,
			S.bSegmentBroken ? TEXT("  ** 세그먼트 파괴 **") : TEXT(""));
	}

	// 인자 없이: 고정 경계 케이스 스위트 + 하강 스윕(파괴 신호 검증)
	// 인자 3개: Sayt.SegHealth.Test <Health> <MaxHealth> <SegmentCount> 단발 계산
	static void RunTest(const TArray<FString>& Args)
	{
		if (Args.Num() >= 3)
		{
			LogOne(FCString::Atof(*Args[0]), FCString::Atof(*Args[1]),
				FCString::Atoi(*Args[2]), nullptr);
			return;
		}

		const float MaxHealth = 3000.f;
		const int32 Orbs = 3;

		UE_LOG(LogSaytUI, Log, TEXT("[SegHealth] ── 경계 케이스 스위트 (3000, 구슬 3) ──"));
		const float Cases[] = { 3000.f, 2500.f, 2000.f, 2000.01f, 1999.9f, 1.f, 0.f, -50.f, 3500.f };
		for (int32 i = 0; i < UE_ARRAY_COUNT(Cases); ++i)
		{
			LogOne(Cases[i], MaxHealth, Orbs, nullptr);
		}

		UE_LOG(LogSaytUI, Log, TEXT("[SegHealth] ── 하강 스윕 (파괴 신호 검증) ──"));
		FSaytSegmentedHealthState Prev;
		Prev = FSaytSegmentedHealth::Calculate(3000.f, MaxHealth, Orbs);
		const float Sweep[] = { 2400.f, 1800.f, 600.f, 0.f };
		for (int32 i = 0; i < UE_ARRAY_COUNT(Sweep); ++i)
		{
			const FSaytSegmentedHealthState Next =
				FSaytSegmentedHealth::Calculate(Sweep[i], MaxHealth, Orbs, Prev);
			LogOne(Sweep[i], MaxHealth, Orbs, &Prev);
			Prev = Next;
		}
	}

	static FAutoConsoleCommand SegHealthTestCommand(
		TEXT("Sayt.SegHealth.Test"),
		TEXT("세그먼트 체력 매핑 검증. 인자 없이 실행하면 경계 스위트, '<HP> <MaxHP> <구슬수>'로 단발 계산."),
		FConsoleCommandWithArgsDelegate::CreateStatic(&RunTest)
	);
}

// ═════════════════════════════════════════════════════════════
// Phase 8 Stage 2-2 — 구슬 트레이 검증
// ═════════════════════════════════════════════════════════════
namespace SaytOrbTrayDebug
{
	static TSharedPtr<SWidget> TrayRoot;
	static TWeakPtr<SSaytOrbTray> TrayWidget;
	static FDelegateHandle WorldCleanupHandle;

	static void OnWorldCleanup(UWorld* World, bool bSessionEnded, bool bCleanupResources)
	{
		TrayRoot.Reset();
		TrayWidget.Reset();
	}

	static void ToggleShow()
	{
		if (!GEngine || !GEngine->GameViewport)
		{
			return;
		}

		if (!WorldCleanupHandle.IsValid())
		{
			WorldCleanupHandle = FWorldDelegates::OnWorldCleanup.AddStatic(&OnWorldCleanup);
		}

		if (TrayRoot.IsValid())
		{
			GEngine->GameViewport->RemoveViewportWidgetContent(TrayRoot.ToSharedRef());
			TrayRoot.Reset();
			TrayWidget.Reset();
		}
		else
		{
			TSharedPtr<SSaytOrbTray> Tray;
			TrayRoot = SNew(SBox)
				.HAlign(HAlign_Center)
				.VAlign(VAlign_Top)
				.Padding(FMargin(0.f, 80.f, 0.f, 0.f))
				[
					SAssignNew(Tray, SSaytOrbTray)
					.OrbCount(4)          // 검증 편의상 4슬롯 (구슬 3개 보스 = 슬롯 2라 좁음)
					.OrbDiameter(22.f)
				];
			TrayWidget = Tray;
			GEngine->GameViewport->AddViewportWidgetContent(TrayRoot.ToSharedRef());
		}
	}

	static void SetRemaining(const TArray<FString>& Args)
	{
		TSharedPtr<SSaytOrbTray> Tray = TrayWidget.Pin();
		if (!Tray.IsValid())
		{
			UE_LOG(LogSaytUI, Warning, TEXT("[OrbTray] 트레이가 없습니다. Sayt.OrbTray.Show 먼저 실행하세요."));
			return;
		}
		
		if (Args.Num() < 1)
		{
			UE_LOG(LogSaytUI, Warning, TEXT("[OrbTray] 사용법: Sayt.OrbTray.Set <남은 구슬 수>"));
			return;
		}
		
		Tray->SetRemainingOrbs(FCString::Atoi(*Args[0]));
	}

	static FAutoConsoleCommand ShowCommand(
		TEXT("Sayt.OrbTray.Show"),
		TEXT("구슬 트레이 검증 위젯을 뷰포트에 켜고 끕니다."),
		FConsoleCommandDelegate::CreateStatic(&ToggleShow)
	);

	static FAutoConsoleCommand SetCommand(
		TEXT("Sayt.OrbTray.Set"),
		TEXT("구슬 트레이의 남은 구슬 수를 설정합니다. 사용법: Sayt.OrbTray.Set <n>"),
		FConsoleCommandWithArgsDelegate::CreateStatic(&SetRemaining)
	);
}

// ═════════════════════════════════════════════════════════════
// Phase 8 Stage 2-3 — 체력 표시 단위(매핑→분배) 검증
// ═════════════════════════════════════════════════════════════
namespace SaytHealthDisplayDebug
{
	static TSharedPtr<SWidget> DisplayRoot;
	static TWeakPtr<SSaytHealthDisplay> DisplayWidget;
	static FDelegateHandle WorldCleanupHandle;

	static constexpr float TestMaxHealth = 4000.f;   // 세그먼트 4 → 구슬 3, 세그먼트당 1000

	static void OnWorldCleanup(UWorld* World, bool bSessionEnded, bool bCleanupResources)
	{
		DisplayRoot.Reset();
		DisplayWidget.Reset();
	}

	static void ToggleShow()
	{
		if (!GEngine || !GEngine->GameViewport)
		{
			return;
		}

		if (!WorldCleanupHandle.IsValid())
		{
			WorldCleanupHandle = FWorldDelegates::OnWorldCleanup.AddStatic(&OnWorldCleanup);
		}

		if (DisplayRoot.IsValid())
		{
			GEngine->GameViewport->RemoveViewportWidgetContent(DisplayRoot.ToSharedRef());
			DisplayRoot.Reset();
			DisplayWidget.Reset();
		}
		else
		{
			TSharedPtr<SSaytHealthDisplay> Display;
			const FSaytHealthDisplayPreset BossPreset = GetSaytHealthDisplayPreset(ESaytHealthDisplayType::Boss);
			DisplayRoot = SNew(SBox)
				.HAlign(HAlign_Center)
				.VAlign(VAlign_Top)
				.Padding(FMargin(0.f, 60.f, 0.f, 0.f))
				[
					SAssignNew(Display, SSaytHealthDisplay)
					.SegmentCount(4)
					.BarStyle(BossPreset.BarStyle)
					.DesiredBarSize(BossPreset.BarSize)
				];
			DisplayWidget = Display;
			GEngine->GameViewport->AddViewportWidgetContent(DisplayRoot.ToSharedRef());

			Display->SetHealthDirect(TestMaxHealth, TestMaxHealth);   // 만피에서 출발
		}
	}

	static void SetHealth(const TArray<FString>& Args)
	{
		TSharedPtr<SSaytHealthDisplay> Display = DisplayWidget.Pin();
		if (!Display.IsValid())
		{
			UE_LOG(LogSaytUI, Warning, TEXT("[HealthDisplay] 표시 단위가 없습니다. Sayt.HealthDisplay.Show 먼저 실행하세요."));
			return;
		}
		if (Args.Num() < 1)
		{
			UE_LOG(LogSaytUI, Warning, TEXT("[HealthDisplay] 사용법: Sayt.HealthDisplay.SetHealth <hp>"));
			return;
		}
		Display->SetHealthDirect(FCString::Atof(*Args[0]), TestMaxHealth);
	}

	static void BindPlayer()
	{
		TSharedPtr<SSaytHealthDisplay> Display = DisplayWidget.Pin();
		if (!Display.IsValid())
		{
			UE_LOG(LogSaytUI, Warning, TEXT("[HealthDisplay] 표시 단위가 없습니다. Sayt.HealthDisplay.Show 먼저 실행하세요."));
			return;
		}

		UWorld* World = (GEngine && GEngine->GameViewport) ? GEngine->GameViewport->GetWorld() : nullptr;
		APlayerController* PC = World ? World->GetFirstPlayerController() : nullptr;
		APawn* Pawn = PC ? PC->GetPawn() : nullptr;
		UAbilitySystemComponent* ASC = Pawn ? UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(Pawn) : nullptr;
		if (!ASC)
		{
			UE_LOG(LogSaytUI, Warning, TEXT("[HealthDisplay] 플레이어 ASC를 찾지 못했습니다."));
			return;
		}

		// 매핑은 비율 기반이라 플레이어의 실제 MaxHealth가 얼마든 세그먼트가 성립한다
		// — 고체력 더미 NPC 없이도 실 ASC 경로(Push+Pull) 검증 가능
		Display->BindToASC(ASC);
		UE_LOG(LogSaytUI, Log, TEXT("[HealthDisplay] 플레이어 ASC에 바인딩 완료"));
	}

	static FAutoConsoleCommand ShowCommand(
		TEXT("Sayt.HealthDisplay.Show"),
		TEXT("세그먼트 체력 표시 단위(바+트레이)를 뷰포트에 켜고 끕니다."),
		FConsoleCommandDelegate::CreateStatic(&ToggleShow)
	);

	static FAutoConsoleCommand SetHealthCommand(
		TEXT("Sayt.HealthDisplay.SetHealth"),
		TEXT("표시 단위에 체력을 직접 주입합니다. 사용법: Sayt.HealthDisplay.SetHealth <hp> (최대 4000 고정)"),
		FConsoleCommandWithArgsDelegate::CreateStatic(&SetHealth)
	);

	static FAutoConsoleCommand BindPlayerCommand(
		TEXT("Sayt.HealthDisplay.BindPlayer"),
		TEXT("표시 단위를 플레이어 ASC에 바인딩합니다 (실 GAS 경로 검증)."),
		FConsoleCommandDelegate::CreateStatic(&BindPlayer)
	);
}

// ═════════════════════════════════════════════════════════════
// Phase 8 Stage 2-5 — 월드→스크린 투영 실험
// ═════════════════════════════════════════════════════════════
namespace SaytProjectionTest
{
	static void RunProjectionTest()
	{
		if (!GEngine || !GEngine->GameViewport)
		{
			return;
		}

		UWorld* World = GEngine->GameViewport->GetWorld();
		if (!World)
		{
			return;
		}

		APlayerController* PC = World->GetFirstPlayerController();
		if (!PC)
		{
			return;
		}

		int32 Count = 0;

		// TActorIterator: 월드의 특정 타입 액터 전체 순회.
		// Unity의 FindObjectsOfType<T>() 대응 — 단, 배열을 만들어 반환하는 게 아니라
		// 순회하면서 하나씩 방문하는 이터레이터라 임시 할당이 없습니다.
		for (TActorIterator<ASaytNPCCharacter> It(World); It; ++It)
		{
			ASaytNPCCharacter* NPC = *It;

			// 머리 위 = 액터 중심 + 캡슐 반높이 + 여유 마진
			const float HalfHeight = NPC->GetSimpleCollisionHalfHeight();
			const FVector HeadLocation = NPC->GetActorLocation() + FVector(0.f, 0.f, HalfHeight + 30.f);

			FVector2D ScreenPos = FVector2D::ZeroVector;
			const bool bInFront = PC->ProjectWorldLocationToScreen(HeadLocation, ScreenPos, true);

			UE_LOG(LogSaytUI, Verbose,
				TEXT("[투영] %s | 머리 월드 %s | 카메라 앞=%s | 스크린 (%.0f, %.0f)"),
				*NPC->GetName(), *HeadLocation.ToCompactString(),
				bInFront ? TEXT("예") : TEXT("아니오"), ScreenPos.X, ScreenPos.Y);

			++Count;
		}

		UE_LOG(LogSaytUI, Verbose, TEXT("[투영] NPC %d개 처리 완료"), Count);
	}

	static FAutoConsoleCommand ProjectionTestCommand(
		TEXT("Sayt.Projection.Test"),
		TEXT("배치된 모든 SaytNPCCharacter의 머리 위치를 스크린 좌표로 투영해 로그 출력"),
		FConsoleCommandDelegate::CreateStatic(&RunProjectionTest));
}

// ═════════════════════════════════════════════════════════════
// Phase 8 Stage 2-6 — 월드 패널 실험 (NPC 머리 위 색 블록)
// ═════════════════════════════════════════════════════════════
namespace SaytWorldPanelTest
{
	static TSharedPtr<SSaytWorldPanel> ActivePanel;
	static FDelegateHandle WorldCleanupHandle;

	static void OnWorldCleanup(UWorld* World, bool bSessionEnded, bool bCleanupResources)
	{
		ActivePanel.Reset();
	}

	static void ToggleShow()
	{
		if (!GEngine || !GEngine->GameViewport)
		{
			return;
		}

		if (!WorldCleanupHandle.IsValid())
		{
			WorldCleanupHandle = FWorldDelegates::OnWorldCleanup.AddStatic(&OnWorldCleanup);
		}

		if (ActivePanel.IsValid())
		{
			GEngine->GameViewport->RemoveViewportWidgetContent(ActivePanel.ToSharedRef());
			ActivePanel.Reset();
			return;
		}

		UWorld* World = GEngine->GameViewport->GetWorld();
		if (!World)
		{
			return;
		}

		APlayerController* PC = World->GetFirstPlayerController();
		if (!PC)
		{
			return;
		}

		ActivePanel = SNew(SSaytWorldPanel, PC);

		static const FLinearColor Colors[] = { FLinearColor::Red, FLinearColor::Green, FLinearColor::Yellow };
		int32 Count = 0;

		for (TActorIterator<ASaytNPCCharacter> It(World); It; ++It)
		{
			ASaytNPCCharacter* NPC = *It;
			
			ActivePanel->AddSlot()
				.TrackedActor(NPC)
				[
					SNew(SBox)
					.WidthOverride(96.f)
					.HeightOverride(12.f)
					[
						SNew(SColorBlock)
						.Color(Colors[Count % 3])
					]
				];

			++Count;
		}

		GEngine->GameViewport->AddViewportWidgetContent(ActivePanel.ToSharedRef(), 10);
		UE_LOG(LogSaytUI, Verbose, TEXT("[월드패널] NPC %d개에 색 블록 부착"), Count);
	}

	static FAutoConsoleCommand ShowCommand(
		TEXT("Sayt.WorldPanel.Show"),
		TEXT("월드 패널 실험 토글 - 배치된 NPC 머리 위에 색 블록 표시"),
		FConsoleCommandDelegate::CreateStatic(&ToggleShow));
}

// ═════════════════════════════════════════════════════════════
// Phase 8 Stage 2-8 — 몬스터 스폰/제거 하네스 (사망 로직은 Phase 11 소관)
// ═════════════════════════════════════════════════════════════
namespace SaytMobHarness
{
	static TArray<TWeakObjectPtr<AActor>> SpawnedMobs;

	static UWorld* GetPIEWorld()
	{
		return (GEngine && GEngine->GameViewport) ? GEngine->GameViewport->GetWorld() : nullptr;
	}

	static void SpawnMob()
	{
		UWorld* World = GetPIEWorld();
		if (!World)
		{
			return;
		}

		APlayerController* PC = World->GetFirstPlayerController();
		APawn* PlayerPawn = PC ? PC->GetPawn() : nullptr;
		if (!PlayerPawn)
		{
			return;
		}

		// 레벨에 이미 배치된 NPC의 클래스를 빌려 쓴다 — BP 경로를 코드에 박지 않기 위해
		UClass* MobClass = nullptr;
		for (TActorIterator<ASaytNPCCharacter> It(World); It; ++It)
		{
			MobClass = It->GetClass();
			break;
		}
		
		if (!MobClass)
		{
			UE_LOG(LogSaytUI, Warning, TEXT("[몹] 레벨에 배치된 NPC가 없어 클래스를 찾지 못했습니다"));
			return;
		}

		const FVector SpawnLocation = PlayerPawn->GetActorLocation()
			+ PlayerPawn->GetActorForwardVector() * 400.f
			+ PlayerPawn->GetActorRightVector() * FMath::FRandRange(-250.f, 250.f);

		FActorSpawnParameters Params;
		Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

		AActor* NewMob = World->SpawnActor<AActor>(MobClass, SpawnLocation, PlayerPawn->GetActorRotation(), Params);
		if (!NewMob)
		{
			return;
		}

		SpawnedMobs.Add(NewMob);

		if (USaytIndicatorManagerComponent* Manager = USaytIndicatorManagerComponent::FindOrAdd(PC))
		{
			Manager->AddIndicator(NewMob, ESaytHealthDisplayType::Mob);
		}
	}

	/** 가장 최근 스폰분을 파괴 — OnDestroyed 경유 자동 해제 경로 검증용 */
	static void RemoveMob()
	{
		while (SpawnedMobs.Num() > 0)
		{
			TWeakObjectPtr<AActor> Last = SpawnedMobs.Pop();
			if (AActor* Actor = Last.Get())
			{
				Actor->Destroy();
				return;
			}
		}
		UE_LOG(LogSaytUI, Verbose, TEXT("[몹] 제거할 스폰 몹이 없습니다"));
	}

	/** 파괴하지 않고 목록에서만 해제 — 명시적 해제 경로 검증용 */
	static void UnregisterMob()
	{
		UWorld* World = GetPIEWorld();
		APlayerController* PC = World ? World->GetFirstPlayerController() : nullptr;
		if (!PC)
		{
			return;
		}

		USaytIndicatorManagerComponent* Manager = USaytIndicatorManagerComponent::FindOrAdd(PC);
		if (!Manager || Manager->GetIndicators().Num() == 0)
		{
			return;
		}

		Manager->RemoveIndicator(Manager->GetIndicators().Last().Actor.Get());
	}

	static void ListMobs()
	{
		UWorld* World = GetPIEWorld();
		APlayerController* PC = World ? World->GetFirstPlayerController() : nullptr;
		USaytIndicatorManagerComponent* Manager = PC ? USaytIndicatorManagerComponent::FindOrAdd(PC) : nullptr;
		if (!Manager)
		{
			return;
		}

		UE_LOG(LogSaytUI, Verbose, TEXT("[몹] 등록 목록 %d개"), Manager->GetIndicators().Num());
		for (const FSaytIndicatorEntry& Entry : Manager->GetIndicators())
		{
			UE_LOG(LogSaytUI, Verbose, TEXT("  - %s (유효=%s)"),
				Entry.Actor.IsValid() ? *Entry.Actor->GetName() : TEXT("(무효)"),
				Entry.Actor.IsValid() ? TEXT("예") : TEXT("아니오"));
		}
	}
	
	static void RebuildHUD()
	{
		UWorld* World = GetPIEWorld();
		APlayerController* PC = World ? World->GetFirstPlayerController() : nullptr;
		if (ASaytCharacter* Character = PC ? Cast<ASaytCharacter>(PC->GetPawn()) : nullptr)
		{
			UE_LOG(LogSaytUI, Verbose, TEXT("[HUD] 재생성 요청"));
			Character->RebuildHUD();
		}
	}

	static FAutoConsoleCommand SpawnCmd(TEXT("Sayt.Mob.Spawn"),
		TEXT("플레이어 전방에 몹을 스폰하고 인디케이터에 등록"),
		FConsoleCommandDelegate::CreateStatic(&SpawnMob));

	static FAutoConsoleCommand RemoveCmd(TEXT("Sayt.Mob.Remove"),
		TEXT("최근 스폰 몹을 파괴 (자동 해제 경로 검증)"),
		FConsoleCommandDelegate::CreateStatic(&RemoveMob));

	static FAutoConsoleCommand UnregisterCmd(TEXT("Sayt.Mob.Unregister"),
		TEXT("파괴 없이 인디케이터 등록만 해제 (명시적 해제 경로 검증)"),
		FConsoleCommandDelegate::CreateStatic(&UnregisterMob));

	static FAutoConsoleCommand ListCmd(TEXT("Sayt.Mob.List"),
		TEXT("현재 인디케이터 등록 목록 출력"),
		FConsoleCommandDelegate::CreateStatic(&ListMobs));
	
	static FAutoConsoleCommand RebuildHUDCmd(TEXT("Sayt.HUD.Rebuild"),
		TEXT("HUD 위젯을 파괴 후 재생성 (리스폰 상황 재현)"),
		FConsoleCommandDelegate::CreateStatic(&RebuildHUD));
}

// ═════════════════════════════════════════════════════════════
// Phase 8 Stage 3 파트 A — 트레이 표시 모델 검증
//
// A-2~A-5의 구독 로직은 USaytEffectTrayComponent로 이관됐다. 여기 남은 것은
// 컴포넌트에 GE를 흘려보내는 입력 수단(Apply)과, 컴포넌트를 붙여 방송을
// 구독하는 얇은 검증 껍데기(Watch/Unwatch)뿐이다.
//   - 표시 항목의 재구축과 로그는 컴포넌트가 한다.
//   - Watch의 콜백은 '방송이 실제로 왔다'만 확인한다.
// ═════════════════════════════════════════════════════════════
namespace SaytEffectTrayDebug
{
	/** Apply/Watch가 같은 경로로 플레이어 캐릭터(Pawn)를 찾도록 한 곳에 모은다. */
	static AActor* FindPlayerAvatar()
	{
		UWorld* World = (GEngine && GEngine->GameViewport) ? GEngine->GameViewport->GetWorld() : nullptr;
		APlayerController* PC = World ? World->GetFirstPlayerController() : nullptr;
		return PC ? PC->GetPawn() : nullptr;
	}

	static UAbilitySystemComponent* FindPlayerASC()
	{
		return UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(FindPlayerAvatar());
	}

	// ── 컴포넌트 방송 구독 (검증 껍데기) ──
	// 구독자(정적 델리게이트 핸들)는 죽지 않고 상대(컴포넌트)가 PIE 종료와 함께 죽는다.
	// 그래서 컴포넌트를 약참조로 들고, 살아 있을 때만 해제한다.
	static TWeakObjectPtr<USaytEffectTrayComponent> WatchedTray;
	static FDelegateHandle ChangedHandle;

	static void OnTrayChanged()
	{
		// 항목별 상세 로그는 컴포넌트(RebuildEntries)가 이미 찍는다.
		// 여기서는 방송이 실제로 도달했다는 사실과 최종 개수만 확인한다.
		USaytEffectTrayComponent* Tray = WatchedTray.Get();
		if (!Tray)
		{
			return;
		}
		UE_LOG(LogSaytUI, Log, TEXT("[트레이] ── 방송 수신: 표시 항목 %d개 ──"), Tray->GetEntries().Num());
	}

	static void StopWatching()
	{
		if (USaytEffectTrayComponent* Tray = WatchedTray.Get())
		{
			Tray->OnTrayChanged.Remove(ChangedHandle);
			UE_LOG(LogSaytUI, Log, TEXT("[트레이] 방송 구독 해제"));
		}
		WatchedTray.Reset();
		ChangedHandle.Reset();
	}

	static void StartWatching()
	{
		// 커맨드를 두 번 쳐서 이중 구독되는 사고를 막는다.
		StopWatching();

		AActor* Avatar = FindPlayerAvatar();
		if (!Avatar)
		{
			UE_LOG(LogSaytUI, Warning, TEXT("[트레이] 플레이어 캐릭터를 찾지 못했습니다"));
			return;
		}

		// 컴포넌트는 BeginPlay에서 이미 ASC를 구독한다. 여기서 하는 일은 그 컴포넌트를
		// 붙이고(없으면 생성), 표시 항목 방송을 구독해 로그로 확인하는 것뿐이다.
		USaytEffectTrayComponent* Tray = USaytEffectTrayComponent::FindOrAdd(Avatar);
		if (!Tray)
		{
			UE_LOG(LogSaytUI, Warning, TEXT("[트레이] 컴포넌트를 붙이지 못했습니다"));
			return;
		}

		ChangedHandle = Tray->OnTrayChanged.AddStatic(&OnTrayChanged);
		WatchedTray = Tray;

		UE_LOG(LogSaytUI, Log, TEXT("[트레이] 방송 구독 시작 — 현재 항목 %d개"), Tray->GetEntries().Num());
	}

	static void CleanupOnWorldTeardown()
	{
		// PIE가 끝나면 컴포넌트가 사라진다. 우리 약참조와 핸들도 함께 비운다.
		WatchedTray.Reset();
		ChangedHandle.Reset();
	}

	static FDelegateHandle WorldCleanupHandle = FWorldDelegates::OnWorldCleanup.AddLambda(
		[](UWorld*, bool, bool) { CleanupOnWorldTeardown(); });

	/**
	 * 콘솔에서 임의 GE를 플레이어 자신에게 적용한다. 트레이에 흘려보내는 입력 수단.
	 *   Sayt.Effect.Apply GE_Debuff_Chill        (Effects 폴더 기준 짧은 이름)
	 *   Sayt.Effect.Apply /Game/경로/GE_X 2      (전체 경로 + 레벨)
	 */
	static void ApplyEffectByName(const TArray<FString>& Args)
	{
		if (Args.Num() < 1)
		{
			UE_LOG(LogSaytUI, Warning, TEXT("[적용] 사용법: Sayt.Effect.Apply <GE 이름 또는 경로> [레벨]"));
			return;
		}

		UAbilitySystemComponent* ASC = FindPlayerASC();
		if (!ASC)
		{
			UE_LOG(LogSaytUI, Warning, TEXT("[적용] 플레이어 ASC를 찾지 못했습니다"));
			return;
		}

		// 짧은 이름이면 Effects 폴더를 붙인다. 반복 입력이 많은 커맨드라 타이핑을 줄인다.
		FString Path = Args[0];
		if (!Path.StartsWith(TEXT("/")))
		{
			Path = TEXT("/Game/AbilitySystem/Effects/") + Path;
		}

		// 블루프린트가 생성한 클래스는 애셋 이름 뒤에 _C가 붙는다.
		if (!Path.EndsWith(TEXT("_C")))
		{
			FString AssetName;
			Path.Split(TEXT("/"), nullptr, &AssetName, ESearchCase::CaseSensitive, ESearchDir::FromEnd);
			Path = FString::Printf(TEXT("%s.%s_C"), *Path, *AssetName);
		}

		UClass* EffectClass = LoadClass<UGameplayEffect>(nullptr, *Path);
		if (!EffectClass)
		{
			UE_LOG(LogSaytUI, Warning, TEXT("[적용] GE를 찾지 못했습니다: %s"), *Path);
			return;
		}

		const float Level = (Args.Num() >= 2) ? FCString::Atof(*Args[1]) : 1.0f;

		FGameplayEffectContextHandle Context = ASC->MakeEffectContext();
		Context.AddSourceObject(ASC->GetOwner());

		const FGameplayEffectSpecHandle Spec = ASC->MakeOutgoingSpec(EffectClass, Level, Context);
		if (!Spec.IsValid())
		{
			UE_LOG(LogSaytUI, Warning, TEXT("[적용] 스펙 생성 실패: %s"), *EffectClass->GetName());
			return;
		}

		// 성공 로그를 여기서 찍지 않는다 — 찍는 것은 컴포넌트의 일이고,
		// 안 찍히는 것 자체가 관찰 대상이다.
		ASC->ApplyGameplayEffectSpecToSelf(*Spec.Data.Get());
	}

	static FAutoConsoleCommand WatchCmd(TEXT("Sayt.EffectTray.Watch"),
		TEXT("플레이어 캐릭터에 트레이 컴포넌트를 붙이고 표시 항목 방송을 구독해 로그로 확인"),
		FConsoleCommandDelegate::CreateStatic(&StartWatching));

	static FAutoConsoleCommand UnwatchCmd(TEXT("Sayt.EffectTray.Unwatch"),
		TEXT("트레이 방송 구독 해제 (대칭 해제 경로 검증)"),
		FConsoleCommandDelegate::CreateStatic(&StopWatching));

	static FAutoConsoleCommand ApplyCmd(TEXT("Sayt.Effect.Apply"),
		TEXT("플레이어에게 GE를 적용. 사용법: Sayt.Effect.Apply <GE 이름 또는 경로> [레벨]"),
		FConsoleCommandWithArgsDelegate::CreateStatic(&ApplyEffectByName));
}

#endif // !UE_BUILD_SHIPPING