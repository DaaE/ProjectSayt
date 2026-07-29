### 클래스/구조

```mermaid
classDiagram
    direction LR

    class SSaytWorldPanel {
        <<SPanel>>
        -TPanelChildren~FSlot~ Children
        -TWeakObjectPtr~APlayerController~ PlayerController
        -TSharedPtr~FActiveTimerHandle~ UpdateTimerHandle
        -float HeadMargin
        +AddSlot() / RemoveSlotForActor()
        +OnArrangeChildren() 캐시 읽기 전용
        +OnPaint() Arrange 후 PaintArrangedChildren
        -UpdateProjections() 투영+거리배율 → 슬롯 캐시
    }
    class FSlot {
        <<TSlotBase>>
        +TWeakObjectPtr~AActor~ TrackedActor
        +FVector2D CachedPixelPosition
        +bool bCachedInFront
        +float CachedScale
    }
    class USaytIndicatorManagerComponent {
        <<UActorComponent>>
        -TArray~FSaytIndicatorEntry~ Indicators
        +FindOrAdd(PC)$
        +AddIndicator() / RemoveIndicator()
        +GetIndicators() 늦은 구독자 따라잡기용
        +OnIndicatorAdded / OnIndicatorRemoved
        -HandleTrackedActorDestroyed()
    }
    class FSaytIndicatorEntry {
        +TWeakObjectPtr~AActor~ Actor
        +ESaytHealthDisplayType DisplayType
    }
    class USaytHUDWidget {
        <<UUserWidget>>
        -TSharedPtr~SSaytHealthDisplay~ HealthBar
        -TSharedPtr~SSaytWorldPanel~ WorldPanel
        -TMap~Actor,Display~ MobDisplays
        -TArray~Display~ MobDisplayPool
        -HandleIndicatorAdded/Removed()
        -AcquireMobDisplay() / ReleaseMobDisplay()
    }
    class SSaytHealthDisplay {
        <<SCompoundWidget>>
        +BindToASC() 구독+현재값 Pull
        +UnbindFromASC()
        SegmentCount가 형태 결정
    }
    class SSaytOrbTray {
        <<SLeafWidget>>
        구슬 스택, 파괴 팝(ActiveTimer)
    }
    class SSaytHealthBar {
        <<SLeafWidget>>
        Stage 1 산출물
    }
    class FSaytSegmentedHealth {
        순수 매핑 — 구슬/세그먼트%/파괴 신호
    }
    class ESaytHealthDisplayType {
        <<enum>>
        Player / Boss / Mob
        위젯 밖에만 존재
    }

    SSaytWorldPanel *-- FSlot : 슬롯이 위치 데이터 소유
    FSlot --> SSaytHealthDisplay : 내용물
    USaytIndicatorManagerComponent *-- FSaytIndicatorEntry
    USaytHUDWidget --> USaytIndicatorManagerComponent : 구독
    USaytHUDWidget --> SSaytWorldPanel : 소유+슬롯 관리
    USaytHUDWidget --> SSaytHealthDisplay : 생성/풀 반납
    SSaytHealthDisplay *-- SSaytHealthBar
    SSaytHealthDisplay *-- SSaytOrbTray : SegmentCount>1 일 때
    SSaytHealthDisplay --> FSaytSegmentedHealth : 매핑 위임
    ESaytHealthDisplayType --> SSaytHealthDisplay : 프리셋 조회 후 개별 값만 전달
```

**경계 요약**
- 게임 쪽(`USaytIndicatorManagerComponent`)은 위젯을 모른다
- UI 쪽(`SSaytWorldPanel`)은 액터를 알지만 그것이 무엇인지 모른다 (위치만 사용)
- 위젯(`SSaytHealthDisplay` 이하)은 액터도 몬스터도 모른다 (값만 받음)
- 글루(`USaytHUDWidget`)만 양쪽을 안다 → 데이터 소스 교체 시 이 한 곳만 수정

---

### 시퀀스: 몹 한 마리의 전체 수명

```mermaid
sequenceDiagram
    participant Game as 게임 코드(스폰)
    participant Mgr as USaytIndicatorManagerComponent
    participant HUD as USaytHUDWidget (글루)
    participant Pool as 위젯 풀
    participant Panel as SSaytWorldPanel
    participant Bar as SSaytHealthDisplay
    participant Slate as Slate 파이프라인

    Game->>Mgr: AddIndicator(액터, Mob)
    Mgr->>Mgr: 액터 OnDestroyed 구독
    Mgr->>HUD: OnIndicatorAdded 브로드캐스트
    HUD->>Pool: AcquireMobDisplay()
    alt 유휴 위젯 있음
        Pool-->>HUD: 재사용 인스턴스
    else 없음
        Pool-->>HUD: 신규 생성 (Mob 프리셋)
    end
    HUD->>Bar: BindToASC(몹 ASC)
    Note over Bar: 구독 등록 + 현재값 1회 Pull<br/>재사용 위젯의 잔여 표시가 여기서 교정됨
    HUD->>Panel: AddSlot().TrackedActor(액터)
    Panel->>Panel: EnsureUpdateTimer() 첫 슬롯이면 타이머 점등

    loop 매 프레임 (ActiveTimer)
        Panel->>Panel: 투영 재료 1회 조립
        Panel->>Panel: 슬롯별 투영 + 거리 배율 → 캐시
        alt 캐시가 실제로 변함
            Panel->>Slate: Invalidate(Layout)
        end
    end

    Slate->>Panel: OnPaint
    Panel->>Panel: OnArrangeChildren (캐시 읽기만)
    Note over Panel: 카메라 뒤면 명단 제외<br/>= 그리기와 입력 모두에서 사라짐
    Panel->>Bar: 배치 확정 후 그리기

    Game->>Game: 액터 Destroy()
    Game-->>Mgr: OnDestroyed 자동 발화
    Mgr->>Mgr: 목록에서 제거
    Mgr->>HUD: OnIndicatorRemoved 브로드캐스트
    HUD->>Bar: UnbindFromASC()
    HUD->>Pool: ReleaseMobDisplay() 상한 미만이면 보관
    HUD->>Panel: RemoveSlotForActor()
    Panel->>Panel: 슬롯 0이면 타이머 Stop (완전 휴면)
```

**이 흐름에서 읽을 것**
- 게임 코드는 등록만 하고 해제를 신경 쓰지 않는다 — 파괴 이벤트가 대신한다
- 위젯 생성은 풀을 거치고, 재사용 시 상태 교정은 `BindToASC`가 겸한다
- 투영(외부 세계 읽기)과 배치(순수 계산)가 분리되어 있다
- 추적 대상이 없으면 타이머가 스스로 내려가 비용이 0이 된다
