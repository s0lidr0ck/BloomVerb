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
- [x] Dedicated custom in-loop FDN architecture

Status: **Done**

## Stage E — Advanced shaping
- [x] Tone/low-cut/high-cut integrated
- [x] Duck/mod/bloom/distance/transient preserve parameters integrated
- [x] Freeze behavior implemented
- [x] Freeze transition polish under extreme automation

Status: **Done**

## Stage F — Productization
- [x] Multi-page placeholder UI (Main/Character/Advanced)
- [x] Factory preset scaffolding + browsing
- [x] Headless regression tests (engine/state/invariance/preset recall)
- [x] Visual polish pass (assetless production skin)
- [x] Optional visualizers/meters
- [ ] Cross-host matrix QA pass

Status: **In Progress**

---

## 3) Check/build/process list (run order)

Use this exact order during development:

1. **Configure**
```bash
cmake --preset linux-tests
```

2. **Build everything**
```bash
cmake --build --preset linux-tests-build
```

3. **Run headless regression tests**
```bash
ctest --preset linux-tests-test
```

4. **Manual audio sanity (loop tester)**
```bash
./out/build/linux-release/BloomVerbLoopTester_artefacts/BloomVerb\ Loop\ Tester
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
- `BloomVerbFDNRegressionTests`
- `BloomVerbFreezeAutomationStressTests`
- `BloomVerbPerformanceGateTests`
- `BloomVerbMonoFoldDownTests`

If any fail, no merge.

---

## 5) Next-up queue (priority)

1. Execute the host QA matrix in `docs/HOST_QA_MATRIX.md`.
2. Add preset-character retuning against the new FDN tank if required by listening tests.
3. Expand performance coverage with multi-instance benchmarks.

