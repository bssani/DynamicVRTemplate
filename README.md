# DynamicVRTemplate

Seated VR driving on a physical seating buck (Unreal Engine 5.7.4, Meta Quest 3 over Link, hand tracking, Chaos Vehicles).
The driver sits in a physical buck with a steering wheel and pedals and drives a virtual car. Car body designs and hood variants can be switched at runtime. A spectator view goes to the PC monitor and an operator window controls the session; nothing is shown as text inside the headset.

Status: project skeleton (S0). See [Docs/Plan.md](Docs/Plan.md) (Korean).

## Setup

1. Unreal Engine 5.7.4, Visual Studio 2022 with the Game development with C++ workload, Meta Quest Link.
2. **Meta XR Plugin 1.205.0 is not in this repo** (parts are under the Oculus SDK License). Copy it to `Plugins/MetaXR` (the folder contains `OculusXR.uplugin`).
3. Build:
   ```
   "C:\Program Files\Epic Games\UE_5.7\Engine\Build\BatchFiles\Build.bat" DynamicVRTemplateEditor Win64 Development -Project="<abs path>\DynamicVRTemplate.uproject" -WaitMutex
   ```

Vehicle CAD is never committed: imported CAD lives under `Content/CAD/`, site-only assets under `Content/Local/` (both git-ignored).
