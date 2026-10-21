# Sayt — 개발 로드맵

UE 5.8.4 / C++ 기반, GAS(Gameplay Ability System) 중심의 액션 RPG 포트폴리오.
엔진 내장 시스템을 우선 검토하고, Lyra 등 공식 레퍼런스를 비판적으로 수용하며,
설계 결정마다 trade-off를 기록하는 것을 원칙으로 한다.

> 2026-10-21: 엔진을 5.7.4에서 5.8.4로, IDE를 Rider에서 Visual Studio 2026으로 전환했다
> (Phase 8 Stage 3 파트 A 종료 시점). 아래 Phase 0 행의 5.7.4와 Rider는 당시 기록이다.

## Phase 개요

| Phase | 주제 | 상태 |
|---|---|---|
| 0 | 프로젝트 세팅 (UE 5.7.4, Rider, Git+LFS, GAS 플러그인) | ✅ |
| 1 | Enhanced Input 기반 입력 체계 | ✅ |
| 2 | GAS 코어 — AttributeSet 컴포지션, Ability 기초 | ✅ |
| 3 | 콤보 전투 — 단일 어빌리티 생명주기 + GameplayEvent 콤보, ExecCalc 데미지, 오브젝트 풀링 | ✅ |
| 4 | 데이터 파이프라인 — GameInstanceSubsystem, DeveloperSettings, DataTable(CSV), SaveGame | ✅ |
| 5 | 인벤토리 — Lyra Fragment/Definition/Instance 컴포지션, Tarkov식 그리드(1D 점유 배열), CommonUI | ✅ |
| 6 | Gameplay Message Subsystem — 전투 이벤트 브로드캐스트, 폴링 제거 | ✅ |
| 7 | Editor Utility Widget — AssetManager C++ API 학습용 점검 도구 | ✅ |
| **8** | **Slate UI 심화 + 전투 HUD** (하단 상세) | 🔧 Stage 1 완료 |
| 9 | 프로파일링/최적화 — Unreal Insights, Slate Insights, 메모리 측정(Phase 7 EUW 재사용), **렌더링 축: Nanite/Lumen 동작 이해·측정·적용 판단 문서화** | 대기 |
| 10 | Replicated GAS — Listen Server, 클라이언트 예측(PredictionKey 검증), **넷코드 이론 비교(딜레이/롤백/데드레커닝), lag compensation, 서버 권위·안티치트 원칙, ASC 위치(PlayerState 이전) 검토** | 대기 |
| 11 | AI/보스 — StateTree, Dynamic NavMesh(NavModifier), 보스 페이즈, **캐릭터 상태 모델링(태그 상태 계층·사망·피격 경직·힐 차단, 세 층위 이론: 태그/StateTree/AnimBP SM), MaxHealth 변동 시 Health 재클램프**, DT 확장·CSV→JSON 재평가, FDataTableRowHandle 재평가, **로코모션·타격감(프레젠테이션 코어 당김분)** | 대기 |
| 12 | 포트폴리오 마무리 — README/아키텍처 다이어그램, 영상, 디버그 코드 정리(콘솔 커맨드·디버그 입력 일괄), 설계 Q&A 문서, **전체 모의 면접 세션**, **CI/CD 소형 편입(GitHub Actions + self-hosted runner 빌드 자동화, 선택이나 기본 포함)**, 
패키징 시 느슨한 UI 텍스처 스테이징 설정(Additional Non-Asset Directories: UI — Stage 2-4 부채)| 대기 |
| 13 (선택) | Mass Entity(ECS 대규모 군중) — 착수 시점에 당시 엔진 버전 기준 공식 문서 재벤치마크 후 커리큘럼 확정. 지원 시작선 이후, 심화 백로그와 병렬 | 선택 |

## 계획 조정 결정 (2026-10-21)

전체 점검 세션에서 확정한 조정이다. 아래 표 밖의 Phase 구성은 그대로다.

| 항목 | 결정 | 근거·비고 |
|---|---|---|
| Stage 3 파트 B, Stage 5 | Slate 직접 제작(STileView 등)에서 **UMG+C++ ViewModel(MVVM) 경로**로 변경 | Slate 심화의 원래 동기는 스마일게이트 클라이언트 공고의 Slate 우대사항. Stage 1~2(Leaf 체력바, Panel 월드 패널, 위젯 풀)로 Slate 증명은 확보. 이후 UI는 실무 표준 경로(UMG+MVVM)를 증명. 쿨다운 라디얼(`OnPaint`)을 UMG로 감싸는 소형 Slate 위젯으로 남길지는 파트 B 설계 시 결정 |
| Stage 4 | **B안 확정**: 타격 이펙트는 GameplayCue Burst 큐로 이전, 데미지 숫자는 UMG 위젯 풀 | 자체 풀링 `UWorldSubsystem` 리팩터 계획은 폐기(엔진의 큐 재활용·사전 할당과 중복). 데미지 숫자는 Lyra처럼 추상 컴포넌트(Strategy 패턴) 아래 구현체로 만들고 UMG 풀(`FUserWidgetPool`)로 구현. Lyra의 Niagara 구현은 Phase 9에서 시간이 되면 같은 인터페이스에 붙여 실측 비교(선택). 근거: Phase 8 UI 주제와 일치, `FText` 서식과 큰 수 표기 유리, Phase 9 위젯 풀 측정 계획과 연결 |
| Phase 10 | **위치 유지 + 리플리케이션 대비 규칙을 지금 적용(C안)** | 규칙은 `CONVENTIONS.md` 리플리케이션 대비 규칙 절. Stage 4 조사 결과를 보고 앞당김 여부(B안)를 재판단. 견적(2주)은 재산정 대상 |
| 수직 슬라이스 | **도입(애자일 방식)** — 얇은 전투 루프를 먼저 만들고 반복 개선 | 범위 고정: 추격하는 적 1종, 피격 반응·사망 처리, 플레이어 사망·재시작. Phase 11(StateTree, 보스 페이즈)은 이 슬라이스를 기반으로 확장. 편성 시점은 Stage 3 종료 시 확정 |
| 자동화 테스트 | 기존 콘텐츠(인벤토리)는 **제외**. 신규 콘텐츠 설계 시 테스트 가능성 검토 | 인벤토리는 변경 가능성이 높고 리소스도 부족. 후보는 `SAYT_BACKLOG.md` 인박스 참조 |
| Phase 0~7 소급 기록 | 변경 없음 (`DECISIONS.md` 기록 시점 규칙 유지) | 필요해지면 별도 결정 |
| 외부 조사 | 완료(2026-10-21) — 공개 샘플·포럼 사례, Paragon 대체, AI 생성 에셋 | 결과는 아래 보류 후보로 기록. 각 후보는 해당 작업 직전에 재검토 |

### 보류 후보 (작업 시점에 재검토)

외부 조사(2026-10-21)에서 나온 반영 후보다. **지금은 확정하지 않는다.** 각 후보는 재검토 시점
직전에 Claude가 먼저 언급하고, 그때 반영 여부를 다시 정한다.

| # | 후보 | 재검토 시점 | 내용 |
|---|---|---|---|
| 1 | MVVM Beta 위험 기록 | Stage 3 파트 B 착수 전 | UMG Viewmodel은 5.8 문서에 Beta·출하 시 주의로 표시됨. 전환은 유지하되 위험과 완화책(Entry 구조체를 안정된 계약으로 두고 ViewModel은 얇게 유지)을 결정 기록에 남길지 |
| 2 | Stage 4 자료 확보 입력 | Stage 4 착수 전 | Lyra 5.8 확인 결과: 데미지 숫자는 `GCNL_Character_DamageTaken` 큐에서 컨트롤러 컴포넌트의 `AddNumberPop`을 호출. 엔진 GameplayCue 액터는 재활용·사전 할당 지원(`Recycle`, `NumPreallocatedInstances`). 상세는 문서에 따로 기록하지 않음 |
| 3 | 로코모션 항목 교체 | 수직 슬라이스 설계 시 + 프레젠테이션 코어 3번 착수 전 | "AnimBP 상태 머신"을 "모션 매칭 평가(Game Animation Sample 참조, CMC 유지)"로 바꿀지. Mover는 5.8 문서에 Experimental이라 채택하지 않는 쪽 |
| 4 | 수직 슬라이스의 AI 에셋 시험 | 수직 슬라이스 착수 전(적 캐릭터 선정 시) | 적 1종을 AI로 생성해 리깅·리타게팅을 1~2일 시험, 실패 시 Paragon 몹. README에 AI 생성 표기 |
| 5 | 콘텐츠 방침 | 수직 슬라이스 착수 전, 스킬킷 구간 착수 전 | Paragon 유지, 시스템과 콘텐츠 분리로 교체 비용 낮게 유지, 환경은 PCG와 기존 환경 에셋, AI 월드 생성 제외 |
| 6 | 견적 재산정 | Stage 3 종료 시 | 수직 슬라이스와 조사 단원을 반영해 일정 재보정(기존 규칙: Phase 종료마다 갱신) |

## 로드맵 재편 세션 결과 (2026-07-18 확정)

### 코드베이스 감사 판정 (Phase 0~7 전 소스)

재작성·삭제 0건. 아키텍처는 현행 표준(Lyra·공식 문서) 대조를 통과.

| 시스템 | 판정 | 비고 |
|---|---|---|
| 캐릭터/GAS 코어 | 유지 | 합성 AttributeSet, 멱등 초기화, 단일 인스턴스 콤보, 표준 ExecCalc 패턴. Replicated props/OnRep 기반 기설치 |
| 이펙트 풀링 | 리팩터(중) | 레벨 배치 액터 → `UWorldSubsystem`화. 실행 시점: **Stage 4**(풀링 재방문 = 재학습 트리거). **2026-10-21 변경: GameplayCue Burst 큐 이전으로 대체** (위 조정 결정 참조) |
| GMS 연동 | 유지 | 리스너의 풀 수동 배선만 풀링 리팩터에 편승 정리 |
| 데이터 계층 | 유지 | 중앙화 FName 조회 유지(Phase 11 재평가). SaveGame은 소비자 생길 때 확장 |
| 인벤토리 | 유지 | Model/View 분리·footprint 판정 견고 |
| UI | 유지 + 리팩터(소) | 입력 모드 권한 이원화 해소: Alt 홀드 경로를 `PushUIRequest("AltHold")` 경유로 통일 |
| 에디터 모듈 | 유지 | Phase 9 측정 도구로 재사용 |
| 인풋 | 유지 | 디버그 액션 정리는 Phase 12 |

- 소액 수리: CharacterBase 상시 틱 → 디버그 활성 시에만 틱 온 (Stage 2 전)
- 크로스커팅: **학습 주석 스윕** — 설계 의도 주석은 유지, 언어 비교/튜토리얼 주석은 제거.
  일괄 스윕은 연기됨(2026-07-19 판단) — 후반 Phase에서 해당 파일을 재방문할 때
  점진 정리하고, Phase 12에서 최종 확인 패스. 기준은 `CONVENTIONS.md` 주석 절 참조
- repo 전략: **단일 repo 유지 + 이력 보존(리셋 안 함)** — 촘촘한 실커밋 이력이
  AI 시대에 "완성본 투척"과 구분되는 프로세스 증명이라는 판단

### 프레젠테이션 코어 확정 스코프

지원 시작선 = **Phase 12 완료 + 아래 4묶음**. 미흡하면 지원보다 폴리싱이 우선.

1. **Aurora 스킬킷 4종 + FX 풀연동** — 원 키트 재현(기본 공격 + 액티브 4).
   시간 딜레이션 연출(빙결/슬로우 필드 = `CustomTimeDilation`+GE, 스피드업 자기 버프) 내장
2. **타격감 최소 패스 3종** — 히트스톱(시간 딜레이션 공유 기술) + 카메라 셰이크 + 히트 플래시.
   넉백/랙돌 등 확장은 백로그
3. **로코모션 + 팔로우 캠** — AnimBP State Machine(Idle/Run 블렌드, 점프),
   이동 방향 회전 정렬, 캠 감도 튜닝. 대시는 스킬킷 이동기로 대체. 고급 트래버설 스코프 밖
4. **HUD 폴리시** — Phase 8 산출물 통합 마감

실행 창구: 1은 전용 구간(스킬킷), 2·3은 Phase 11~12에 편성.

### 기간 산정 (v2 — Stage 1은 특수 일정으로 예외 처리, 스테이지당 1주 기준)

| 구간 | 견적 |
|---|---|
| Phase 8 잔여 (Stage 2~5) | 3~4주 |
| Phase 9 | 1주 |
| Phase 10 | 2주 |
| Phase 11 (+상태 모델링·로코모션·타격감) | 2~3주 |
| Phase 12 (+CI/CD 소형) | 1~2주 |
| 프레젠테이션 코어 잔여 (스킬킷 4종+FX, HUD) | 2~3주 |
| (병렬) 리네이밍+주석 스윕 | 1일, 크리티컬 패스 밖 |

**합계 11~15주 → 지원 시작선 2026-10초~11초.**
재보정 규칙: Stage 2 종료 시 실측으로 1차 재보정, 이후 매 Phase 종료마다 갱신.
일정 압박을 이유로 학습 루프(개념 이론→실습 검증→문답)를 압축하지 않는다.

## Phase 8 상세 — Slate UI 심화

| Stage | 내용 | 상태 |
|---|---|---|
| 0 | Live-tuning 워밍업 — 직접 마우스 입력(FReply), TAttribute Pull | ✅ |
| 1 | SHealthBar 코어 — GAS Push 델리게이트, TSlateAttribute, 고스트 바(ActiveTimer), 스타일셋 | ✅ |
| 2 | Boss/Monster HP — 세그먼트 보스 바(구슬 스택 방식 main + 줄 체력 색 스왑 변형), SActorCanvas형 월드 패널 | ✅ (폴리시·측정 이연) |
| 3 | 버프/디버프 트레이 — 파트 A(표시 모델 컴포넌트) 완료. 파트 B는 UMG+MVVM 경로로 변경(2026-10-21, 아래 조정 결정 참조) | 🔧 |
| 4 | 데미지 숫자 + 타격 이펙트 이전 — 타격 이펙트는 GameplayCue Burst 큐, 데미지 숫자는 UMG 위젯 풀(B안 확정, 2026-10-21). 착수 전 자료 확보(큐 타입·`FUserWidgetPool` API 전체) | 대기 |
| 5 | 스킬 슬롯 바 — 데이터 기반 슬롯 구성, UMG+MVVM 경로(2026-10-21 변경) | 대기 |

**전 Stage 공통 체크리스트**
1. 데이터 바인딩 경계 명시 — 소스가 통지를 제공하면 Push(델리게이트), 아니면 Pull(TAttribute)
2. Widget Role 선택 근거 — Leaf / Compound / Panel
3. Invalidation 전략 — 신고 등급 명시, SlateDebugger/stat slate로 검증
4. 비주얼 폴리시 패스 필수 — 회색 박스 상태로 종료 금지
5. 측정 증거 — before/after 캡처 또는 stat 수치를 Docs/Media에 보존

**Stage 2 이연 항목** (공통 체크리스트 4·5의 의도된 예외 — 사유는 `Docs/DECISIONS.md`)

| 항목 | 시점 |
|---|---|
| 몹 바 비주얼 폴리시, 기준점 높이(`HeadMargin`), 거리 수치 3종 | Phase 11 |
| `UWidgetComponent` 두 모드 실측 비교 캡처 | Phase 11 |
| 위젯 풀 유무 비용 차이, 거리 컬링 도입 판단 | Phase 9 |
| 뷰포트 직접 부착 vs `SInvalidationPanel` 래핑 비용 비교 | Phase 9 |
| 무효화 종류별(Layout/Paint/Volatile) 차이 학습 | Phase 9 |
| 보스 프레임 알파 0.7 배경 간섭 재평가 | 보스 바 작업 시 |

### 설계 결정과 trade-off

- **범용 `SProgressBar` 재사용 대신 특화 위젯 제작.** SProgressBar는 소스를
  모르는 범용 위젯이라 Pull(TAttribute) 인터페이스를 앞세우고, 그 대가로
  Volatility(캐싱 면제) 비용을 진다. 이 위젯은 소스가 GAS로 특정되어 있고
  GAS가 변경 통지(`GetGameplayAttributeValueChangeDelegate`)를 이미 제공하므로,
  이벤트 구동 + 최소 등급(`Paint`) 무효화로 캐싱(Global Invalidation)의 수혜를
  온전히 받는다. 검증: 유휴 시 재페인트 없음, 피격 순간 해당 위젯 1개만
  재페인트(SlateDebugger 플래시 + `SWidget::Paint (Count)` 스파이크로 확인).
- **바인딩 전략: 초기 1회 Pull + 이후 Push.** 델리게이트는 구독 이후의 변화만
  통지하므로, 구독 직후 현재값을 1회 직접 조회해 "위젯 생성 시점 ≠ 대상 생성
  시점" 문제를 해소했다. Health와 MaxHealth를 각각 구독한다(최대체력 변동 대응).
- **Widget Role = Leaf.** 채움 표현이 이 위젯의 최빈 변화인데, 자식 위젯
  조립(Compound)으로 구현하면 크기 변경(Layout급 무효화)이 되는 반면 직접
  그리기는 Paint급으로 끝난다. 가장 빈번한 변화를 가장 싼 무효화 등급에
  고정하는 선택이며, 엔진의 `SProgressBar`가 Leaf인 것과 같은 근거다.
- **런타임 틴트는 `TSlateAttribute`.** 값 변경 시 등록된 등급(Paint)의
  무효화를 자동 발행하고, 동일 값 재설정은 자동 무시된다(수동
  `Invalidate` 호출 및 무변화 필터를 프레임워크 계약으로 대체). 채움/배경
  틴트를 외부에서 스왑할 수 있어, 보스 세그먼트 체력(Stage 2에서 구슬 스택
  main + 줄 체력 색 스왑 변형으로 확정, 2026-07-19)에 위젯 수정 없이 대응한다.
  위젯은 '퍼센트 1개 + 색 2개'만 알며, 체력→세그먼트 매핑은 위젯 밖 게임
  로직 소관으로 분리했다.
- **피격 잔상은 ActiveTimer.** 상시 Tick 대신, 피격 시 등록 → 잔상이 현재
  체력에 도달하면 `Stop` 반환으로 해제되는 유한 수명 타이머를 사용했다.
  애니메이션 구간에만 프레임 갱신이 존재하고 평시에는 완전 휴면한다.
  지연/감쇠 속도는 스타일 파라미터로 노출해 위젯 코드 무접촉 튜닝이 가능하다.
- **생명주기 안전.** SWidget은 GC 추적 밖이므로 ASC 참조는
  `TWeakObjectPtr`로 보관하고, 구독은 `AddSP`(약참조 자동 무효화) +
  `FDelegateHandle` 보관 + 소멸자/`UnbindFromASC()` 대칭 해제로 관리한다.
  Unbind→재바인딩이 멱등이라 위젯 풀에서의 대상 교체(Stage 2 몬스터 패널)에
  그대로 재사용된다.
- **HUD의 체력 표시 채널 교정.** 기존 표시는 전투 이벤트 채널(Gameplay
  Message)로 상태를 갱신해, 데미지 외 경로(힐·버프)의 변경이 누락되는 틈이
  있었다. 상태는 상태 채널(Attribute 델리게이트), 사건 연출은 이벤트
  채널(GMS)로 역할을 재배정했다 — 후자는 Stage 4 데미지 폰트가 사용한다.
