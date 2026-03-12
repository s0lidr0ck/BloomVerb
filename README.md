# BloomVerb

Implementation spec and phased execution blueprint:

- [`BLOOMVERB_IMPLEMENTATION_SPEC.md`](./BLOOMVERB_IMPLEMENTATION_SPEC.md)
- [`GUI_DIRECTION_OPTIONS.md`](./GUI_DIRECTION_OPTIONS.md)
- [`DEVELOPMENT_CHECKLIST.md`](./DEVELOPMENT_CHECKLIST.md)

## Current implementation status

This repository now contains a JUCE-based BloomVerb plugin implementation with:

- full APVTS target parameter set (essential + signature + advanced)
- staged DSP engine (detectors, pre-delay, early reflections, diffusion stage, reverb tank, harmonic shaping, warp/dynamic behavior, width growth, tone filtering, wet/dry/output)
- multi-page UI (Main / Character / Advanced) with live parameter attachments
- preset browser controls (Prev/Next + selector) backed by factory preset banks
- VST3 + Standalone build targets via CMake

## Build (Linux)

Install dependencies (scripted):

```bash
bash ./scripts/setup-linux-build-env.sh
```

Or manually:

```bash
sudo apt-get update
sudo apt-get install -y g++ gcc cmake \
  libasound2-dev libfreetype-dev libfontconfig1-dev libgl1-mesa-dev \
  libx11-dev libxext-dev libxinerama-dev libxcursor-dev libxrandr-dev \
  libxrender-dev libxcomposite-dev libxdamage-dev libxfixes-dev \
  libx11-xcb-dev libxkbcommon-dev
```

Configure and build:

```bash
cmake --preset linux-release
cmake --build --preset linux-release-build
```

Or run the shared local validation entrypoint:

```bash
bash ./scripts/validate-local.sh --mode full --preset linux-release
```

Run tests (when `Tests/BloomVerbEngineTests.cpp` and `Tests/BloomVerbStateTests.cpp` are present):

```bash
cmake --preset linux-tests
cmake --build --preset linux-tests-build
ctest --preset linux-tests-test
```

## Build (Windows)

With Visual Studio Build Tools / Visual Studio + CMake installed:

```powershell
cmake --preset windows-release
cmake --build --preset windows-release-build
ctest --preset windows-release-test
```

Or use the shared local validation script:

```powershell
./scripts/validate-local.ps1 -Mode full -Preset windows-release
```

Artifacts:

- Linux standalone: `out/build/linux-release/BloomVerb_artefacts/Standalone/BloomVerb`
- Linux VST3: `out/build/linux-release/BloomVerb_artefacts/VST3/BloomVerb.vst3`
- Windows standalone: `out/build/windows-release/BloomVerb_artefacts/Release/Standalone/BloomVerb.exe`
- Windows VST3: `out/build/windows-release/BloomVerb_artefacts/Release/VST3/BloomVerb.vst3`

## Fast headless regression tests (ctest)

The repository includes quick non-GUI regression checks that run without an audio device:

- `BloomVerbEngineTests`: DSP smoke test (finite output/no crash under stressed parameter sets)
- `BloomVerbEngineInvarianceTests`: sample-rate/block-size invariance sanity checks
- `BloomVerbStateTests`: parameter/state contract test (parameter IDs/defaults + APVTS round-trip)
- `BloomVerbPresetRecallTests`: preset apply + state restore regression checks
- `BloomVerbFDNRegressionTests`: deterministic guardrail for the custom FDN tank across types/sample rates/block sizes
- `BloomVerbFreezeAutomationStressTests`: rapid freeze on/off stress with bounded tail assertions
- `BloomVerbPerformanceGateTests`: release-build CPU budget gate for the DSP engine
- `BloomVerbMonoFoldDownTests`: mono render vs stereo-fold-down survivability regression

Configure, build tests, and run:

```bash
cmake --preset linux-tests
cmake --build --preset linux-tests-build
ctest --preset linux-tests-test
```

Fast shared validation entrypoints:

```bash
bash ./scripts/validate-local.sh --mode tests-only --preset linux-tests
```

```powershell
./scripts/validate-local.ps1 -Mode tests-only -Preset windows-tests
```

## Loop test environment (no DAW required)

A dedicated test app is included so you can audition BloomVerb with a looping file:

- target: `BloomVerbLoopTester`
- source: `Source/Tester/LoopTesterMain.cpp`

Build and run:

```bash
cmake --preset linux-release
cmake --build --preset linux-release-build --target BloomVerbLoopTester
./out/build/linux-release/BloomVerbLoopTester_artefacts/BloomVerb\ Loop\ Tester
```

Optional: pass an audio file path at launch to auto-load it:

```bash
./out/build/linux-release/BloomVerbLoopTester_artefacts/BloomVerb\ Loop\ Tester "/path/to/loop.wav"
```

Inside the tester app:

- **Load Audio File** to choose a source
- **Loop** keeps playback cycling
- **Play/Stop** controls transport
- **Copy QA Snapshot** copies the current manual-test state to the clipboard
- **Freeze** lets you exercise the transition controller directly in the tester
- status text reports transport / loop / freeze state during manual QA
- quick controls (Type, Mix, Decay, Size, Motion, Harmonic, Warp) let you audition behavior rapidly

## Productization QA notes

- Host QA matrix and evidence template: `docs/HOST_QA_MATRIX.md`
- Current UI pass keeps the locked Main / Character / Advanced layout and adds lightweight signal metering for fast local validation
- CI now runs the shared preset-based validation entrypoints on both Linux and Windows
