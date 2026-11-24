# S6 리뷰 — 진행자 창

명세: `Docs/Specs/S6-operator-window.md`

작업 브랜치: `codex/S6-operator-window` (베이스 `main@b143905`)

## 리뷰 1회차 — 통과 (Claude, 2026-11-24, `c0b3770`)

고칠 것 없음. 머지 가능(사용자 승인 뒤).

- **판정 = 명령(D4):** `FSelection`의 `SelectCar`·`SelectHood`·`StepCar`·`StepHood`가 새 `Can…` 판정을 먼저 부르고, 옛 조건식과 같은 결과다(손으로 대조: 범위 밖·같은 번호·1개 이하·쿨다운). `FResetSequence::Request`도 `CanRequest`를 부른다. 컨트롤러 `ResetVehicle`의 다른 조건은 `IsValid(Vehicle)`뿐이고 `CanResetVehicle`이 같은 것을 본다. 정렬은 `IsValid(Alignment) && !IsFindingAnchor()` — 키트 `RequestReapply`(`:194-199`)와 같다. E3 시험이 차 0·1·3 × hood 0·1·2 × 시각 0·0.4·0.5에서 판정과 명령 결과를 하나하나 비교한다.
- **버튼 = 키(D3):** `Execute`가 키에 묶인 같은 controller 함수를 부른다. E2가 키 바인딩 실행과 버튼을 두 world에서 나란히 돌려 차·hood 번호·쿨다운·Reset 단계·정렬 호출 수가 같은지 본다.
- **초점(D2):** 창 `ActivationPolicy(Never)`·`FocusWhenFirstShown(false)`, 버튼·스크롤 상자 `IsFocusable(false)`. 버튼 처리 뒤(명령이 버려져도)·창 띄운 뒤·스크롤 뒤 `RestoreGameFocus` = `SetAllUserFocusToGameViewport` + 게임 창 `BringToFront(true)`. E8이 호출 횟수를 본다. Codex가 헤드셋 없는 PIE에서 실제 마우스 클릭 뒤 W 키가 게임으로 가는 것을 Slate 로그로 확인했다.
- **창(D1):** 닫기 버튼 없음, 최대화 없음, 자리·크기 세션 기억(최소화 상태 크기는 저장 안 함), 자동화·`CanEverRender` 거짓이면 안 만듦, OS가 닫으면 툴바로 다시 만듦, `Deinitialize`에서 닫음.
- **손(D6):** `ApplyHandSample`에서 값만 저장하고, Pawn 연결 전에는 거짓으로 둔다. 잡기 규칙·이벤트·보이기는 그대로.
- **툴바(D7):** 에디터 모듈만, 게임 타깃 `DynamicVRTemplate.target`에 `DynamicVRTemplateEditorTools` 없음(직접 확인). SVG가 vBoR 원본과 바이트까지 같다. 등록 해제·스타일 해제 있음.
- **디자인(D9):** 창용 색 6개를 `DvtUiStyle.h`에 더했고 S5 상수는 그대로. 주 버튼은 `NeedsCheck` + 받을 수 있을 때의 `Reapply alignment`뿐.
- **시험·빌드:** 로그에서 직접 셈 — `DynamicVRTemplate.*` 49, `VRCalibrationKit.*` 46, 모두 성공. 빌드 3개(non-unity, unity, 게임) `Succeeded`, `warning C`/`error C` 없음. S1–S5 시험 파일 변경 없음.
- **README:** 명세 §7 항목 모두 있음.

### 참고 — T-06에서 볼 것 (명세 §8-1에 더함)

되돌림은 버튼 처리·창 띄움·스크롤 뒤에만 한다. 진행자가 창의 **빈 곳이나 제목 줄**(창 끌기)을 누르면 Windows가 진행자 창을 활성화하고 되돌림이 없다. Codex는 창 활성화 이벤트에서 되돌리는 방법을 시도했으나 실제 클릭의 mouse-up을 막아 뺐다(보고 ③). 명세는 버튼에 대해서만 요구했으므로 구현은 맞다. 기기에서 이 경우 운전이 끊기는지 §8-1에서 함께 보고, 끊기면 그때 방법을 정한다.

T-06은 `not-tested`(Quest 3 + Link + 벅, 사용자).
