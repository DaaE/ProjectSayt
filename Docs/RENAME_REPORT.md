# 리네이밍 작업 결과 리포트

**작업일**: 2026-07-19
**범위**: 로컬 폴더 / GitHub repo / 프로젝트 / 모듈 `ProjectGAS` → `ProjectSayt`,
클래스 접두어 `Sayu` → `Sayt`
**지시서**: `Docs/RENAME_PLAN.md` (완료 처리됨, 이력 참조용)
**환경 이슈**: `Docs/BUILD_TROUBLESHOOTING.md`

---

## 1. 요약

Phase 8 Stage 1 종료 후 로드맵 재편 세션에서 확정된 리네이밍을 실행했다.
`ProjectGAS`/`Sayu`라는 이름은 이제 git 이력에만 남는다(이력 리셋 없음).

전체 5단계 중 **0~3단계 완료, 4단계는 연기, 5단계 완료**.

---

## 2. 완료 항목

| 단계 | 내용 | 커밋 |
|---|---|---|
| 0 | `rename-sayt` 브랜치 생성 | — |
| 1 | GitHub repo 개명, 로컬 폴더, remote URL | — |
| 2 | 프로젝트/모듈명 + PackageRedirects | `90f59ea` |
| 3 | 클래스 접두어 + 에셋 개명 + 재저장 | `81f3117` |
| 3 | 리네임 누락분 수정 | `2655cf8` |
| 5 | Docs 표기 갱신 | (이번 커밋) |

### 세부 결과

- **모듈**: `ProjectGAS` → `ProjectSayt`, `SayuEditor` → `SaytEditor`
- **클래스**: UCLASS 33개, USTRUCT 4개, Slate 위젯 2개, 비-UObject 타입 3개
  (`FSaytStyle`, `FSaytTuningModel`, `FSaytDamageStatics`), UENUM 1개
  (`ESaytPlacementHighlight`)
- **문자열 식별자**: 로그 카테고리 4종(`LogSayt*`), 스타일 등록 키(`Sayt.HealthBar`),
  콘솔 커맨드(`Sayt.HealthBar.Show/Tint`, `Sayt.ToggleTuningDemo`),
  설정 카테고리명, UPROPERTY 카테고리
- **에셋**: `GM_ProjectSayt`, `BP_SaytCharacter`, `ABP_SaytCharacter`,
  `DT_SaytCombatStats` (Fix Up Redirectors 완료, 리디렉터 잔재 없음)
- **검증**: 빌드 클린 / Output Log 로드 오류 0건 / GameplayEffect Attribute 슬롯 5종 정상 /
  DataTable RowStruct 정상 / WBP 부모 클래스·BindWidget·Compile 정상 /
  Enhanced Input 참조 정상 / PIE 스모크(콤보·데미지·체력바·인벤·세이브) 정상 /
  `grep Sayu Source/` 잔존 0건(주석 제외)

---

## 3. 작업 중 발생한 이슈

상세는 각 문서 참조. 여기서는 요약만.

### 빌드 환경 (→ `Docs/BUILD_TROUBLESHOOTING.md`)
`MSB4236`(워크로드 리졸버) + `MSB4019`(C++ 타겟 부재) 두 건. **리네임과 인과관계 없는
로컬 MSBuild/.NET SDK 문제**였으나, 발생 시점이 겹쳐 원인 판단에 시간이 걸렸다.
해결: VS 18 MSBuild 사용 + `MSBuildEnableWorkloadResolver=false` + 시스템 .NET SDK 설치.

### 리다이렉트 체인 단절
구 `ProjectGAS*` 시절 리다이렉트의 `NewName`이 이번 리네임으로 죽은 경로가 되면서,
`GE_ComboWindow` / `GE_Cooldown_BasicAttack` 등 오래된 에셋이 부모 클래스를 잃었다.
해결: 구 리다이렉트의 `NewName`을 중간 경유 없이 최종 목적지로 직접 갱신.

### 리다이렉트 `OldName` 모듈 경로
에셋에 박힌 경로가 구 모듈(`/Script/ProjectGAS`) 기준이라, 신 모듈 기준으로만
등록하면 매칭되지 않았다. 해결: **양쪽 모두 등록**(중복 무해).

### 지시서 누락분
`RENAME_PLAN.md`의 클래스 목록에 UENUM과 비-USTRUCT 구조체가 빠져 있었고,
`Config/DefaultGame.ini`의 `AssetBaseClass`, `CategoryName` 문자열도 누락되어 있었다.
상세는 `RENAME_PLAN.md` 상단 완료 헤더 참조.

---

## 4. 미결 부채

### 4-1. 리다이렉트 제거 (예정: Stage 2 완료 후)
`Config/DefaultEngine.ini`의 `[CoreRedirects]` 섹션에 현재 리다이렉트 90여 줄이 있다.
재저장 누락분에 대한 안전망으로 한 Stage 유지 후 별도 커밋으로 제거한다
(`RENAME_PLAN.md` 3단계 6항).

> **주의**: 3단계의 `File > Save All`에서 실제로 재저장된 에셋은 5개뿐이었다
> (나머지는 dirty 상태가 아니어서 대상에서 제외). 리다이렉트 제거 시 로드 오류가
> 나면 해당 에셋을 열어 강제 재저장해야 한다.

### 4-2. 학습 주석 스윕 (연기)
`Source/` 전체 주석 731줄. 일괄 스윕 대신 **해당 파일을 재방문할 때 점진 정리**로
전환했다. Phase 12에서 최종 확인 패스. 기준은 `CONVENTIONS.md` 주석 절.

### 4-3. 기존 버그 3건 (리네임 무관, 리네임 전부터 존재)
| # | 증상 | 확인된 사실 |
|---|---|---|
| 1 | 인벤토리 아이템 추가 시 UI 즉시 갱신 안 됨 | `PlaceAt` 실행·로그 정상. 인벤토리를 껐다 켜면 표시됨 |
| 2 | 세이브 로드 후 체력바 UI 갱신 안 됨 | 로그상 저장·로드 정상. Attribute 델리게이트를 안 거치는 경로로 추정 |
| 3 | HitImpact FX 텍스처 깨짐 | 간헐적, 현재 미재현. Cascade→Niagara 컨버팅 이슈 의심(미확정) |

1·2는 같은 구조(데이터는 정상, 상태 변경 통지가 UI로 안 감).

### 4-4. 빌드 환경 이식성
현재 빌드는 로컬 환경변수 `MSBuildEnableWorkloadResolver=false`에 의존한다.
다른 PC 클론 시 재현 가능하며, CI/CD 도입 시(Phase 12) 빌드 스크립트에 명시 필요.

### 4-5. README 부재
repo 루트에 README가 없다. 포트폴리오 첫인상에 직결되므로 Phase 12 편성 대상.

---

## 5. 다음 착수 지점 — Phase 8 Stage 2

### 선행 작업
- **CharacterBase 상시 틱 → 디버그 활성 시에만 틱 온** (`ROADMAP.md` 소액 수리 항목,
  "Stage 2 전" 명시)

### 설계 정본
- `ROADMAP.md` Phase 8 상세 표 — Stage 2는 **줄(라인) 체력 보스 바 + SActorCanvas형
  월드 패널**로 확정(원안의 페이즈 세그먼트 분할 아님)
- `ROADMAP.md` Stage 1 산출물 절 — `SSaytHealthBar`의 Unbind→재바인딩 멱등성이
  Stage 2 몬스터 패널의 위젯 풀 대상 교체에 그대로 재사용됨
- `SAYT_BACKLOG.md` — 줄 체력 기획(현재 줄 색=채움, 다음 줄 색=배경, 색 페어 스왑 +
  xN 카운터). `SSaytHealthBar`는 이미 "퍼센트 1개 + 색 2개"만 아는 구조로 정렬되어 있음
- Lyra `IndicatorManagerComponent`/`SActorCanvas` 패턴 참고하되 **단순화 자체 구현**
  (통짜 이식 금지)

### Stage 2 종료 시 필수
기간 견적 **1차 실측 재보정** (`ROADMAP.md` 기간 산정 절). 현행 견적은
Stage 1을 특수 일정으로 예외 처리한 낙관 단위이므로, Stage 2가 첫 검증점이다.
