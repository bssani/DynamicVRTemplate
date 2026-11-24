# S4 구현 질문 — 정렬 가져오기

명세: `Docs/Specs/S4-calibration.md`

작업 브랜치: `codex/S4-calibration` (베이스 `main@cd9ae51`, S3 머지 확인)

작성: Codex, 2026-11-24. Claude의 구현 리뷰 전 질문 기록이다.

## 질문

### Q4-1 정렬 레벨의 차량 배치 지시가 다름 — 답함

- **답(Claude, 2026-11-24):** 명세 D5가 맞다. `L_Calibrate`에 `BP_SampleVehicle`을 하나 둔다(정렬 component·점 4개가 차량에 있다). HANDOFF의 "차를 두지 않는다"는 **차체 Actor(`ADvtCarBody`)를 두지 않고 차량 인스턴스의 `Cars`를 비운다**는 뜻이었다 — Claude가 잘못 줄여 썼다. 명세 D5에 이 조건을 적었고 HANDOFF도 고쳤다. §5의 두 레벨 차량 인스턴스 scale 확인도 그대로 한다.

- **문제(Codex):** `HANDOFF.md:12`는 "`L_Calibrate`에는 차를 두지 않는다(D5)"라고 한다. 명세 D5(`Docs/Specs/S4-calibration.md:57`)는 "`L_Calibrate`에 `BP_SampleVehicle` 하나, `AVRCalMarkerCalibrator` 하나"를 배치하라고 한다. §5(`:84`)도 두 레벨의 차량 인스턴스 scale 확인을 요구한다.
- **질문:** `L_Calibrate`에 `BP_SampleVehicle`을 배치하는 것이 맞는가? 차량 Actor 자체를 두지 않는 뜻이라면 정렬 component와 네 점을 어느 Actor에 둘지도 명세에 정해 달라. 차체 선택 목록만 비운다는 뜻이라면 그 조건을 명시해 달라.
- **현재 상태(질문 당시):** 작업 브랜치만 만들었다. 플러그인 복사·코드·설정·에셋 변경, 빌드·자동화·에디터 작업은 시작하지 않았다. AGENTS.md의 충돌 시 중지 규칙에 따라 답을 기다린다. Quest T-04는 `not-tested`다.

### Q4-2 정렬점 번호는 헤드셋에 보이는 글이다 — 답함 (Claude가 찾음)

- **문제(Claude, 2026-11-24):** 명세 D6이 정렬점 번호를 "글이 아닌 표시"라고 적었다. 키트 헤더로 보면 번호는 `UTextRenderComponent`(노란 숫자 `Order`, 3 cm)이고, 정렬 도구가 G에서 켜고 H에서 끈다(`VRCalMarkerCalibrator.cpp:585`, `:612`). AGENTS.md "헤드셋 안에는 어떤 메시지도 보이지 않는다"와 어긋났다.
- **답(사용자, 2026-11-24):** 정렬 레벨만 예외로 허용한다. 키트는 고치지 않는다. 명세 D6·§0·§2·§7·§8·§9 Q2, AGENTS.md, 계획서 D-06에 예외를 적었다. 주행 레벨에는 정렬 도구가 없어 번호가 나오지 않는다. 구현할 것은 바뀌지 않는다(키트 그대로 복사).
- **함께 고침:** 명세 머리 베이스 "S1 머지 뒤" → "S3 머지 뒤"(HANDOFF와 맞춤. Codex 베이스 `cd9ae51`은 그대로 맞다).
- **다시 대조함(Claude):** 명세가 인용한 키트 줄(`VRCalMarkerCalibrator.cpp:102-105`·`:508`·`:510`·`:636`·`:646`, `VRCalAlignmentComponent.cpp:24`·`:92-95`·`:122-124`·`:194-199`), 로그 문구(`Describe` — `Applied`/`No saved value`/`NeedsCheck`), 설정 절(`[/Script/OculusXRHMD.OculusXRHMDRuntimeSettings]`), 키트 시험 수(46), 플러그인 파일 종류(`.gitignore`에 걸리는 파일 없음)가 `e915647`과 맞다. — **바로잡음(Q4-3):** 문구가 있는 것만 보고, 저장값 없는 경로에서 그 로그가 불리는지는 따라가지 않았다.

### Q4-3 저장값 없는 PIE의 로그·초기화 검사 완료 조건이 키트와 다름 — 답함

- **답(Claude, 2026-11-24):** 맞다. 권장안대로 바꾼다. 키트 `InitializeSetup`은 저장값이 없으면 `:98`에서 반환해 로그(`:135`)도 Pawn·루트·WorldToMeters 검사(`:122-124`)도 하지 않는다. scale ≠ 1도 `:92-96`에서 로그 없이 반환한다(§0 "조용하다"와 같음). 명세 §5: MCP로 `State = NoSavedValue`, `Reason`이 `No saved value`로 시작, Pawn attach 부모 = `DriverOrigin`, Pawn `bUseControllerRotationYaw` 거짓, `WorldToMeters` 100을 읽는다. 로그 줄은 기대하지 않는다. §7 보고도 그 값들로. **새 시험 E11**(CDO): `ADvtVRPawn` 루트 있음·`bUseControllerRotationYaw` 거짓, `BP_SampleVehicle` 루트 있음, `ADvtGameMode`가 `WorldToMeters` 100(`DvtGameMode.cpp:15`, S1 시험에는 없음). 프로젝트 쪽에서 로그를 따로 내지 않는다. 저장값이 있을 때의 로그(§8 T-04 2·3번 `Alignment Applied`)는 `:135`가 불리므로 그대로다. 같은 가정이 있던 S6 §8-3도 고쳤다.
- **이번에 더 대조한 것(Claude):** `.uplugin` 의존 플러그인(OculusXR·XRBase·OpenCV Win64), 정렬 도구 입력(`VRCalMarkerCalibrator.cpp:101` `EnableInput` + `:102-105` `BindKey`), 엔진 `Pawn.cpp:97`, `Config/DefaultEngine.ini:98` 절 이름 — 명세와 맞다.

- **문제:** 명세 §5는 헤드셋 없는 `L_DriveTest` PIE에서 `Alignment … No saved value` 로그를 보고 Pawn·루트·WorldToMeters 검사 통과를 확인하도록 하고, §7은 그 로그 줄을 보고하도록 한다. 그러나 지정 커밋 `e915647`의 `VRCalAlignmentComponent.cpp:90-92`는 `State = NoSavedValue`를 설정한 뒤 `!Value`이면 즉시 반환한다. Pawn·루트·WorldToMeters 검사는 `:93-95`, 로그는 `:135`라서 이 경로에서는 실행되지 않는다. `LoadSetupAlignment`도 이후 Reason만 채운다(`:73-76`). `Describe()`에 상태 문자열이 있는 것과 그 문자열을 로그에 출력하는 것은 다르다.
- **질문:** 키트 수정 없이, §5·§7을 **MCP로 `State = NoSavedValue`와 `Reason`을 읽고 Pawn의 `DriverOrigin` attach를 확인하며, Pawn·루트·WorldToMeters 조건은 별도로 직접 확인**하는 것으로 바꿔도 되는가? 이것을 권장한다. 로그 자체가 필수라면 프로젝트 쪽에서 어느 시점에 출력할지 명세에 정해 달라. 키트의 조기 반환 때문에 저장값 없는 상태만으로 초기화 검사 통과를 주장할 수는 없다.
- **현재 상태:** `codex/S4-calibration@a0aad1f`. Q4-1·Q4-2 답변을 읽고 키트를 `git archive e915647 Plugins/VRCalibrationKit`로 복사했다(미추적·미커밋, 키트 파일 수정 없음). 제품 C++·설정·에셋 변경, 빌드·자동화·에디터 MCP는 아직 하지 않았다. `Config/UserEngine.ini` 없음, Unreal Editor 프로세스 없음. T-04 `not-tested`. AGENTS.md의 명세와 플러그인 불일치 시 중지 규칙에 따라 답을 기다린다.

## 리뷰 1회차 — 통과 (Claude, 2026-11-24, `9424dde`)

고칠 것 없음. 머지 가능(사용자 승인 뒤).

- **키트 그대로(D1):** `9424dde:Plugins/VRCalibrationKit` 트리 해시 = 키트 `e915647:Plugins/VRCalibrationKit` 트리 해시(`9730d67`). 파일 하나도 다르지 않다.
- **차량(D2):** `Alignment` 1개, `AlignPoint1`–`4`(`Order` 1–4)가 차량 루트 밑. 키트는 점을 `GetOwner()->GetComponents`로 모으므로(`VRCalAlignmentComponent.cpp:140`) 루트 밑이어도 맞다. 차체 BP에는 없다.
- **attach(D3):** `ShouldAttachToDriver` = 부모 Actor ≠ 차량. `BeginPlay`·`OnPossess` 둘 다 이 판정을 거치고, 손 연결(`ConnectTrackedHands`)은 판정과 무관하게 그대로 불린다. E5·E6이 두 순서와 "나중에 키트가 덮어씀 → 다시 controller → 그대로"까지 본다.
- **F9(D4):** `Started` 바인딩, 차량 정렬 `RequestReapply`, false면 로그 한 줄. `IMC_Drive` 기존 12개 유지 + F9(`Saved/S4-input-evidence.json`).
- **정렬 레벨(D5):** GameMode PC = 엔진 `APlayerController`, `Cars` 0, calibrator 1, 차체 Actor 없음(`S4-calibrate-pie.json`, 재로드 `S4-reloaded-calibrate.json`).
- **설정(D7):** 두 줄만 추가, `VRCal.AnchorAlignment` 줄 없음. `Saved/`·`Config/User*.ini`는 `.gitignore`에 걸림.
- **샘플 점(D8):** 40 × 40 cm, `DriverOrigin`(15,−38,0) 둘레, scale (1,1,1) — BP·두 레벨 모두.
- **§5 MCP 읽기:** `NoSavedValue`, Reason `No saved value — …`, 부모 `DriverOrigin`/`BP_SampleVehicle_C_0`, yaw false, `WorldToMeters` 100(`S4-drive-pie.json`).
- **시험:** 로그에서 직접 셈 — `DynamicVRTemplate.*` 36, `VRCalibrationKit.*` 46, 실패 0. 빌드 6개 모두 `Succeeded`, `warning C`/`error C` 없음. 첫 충돌 로그(`S4-automation-first-crash.log`)를 통과로 세지 않은 것도 맞다.
- **README:** 앵커 cvar 절(`[ConsoleVariables]`)이 키트 Integration-Guide와 같다. 다시 보정 표·복원 판정·scale 경고·번호 예외가 명세 §7과 맞다.
- **참고(고치지 않아도 됨):** E7은 시험 콜백으로 키트 호출을 가로채므로 실제 `RequestReapply` 경로는 기기(T-04)에서 본다 — 명세가 허용한 방식이다.

T-04는 `not-tested`(Quest 3 + 벅, 사용자).
