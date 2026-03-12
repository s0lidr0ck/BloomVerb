# BloomVerb Host QA Matrix

This file tracks the productization pass for host sanity after the local-agent migration work.

## Test focus

- plugin scan and instantiate
- preset browse / preset recall
- rapid automation on `mix`, `decay_s`, `freeze`, and `output_db`
- mono fold-down sanity
- UI responsiveness while audio is running

## Manual host matrix

| Host | Format | Scan/Load | Automation | Preset Recall | Freeze Stress | Mono Fold-down | Notes |
|---|---|---|---|---|---|---|---|
| JUCE Standalone | Standalone | Pass | Manual Pending | Manual Pending | Manual Pending | Manual Pending | Executable launched successfully on local Windows smoke pass |
| Reaper | VST3 | Pending | Pending | Pending | Pending | Pending | |
| Ableton Live | VST3 | Pending | Pending | Pending | Pending | Pending | |
| FL Studio | VST3 | Pending | Pending | Pending | Pending | Pending | |

## Loop tester checks

- Load a short loop and a longer loop in `BloomVerb Loop Tester`.
- Sweep `Type`, `Mix`, `Decay`, `Size`, `Motion`, `Harmonic`, and `Warp` during playback.
- Toggle the built-in `Freeze` control on and off repeatedly while a loop plays.
- Confirm stop/start and file reload keep the engine stable.
- Use `Copy QA Snapshot` to capture file, transport, freeze, and control state as evidence.

## Local validation baseline

- Linux: `bash ./scripts/validate-local.sh --mode tests-only --preset linux-tests`
- Windows: `./scripts/validate-local.ps1 -Mode tests-only -Preset windows-tests`

## Evidence log

- 2026-03-12: local Windows configure/build/ctest pass completed after FDN, freeze stress, and performance-gate rollout.
- 2026-03-12: `BloomVerb.exe` and `BloomVerb Loop Tester.exe` both launched successfully in a local smoke check.
