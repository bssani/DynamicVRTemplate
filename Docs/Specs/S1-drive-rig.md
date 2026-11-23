# S1 주행 rig — VR Pawn·차량 분리, RawInput 휠·페달, 가상 휠, Reset point + 페이드

| 항목 | 값 |
|---|---|
| 단계 | 계획서 §6 S1. 시험 T-01 |
| 결정 근거 | 계획서 D-02(차량 Actor 하나), D-03(RawInput, 포스 피드백 없음), D-04(VR Pawn과 차량 분리), D-06(헤드셋 글 없음), D-09(리포는 기능만), D-12(주행 튜닝에 일 들이지 않음), D-13(Reset), Q-04(휠 위치 고정) |
| 베이스 브랜치 | `main` @ 이 명세 커밋 |
| 작업 브랜치 | `codex/S1-drive-rig` |
| 리뷰 파일 | `Docs/Reviews/S1-drive-rig.md` |
| 에디터 작업 | 있음 — 에셋·맵을 MCP로(§5). Agent-Workflow §2.8 순서 |

## 0. 확인한 사실 (헤더 근거)

엔진 경로는 `C:\Program Files\Epic Games\UE_5.7\Engine\` 기준이다.

- **Chaos 차량은 기본값으로는 controller가 없으면 입력을 버린다.** `bRequiresControllerForInputs`(`Plugins/Experimental/ChaosVehiclesPlugin/Source/ChaosVehicles/Public/ChaosVehicleMovementComponent.h:1115`, 생성자 기본 true `Private/ChaosVehicleMovementComponent.cpp:622`). 입력 처리 조건은 `bRequiresControllerForInputs ? (Controller && Controller->IsLocalController()) : true`(`.cpp:1177`). 끄는 함수는 `SetRequiresControllerForInputs`(`.h:947`).
  - false로 두면 controller가 없어도 입력이 보간·적용되고(`.cpp:1213-1229`), 같은 자리에서 `ServerUpdateState`로 `ReplicatedState`가 채워진다(`:1229`).
  - controller가 없을 때 잠든 차를 깨우는 판정은 `ReplicatedState`를 본다(`.cpp:1273-1287`). 그래서 깨어날 것으로 **보지만 기기에서 확인한다**(§6 E1, T-01).
- 입력 함수: `SetThrottleInput`(`ChaosVehicleMovementComponent.h:887`), `SetBrakeInput`(`:899`), `SetSteeringInput`(`:903`). 상태 초기화는 `ResetVehicle`(`:992`, 내부 `ResetVehicleState` `:1026`).
- 차량 Pawn: `AWheeledVehiclePawn`(`Plugins/Experimental/ChaosVehiclesPlugin/Source/ChaosVehicles/Public/WheeledVehiclePawn.h:20`), `GetVehicleMovementComponent`(`:41`), `GetMesh`(`:48`).
- 페이드: `APlayerCameraManager::StartCameraFade(FromAlpha, ToAlpha, Duration, Color, bShouldFadeAudio, bHoldWhenFinished)`(`Source/Runtime/Engine/Classes/Camera/PlayerCameraManager.h:856`). AI_Driving은 Reset 때 즉시 검게 한 뒤 페이드 인했다(`C:\Git\AI_Driving\Source\AI_Driving\AI_DrivingPawn.cpp:609-618`, 페이드 `:616`). 이어서 recenter도 했는데(`:620`, `:640`), **이 프로젝트는 recenter하지 않는다**(D-13).
- RawInput: 플러그인 `Plugins/Experimental/RawInput/RawInput.uplugin`(Win64 전용, 기본 꺼짐). 장치 설정은 `URawInputSettings::DeviceConfigurations`(`Source/RawInput/Public/RawInputSettings.h:107-108`, config=Input → `DefaultInput.ini [/Script/RawInput.RawInputSettings]`). 장치마다 `VendorID`·`ProductID`(16진 문자열, 비면 모두와 맞음)와 축별 `Key`·`bInverted`·`Offset`(`:12-42`, `:65-84`). 축 키 이름은 `GenericUSBController_Axis1`…(`Private/RawInput.cpp:17`). AI_Driving 예시 `C:\Git\AI_Driving\Config\DefaultInput.ini:1-2`.
- HMD: `SetTrackingOrigin`(`Plugins/Runtime/XRBase/Source/XRBase/Public/HeadMountedDisplayFunctionLibrary.h:192`), `SetWorldToMetersScale`(`:178`). VRCalibrationKit은 Stage 원점·WorldToMeters 100을 요구한다. 키트 샘플 GameMode `C:\Git\VRCalibrationKit\Source\VRCalibrationKit\VRCalSampleGameMode.cpp:10-15`(S4에서 키트를 넣을 때 그대로 맞도록 S1에서 미리 둔다).
- 카메라 HMD 고정: `UCameraComponent::bLockToHmd`(`Source/Runtime/Engine/Classes/Camera/CameraComponent.h:198`).
- 화면 메시지 끄기: 전역 `GAreScreenMessagesEnabled`(`Source/Runtime/Core/Public/CoreGlobals.h:426`). ReachStudy S4b가 world subsystem으로 끄고 되돌렸다(`C:\Git\ReachStudy\Source\ReachStudy\Screen\ReachScreenMessageSubsystem.cpp:8-33`).
- VR 렌더 설정: 가상 텍스처를 켜면 Instanced Stereo가 PSO 생성 실패로 죽는다. AI_Driving은 `r.VirtualTextures=False`(`C:\Git\AI_Driving\Config\DefaultEngine.ini:46`), `vr.InstancedStereo=True`(`:68`), `r.RayTracing=False`(`:34`, 쓰지 않는 BVH 비용)로 해결했다.
- 지금 이 리포 `Config/DefaultEngine.ini`에는 `r.RayTracing=True`가 있고 `vr.InstancedStereo`·`r.VirtualTextures` 줄은 없다(S0에서 ReachStudy를 복사).

## 1. 결정

| # | 결정 | 내용 |
|---|---|---|
| D1 | **차량 = `ADvtVehicle`(`AWheeledVehiclePawn` 자식), possess하지 않는다** | 생성자에서 `AutoPossessPlayer`·`AutoPossessAI`를 끄고 이동 컴포넌트 `SetRequiresControllerForInputs(false)`(§0). 레벨에 **정확히 하나**(D-02). 컴포넌트 두 개를 더한다. ① `DriverOrigin`(Scene) — VR Pawn이 붙는 자리, S1 임시(D4). ② `SteeringWheelPivot`(Scene) + 그 밑 `SteeringWheelMesh`(StaticMesh) — 가상 휠, 위치 고정(Q-04). 바퀴 클래스는 `UChaosVehicleWheel` 자식 앞·뒤 두 개(값은 Vehicle 템플릿과 같게, AI_Driving `AI_DrivingWheelFront.*`·`AI_DrivingWheelRear.*` 참고) |
| D2 | **VR Pawn = `ADvtVRPawn`, 카메라만** | 루트 Scene + `UCameraComponent`(`bLockToHmd` true). 이동 컴포넌트·충돌·입력 없음. 손은 S2에서 더한다. 플레이어 0이 possess한다(GameMode 기본 Pawn) |
| D3 | **입력 = `ADvtPlayerController`가 차량으로 넘긴다** | Enhanced Input. 액션 `IA_Steer`(축1D, −1..1), `IA_Throttle`(축1D, 0..1), `IA_Brake`(축1D, 0..1), `IA_ResetVehicle`(버튼). 매핑 `IMC_Drive`: RawInput 축(자리표시 `GenericUSBController_Axis1` 조향, `Axis2` 가속, `Axis3` 브레이크) + **개발용 키보드**(`A`/`D` 조향, `W` 가속, `S` 브레이크) + `Backspace` Reset. 축 방향·범위는 RawInput 설정(`bInverted`·`Offset`)과 IMC modifier로 맞춘다 — **코드에 장치 값을 두지 않는다**. `Triggered`에서 값을 넣고 `Completed`에서 0을 넣는다(키를 떼거나 페달을 놓으면 0). 넣기 전에 범위를 자른다(조향 −1..1, 가속·브레이크 0..1). 후진은 이동 컴포넌트의 `bReverseAsBrake` 기본 동작(정지 중 브레이크 = 후진)에 맡긴다. 기어·핸드브레이크 입력 없음 |
| D4 | **S1에서는 VR Pawn을 `DriverOrigin`에 붙인다** | controller가 Pawn을 possess하고 차량을 찾은 뒤 Pawn을 `DriverOrigin`에 위치·회전을 맞춰 attach한다(Stage 원점 = `DriverOrigin`). **S4에서 VRCalibrationKit이 이 attach를 대신한다**(키트가 Pawn을 차량 Actor에 붙임) — S1은 그 전의 임시다. 정렬이 없으므로 S1의 운전석 위치는 대략이다 |
| D5 | **가상 휠 각도 = 실물 휠 축 값 그대로** | `SteeringWheelPivot`의 회전 = 원래 회전 × (스핀 축 둘레 `조향 축 값 × SteeringLockToLockDegrees / 2`). 기본 900°(한쪽 450°, AI_Driving `WheelLockDegrees` 기본 450 `VRHandPresenceComponent.h:131`). 스핀 축은 프로퍼티(`EAxis`, 기본 X — 휠 mesh가 앞을 보는 경우; 구현자가 샘플 mesh에 맞춰 정함). **이동 컴포넌트의 보간된 조향값(`GetSteeringInput`)을 쓰지 않는다** — 보간 때문에 실물 휠보다 늦게 돈다. 값은 controller가 넘긴 마지막 조향 축 값(D3 자르기 뒤). 값 → 각도 계산은 순수 함수로 뺀다(§6) |
| D6 | **Reset = `ADvtResetPoint` 중 가장 가까운 곳, 페이드 아웃·인** | `ADvtResetPoint`: 루트 Scene + 화살표(에디터 전용, 게임에서 숨김) — 화살표 방향 = 차 방향. 레벨에 몇 개든. 명령 `ResetVehicle()`(controller, 키 `Backspace`; S6 진행자 창 버튼이 같은 명령을 부른다). **고르기:** 차량 위치에서 3D 거리가 가장 짧은 point. 없으면 **Play 시작 때 차량 자리**(BeginPlay에서 기억). 놓는 transform = 고른 자리 위치 + `ResetLiftCm`(기본 20 cm, 위로 — 도로에 박히지 않게) / 회전은 **yaw만**(pitch·roll 0). **순서:** ① 검게 페이드 아웃 `ResetFadeOutSeconds`(기본 0.3 s, 끝에서 유지) ② 차량을 옮긴다(`ETeleportType::TeleportPhysics`) + 선속도·각속도 0 + 이동 컴포넌트 `ResetVehicle()` ③ 페이드 인 `ResetFadeInSeconds`(기본 0.4 s). **진행 중에 다시 Reset이 오면 버린다.** recenter하지 않는다(D-13). VR Pawn은 차량에 붙어 있어 같이 옮겨진다. 순서 상태(`Idle`/`FadingOut`/`FadingIn`)와 고르기는 엔진 없이 시험할 수 있는 구조체·함수로 뺀다(§6) |
| D7 | **GameMode = `ADvtGameMode`** | `StartPlay`에서 `WorldToMeters` 100, 추적 원점 Stage(§0 키트 샘플과 같은 순서). 기본 Pawn `ADvtVRPawn`, PlayerController `ADvtPlayerController`(에디터에서 BP 자식을 쓰면 그것) |
| D8 | **헤드셋 글 없음** | ReachStudy `UReachScreenMessageSubsystem`을 `UDvtScreenMessageSubsystem`으로 옮긴다(Game·PIE world만, 시작 때 `GAreScreenMessagesEnabled` false, 끝날 때 원래 값). 이 명세의 모든 알림은 `UE_LOG` |
| D9 | **차량이 0대·2대 이상이면 주행하지 않는다** | controller가 차량을 못 찾거나 여럿 찾으면 `UE_LOG` Error 한 줄, 입력·attach·Reset을 하지 않는다(어느 차를 몰지 추측하지 않는다). 판정은 순수 함수(§6) |
| D10 | **설정 바꿈** | `DynamicVRTemplate.uproject`에 `RawInput` 플러그인(Win64). `Config/DefaultEngine.ini`: `r.RayTracing=False`, `r.VirtualTextures=False`, `vr.InstancedStereo=True`(§0 AI_Driving 근거, 주석 한 줄씩 영어로), 기본 맵 = `L_DriveTest`(GameDefaultMap·EditorStartupMap), 기본 GameMode = `ADvtGameMode`(또는 BP 자식). `Config/DefaultInput.ini`에 RawInput 장치는 **넣지 않는다**(장치 ID는 회사 PC에서, README) |
| D11 | **샘플 차 에셋은 git에 넣는다** | `C:\Git\AI_Driving\Content\Vehicles\`(SportsCar·PhysicsMaterials)를 **같은 경로** `Content/Vehicles/`로 파일 복사한다(참조 경로 `/Game/Vehicles/...`가 그대로 맞게). 사용자 결정(2026-11-24): Epic Vehicle 템플릿 에셋이지만 리포에 올려도 된다. 쓰지 않는 것(OffroadCar 등)은 복사하지 않는다 |

## 2. 지켜야 할 조건

- 차량을 possess하지 않는다(D-04). VR Pawn은 이동·충돌이 없다.
- 헤드셋에도 PC 화면에도 글을 쓰지 않는다: `AddOnScreenDebugMessage`·`AddToViewport`·`PrintString` 화면 출력 금지. Reset의 페이드는 글이 아니므로 된다.
- Reset·주행 어디에서도 recenter(`ResetOrientationAndPosition` 등)를 부르지 않는다.
- 장치 VID/PID·축 번호를 C++에 두지 않는다.
- 차량 물리 값(엔진·변속·서스펜션)은 Vehicle 템플릿 기본값. 튜닝하지 않는다(D-12).
- public: Meta XR, MCP 플러그인을 커밋하지 않는다. 커밋 전에 `git status`로 확인.

## 3. 범위 밖

- 손(추적 손·휠에 붙는 손) — S2.
- 차·hood 교체, 차 세트별 휠 mesh — S3. S1의 `SteeringWheelMesh`는 샘플 mesh 하나.
- VRCalibrationKit — S4. 정렬점·정렬 레벨 없음.
- 관전 화면 — S5, 진행자 창 — S6(Reset 버튼은 S6이 `ResetVehicle()`을 부른다).
- 포스 피드백, 기어 변속 입력, 속도 표시, 자동 충돌 감지, 사운드.
- 회사 맵·실제 장치 설정.

## 4. 구현자가 정할 것

- 파일 배치(`Source/DynamicVRTemplate/` 아래 폴더 이름), 클래스 내부 구조.
- 타이머 방식(Reset 순서를 `FTimerManager`로 할지 Tick으로 할지). 단 상태·고르기는 §6처럼 순수하게 시험할 수 있어야 한다.
- 휠 스핀 축 기본값(샘플 휠 mesh 방향에 맞춰), 샘플 휠 mesh(엔진 기본 도형을 납작하게 등).
- `DriverOrigin`의 샘플 BP 안 위치(SportsCar 운전석 바닥 쯤).
- 시험 맵의 모양(§5 조건 안에서).

## 5. 에디터 작업 (MCP, 코드 빌드·시험 뒤)

- `Content/Vehicles/` 복사(D11) 뒤 에디터에서 SportsCar 에셋이 참조 오류 없이 열리는지 본다.
- `Content/Input/`: `IA_Steer`·`IA_Throttle`·`IA_Brake`·`IA_ResetVehicle`, `IMC_Drive`(D3 매핑).
- `Content/Samples/Blueprints/`: `BP_SampleVehicle`(`ADvtVehicle` 자식, SportsCar mesh·physics asset·anim BP, 바퀴 클래스, `DriverOrigin`·휠 배치), 필요하면 `BP_DvtPlayerController`(IMC·IA 지정)·`BP_DvtGameMode`.
- `Content/Samples/Maps/L_DriveTest`: 넓은 평지(엔진 기본 도형) + 거리감을 볼 기준물 몇 개(기본 도형), 빛, `BP_SampleVehicle` 하나, `PlayerStart` 하나, `ADvtResetPoint` 3개(서로 떨어지게), World Settings GameMode.
- PIE로 키보드 주행·Reset이 되는지 본다(헤드셋 없이). 저장하고 에디터를 닫는다.

## 6. 경계 상황과 시험 (자동화 `DynamicVRTemplate.S1.*`)

| # | 상황 | 기대 |
|---|---|---|
| E1 | 차량 생성자 기본값 | `bRequiresControllerForInputs` false, `AutoPossessPlayer` Disabled, `AutoPossessAI` Disabled |
| E2 | 입력 자르기 | 조향 1.7 → 1, −2 → −1. 가속 −0.3 → 0, 1.2 → 1. 브레이크도 같음 |
| E3 | 놓음 | `Completed` 처리 뒤 그 축 값 0(조향·가속·브레이크 각각) |
| E4 | 휠 각도 | 축 0 → 0°, 1 → +450°, −1 → −450°(900° 기본), 0.5 → 225°. `SteeringLockToLockDegrees` 540 → 1 → 270° |
| E5 | Reset 고르기 — point 없음 | 시작 자리 + lift, yaw만 |
| E6 | Reset 고르기 — 셋 중 가장 가까운 것 | 차 (0,0,0), point (1000,0,0)·(−300,0,0)·(0,500,0) → (−300,0,0) |
| E7 | Reset 고르기 — point 회전 pitch 30·roll 10·yaw 90 | 결과 회전 pitch 0·roll 0·yaw 90, 위치 z + `ResetLiftCm` |
| E8 | Reset 순서 | `Idle` → Reset → `FadingOut` → 0.3 s 뒤 옮김 1회 → `FadingIn` → 0.4 s 뒤 `Idle`. 옮김은 정확히 한 번 |
| E9 | Reset 진행 중 다시 Reset | `FadingOut`·`FadingIn` 중 둘 다 버림, 옮김 수 그대로 1 |
| E10 | 차량 찾기 | 0대 → 없음(Error), 1대 → 그 차, 2대 → 없음(Error) |
| E11 | 화면 메시지 | world 시작 → `GAreScreenMessagesEnabled` false, 끝 → 원래 값(true·false 둘 다에서) |
| E12 | 페이드 시간 0 | `ResetFadeOutSeconds` 0이어도 옮김 1회 뒤 `FadingIn`(옮김을 건너뛰지 않음), 둘 다 0이면 같은 프레임에 `Idle` |

E1은 CDO로, E10은 순수 함수로, E11은 ReachStudy S4b 시험 방식으로 시험한다. 실제 페이드·텔레포트·attach는 Quest(T-01)와 PIE에서 본다.

## 7. 완료 조건

- non-unity(`-DisableUnity -Module=DynamicVRTemplate`)와 unity 빌드 성공, 경고·오류 0.
- `DynamicVRTemplate.*` 전체 시험 통과(E1~E12).
- §5 에셋·맵이 있고 `L_DriveTest`가 GUI 에디터에서 오류 없이 열린다. PIE에서 키보드 주행·Reset 동작(헤드셋 없이).
- README 새 절 "Wheel and pedals"에 RawInput 장치 설정(Project Settings → Raw Input: VID/PID, 축 `Key`·`bInverted`·`Offset`)과 `IMC_Drive` 축 맞추는 법, 휠 회전 각도(`SteeringLockToLockDegrees`를 simracing 소프트웨어의 회전 범위와 같게) — 영어.
- `git status`에 Meta XR·MCP 플러그인 없음. `Content/Vehicles/`(D11)는 커밋에 있다.

## 8. 로컬에서 확인할 것 (사용자, Quest 3 + 벅)

- T-01: 벅 휠·페달로 출발·가속·제동·조향. 가상 휠이 실물 휠과 같은 각도로 늦지 않게 돈다. 완전히 선 뒤 5 s 기다렸다가 가속 → 다시 움직인다(§0 깨우기). Reset → 검게 → 가장 가까운 point → 밝아짐, 시야가 차와 같이 옮겨지고 recenter되지 않는다. 헤드셋에 글 없음.
- 회사 PC에서 RawInput 장치 VID/PID·축 맞추기(README).
