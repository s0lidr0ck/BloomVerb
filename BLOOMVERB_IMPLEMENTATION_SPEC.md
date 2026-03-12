# BloomVerb Cursor Implementation Spec (Target Blueprint)

This document converts the BloomVerb end-goal product vision into an execution-ready engineering spec for a JUCE plugin project.

---

## 1) Build brief (north star)

Build **BloomVerb** as a JUCE-based stereo character reverb plugin with a shared modular DSP architecture including:

- pre-delay
- early reflections
- diffuser network
- FDN tank
- in-loop modulation
- in-loop harmonic shaping
- time-warped decay behavior
- dynamic wet shaping
- width growth
- multi-page UI exposing both standard controls and BloomVerb signature controls

BloomVerb must preserve type + behavior layering:

- **Type** = base algorithm personality (Plate/Hall/etc.)
- **Character controls** = behavior modifiers (Motion/Dynamic/Harmonic/Warp/Swell/Texture)

---

## 2) Suggested source layout and module responsibilities

> Paths are suggestions for a standard JUCE plugin generated from `AudioPluginHost` templates.

### 2.1 Root plugin shell

1. `Source/PluginProcessor.h/.cpp`
   - Own APVTS, preset manager, DSP engine instance
   - Lifecycle: `prepareToPlay`, `releaseResources`, `processBlock`
   - State: serialize/deserialize APVTS + preset metadata
   - Host safety: bypass-safe, block-size independent behavior

2. `Source/PluginEditor.h/.cpp`
   - Page routing and layout container
   - LookAndFeel registration
   - Global controls: type selector, page navigation, resize, branding

### 2.2 Parameters and metadata

3. `Source/Parameters/BloomVerbParameterIDs.h`
   - Canonical `constexpr` string IDs
   - Enum wrappers for safe switching

4. `Source/Parameters/BloomVerbParameterLayout.h/.cpp`
   - APVTS layout factory
   - Ranges/skews/defaults/units/text formatting
   - Page/category metadata (Main/Character/Advanced)

5. `Source/Parameters/BloomVerbParameterAccess.h`
   - Cached atomics/smoothed accessor objects
   - Avoid repeated APVTS lookup in audio thread

### 2.3 DSP core orchestration

6. `Source/DSP/BloomVerbEngine.h/.cpp`
   - Top-level DSP graph controller
   - Fixed process order and per-sample/per-block policy
   - Type voicing profile application

7. `Source/DSP/BloomVerbVoiceProfile.h/.cpp`
   - Type-specific voicing tables (Plate/Hall/Room/Cloud/Bloom...)
   - Maps type -> default weighting and internal bias values

8. `Source/DSP/BloomVerbSmoothers.h`
   - Parameter smoothing definitions (time constants)
   - Anti-zipper utilities for all automatable params

### 2.4 DSP subsystems

9. `Source/DSP/InputConditioner.h/.cpp`
   - Input trim
   - Optional transient preserve split
   - Optional subtle pre-saturation

10. `Source/DSP/EnvelopeDetectorBank.h/.cpp`
    - Fast envelope, slow envelope, release-state detector
    - Shared detector outputs for Swell/Dynamic/Warp/Width growth

11. `Source/DSP/PredelayStage.h/.cpp`
    - Stereo pre-delay line
    - Type-aware effective timing offsets
    - Swell and Distance interactions

12. `Source/DSP/EarlyReflectionEngine.h/.cpp`
    - Pattern bank (Room/Plate/Cloud/Chamber style)
    - Near/Far, hard/soft, discrete/smeared blending

13. `Source/DSP/DiffuserNetwork.h/.cpp`
    - Multi-stage allpass diffuser chain
    - Dynamic diffusion curve (onset clearer, late tail smoother)
    - Texture and Warp integration points

14. `Source/DSP/FDNCore.h/.cpp`
    - Main tank (target architecture 8-line FDN, optional 4-line interim)
    - Feedback matrix, asymmetrical delays, stability protections
    - Per-line damping/weighting hooks

15. `Source/DSP/ModulationSystem.h/.cpp`
    - Motion engine: delay drift, decorrelation drift, modulation depth
    - Rate/depth modulation mapping and clamping

16. `Source/DSP/HarmonicLoopColor.h/.cpp`
    - In-loop saturation/color shaping
    - Modes: clean/tape-ish/tube-ish/soft clip/warm compression feel
    - Feedback-safe nonlinear processing limits

17. `Source/DSP/WarpShaper.h/.cpp`
    - Time-warped decay shaping curves
    - Controls brightness/density/width over tail time
    - Warp modes (normal, bloom, expand, collapse, reverse-ish, halo)

18. `Source/DSP/OutputToneStage.h/.cpp`
    - Damping/Tone/LowCut/HighCut final shaping
    - Type-aware defaults

19. `Source/DSP/SpatialWidthStage.h/.cpp`
    - Width + decorrelation with time-aware growth behavior
    - Type-specific width growth scaling

20. `Source/DSP/DynamicWetShaper.h/.cpp`
    - Ducking and release-bloom logic
    - Early-vs-tail differential ducking

21. `Source/DSP/FreezeController.h/.cpp`
    - Infinite/freeze behavior
    - Safe entry/exit, feedback clamp and transition smoothing

### 2.5 UI components

22. `Source/UI/BloomVerbLookAndFeel.h/.cpp`
    - Shared drawing style and control theming

23. `Source/UI/Controls/KnobWithLabel.h/.cpp`
    - Reusable parameter knob widget

24. `Source/UI/Pages/MainPage.h/.cpp`
25. `Source/UI/Pages/CharacterPage.h/.cpp`
26. `Source/UI/Pages/AdvancedPage.h/.cpp`
   - One page class per control set

27. `Source/UI/Visualization/BloomScope.h/.cpp` (optional phase)
   - Motion activity + width visualization

### 2.6 Presets/state

28. `Source/Presets/BloomVerbPreset.h`
29. `Source/Presets/BloomVerbPresetBank.h/.cpp`
30. `Source/Presets/BloomVerbPresetManager.h/.cpp`
   - Curated preset definitions, recall, and future state migration utilities

---

## 3) Full target parameter inventory

### 3.1 Essential

- Type
- Size
- Decay
- PreDelay
- Diffusion
- Damping
- Early
- Width
- Mix
- Output

### 3.2 Signature

- Motion
- Dynamic
- Harmonic
- Warp
- Swell
- Texture

### 3.3 Advanced

- Tone
- Low Cut
- High Cut
- Mod Rate
- Mod Depth
- Duck Amount
- Bloom Amount
- Freeze
- Distance
- Transient Preserve

---

## 4) Canonical APVTS parameter IDs and defaults

Use these IDs exactly for long-term preset compatibility.

| Name | ID | Type/Range | Default |
|---|---|---|---|
| Type | `type` | choice | `Hall` |
| Size | `size` | 0.0..1.0 | 0.50 |
| Decay | `decay_s` | 0.10..20.0 s | 2.80 |
| PreDelay | `predelay_ms` | 0..250 ms | 20 |
| Diffusion | `diffusion` | 0.0..1.0 | 0.65 |
| Damping | `damping` | 0.0..1.0 | 0.45 |
| Early | `early` | 0.0..1.0 | 0.35 |
| Width | `width` | 0.0..2.0 | 1.00 |
| Mix | `mix` | 0.0..1.0 | 0.25 |
| Output | `output_db` | -24..+12 dB | 0 dB |
| Motion | `motion` | 0.0..1.0 | 0.30 |
| Dynamic | `dynamic` | 0.0..1.0 | 0.35 |
| Harmonic | `harmonic` | 0.0..1.0 | 0.20 |
| Warp | `warp` | 0.0..1.0 | 0.30 |
| Swell | `swell` | 0.0..1.0 | 0.20 |
| Texture | `texture` | 0.0..1.0 | 0.25 |
| Tone | `tone` | -1.0..+1.0 | 0.00 |
| Low Cut | `lowcut_hz` | 20..1200 Hz | 80 |
| High Cut | `highcut_hz` | 1000..20000 Hz | 12000 |
| Mod Rate | `mod_rate_hz` | 0.01..8.0 Hz | 0.60 |
| Mod Depth | `mod_depth` | 0.0..1.0 | 0.30 |
| Duck Amount | `duck_amount` | 0.0..1.0 | 0.35 |
| Bloom Amount | `bloom_amount` | 0.0..1.0 | 0.50 |
| Freeze | `freeze` | bool | `false` |
| Distance | `distance` | 0.0..1.0 | 0.40 |
| Transient Preserve | `transient_preserve` | 0.0..1.0 | 0.50 |

### Type choices (target catalog)

`Plate`, `Hall`, `Room`, `Cloud`, `Bloom`, `Grain`, `Chamber`, `Dream`

Recommended stage rollout:

1. Stage C initial: Plate, Hall, Room, Cloud, Bloom
2. Later: Grain, Chamber, Dream

---

## 5) UI page map (shipping structure)

### Main page

- Type
- Size
- Decay
- PreDelay
- Mix
- Motion
- Texture
- Swell

### Character page

- Dynamic
- Harmonic
- Warp
- Damping
- Width
- Early

### Advanced page

- Diffusion
- Tone
- Low Cut
- High Cut
- Duck Amount
- Mod Rate
- Mod Depth
- Bloom Amount
- Output
- Freeze
- (optional) Distance, Transient Preserve

---

## 6) Audio process order (hard requirement)

Audio path in `BloomVerbEngine::processBlock`:

1. Input conditioning
2. Envelope detector update
3. Swell/transient preserve stage
4. Pre-delay
5. Early reflections
6. Diffuser network
7. Main FDN tank
8. In-loop modulation
9. In-loop harmonic color
10. Time-warp/decay shaping
11. Output damping + tone/EQ
12. Width/decorrelation stage
13. Dynamic wet shaping (duck/bloom)
14. Wet/dry mix + output gain

Implementation note:
The in-loop processes (modulation/harmonic/warp) must affect recirculating energy, not only post-FDN output.

---

## 7) Macro mapping philosophy (do not collapse)

Each signature macro intentionally drives multiple internals:

- **Motion**: modulation depth, delay drift, stereo drift, tail animation
- **Dynamic**: ducking amount, release bloom, input-sensitive width/density
- **Harmonic**: in-loop saturation amount, warm compression feel, tail thickening
- **Warp**: decay contour, density-vs-time, brightness-vs-time, width-vs-time
- **Swell**: wet fade-in, early suppression, transient bypass balance, predelay interaction
- **Texture**: diffusion contour, grain density, reflection sharpness

Constraint:
Never map these to single-parameter shallow proxies.

---

## 8) Type voicing targets (first-order behavior)

- **Plate**: tight, immediate, bright, dense; lower width growth; less swell by default
- **Hall**: balanced, smooth, all-purpose
- **Room**: shorter, clearer, stronger early reflections, lower modulation
- **Cloud**: larger, darker, softer onset, stronger width growth
- **Bloom**: restrained front edge, obvious late expansion, release-driven opening
- **Grain**: lower diffusion, deliberate particulate texture
- **Chamber**: audible early character, intimate mids, dimensional
- **Dream**: lush stylized modulation, soft-focus cinematic tail

---

## 9) Milestone checklist for agent execution

## Stage A — Foundation

- [ ] JUCE plugin builds (VST3/AU if enabled) and passes smoke test
- [ ] APVTS created with canonical IDs and defaults
- [ ] Basic stereo-safe process path with dry passthrough
- [ ] Parameter smoothing framework in place

Exit criteria:
- No crashes when automating any exposed parameter

## Stage B — Standard core

- [ ] Pre-delay stage implemented
- [ ] Early reflection generator implemented
- [ ] Diffuser chain implemented
- [ ] FDN core integrated (4-line acceptable as interim)
- [ ] Damping/width/mix/output stable and musical

Exit criteria:
- Tail free from severe metallic ringing at moderate settings
- Block-size independent output

## Stage C — Type system

- [ ] Type enum and voicing profile map integrated
- [ ] Plate/Hall/Room/Cloud/Bloom implemented and audibly distinct
- [ ] Type changes are click-safe and automatable

Exit criteria:
- Type-switch AB tests demonstrate personality differences without level jumps

## Stage D — Signature behavior layer

- [ ] Motion subsystem connected (diffuser/tank/decorrelation)
- [ ] Swell behavior integrated with detector system
- [ ] Dynamic wet shaping (duck + release bloom) operational
- [ ] In-loop Harmonic color implemented
- [ ] Warp time-shaping implemented
- [ ] Texture mapped to diffusion/particulate behavior

Exit criteria:
- “Living tail” behavior is obvious at moderate settings without artifacts

## Stage E — Advanced shaping

- [ ] Tone/LowCut/HighCut finalized
- [ ] Duck Amount/Mod Rate/Mod Depth exposed and stable
- [ ] Bloom Amount, Distance, Freeze, Transient Preserve integrated
- [ ] Freeze entry/exit transition smoothing validated

Exit criteria:
- Freeze and dynamic modes remain stable at 44.1/48/96 kHz

## Stage F — Productization

- [ ] Multi-page UI polished with clear grouping
- [ ] Preset manager + curated factory banks shipped
- [ ] Optional visualization implemented (if budget allows)
- [ ] CPU optimization pass (hot loops, denormals, SIMD opportunities)
- [ ] QA: automation stress, preset recall, mono compatibility, host matrix

Exit criteria:
- Mix-ready subtle settings and cinematic extreme settings both viable

---

## 10) Preset bank starter matrix

Ship curated families:

- **Utility**: Small Plate, Vocal Hall, Tight Room, Worship Verb, Drum Plate, Clean Hall
- **Ambient**: Bloom Pad, Cloud Lift, Infinite Veil, Dream Wash, Slow Cathedral
- **Guitar**: Swell Plate, Ambient Bloom, Halo Clean, Post-Rock Cloud, Lead Lift
- **Vocal**: Intimate Chamber, Modern Hall, Air Plate, Soft Bloom, Dream Verb
- **Cinematic**: Suspended Space, Distant Signal, Hollow Chamber, Blooming Void, Rising Sky

---

## 11) Engineering hard requirements

### Audio quality

- Smooth tails with minimal metallic ringing
- Strong mono survivability
- No zipper noise
- Stable feedback behavior

### Performance

- Multiple-instance practical CPU use
- No runaway modulation cost
- Denormal protection everywhere

### Stability

- Full parameter automation support
- Reliable state/preset recall
- Sample-rate aware and block-size independent
- No host crashes

---

## 12) Non-negotiables (guardrails)

Do **not**:

- collapse product into a 6-knob utility reverb
- remove type/behavior separation
- reduce Motion to single LFO depth
- reduce Texture to simple tone EQ
- reduce Warp to decay multiplier only
- move Harmonic to output-only processing
- skip early reflections or proper diffusion

These constraints preserve BloomVerb identity.
