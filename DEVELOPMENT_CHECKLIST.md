# BloomVerb Development Checklist

This is the single source of truth for **build order + check order + current status**.

Last updated: 2026-03-12

---

## 1) Development order (phase sequence)

Follow this order unless a blocker forces a temporary swap:

1. **Stage A — Foundation**
2. **Stage B — Standard reverb core**
3. **Stage C — Type system**
4. **Stage D — Signature behavior**
5. **Stage E — Advanced shaping**
6. **Stage F — Productization**

Reference design source: `BLOOMVERB_IMPLEMENTATION_SPEC.md`

---

## 2) Current phase status

## Stage A — Foundation
- [x] JUCE plugin shell + CMake
- [x] APVTS canonical parameter set
- [x] Stereo-safe process path
- [x] Basic state serialization

Status: **Done**

## Stage B — Standard reverb core
- [x] Pre-delay
- [x] Early reflections
- [x] Diffusion stage
- [x] Main reverb tank (interim implementation)
- [x] Damping/width/mix/output in active path

Status: **Done (interim core)**

## Stage C — Type system
- [x] Type enum + selectable types
- [x] Type voicing module with per-type traits
- [x] Type affects internal behavior

Status: **Done (first voicing pass)**

## Stage D — Signature behavior
- [x] Motion behavior integrated
- [x] Dynamic wet shaping integrated
- [x] Harmonic tail coloring integrated
- [x] Warp tail contour behavior integrated
- [x] Swell + texture interactions integrated
- [ ] Dedicated custom in-loop FDN architecture

Status: **In Progress**

## Stage E — Advanced shaping
- [x] Tone/low-cut/high-cut integrated
- [x] Duck/mod/bloom/distance/transient preserve parameters integrated
- [x] Freeze behavior implemented
- [ ] Freeze transition polish under extreme automation

Status: **In Progress**

## Stage F — Productization
- [x] Multi-page placeholder UI (Main/Character/Advanced)
- [x] Factory preset scaffolding + browsing
- [x] Headless regression tests (engine/state/invariance/preset recall)
- [ ] Visual polish pass (asset phase)
- [ ] Optional visualizers/meters
- [ ] Cross-host matrix QA pass

Status: **In Progress**

---

## 3) Check/build/process list (run order)

Use this exact order during development:

1. **Configure**
```bash
cmake -S . -B build -DBUILD_TESTING=ON -DCMAKE_C_COMPILER=/usr/bin/gcc -DCMAKE_CXX_COMPILER=/usr/bin/g++
```

2. **Build everything**
```bash
cmake --build build -j4
```

3. **Run headless regression tests**
```bash
ctest --test-dir build --output-on-failure
```

4. **Manual audio sanity (loop tester)**
```bash
./build/BloomVerbLoopTester_artefacts/BloomVerb\ Loop\ Tester
```

5. **DAW/plugin sanity**
- Load VST3 and verify:
  - parameter automation does not click/zipper
  - preset recall is stable
  - freeze is stable
  - mono fold-down remains usable

---

## 4) Current test gate (must pass before merge)

- `BloomVerbEngineTests`
- `BloomVerbEngineInvarianceTests`
- `BloomVerbStateTests`
- `BloomVerbPresetRecallTests`

If any fail, no merge.

---

## 5) Next-up queue (priority)

1. Replace interim tank with a more explicit custom FDN architecture.
2. Add stronger freeze-entry/exit smoothing and automation stress QA.
3. Add performance benchmark target + CI perf gate.

