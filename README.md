# DynamicVRTemplate

Seated VR driving on a physical seating buck (Unreal Engine 5.7.4, Meta Quest 3 over Link, hand tracking, Chaos Vehicles).
The driver sits in a physical buck with a steering wheel and pedals and drives a virtual car. Car body designs and hood variants can be switched at runtime. A spectator view goes to the PC monitor and an operator window controls the session; nothing is shown as text inside the headset.

Status: S1 drive rig. Device testing remains not-tested. See [Docs/Plan.md](Docs/Plan.md) (Korean).

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

The sample SportsCar and PhysicsMaterials assets under `Content/Vehicles/` come from Epic's Vehicle template, copied with their original package paths. They are development samples; replace them with local assets for site use. S1 uses an approximate driver floor origin; buck alignment is added in S4. Quest 3 + buck validation is T-01 in the plan and remains `not-tested`.
