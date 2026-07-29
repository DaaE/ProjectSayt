# 빌드 환경 트러블슈팅 노트

프로젝트 리네임(ProjectGAS → ProjectSayt) 작업 중 발생한 빌드 실패 사례 기록.

## 발생 시점

`Docs/RENAME_PLAN.md`의 전체 5단계(+0단계) 중 **2단계 진행 도중**에 발생했다.

| 단계 | 내용 | 발생 시점 기준 상태 |
|---|---|---|
| 0단계 | 에디터 종료, `rename-sayt` 브랜치 생성 | ✅ 완료 |
| 1단계 | GitHub repo 개명, 로컬 폴더명, remote URL 갱신 | ✅ 완료 |
| **2단계** | **프로젝트/모듈명 `ProjectGAS`→`ProjectSayt`** | **⚠️ 여기서 발생** |
| 3단계 | 클래스 접두어 `Sayu`→`Sayt`, 에셋 개명, Fix Up Redirectors | ⬜ 미착수 |
| 4단계 | 주석 스윕 | ⬜ 미착수 |
| 5단계 | Docs/README 갱신 | ⬜ 미착수 |

### 2단계 내부 진행 상황

2단계 자체를 세분하면, **코드 수정은 전부 끝났고 첫 리빌드를 시도한 순간** 터졌다.

| 2단계 세부 항목 | 상태 |
|---|---|
| 1항. 파일/폴더 개명 (`.uproject`, `Source/ProjectSayt/`, `SaytEditor/`, `*.Target.cs`, `*.Build.cs`, 모듈 진입점 `.h/.cpp`) | ✅ 완료 |
| 2항. 내부 클래스명·`ExtraModuleNames`·`IMPLEMENT_PRIMARY_GAME_MODULE`·`LOCTEXT_NAMESPACE` 수정 | ✅ 완료 |
| 3항. `PROJECTGAS_API`→`PROJECTSAYT_API`, `SAYUEDITOR_API`→`SAYTEDITOR_API` 전수 치환 | ✅ 완료 |
| 4항. `Config/DefaultGame.ini`, `DefaultEngine.ini` 수정 (`/Script/` 섹션 헤더, PackageRedirects 2줄 추가) | ✅ 완료 |
| **5항. `Intermediate/`·`Saved/`·구 `.sln` 삭제 → 프로젝트 파일 재생성 → 리빌드** | **❌ 여기서 실패** |
| 6항. 에디터 기동 + PIE 스모크 테스트 (전투/인벤/세이브) | ⬜ 5항 실패로 진입 불가 |
| 7항. 2단계 커밋 | ⬜ 미착수 |

즉 **"리네임 코드 수정을 다 마치고, 결과를 처음 검증하려는 순간"** 막힌 것이다.
이 위치 때문에 원인 판단이 어려웠다 — 직전에 수백 곳을 치환했으니 그게 원인처럼 보였다.

**중요: 아래 문제들은 리네임 작업과 인과관계가 없는 로컬 환경 문제였다.**
타이밍이 겹쳐 리네임이 원인으로 보였으나, 실제로는 독립적인 MSBuild/.NET SDK 구성 문제였다.

---

## 사건 1 — MSB4236: WorkloadAutoImportPropsLocator SDK를 찾을 수 없음

### 증상

```
Microsoft.NET.Sdk.ImportWorkloads.props(14,38): Error MSB4236 :
지정된 'Microsoft.NET.SDK.WorkloadAutoImportPropsLocator' SDK를 찾을 수 없습니다.
```

빌드 시 51개 프로젝트가 동시에 실패.

### 오진했던 지점

처음엔 리네임 직후 발생해서 리네임이 원인이라고 판단했다. 실제로는 아니었다.

**판별 근거**: 실패한 51개 중 우리가 만진 건 `ProjectSaytModuleRules.csproj` 1개뿐이고,
나머지 50개는 `UnrealBuildTool.csproj`, `EpicGames.Core.csproj`, `UE5Rules.csproj` 등
**Epic이 배포한 엔진 원본 파일**이었다. 우리가 코드 한 줄 안 건드린 파일까지 전부 같은
에러로 실패했다면 원인은 리네임이 아니다.

> **교훈**: 빌드 에러가 대량 발생할 때는 "실패 목록 중 내가 실제로 수정한 파일이
> 몇 개인가"를 먼저 세어볼 것. 내가 안 건드린 파일까지 실패하면 환경 문제다.

### 시도했다가 틀린 가설

- **가설 1: `Intermediate/` 삭제가 원인** → 틀림. 이전에도 여러 번 삭제 후 재생성했는데
  문제없었다는 사실과 모순.
- **가설 2: dotnet SDK 미설치가 원인** → 부분적으로만 맞음. 시스템에 dotnet이 없던 건
  사실이고 설치가 필요했지만, 설치 후에도 같은 에러가 경로만 바뀌어 재발했다.
  (`UE_5.7\Engine\Binaries\ThirdParty\DotNet\8.0.412` → `C:\Program Files\dotnet\sdk\8.0.423`)

### 실제 원인과 해결

UE 빌드는 .NET 워크로드(Android/iOS/wasm 등)를 쓰지 않으므로 워크로드 리졸버 자체가 불필요하다.
리졸버를 비활성화하면 회피된다.

```powershell
[Environment]::SetEnvironmentVariable("MSBuildEnableWorkloadResolver", "false", "User")
```

설정 후 **Rider 완전 종료 → 재시작** (환경변수 반영에 필수).

---

## 사건 2 — MSB4019: Microsoft.Cpp.Default.props를 찾을 수 없음

### 증상

MSBuild 버전을 바꿔가며 사건 1을 해결하려다 발생.

```
UECommon.props(15,3): Error MSB4019 :
가져온 프로젝트 "...\Microsoft\VC\v180\Microsoft.Cpp.Default.props"을(를) 찾을 수 없습니다.
```

### 원인

**dotnet SDK 내장 MSBuild와 Rider 번들 MSBuild에는 C++ 빌드 지원(`Microsoft.Cpp.*` 타겟)이 없다.**
둘 다 C#/.NET 전용이다. C# 프로젝트(`.csproj`)는 통과하지만 C++ 프로젝트(`.vcxproj`)에서 막힌다.

시도했다가 실패한 MSBuild 선택지:

| 선택지 | 결과 |
|---|---|
| `17.11 - C:\Program Files\dotnet\sdk\8.0.423\MSBuild.dll` | ❌ C++ 타겟 없음 |
| `18.0 - JetBrains Rider 2026.1.2\tools\MSBuild\...` | ❌ C++ 타겟 없음 |
| `4.0 - Microsoft.NET\Framework\v4.0.30319` | ❌ 레거시, UE 부적합 |
| `Auto detected (18.0) - Visual Studio\18\Community\...` | ✅ C++ 타겟 보유 |

> **교훈**: UE는 C++/C# 혼합 솔루션이다. MSBuild는 **반드시 Visual Studio 설치본**을 써야 한다.
> JetBrains 지원 문서의 "dotnet SDK MSBuild를 선택해보라"는 처방은 순수 C# 프로젝트 맥락이라
> UE에는 그대로 적용되지 않는다.

---

## 사건 3 — RiderLink RD.dll 로드 실패 → 엔진 자동화 모듈 실종 발견 → Verify로 근치

### 증상
- 에디터 스플래시 단계에서 종료: 'RiderLink' 플러그인 로드 실패, 'RD' 모듈을 로드할 수 없음
- 로그 시그니처 (Saved/Logs):
  - UnrealEditor-RD.dll 로드 실패, GetLastError=4551 — 파일은 존재하는데 OS가 로드 거부
    (파일이 없을 때의 126 + File does not exist 조합과 구분됨)
  - 이후 RiderLink/RiderBlueprint/RiderLogging.dll의 126 에러는 전부
    Missing import: UnrealEditor-RD.dll — RD 하나가 죽어서 생긴 연쇄

### 원인
- 1차 원인: 프로젝트 쪽 RiderLink 플러그인(Plugins/Developer/RiderLink)의 RD.dll 바이너리 손상
  (불완전 기록으로 추정 — 에러 코드 4551의 정확한 명칭은 미확정)
- 2차 발견: 수리 경로인 RiderLink 재설치가 UAT(AutomationTool)를 기동시키자
  엔진 배송본 DLL 실종이 드러남 —
  Engine/Intermediate/ScriptModules/Android.Automation.json 기록은 있는데
  대응하는 Engine/Binaries/DotNET/AutomationTool/.../Android.Automation.dll이 없음
- 뿌리: 과거 솔루션 전체 빌드가 엔진 C# 도구를 재빌드하며 Launcher 설치본을 훼손
  (본 문서 상단의 만성 이슈, ScriptGeneratorUbtPlugin/ref 수정 흔적과 같은 계열)
- 참고: RiderLink 설치 위치 옵션(게임/엔진)은 결과물 위치만 결정 —
  빌드 자체는 항상 엔진의 UAT가 수행하므로 엔진 도구 체인이 건강해야 함

### 해결
1. Epic Games Launcher → 라이브러리 → UE 5.7 → 확인(Verify) 실행 (미뤄뒀던 것)
2. Rider → Settings → Unreal Engine → RiderLink → 게임에 RiderLink 설치 (재설치)
3. 에디터 정상 기동 확인
- 효과: 엔진 자동화 모듈 실종 문제는 해결됨.
  단, RD.dll의 GetLastError=4551 자체의 원인은 이 시점에 확인되지 않았음 (사건 4에서 규명)

  ※ 사건 4 확정 후 재평가: RD.dll 4551의 실제 원인은 Smart App Control이었으며,
   이 사건에서 수행한 RiderLink 재설치·엔진 Verify는 원인 해결과 무관했다.
   당시 성공한 것은 재빌드된 DLL이 우연히 차단을 피했기 때문으로 추정된다.

---

## 사건 4 — Smart App Control이 서명 없는 게임 모듈 DLL 로드를 차단

### 증상
- 서로 다른 DLL에서 GetLastError=4551로 로드 실패가 반복 (때때로 연쇄로 126 동반)
  - 2026-07-26: UnrealEditor-RD.dll (RiderLink) — 사건 3으로 잘못 분류했던 건
  - 2026-07-28: UnrealEditor-ProjectSayt.dll (게임 모듈) 2회
  - 2026-07-29: UnrealEditor-RiderBlueprint.dll (RiderLink)
- 공통 시그니처: 파일이 존재하고 방금 빌드된 새 파일인데 OS가 로드를 거부
- 간헐적 — 같은 코드를 재빌드하면 통과할 때도 있음 (클라우드 평판 판정 개입)
- Windows 보안 알림에 "이 앱의 일부가 차단되었습니다 / 앱에서 로드하려고 <DLL명>"
  ※ 격리가 아니라 로드 거부이므로 보호 기록의 격리 목록에는 나타나지 않음.
    알림 목록에서만 확인 가능 — 이것 때문에 원인 규명이 크게 지연됨

### 원인
- Windows 11 Smart App Control(스마트 앱 제어)이 켜져 있었음
- 이 기능은 서명이 없거나 유효하지 않은 바이너리를 신뢰 불가로 판정해 차단
- 로컬 빌드 산출물은 매번 새로 생성되는 서명 없는 DLL이므로 정확히 차단 대상

### 해결
- Windows 보안 → 앱 및 브라우저 컨트롤 → 스마트 앱 제어 → 설정 → 끄기
- ⚠️ 비가역: 한 번 끄면 Windows 재설치 전까지 다시 켤 수 없음
- Defender 백신 / SmartScreen / 방화벽은 그대로 유지됨

### 왜 다른 해법이 없었나
- 제외 목록이 존재하지 않음 (켜짐/꺼짐 두 상태뿐)
- 정식 인증서로 서명해도 평판 판정 때문에 차단되는 사례가 보고됨
- IDE 무관 — Visual Studio 빌드 차단 사례도 존재. Rider 교체는 해법이 아니었음

### 오진 기록 (같은 실수 반복 방지)
아래는 모두 무죄였고, 확인에 상당한 시간이 소모됨:
- RiderLink 플러그인 (제거·재설치 반복)
- 엔진 설치본 오염 (Launcher Verify 실행)
- 백신 검사 제외 목록 / Rider 버전 업데이트에 따른 경로 변경
- 빌드 산출물 손상 (Binaries·Intermediate 삭제 반복)

### 교훈
- DLL 로드 실패 시 **Windows 보안 알림 목록을 최우선으로 확인**할 것.
  보호 기록(격리)만 보고 "차단 항목 없음"으로 판단하면 안 됨
- 오류 코드가 서로 다른 DLL에서 반복되면 그 DLL들의 공통점이 아니라
  **그 DLL들을 다루는 외부 주체**를 의심할 것

---

## 최종 해결 조합

두 에러는 **서로 다른 원인**이었고 각각 다른 방법으로 잡아야 했다.

1. **MSBuild = Visual Studio 18 Community** (Rider 설정 → 빌드, 실행, 배포 → 도구 모음 및 빌드)
   → C++ 타겟 확보, MSB4019 해결
2. **`MSBuildEnableWorkloadResolver=false`** 환경변수
   → 워크로드 리졸버 회피, MSB4236 해결
3. **시스템 .NET SDK 설치** (`C:\Program Files\dotnet\`)
   → 사전 조건. Windows 기능의 .NET Framework 3.5/4.8과는 **완전히 다른 제품**이니 혼동 주의.

---

## 이식성 주의사항 (미해결 부채)

현재 빌드는 **로컬 환경변수 `MSBuildEnableWorkloadResolver=false`에 의존**한다.

- 다른 PC에서 clone 시 동일 문제 재현 가능
- CI/CD 도입 시(로드맵 Phase 13 후보) 빌드 스크립트에 이 설정을 명시해야 함
- 근본 해결책(VS 워크로드 재설치 등)은 시도하지 않았음

---

## 작업 흐름에 미친 영향

2단계 검증(에디터 기동 + PIE 스모크 테스트)이 이 두 에러 때문에 지연됐다.
해결 후 재빌드에서 `SayuAssetAuditRowObject.cpp`의 구 include 경로
(`SayuEditor/SayuEditor.h` → `SaytEditor/SaytEditor.h`) 누락 1건이 컴파일 에러로 잡혔는데,
이건 환경 문제와 무관한 **실제 2단계 리네임 누락분**이었다.

> **교훈**: 환경 문제로 빌드가 막혀 있는 동안은 리네임 누락분이 드러나지 않는다.
> 환경을 먼저 정상화해야 컴파일러가 진짜 누락을 잡아준다.

---

## 부수 확인 사항

- **Rider가 제안하는 OpenTelemetry / ApplicationInsights 플러그인 알림**은 무시해도 된다.
  엔진의 `EpicGames.*.csproj`가 원래 참조하는 NuGet 종속성이며, UE C++ 빌드와 무관하다.
- **`dotnet --list-sdks` 실행은 PowerShell에서** 할 것.
- **`Test-Path "C:\Program Files\dotnet\dotnet.exe"`** 로 시스템 dotnet 설치 여부를 빠르게 확인 가능.
