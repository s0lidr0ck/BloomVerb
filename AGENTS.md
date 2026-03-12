# BloomVerb

JUCE-based stereo character reverb audio plugin (C++20). Produces VST3 + Standalone targets, plus a LoopTester GUI harness.

## Cursor Cloud specific instructions

### Build & Run

See `README.md` for canonical build commands. Summary:

```
cmake -S . -B build -DCMAKE_C_COMPILER=/usr/bin/gcc -DCMAKE_CXX_COMPILER=/usr/bin/g++
cmake --build build -j$(nproc)
```

Run standalone: `./build/BloomVerb_artefacts/Standalone/BloomVerb`
Run loop tester: `./build/BloomVerbLoopTester_artefacts/BloomVerb\ Loop\ Tester`

### Caveats

- **JUCE is fetched at CMake configure time** via `FetchContent` (clones from GitHub). The initial `cmake -S . -B build` takes ~60-80 seconds due to JUCE download + juceaide build. Subsequent reconfigures reuse the cached clone in `build/_deps/`.
- **No automated test suite exists** — this project has no unit tests, integration tests, or CI. Validation is manual (run standalone or LoopTester and interact with the GUI).
- **No lint tooling is configured** — no clang-tidy, cppcheck, or similar is set up in the project.
- The standalone app shows "Audio input is muted to avoid feedback loop" on launch — this is normal JUCE standalone behavior.
- The UI currently shows "placeholder hardware layout" — parameter knobs/sliders are not yet wired into the page bodies, but the multi-page navigation (MAIN / CHARACTER / ADVANCED) and Type selector dropdown are functional.
- When running GUI apps, set `DISPLAY=:1` for the VM's X server.
