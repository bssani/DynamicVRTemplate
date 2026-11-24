# S2 리뷰 — 휠 손 붙기

| 항목 | 값 |
|---|---|
| 명세 | `Docs/Specs/S2-steering-grip.md` |
| 브랜치 | `codex/S2-steering-grip` (`9b92941` 구현, `586c785` 기록) |
| 리뷰 | Claude, 2026-11-24 |

## 1회차 — 통과

- **판정:** 통과. 막는 항목 없음. 아래 N2-1은 고치지 않아도 머지할 수 있다.
- **범위:** 명세 D1~D9 안이다. S3·S4 작업 없음, Config·Meta XR·MCP 플러그인 변경 없음(`git ls-files`에 `Plugins/` 0개).
- **명세 대조:**
  - D2 판정: `DvtGrip::DistanceToRim`(도넛 거리, 로컬·스케일 뺌 `InverseTransformPositionNoScale`)과 `NextGrip`(enter "작으면", release "크면", 추적 끊김 유지, 기능 무효 → 잡음, release < enter → `Max`)이 명세 그대로다. 상태가 바뀔 때만 `UE_LOG` 한 줄 + `OnGripChanged`.
  - D3 보이기: 놓음 + 끊김 → 둘 다 숨김. Meta 손 `ConfidenceBehavior = None`.
  - D4 표시: `bHiddenInGame` + game world·`ShowFlags.Game`에서 proxy 없음/그리지 않음, 패키지(`!WITH_EDITOR`)에서 proxy 없음. 런타임 `DrawDebug*`·화면 글 없음(`grep` 확인). `PostEditChangeProperty`에서 bounds·render state 갱신.
  - D6 tick 순서: 차량 Actor tick과 grip component가 Meta 손 component 뒤(`AddTickPrerequisiteComponent`). 붙임(attach) 쪽 선행 조건(`SceneComponent.cpp:2465`)은 tick하지 않는 scene component라 순환이 생기지 않는다. S2 PIE 로그에 tick 순환 경고 없음.
  - D7 연결: `ConnectTrackedHands`가 `AttachVRPawn`과 따로 불린다(S4 D3와 맞음). 연결 전에는 붙은 손 보임.
  - D9 손 에셋 9개: AI_Driving 원본과 SHA-256 모두 같음(Claude 재확인).
- **재현(Claude):** S2 `.cpp`를 모두 다시 컴파일했다. non-unity(`-DisableUnity -Module=DynamicVRTemplate`)와 unity 모두 `Result: Succeeded`, 경고·오류 0(`Saved/Logs/ClaudeS2-nonunity.log`, `ClaudeS2-unity.log`). `DynamicVRTemplate.*` 17/17 통과, EXIT 0(`Saved/Logs/ClaudeS2Review.log`, `Config/UserEngine.ini` 없음).
- **에디터 증거:** `Saved/Screenshots/S2-blueprint-grip.png`에서 테두리(흰)·enter(초록)·release(주황) 원이 휠과 맞고, `S2-pie-grip-hidden.png`(PIE)에서 원이 없고 붙은 손이 9시·3시에 있다.
- **S1 R1-1(휠 피벗 기울기):** 닫음. BP Details의 `SteeringWheelPivot` 회전은 `(X 0, Y 20, Z 0)` = pitch 20°(rake)이고 회전축 X 둘레 roll은 0이다. 조향은 `Initial * FQuat(X, 각)`이라 기울어진 피벗의 X축 둘레로 돈다. S1 PIE 기록(`roll 20`)과 다른 이유는 확인하지 못했다. 지금 에셋과 PIE 화면이 맞으므로 더 보지 않는다.

### N2-1 기능 무효로 잡음이 될 때 로그 거리가 −1 — 열림 (작음, 선택)

- 추적 기능이 무효가 되어 놓음 → 잡음으로 바뀌면 로그가 `palm -1.0 cm from rim`으로 나온다(추적 안 된 손의 거리 자리값). 판정에는 영향이 없다.
- **제안:** 거리가 없을 때는 `(hand tracking unavailable)`이나 `(hand not tracked)`처럼 적는다. S3 작업 때 같이 고쳐도 된다.

### 기기에서 볼 것 (T-02, 사용자)

- Codex 보고 ⑤ 그대로: 가까이·멀리 손 교체, 경계에서 깜빡임, 크게 돌릴 때 붙은 손 유지, 무릎으로 내린 손이 허공에 남지 않음, enter 10 / release 18 cm가 벅 휠에 맞는지, 헤드셋에 원·선·글 없음.
- 추가: 차가 달리는 중에 추적 손이 손목을 따라가며 한 프레임 늦게 끌리지 않는지(손목 자리는 tracking → world 변환 값이고, 차량 물리 뒤에 오는지는 기기에서만 보인다).
