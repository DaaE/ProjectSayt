# 프로젝트 컨벤션

## 커밋
- Conventional Commits: `feat:` `fix:` `refactor:` `docs:` `chore:` `wip:`,
  필요시 스코프 `feat(phase8-stage1):`
- 접두어/스코프는 영문, 본문은 한글
- 커밋 메시지에 큰따옴표(") 금지 (셸 `git commit -m "..."` 호환)

## 네이밍
- 언리얼 타입 접두어 준수: `U`(UObject) / `A`(Actor) / `F`(struct) / `E`(enum) /
  `I`(interface) / `S`(Slate 위젯) / `T`(템플릿)
- 프로젝트 접두어: 클래스는 `Sayu` (예: `USayuHUDWidget`, `SSayuHealthBar`)
- 파일명: UObject/Actor 계열은 타입 접두어 생략(`SayuHUDWidget.h`),
  Slate 위젯은 `S` 유지(`SSayuHealthBar.h`) — 엔진 관례와 일치, 검색성 확보
- Docs/Media 파일명: 소문자_언더스코어 (`healthbar_ghost.gif`)

## 코드
- IWYU: 직접 사용하는 타입은 소속 헤더를 명시적으로 include
  (전이 포함/PCH/유니티 빌드에 기대지 않기)
- float 비교: 산술 결과의 동등 판정은 `FMath::IsNearlyEqual`,
  단 "같은 저장소에서 온 값의 재설정 감지"는 정확 비교(`==`)가 옳음 (오차 누적 방지)
- 델리게이트: 구독자 생명주기에 맞는 Add 계열 사용(UObject→AddUObject,
  TSharedPtr→AddSP), 핸들 보관 + 대칭 해제
- Slate 위젯: `SetCanTick(false)` 기본, 이벤트 구동 + 최소 등급 Invalidate,
  애니메이션 구간만 ActiveTimer
- 개발 전용 코드(콘솔 커맨드 등)는 `#if !UE_BUILD_SHIPPING`으로 격리

## 검증 원칙
- 커스텀 도구/시스템 제작 전, 엔진 내장 기능·공식 플러그인이 같은 요구를
  커버하는지 먼저 확인하고 비교를 기록한다
- 에디터 UI 절차·API를 문서로 인용할 때 UE 5.7.4 기준 여부를 확인한다
  (dev.epicgames.com은 `?application_version` 파라미터로 구버전 스냅숏이 검색에 노출됨)
- 새 Phase 커리큘럼은 공식 문서/현행 레퍼런스 대비 벤치마크 후 확정한다

## Phase 종료 산출물
- 클래스 다이어그램 + 핵심 흐름 시퀀스 다이어그램
- 진행 트래커 갱신, 측정 증거(Docs/Media)
- 설계 결정 회고 (왜 이 구조인가 — trade-off 기록)

## 로깅 (도입 예정)
- `LogTemp` 사용 금지 예정 → `LogSayuCombat` / `LogSayuUI` / `LogSayuStudy` 등
  카테고리 분리 (`DECLARE_LOG_CATEGORY_EXTERN`)