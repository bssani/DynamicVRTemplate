# S3 구현 질문 — 차·hood 교체

명세: `Docs/Specs/S3-car-hood-switch.md`

작업 브랜치: `codex/S3-car-hood-switch`

작성: Codex, 2026-11-24. Claude의 구현 리뷰 전 질문 기록이다.

## 질문

### Q3-1 S1 샘플 차량에 남아 있는 static 차체의 처리 — 열림

- **사실:** `BP_SampleVehicle`에는 native `VehicleMesh` 외에 S1에서 만든 `Body`, `Glass`, `Wheel_FL`, `Wheel_FR`, `Wheel_BL`, `Wheel_BR` static mesh component가 있다. S3 D8대로 `VehicleMesh`만 숨기면 이 여섯 component는 그대로 보인다. 새 차 `BP_SampleCar_B`를 골라도 기존 주황색 샘플 차체가 겹친다.
- **확인:** PIE에서 선택 `(CarIndex, HoodIndex) = (1, 1)`, A Actor `hidden = true`, B Actor `hidden = false`, `VehicleMesh.hidden_in_game = true`를 읽었다. 보이는 SportsCar static mesh 6개의 owner는 A가 아니라 `BP_SampleVehicle_C_0`였다. 증거: `Saved/Logs/CodexS3-editor.log`, `Saved/Screenshots/WindowsEditor/S3-B-game-camera.png`. 사진은 기기 시야가 아니라 검증용 임시 카메라다.
- **질문:** S1의 기존 static mesh 6개를 차량 BP에서 없애고 차 A로 옮기는 것인지, `Cars` 유무에 따라 이 6개도 숨겼다가 복원할 것인지 명세에 정해 달라. 후자는 대상 지정 방법과 D8의 차 없는 경우 동작도 정해야 한다. 임의로 이름을 코드에 넣거나 전체 자식을 숨기지는 않았다(휠·고정 손도 같은 skeletal mesh 아래에 있다).
- **현재 상태:** 코드는 D8의 skeletal mesh만 숨긴다. S1 static mesh 6개는 수정하지 않았다. 샘플 A/B와 hood 및 입력 액션은 만들었으나 §5의 최종 시각 확인·완료 조건은 **미완료**다. 질문 답변 전 구현 작업을 멈춘다.

## 확인한 시험

- non-unity `-DisableUnity -Module=DynamicVRTemplate`, unity 빌드 성공. 최종 컴파일 경고·오류 0. 로컬 MCP의 `StructUtils` 폐기 예정 안내는 별도다.
- `DynamicVRTemplate.*`: 발견 28, 완료 28, Success 28, Fail 0, `TEST COMPLETE. EXIT CODE: 0`. S1·S2 17 + S3 11. 로그 `Saved/Logs/CodexS3-automation.log`.
- PIE 키 입력: `]` A hood 1→2, `PageDown` B hood 1, `[` B hood 0, `PageUp` A hood 0. 주행 기록 19개에서 Pawn attach 유지. 교체 당시 속도는 이어졌지만 최종 보이기 문제 때문에 §5 통과로 쓰지 않는다.
- T-03 Quest 3 + 벅: `not-tested`.
