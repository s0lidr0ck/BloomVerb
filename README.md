# BloomVerb

Implementation spec and phased execution blueprint:

- [`BLOOMVERB_IMPLEMENTATION_SPEC.md`](./BLOOMVERB_IMPLEMENTATION_SPEC.md)
- [`GUI_DIRECTION_OPTIONS.md`](./GUI_DIRECTION_OPTIONS.md)

## Current implementation status

This repository now contains a JUCE-based BloomVerb plugin implementation with:

- full APVTS target parameter set (essential + signature + advanced)
- staged DSP engine (detectors, pre-delay, early reflections, diffusion stage, reverb tank, harmonic shaping, warp/dynamic behavior, width growth, tone filtering, wet/dry/output)
- multi-page UI (Main / Character / Advanced) with live parameter attachments
- VST3 + Standalone build targets via CMake

## Build (Linux)

Install dependencies (scripted):

```bash
./scripts/setup-linux-build-env.sh
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
cmake -S . -B build -DCMAKE_C_COMPILER=/usr/bin/gcc -DCMAKE_CXX_COMPILER=/usr/bin/g++
cmake --build build -j4
```

Run tests (when `Tests/BloomVerbEngineTests.cpp` and `Tests/BloomVerbStateTests.cpp` are present):

```bash
cmake -S . -B build -DBUILD_TESTING=ON -DCMAKE_C_COMPILER=/usr/bin/gcc -DCMAKE_CXX_COMPILER=/usr/bin/g++
cmake --build build -j4
ctest --test-dir build --output-on-failure
```

Artifacts:

- Standalone: `build/BloomVerb_artefacts/Standalone/BloomVerb`
- VST3: `build/BloomVerb_artefacts/VST3/BloomVerb.vst3`

## Loop test environment (no DAW required)

A dedicated test app is included so you can audition BloomVerb with a looping file:

- target: `BloomVerbLoopTester`
- source: `Source/Tester/LoopTesterMain.cpp`

Build and run:

```bash
cmake -S . -B build -DCMAKE_C_COMPILER=/usr/bin/gcc -DCMAKE_CXX_COMPILER=/usr/bin/g++
cmake --build build -j4 --target BloomVerbLoopTester
./build/BloomVerbLoopTester_artefacts/BloomVerb\ Loop\ Tester
```

Optional: pass an audio file path at launch to auto-load it:

```bash
./build/BloomVerbLoopTester_artefacts/BloomVerb\ Loop\ Tester "/path/to/loop.wav"
```

Inside the tester app:

- **Load Audio File** to choose a source
- **Loop** keeps playback cycling
- **Play/Stop** controls transport
- quick controls (Type, Mix, Decay, Size, Motion, Harmonic, Warp) let you audition behavior rapidly
