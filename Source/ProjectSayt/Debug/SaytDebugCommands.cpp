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
#include "UI/SaytSegmentedHealth.h"
#include "UI/Slate/SSaytHealthBar.h"
#include "UI/Slate/SSaytHealthDisplay.h"
#include "UI/Slate/SSaytOrbTray.h"
#include "UI/Slate/SSaytTuningPanel.h"
#include "Widgets/Layout/SBox.h"

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
			DisplayRoot = SNew(SBox)
				.HAlign(HAlign_Center)
				.VAlign(VAlign_Top)
				.Padding(FMargin(0.f, 60.f, 0.f, 0.f))
				[
					SAssignNew(Display, SSaytHealthDisplay)
					.SegmentCount(4)
					.DesiredBarSize(FVector2D(500.f, 26.f))
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

#endif // !UE_BUILD_SHIPPING