# DynamicVRTemplate

Seated VR driving on a physical seating buck (Unreal Engine 5.7.4, Meta Quest 3 over Link, hand tracking, Chaos Vehicles).
The driver sits in a physical buck with a steering wheel and pedals and drives a virtual car. Car body designs and hood variants can be switched at runtime. A spectator view goes to the PC monitor and an operator window controls the session; nothing is shown as text inside the headset.

Status: S5 spectator screen. Device testing remains not-tested. See [Docs/Plan.md](Docs/Plan.md) (Korean).

## Setup

1. Unreal Engine 5.7.4, Visual Studio 2022 with the Game development with C++ workload, Meta Quest Link.
2. **Meta XR Plugin 1.205.0 is not in this repo** (parts are under the Oculus SDK License). Copy it to `Plugins/MetaXR` (the folder contains `OculusXR.uplugin`).
3. Build:
   ```
   "C:\Program Files\Epic Games\UE_5.7\Engine\Build\BatchFiles\Build.bat" DynamicVRTemplateEditor Win64 Development -Project="<abs path>\DynamicVRTemplate.uproject" -WaitMutex
   ```

Vehicle CAD is never committed: imported CAD lives under `Content/CAD/`, site-only assets under `Content/Local/` (both git-ignored).

## Wheel and pedals

Open `L_DriveTest`. The player possesses the camera-only VR Pawn; the separate sample vehicle receives driving input through the PlayerController. Keyboard controls for development: A/D steer, W throttle, S brake (reverse at rest), Backspace reset. Reset fades out, moves to the nearest reset point, clears velocity, then fades in without recentering.

On the buck PC, enable the simracing device and configure **Project Settings → Raw Input**: add its hexadecimal VID/PID, then assign each physical axis a `Key`, `bInverted`, and `Offset`. Device IDs are site-specific and are not included in this repository.

`Content/Input/IMC_Drive` uses placeholder `GenericUSBController_Axis1` for steering, `Axis2` for throttle and `Axis3` for brake. Match these to your device in Raw Input settings and adjust mapping modifiers so steering covers -1 to +1 (center 0), and each pedal covers 0 to 1 (released 0). Verify both endpoints and release before driving. Set the vehicle's `SteeringLockToLockDegrees` to the same total rotation range as the simracing software (default 900 deg). The visible wheel follows this raw axis directly.

The sample SportsCar and PhysicsMaterials assets under `Content/Vehicles/` come from Epic's Vehicle template, copied with their original package paths. They are development samples; replace them with local assets for site use. Without saved alignment, the VR Pawn uses the approximate `DriverOrigin`. Quest 3 + buck validation is T-01 in the plan and remains `not-tested`.

## Cars and hoods

Create a Blueprint with `ADvtCarBody` as its parent. Add body meshes under its root and set `DisplayName`. Place each car in the driving level and attach it to the sample vehicle with zero relative location and rotation, and unit scale: car origin equals vehicle origin. Assign these level instances to the vehicle's `Cars` array in the desired selection order. All cars share the existing chassis physics and driver origin; car primitives have collision disabled at BeginPlay. When cars exist, the vehicle's own primitives outside `SteeringWheelPivot` are hidden unless `bHideChassisWhenCarsExist` is disabled. This includes its original static body meshes, which stay available when no cars are configured. Original game-hidden flags are restored at EndPlay.

For each hood option, add a `DvtHoodVariantComponent` and attach its meshes beneath it. Set its `DisplayName` and `SortOrder`. Hood indices follow ascending `SortOrder`, then case-sensitive component name for ties. A car may have no hood variants. The sample cars have three and two hood options. Parts that were authored with visibility disabled stay hidden when their car or hood is selected.

Set each car's `SteeringWheelMesh`, `SteeringWheelMeshTransform`, `RimRadiusCm`, `LeftGripHandTransform`, and `RightGripHandTransform`. Wheel and hand transforms are relative to the vehicle's fixed `SteeringWheelPivot`; keep that pivot at the buck wheel position. If a CAD wheel mesh uses the car origin as its pivot, use `SteeringWheelMeshTransform` to offset it into the wheel pivot frame before it rotates. An empty wheel mesh hides the wheel. Enter and release distances remain shared settings on `SteeringGrip`.

During Play, PageUp / PageDown select the previous / next car; [ / ] select the previous / next hood. Relative commands wrap around and have separate car and hood cooldowns (default 0.5 s). Direct `SelectCar(index)` and `SelectHood(index)` commands bypass cooldowns. Repeating the current index does nothing. A car switch retains the requested hood index, displaying the last available hood if the new car has fewer; returning to the original car restores the requested index. Selection only changes visuals, wheel mesh, and hand placement, including while driving. Quest 3 + buck validation is T-03 and remains `not-tested`.

## Buck alignment

`Plugins/VRCalibrationKit` is copied unchanged from [VRCalibrationKit commit e915647](https://github.com/bssani/VRCalibrationKit/tree/e915647/Plugins/VRCalibrationKit). Print marker PDFs from that repository's `Docs/Markers/` at **100% scale, 60 mm**. Attach markers to fixed parts of the seating buck. Default marker IDs are 1–4, matching `AlignPoint1`–`AlignPoint4` on the vehicle, in `Order` 1–4.

In `BP_SampleVehicle`, replace the four placeholder point positions with the measured buck marker centers relative to the shared vehicle/SgRP coordinate frame (+X forward, +Y right, +Z up, cm). The placeholders span a 40 × 40 cm square; real points must span at least 20 cm. Set `Alignment.FloorLocalZCm` to the real floor height in that same local frame. Keep the vehicle root scale **(1,1,1)** in the Blueprint and every level instance. A scaled root silently prevents alignment and sets `NeedsCheck`; check the component's `Reason`. All car designs share these points and alignment.

Open `Content/Samples/Maps/L_Calibrate` and Play. This level uses the engine PlayerController without driving bindings, so S saves calibration rather than braking. Use **M → G → S**: measure all four markers, preview their fit, then save. H undoes the preview. Read progress and `Max XY` in the PC Output Log (`LogVRCalMarker`). Headset messages are disabled; only the four alignment point numbers 1–4 may appear between G and H in this calibration level. The driving level has no calibrator or point numbers.

Open `L_DriveTest` to drive with the saved alignment. **F9** requests reapplication; a request during anchor discovery is deferred with one log line. Without saved alignment, the Pawn stays attached to `DriverOrigin`. Reapply and vehicle Reset never recenter. Saved values live under `Saved/Calibration/` and are not committed.

Anchors are off by default. On each buck PC, enable them in the ignored `Config/UserEngine.ini`:

```ini
[ConsoleVariables]
VRCal.AnchorAlignment=1
```

Enable Spatial Data in Link and look toward the calibrated area on Play. Wait for `Alignment Applied` (anchor restore usually takes a few seconds). To check restoration after a Link reconnect or editor restart, open `L_Calibrate`, wait for restore, then press **M only**, without G or S. The log `Marker fit vs pawn (Applied) | dX … | dY …` must say `Applied` and both absolute dX and dY must be within **2 cm**. `NoSavedValue` or `NeedsCheck` does not pass. If outside the limit, recalibrate and record the result. Reboot and next-day restoration still need device testing.

| Event | Action |
|---|---|
| Play restart, Link reconnect, headset sleep | Let the anchor restore; use the M-only check if uncertain. |
| Boundary (guardian) redrawn | Recalibrate in `L_Calibrate`: M → S. |
| Buck or markers moved | Recalibrate: M → S. |
| `anchor not found` | Check Link Spatial Data, look toward the calibrated area, restart Play. Recalibrate if it persists. |

With anchors disabled, saved values are valid only while the Stage origin stays unchanged; a Link reconnect can invalidate them. Remove `Config/UserEngine.ini` before automated tests because it overrides test defaults. Quest 3 + buck validation is T-04 and remains `not-tested`.

## Spectator screen

With Quest 3 + Link active in `L_DriveTest`, the PC game window shows the participant's eye view at 4:3 on the left and the current car and hood names in an opaque panel on the right. The panel goes only to the PC spectator screen; it never appears inside the headset. It starts only when exactly one vehicle has a selected car. `L_Calibrate` keeps the engine's default spectator view. Without an active headset, the subsystem stays silent and stops trying after 30 s.

Set names in `ADvtCarBody::DisplayName` and `UDvtHoodVariantComponent::DisplayName`. Empty names use `Car k` or `Hood k` (one-based). Cars without hood variants hide the entire hood group. Keep names short: **24 characters or fewer recommended**. Names wrap within the panel and use an ellipsis beyond three lines. Keyboard car and hood switches refresh the panel within 0.1 s at the default redraw rate.

To override defaults, add this section to `Config/DefaultGame.ini` and reopen the editor:

```ini
[/Script/DynamicVRTemplate.DvtSpectatorSubsystem]
EyeAspect=1.5
MinPanelWidth=0.4
RedrawsPerSecond=10
HeadingFontSize=16
CarFontSize=40
HoodFontSize=36
```

`EyeAspect` defaults to 4/3. `MinPanelWidth` defaults to 0.37 of window height; the eye shrinks proportionally if the window is too narrow. The panel follows the game window's client size, including moving between monitors. `BackgroundColor`, `TextColor`, `SecondaryTextColor`, and `LineColor` also accept config overrides as linear `(R=...,G=...,B=...,A=1)` values; defaults are converted from sRGB #131a26, #eef2f7, #9aa6b8, and #2e3a4e.

In Windows Display settings, extend the desktop to the spectator monitor, then move the PC game window onto that monitor. Quest 3 + Link validation is T-05 and remains `not-tested`: confirm the eye view is visible, the headset has no panel, switching updates names, and resizing preserves the layout.
