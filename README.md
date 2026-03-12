# BloomVerb

Implementation spec and phased execution blueprint:

- [`BLOOMVERB_IMPLEMENTATION_SPEC.md`](./BLOOMVERB_IMPLEMENTATION_SPEC.md)

## Current implementation status

This repository now contains a JUCE-based BloomVerb plugin implementation with:

- full APVTS target parameter set (essential + signature + advanced)
- staged DSP engine (detectors, pre-delay, early reflections, diffusion stage, reverb tank, harmonic shaping, warp/dynamic behavior, width growth, tone filtering, wet/dry/output)
- multi-page UI (Main / Character / Advanced) with live parameter attachments
- VST3 + Standalone build targets via CMake

## Build (Linux)

Install dependencies:

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

Artifacts:

- Standalone: `build/BloomVerb_artefacts/Standalone/BloomVerb`
- VST3: `build/BloomVerb_artefacts/VST3/BloomVerb.vst3`
