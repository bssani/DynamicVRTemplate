# S1 주행 rig — 리뷰

## 1회차 (2026-11-24, `8108109`)

**판정: 통과. `main`에 머지한다.** Quest 3 + 벅 T-01은 `not-tested`.

- 범위: 명세 D1~D11 안. 명세 밖 기능 없음. 금지 사항(차량 possess, recenter, 화면 글, 장치 ID 코드화) 없음.
- 확인한 것:
  - 차량 생성자에서 `SetRequiresControllerForInputs(false)`, 자동 possess 끔(`DvtVehicle.cpp`).
  - 입력은 `Triggered`/`Completed`에 묶였고 범위를 자른다(`DvtPlayerController.cpp`).
  - 휠은 controller가 넘긴 축 값으로 돈다(보간된 조향값을 쓰지 않음).
  - Reset은 순수 상태(`DvtDriveLogic.h FResetSequence`)로 옮김을 한 번만 하고, recenter하지 않는다.
  - 화면 메시지 subsystem은 ReachStudy와 같다. Config 세 줄(`r.RayTracing`·`r.VirtualTextures`·`vr.InstancedStereo`)에 근거 주석이 있다.
- 재현(Claude): non-unity `-DisableUnity -Module=DynamicVRTemplate`, unity 둘 다 `Result: Succeeded`, 경고·오류 0. `DynamicVRTemplate.*` 7/7 통과, EXIT 0(`Saved/Logs/ClaudeS1Review.log`). 에디터 쪽은 Codex 보고와 PIE 기록(`Saved/S1-PIE-trace.json`)으로 확인 — 입력 → 속도, 놓음 = 0, attach 유지.

### R1-1 샘플 휠 피벗이 휠 회전축 둘레로 20° 돌아가 있다 — 반영함 (`9b92941`)

- **제안:** PIE 기록에서 조향 0일 때 `SteeringWheelPivot` 회전이 `roll 20`이다. 오른쪽 끝까지 돌리면 `roll −70`(= 20 − 450, 360 감김). `SteeringSpinAxis`가 X이므로 roll은 **휠이 제 평면 안에서 돈 것**이다. 운전대 기울기(rake)라면 Y축 둘레(pitch)여야 한다.
  - 지금 샘플 휠은 원기둥이라 보이지 않는다. S2에서 붙은 손을 9시·3시에 두거나 S3에서 실제 휠 mesh를 쓰면, 휠과 손이 20° 돌아간 채로 보인다.
  - S2 §5 에디터 작업에서 `BP_SampleVehicle`의 피벗 기울기를 회전축이 아닌 축으로 고친다(또는 rake를 0으로 두고 휠 mesh 쪽에 기울기를 준다). 고친 뒤 조향 0에서 휠 위쪽이 12시인지 스크린샷으로 확인한다.
- **확인할 시험:** S2 §5 스크린샷. S2 E-시험에는 넣지 않는다(에셋 배치).
- **Codex 반영·확인:** S2 MCP 작업에서 BP CDO와 배치 인스턴스를 읽었을 때 이미 `pitch 20 / yaw 0 / roll 0`이었다(기존 리뷰의 `roll 20` 기록과 다름). 이름을 명시한 `Rotator(pitch=20, yaw=0, roll=0)`를 적용했고, PIE 조향 0에서도 같은 값을 읽었다. 고정 손을 9시·3시로 놓고 휠 위 방향과 함께 확인했다. 증거: `Saved/Screenshots/S2-editor-grip.png`, `S2-blueprint-grip.png`, `S2-pie-grip-hidden.png`(PIE 자유 카메라). Quest 3 T-02는 `not-tested`.

### R1-2 FBX 원본 4개(2.5 MB)가 커밋에 들어갔다 — 열림 (사용자 결정)

- **제안:** `Content/Vehicles/SportsCar/*.FBX`는 가져오기 원본이라 엔진이 읽지 않는다. 지우면 리포가 2.5 MB 줄어든다. 머지를 막지 않는다 — 지울지는 사용자가 정한다.

### 참고 (지적 아님)

- Reset 자리는 Reset을 누른 순간의 차 위치로 고른다(페이드 0.3 s 동안 차가 더 가도 다시 고르지 않음). 명세는 시점을 정하지 않았고, 누른 순간이 진행자 의도에 맞다.
- PIE 기록에는 "완전히 선 뒤 5 s 기다렸다가 다시 출발" 장면이 없다(1.7 s 정지 뒤 출발만 있음). controller 없이 잠든 차가 깨어나는지는 T-01에서 본다(명세 §8).
