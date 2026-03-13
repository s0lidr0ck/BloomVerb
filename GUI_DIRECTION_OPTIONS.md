# BloomVerb GUI Direction Options (Assetless Phase)

This document captures GUI directions inspired by Brainworx 9099, UAD EQP-1A, FabFilter, Scheps Omni, and iZotope while keeping implementation asset-free until DSP/feature lock.

For the current asset-pass handoff, see [`docs/UI_ASSET_SPEC.md`](./docs/UI_ASSET_SPEC.md).

## Shared constraints for current phase

- No custom image assets yet
- Controls must be placed where they will live in final product
- Page structure is fixed now to avoid future migration pain
- Placeholder style should still communicate "hardware instrument"

## Option A - Console Strip Hybrid (Recommended now)

**Influence mix:** Brainworx + FabFilter  
**Feel:** Analog panel materials with modern clarity

- Dark metal/chassis background
- Brass/cream accent colors
- Rotary controls with visible pointer caps
- Fixed page tabs: Main / Character / Advanced
- Strong visual grouping by role, not by implementation detail

Pros:

- Fast to implement with JUCE primitives only
- Reads clearly during heavy parameter development
- Easy to skin later without layout changes

Cons:

- Less photoreal "vintage hardware" than a full asset pass

## Option B - Vintage Rack Faceplate

**Influence mix:** UAD EQP-1A + analog outboard  
**Feel:** Retro panel, engraved labels, larger knob spacing

- Flat faceplate zones with pseudo-engraving text
- More generous spacing and fewer controls per row
- Hardware-style switch clusters

Pros:

- Strong analog identity
- Great for a "boutique" vibe

Cons:

- Consumes more screen space
- Harder to preserve dense advanced-page ergonomics

## Option C - Modern Utility Instrument

**Influence mix:** FabFilter + iZotope  
**Feel:** Highly legible, precise, minimal ornament

- Clean neutral panels and subtle animations
- Focus on metering/visual overlays
- Utility-forward controls

Pros:

- Excellent clarity and learning curve
- Very scalable for feature growth

Cons:

- Risks losing analog/character positioning if over-simplified

## Chosen direction for the current code phase

Use **Option A** now:

- "Analog hardware feel" through placeholder colors/shapes only
- Final-page control placements locked:
  - **Main:** Type (header), Size, Decay, PreDelay, Mix, Motion, Texture, Swell
  - **Character:** Dynamic, Harmonic, Warp, Damping, Width, Early
  - **Advanced:** Diffusion, Tone, Low/High Cut, Duck, Mod Rate/Depth, Bloom Amount, Distance, Transient Preserve, Output, Freeze

When visuals are ready, replace drawing/theme code only, without re-laying out controls.
