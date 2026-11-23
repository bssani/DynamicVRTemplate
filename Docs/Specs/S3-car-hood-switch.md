# S3 차·hood 교체 — 차 = 차량에 붙은 차체 Actor, hood = 그 안의 변형 component, Play 중 교체

| 항목 | 값 |
|---|---|
| 단계 | 계획서 §6 S3. 시험 T-03 |
| 결정 근거 | 계획서 D-02(차량 Actor 하나, 차·hood는 보이는 mesh만 바꿈, 주행 중에도), Q-04(휠 위치 고정, 휠 mesh는 차마다), Q-06(키), D-06(헤드셋 글 없음), D-12. 변형 전환 규칙은 ReachStudy S4(`C:\Git\ReachStudy\Docs\Specs\S4-door-switch.md` D2·D3) |
| 베이스 브랜치 | **S2가 `main`에 머지된 뒤의 `main`** (S2의 `UDvtSteeringGripComponent`·붙은 손 값을 차마다 바꾼다) |
| 작업 브랜치 | `codex/S3-car-hood-switch` |
| 리뷰 파일 | `Docs/Reviews/S3-car-hood-switch.md` |
| 에디터 작업 | 있음 — 샘플 차 둘, hood 변형(§5) |

## 0. 확인한 사실

- **ReachStudy 변형 전환**(`C:\Git\ReachStudy\Source\ReachStudy\Door\ReachDoorVariants.h`): 명령 `SelectVariant`·`SelectNext`·`SelectPrevious`(`:46-52`), 쿨다운 `SwitchCooldownSeconds` 0.5 s(`:40`), 이벤트 `OnVariantChanged`(`:43`).
  - 규칙(ReachStudy S4 D2): 같은 번호 = 아무것도 안 함, 범위 밖 = 무시 + Warning, 상대 명령은 끝에서 돌아감, 실제로 바뀐 뒤 쿨다운 안의 **상대** 명령만 무시. 시작 적용은 변경이 아니다.
  - 적용(D3): actor 숨김·collision만 바꾸고 EndPlay에서 원래 값으로.
- **CAD는 피벗이 차 원점에 묶여 온다**(ReachStudy AGENTS "움직이는 mesh 피벗에 기대지 않는다"). 그래서 차체 Actor를 차량 원점에 붙이면 CAD 위치가 그대로 맞는다. 단 **휠 mesh도 피벗이 차 원점**이라, 휠 회전축(S1 `SteeringWheelPivot`) 둘레로 돌리려면 mesh를 피벗 기준으로 옮겨 놓아야 한다(D4 `SteeringWheelMeshTransform`).
- Datasmith로 가져온 CAD는 레벨 Actor로 온다. "Convert to Blueprint"(Harvest Components)로 BP 하나로 묶는 것이 ReachStudy S5의 방식이다(계획서 ReachStudy §6 S5).
- 차량 물리는 S1 `ADvtVehicle`의 skeletal mesh(physics asset)다. 그 mesh에 붙은 다른 Actor의 primitive가 충돌하면 차량 자신과 부딪힌다 — **차체·hood primitive는 충돌을 꺼야 한다**(D3).

## 1. 결정

| # | 결정 | 내용 |
|---|---|---|
| D1 | **차 = `ADvtCarBody`(C++ Actor) 자식 BP 하나** | 레벨에 놓고 **차량 Actor에 attach**한다(에디터에서, 차 원점 = 차량 원점, 상대 transform 0). 프로퍼티: `DisplayName`(FText, 관전·진행자 창 글), 휠(D4), 손(D4). 차 BP 안의 mesh 중 **hood 변형 component 밑에 있지 않은 것 = 차체**(그 차가 활성이면 늘 보임) |
| D2 | **hood 변형 = `UDvtHoodVariantComponent`(C++ `USceneComponent`)** | 차 BP 안에 몇 개든. `DisplayName`(FText). 그 component 밑에 붙인 mesh들이 그 변형이다. **순서 = BP 안의 component 순서가 아니라 `SortOrder`(int32) 오름차순, 같으면 이름순**(BP 계층 순서는 믿을 수 없으므로). 변형이 없는 차도 된다(hood 하나로 고정된 차) |
| D3 | **켜고 끄기 = 보이기만, 충돌은 늘 끔** | BeginPlay에서 차량 `Cars`의 모든 차 Actor 안 primitive의 충돌을 끈다(`NoCollision`, §0 이유) — 원래 값으로 되돌리지 않는다(차량에 붙은 차체가 충돌하면 안 되므로). 활성 차: Actor 보임 + 활성 hood 변형 밑 component 보임 + 다른 변형 밑 component 숨김. 비활성 차: Actor 숨김(`SetActorHiddenInGame`). 변형 component는 `SetVisibility(…, true)`(자식까지). material·mesh·transform은 건드리지 않는다. EndPlay에서 보이기를 원래 값으로 |
| D4 | **차마다 휠·손 값** | `ADvtCarBody` 프로퍼티: `SteeringWheelMesh`(UStaticMesh, 비면 휠 mesh를 숨김), `SteeringWheelMeshTransform`(`SteeringWheelPivot` 기준 상대 transform — CAD 피벗 보정, §0), `RimRadiusCm`(S2 D1 값), `LeftGripHandTransform`·`RightGripHandTransform`(`SteeringWheelPivot` 기준). 차가 활성이 되면 차량의 `SteeringWheelMesh` component(S1 D1)에 mesh·transform을, `SteeringGrip`에 반지름을, 붙은 손 두 개에 transform을 넣는다. **`SteeringWheelPivot` 자리·회전 기준은 바꾸지 않는다**(벅 휠 위치 고정, Q-04). enter·release 거리는 차마다 두지 않는다(손·벅에 달린 값) |
| D5 | **차량 = `ADvtVehicle`에 목록과 명령** | `Cars`(TArray<TObjectPtr<ADvtCarBody>>, 레벨 인스턴스에서 고름, 이 순서 = 차 번호). 명령: `SelectCar(int32)`·`NextCar()`·`PreviousCar()`, `SelectHood(int32)`·`NextHood()`·`PreviousHood()`. 규칙은 ReachStudy S4 D2와 같다(§0): 같은 번호 = 아무것도 안 함(이벤트 없음), 범위 밖 = 무시 + Warning, 상대 = 끝에서 돌아감·1개 이하면 아무것도 안 함, `SwitchCooldownSeconds`(기본 0.5 s, 0 = 끔) — **차와 hood는 쿨다운을 따로 센다**(차를 바꾼 직후 hood 다음 키를 막지 않게) |
| D6 | **차를 바꾸면 hood는 0번** | 새 차의 첫 hood 변형(`SortOrder` 가장 작은 것). 변형이 없으면 hood 번호 −1. 차를 바꾼 것은 hood 쿨다운을 시작하지 않는다 |
| D7 | **이벤트·게터** (S5 관전·S6 진행자 창이 쓴다) | `OnSelectionChanged(CarIndex, HoodIndex)` — 실제로 차나 hood가 바뀔 때 한 번(차를 바꿔 hood가 0이 되어도 한 번). 게터: `GetCarIndex`·`GetCarCount`·`GetCarName`·`GetHoodIndex`·`GetHoodCount`·`GetHoodName`(없으면 빈 글). 관전·진행자 글은 `DisplayName` 그대로 — `(k/N)` 붙이기는 S5·S6 몫 |
| D8 | **차량 skeletal mesh(물리 몸체)는 차가 있으면 숨긴다** | `bHideChassisWhenCarsExist`(기본 true). `Cars`에 유효한 차가 하나 이상이면 BeginPlay에서 차량 skeletal mesh를 숨긴다 — **보이기만**, 물리·충돌은 그대로. 차가 없으면 S1처럼 보인다 |
| D9 | **시작** | `InitialCar`(기본 0)·hood 0을 이벤트 없이 적용한다. `Cars`의 null 항목·중복 항목은 BeginPlay에서 빼고 항목마다 Warning(번호는 뺀 뒤 순서). `Cars`가 비면 명령은 아무것도 하지 않고 BeginPlay에서 Warning 한 줄 |
| D10 | **키** (Q-06) | `IMC_Drive`(S1)에 액션 `IA_PreviousCar`(`PageUp`), `IA_NextCar`(`PageDown`), `IA_PreviousHood`(`[`), `IA_NextHood`(`]`). `ADvtPlayerController`가 S1에서 찾은 차량의 명령을 부른다. 번호로 고르기는 S6 진행자 창 |
| D11 | **물리 영향 없음** | 교체는 보이기·휠 mesh·손 transform만 바꾼다. 차량 위치·속도·입력·Reset(S1)과 VR Pawn attach에 손대지 않는다 |

## 2. 지켜야 할 조건

- 헤드셋·PC 화면에 글 없음(D-06). 교체 알림은 `UE_LOG`.
- 차량 물리·입력·Reset·attach(S1)는 바뀌지 않는다. S2 판정 규칙은 바뀌지 않는다(반지름 값만 차가 넣는다).
- 차 이름·hood 이름을 코드에 두지 않는다(데이터).
- public: 샘플 차는 엔진 기본 도형과 이미 리포에 있는 샘플 에셋으로 만든다. CAD·회사 차명은 쓰지 않는다.

## 3. 범위 밖

- 관전 화면 글(S5), 진행자 창 버튼·목록(S6).
- 차마다 다른 물리·눈 위치(D-02: 같다).
- 페이드 교체 — 바로 바뀐다(필요하면 사용자가 꺼낸다).
- CAD 가져오기·실제 차 BP(S7).

## 4. 구현자가 정할 것

- `SortOrder`가 같을 때 이름 비교 방식(component 이름 `GetName`).
- 쿨다운 시각을 읽는 방법(시험에서 바꿀 수 있게 — ReachStudy `ReadSwitchTimeSeconds` `ReachDoorVariants.h:68`처럼).
- 선택 상태를 순수 구조체로 뺄지(권장 — §6 E1~E8을 엔진 없이).

## 5. 에디터 작업 (MCP, 코드 빌드·시험 뒤)

- `Content/Samples/Blueprints/`: `BP_SampleCar_A`(차체 = `SM_SportsCar` 등 `Content/Vehicles/` 정적 mesh, 휠 = S1·S2 샘플 휠), `BP_SampleCar_B`(차체 = 엔진 기본 도형 몇 개로 다른 모양, 휠 = 다른 기본 도형 mesh·다른 반지름). 각 차에 hood 변형 2–3개(엔진 큐브를 hood 자리에 높이를 다르게 — `Hood Low`·`Hood Mid`·`Hood High` 같은 이름).
- `L_DriveTest`: 두 차를 `BP_SampleVehicle`에 attach, `Cars` = [A, B].
- PIE에서 키 네 개로 교체, 주행 중 교체가 차를 흔들지 않는지 본다. 저장하고 에디터를 닫는다.

## 6. 경계 상황과 시험 (자동화 `DynamicVRTemplate.S3.*`)

| # | 상황 | 기대 |
|---|---|---|
| E1 | 차 3대, 0번에서 `SelectCar(0)` | 아무것도 안 함, 이벤트 0 |
| E2 | `SelectCar(5)` / `SelectCar(-1)` | 무시, Warning, 상태 그대로 |
| E3 | 0번에서 `PreviousCar` / 2번에서 `NextCar` | 2번 / 0번 |
| E4 | 차 1대에서 `NextCar` | 아무것도 안 함 |
| E5 | `NextCar` 뒤 0.3 s에 `NextCar` (쿨다운 0.5) | 두 번째 무시. 0.5 s 정확히 → 받음 |
| E6 | `NextCar` 직후 `NextHood` | 받음(쿨다운 따로) |
| E7 | `NextCar` 직후 `SelectCar(0)` | 받음(절대 명령은 쿨다운에 안 막힘) |
| E8 | hood 2번인 차에서 다른 차로 | 새 차 hood 0, 이벤트 1회(새 차, 0) |
| E9 | 변형 없는 차 | hood −1, 이름 빈 글, `NextHood` 아무것도 안 함 |
| E10 | `SortOrder` 2·0·1인 변형 셋 | 번호 0 = SortOrder 0인 것 |
| E11 | 보이기 | 활성 차 Actor 보임, 활성 변형 밑 mesh 보임, 다른 변형 밑 mesh 숨김, 차체 mesh 보임, 비활성 차 Actor 숨김 |
| E12 | 충돌 | BeginPlay 뒤 모든 차 Actor의 primitive가 `NoCollision` |
| E13 | 휠·손 값 | 차 B 활성 → 차량 휠 mesh = B의 mesh, 상대 transform = B 값, grip 반지름 = B 값, 손 transform = B 값. `SteeringWheelPivot` transform은 그대로 |
| E14 | 차 없음 | 명령 아무것도 안 함, 차량 skeletal mesh 보임(D8) |
| E15 | `Cars`에 null·중복 | 빠짐, 남은 순서로 번호 |
| E16 | 교체 전후 차량 | 차량 루트 transform과 VR Pawn attach 부모가 그대로 |
| E17 | EndPlay | 모든 차·변형 보이기 원래 값 |

E1~E10은 순수 구조체로, E11~E17은 시험 world(차량·차 Actor를 C++로 만든 시험용)로. 실제 주행 중 교체는 PIE와 Quest(T-03).

## 7. 완료 조건

- non-unity와 unity 빌드 성공, 경고·오류 0.
- `DynamicVRTemplate.*` 전체 시험 통과(S1·S2 포함).
- §5 결과와 `L_DriveTest`가 오류 없이 열림.
- README에 "Cars and hoods" 절(영어): 차 BP 만들기(`ADvtCarBody` 부모, 차 원점 = 차량 원점에 attach), hood 변형 component, `SortOrder`, 휠·손 값, CAD 휠 피벗 보정(`SteeringWheelMeshTransform`), 차·hood 키.

## 8. 로컬에서 확인할 것 (사용자, Quest 3 + 벅)

- T-03: 주행 중 `PageUp`/`PageDown`·`[`/`]`로 차·hood 교체. 차가 튀거나 멈추지 않는다. 바뀐 hood가 운전석에서 보인다. 휠 mesh가 차에 따라 바뀌고 실물 휠과 같이 돈다. 붙은 손이 새 휠 테두리에 있다. 헤드셋에 글 없음.
