# BloomVerb UI Asset Specification

This document captures the current BloomVerb editor geometry, the target visual direction for a full asset pass, and prompt guidance for generating image assets that slot into the existing JUCE UI without re-laying out controls.

It is intended to replace ad hoc chat notes with a reusable handoff for future UI work.

## 1. Scope and intent

The current editor already locks the core layout in code:

- fixed window size
- five left-side parameter strips
- one right-side master rail
- live display panel
- live dual meter section
- live output fader
- live labels, preset controls, and status text

The asset pass should improve the visual finish without changing control placement or requiring a structural rewrite of the editor.

## 2. Current editor geometry

Design to the current fixed editor size:

- editor canvas: `1480 x 860`
- left content block: `1094 x 716`
- strip count: `5`
- strip size: `212 x 716`
- strip gap: `8`
- right master rail: `332 x 716`
- display panel: `328 x 250`
- meter block: `250 x 458`
- fader lane: `78 x 458`

Top-level frame coordinates:

| Element | x | y | w | h |
|---|---:|---:|---:|---:|
| Editor canvas | 0 | 0 | 1480 | 860 |
| Space strip | 22 | 124 | 212 | 716 |
| Bloom strip | 242 | 124 | 212 | 716 |
| Character strip | 462 | 124 | 212 | 716 |
| Tone / Filters strip | 682 | 124 | 212 | 716 |
| Mod / Utility strip | 902 | 124 | 212 | 716 |
| Master rail | 1126 | 124 | 332 | 716 |
| Display panel | 1128 | 124 | 328 | 250 |
| Level meter block | 1126 | 382 | 250 | 458 |
| Output fader lane | 1378 | 382 | 78 | 458 |

Header control coordinates:

| Element | x | y | w | h |
|---|---:|---:|---:|---:|
| Title label | 22 | 20 | 280 | 38 |
| Status label | 1138 | 20 | 320 | 38 |
| Subtitle label | 22 | 58 | 280 | 22 |
| Type label | 302 | 58 | 34 | 22 |
| Type combo | 342 | 58 | 192 | 22 |
| Preset label | 22 | 80 | 42 | 30 |
| Preset prev | 64 | 80 | 30 | 30 |
| Preset combo | 98 | 80 | 244 | 30 |
| Preset next | 346 | 80 | 30 | 30 |

## 3. Visual direction

### 3.1 Core feel

Target a UI that reads as:

- classic British large-format console adjacent
- fully original, not a brand copy
- dark graphite and smoked-black chassis
- champagne or warm aluminum trim
- muted mint, blue, red, and cream accents
- orthographic front-facing hardware presentation

### 3.2 Finish target

Do not make the art look factory new or luxury-render polished.

Preferred finish:

- well-maintained analog studio hardware
- light patina from several years of regular use
- subtle edge wear on corners and screw heads
- faint hairline scratches in paint and anodized metal
- softened sheen from repeated handling and cleaning
- clean, cared-for, but not pristine

Avoid:

- mirror-polished showroom finish
- toy-like plastic gloss
- excessive reflections
- heavy rust, grime, damage, dents, or neglect

## 4. Generation rules

- Generate major assets at `2x` resolution and downsample when integrating.
- Keep text, numbers, arrows, meter fill, room graphics, and dynamic indicators out of the generated art unless a specific asset explicitly needs them.
- Prefer reusable modular assets over one flattened image whenever possible.
- Keep alpha edges clean with no cast shadows extending outside the sprite bounds.
- Favor realistic material separation over exaggerated contrast.

## 5. Prompt framework

Use a shared base prompt plus an asset-specific suffix.

### 5.1 Base prompt

High-end analog console plugin UI, inspired by classic British large-format recording console aesthetics, AMEK-9099-adjacent feel but fully original, front-facing orthographic product shot, dark graphite and smoked-black chassis, champagne aluminum trim, well-maintained analog studio hardware with light patina from several years of regular professional use, subtle edge wear on corners and screw heads, faint hairline scratches in painted metal, softened sheen from handling and cleaning, slightly dulled anodized surfaces, pale mint, ice-blue, red, and cream accent details, restrained smoked meter glass, realistic materials, clean industrial geometry, no perspective distortion, no environment, no hands, no cables, transparent background.

### 5.2 Negative prompt

Angled camera, 3/4 perspective, warped circles, blurry text, random labels, visible room/background, desk, cables, hands, logos or trademarks, duplicate controls, distorted proportions, baked shadows outside bounds, brand-new showroom finish, flawless luxury render, mirror-polished metal, excessive gloss, toy-like plastic shine, pristine untouched surfaces, obvious rust, dirt, corrosion, or damage.

### 5.3 Asset prompt template

Use this structure for every prompt:

`[base prompt]. Asset role: [what this image is used for]. Composition: [what hardware parts must be visible]. Must include: [recesses, screws, trim, glass, wells, plates, bezels]. Keep empty: [areas where JUCE will draw text, controls, meters, or screen content]. Do not include: [what must stay live or be omitted]. Material emphasis: [what finish distinguishes this asset]. Exact canvas: [W] x [H]. Front-on orthographic sprite. Transparent background.`

### 5.4 Example: full chassis prompt

High-end analog console plugin UI, inspired by classic British large-format recording console aesthetics, AMEK-9099-adjacent feel but fully original, front-facing orthographic product shot, dark graphite and smoked-black chassis, champagne aluminum trim, well-maintained analog studio hardware with light patina from several years of regular professional use, subtle edge wear on corners and screw heads, faint hairline scratches in painted metal, softened sheen from handling and cleaning, slightly dulled anodized surfaces, pale mint, ice-blue, red, and cream accent details, restrained smoked meter glass, realistic materials, clean industrial geometry, no perspective distortion, no environment, no hands, no cables, transparent background. Asset role: full plugin chassis background behind all live JUCE controls. Composition: one large outer chassis, recessed top header strip, five equal vertical strip wells on the left, one wider right-side master rail, subtle screw heads, engraved module plate recesses, display bay at top-right, dual meter bay below it, narrow vertical fader trench at far right. Must include: premium but used metal chassis, panel seams, recessed wells, trim, screws, gentle studio wear, analog console depth. Keep empty: all knob positions, label areas, preset and type areas, display content area, meter content areas, and fader thumb path. Do not include: knobs, text, numbers, arrows, logos, legends, meter fill, screen graphics, heavy rust, grime, dents, chipped corners, or background scene. Material emphasis: dark anodized chassis with champagne trim, subtle wear, believable studio age, cared-for but not pristine. Exact canvas: 2960 x 1720. Front-on orthographic sprite. Transparent background.

## 6. Asset inventory

### 6.1 Structural assets

| Asset name | 1x size | Generate at | Notes |
|---|---:|---:|---|
| `ui_bg_full` | 1480 x 860 | 2960 x 1720 | Full editor chassis background |
| `strip_space_bg` | 212 x 716 | 424 x 1432 | Warm two-column strip |
| `strip_bloom_bg` | 212 x 716 | 424 x 1432 | Cool single-column strip |
| `strip_character_bg` | 212 x 716 | 424 x 1432 | Neutral two-column strip |
| `strip_tone_bg` | 212 x 716 | 424 x 1432 | Cool EQ/filter strip |
| `strip_utility_bg` | 212 x 716 | 424 x 1432 | Warm utility strip |
| `master_rail_bg` | 332 x 716 | 664 x 1432 | Right-side rail background |
| `display_frame` | 328 x 250 | 656 x 500 | Metal frame for live display |
| `meter_frame` | 250 x 458 | 500 x 916 | Housing for meter section |
| `fader_lane_bg` | 78 x 458 | 156 x 916 | Narrow output fader trench |

### 6.2 Reusable overlays and plates

| Asset name | 1x size | Generate at | Notes |
|---|---:|---:|---|
| `module_nameplate_bg` | 176 x 24 | 352 x 48 | Empty nameplate background |
| `display_title_badge_bg` | 120 x 30 | 240 x 60 | Empty display title badge |
| `display_glass_overlay` | 292 x 146 | 584 x 292 | Smoked display glass only |
| `meter_badge_bg` | 94 x 38 | 188 x 76 | Empty signal badge plate |
| `meter_slot_bg` | 98 x 314 | 196 x 628 | Reusable single meter slot |
| `status_lamp_bezel` | 36 x 26 | 72 x 52 | Lamp housing only |
| `fader_track_overlay` | 20 x 390 | 40 x 780 | Dark recessed fader slot |
| `fader_thumb` | 46 x 16 | 92 x 32 | Output fader cap |
| `type_combo_bg` | 192 x 22 | 384 x 44 | Type selector shell |
| `preset_combo_bg` | 244 x 30 | 488 x 60 | Preset selector shell |
| `nav_button_off` | 30 x 30 | 60 x 60 | Header nav button |
| `nav_button_hover` | 30 x 30 | 60 x 60 | Hover/pressed nav button |
| `freeze_toggle_off` | 88 x 30 | 176 x 60 | Utility switch, off state |
| `freeze_toggle_on` | 88 x 30 | 176 x 60 | Utility switch, on state |

### 6.3 Reusable knob family

Generate one knob family and recolor per role rather than making 23 unique controls.

| Asset name | 1x size | Generate at | Suggested use |
|---|---:|---:|---|
| `knob_blue` | 92 x 92 | 184 x 184 | Motion, Swell, Width, Mod Depth |
| `knob_teal` | 92 x 92 | 184 x 184 | PreDelay, Low Cut |
| `knob_mint` | 92 x 92 | 184 x 184 | Texture, Dynamic, Damping, Transient |
| `knob_red` | 92 x 92 | 184 x 184 | Mix, Bloom Amount |
| `knob_bronze` | 92 x 92 | 184 x 184 | Size, Tone, Early |
| `knob_gray` | 92 x 92 | 184 x 184 | Decay, Diffusion, general neutral use |
| `knob_dark_master` | 92 x 92 | 184 x 184 | Optional darker utility variant |

## 7. Exact control placement map

These coordinates describe the current live control bounds. Keep these regions clear or aligned when generating strip assets.

### 7.1 Space strip

| Element | x | y | w | h |
|---|---:|---:|---:|---:|
| Nameplate | 40 | 142 | 176 | 24 |
| Size knob | 37 | 242 | 86 | 86 |
| Decay knob | 133 | 242 | 86 | 86 |
| PreDelay knob | 37 | 463 | 86 | 86 |
| Mix knob | 133 | 463 | 86 | 86 |
| Distance knob | 37 | 684 | 86 | 86 |

### 7.2 Bloom strip

| Element | x | y | w | h |
|---|---:|---:|---:|---:|
| Nameplate | 260 | 142 | 176 | 24 |
| Motion knob | 302 | 211 | 92 | 92 |
| Texture knob | 302 | 376 | 92 | 92 |
| Swell knob | 302 | 541 | 92 | 92 |
| Bloom knob | 302 | 706 | 92 | 92 |

### 7.3 Character strip

| Element | x | y | w | h |
|---|---:|---:|---:|---:|
| Nameplate | 480 | 142 | 176 | 24 |
| Dynamic knob | 477 | 242 | 86 | 86 |
| Harmonic knob | 573 | 242 | 86 | 86 |
| Warp knob | 477 | 463 | 86 | 86 |
| Damping knob | 573 | 463 | 86 | 86 |
| Width knob | 477 | 684 | 86 | 86 |
| Early knob | 573 | 684 | 86 | 86 |

### 7.4 Tone / Filters strip

| Element | x | y | w | h |
|---|---:|---:|---:|---:|
| Nameplate | 700 | 142 | 176 | 24 |
| Diffusion knob | 742 | 211 | 92 | 92 |
| Tone knob | 742 | 376 | 92 | 92 |
| Low Cut knob | 742 | 541 | 92 | 92 |
| High Cut knob | 742 | 706 | 92 | 92 |

### 7.5 Mod / Utility strip

| Element | x | y | w | h |
|---|---:|---:|---:|---:|
| Nameplate | 920 | 142 | 176 | 24 |
| Mod Rate knob | 917 | 242 | 86 | 86 |
| Mod Depth knob | 1013 | 242 | 86 | 86 |
| Duck knob | 917 | 463 | 86 | 86 |
| Transient knob | 1013 | 463 | 86 | 86 |
| Freeze toggle | 916 | 704 | 88 | 30 |

### 7.6 Right rail internals

| Element | x | y | w | h |
|---|---:|---:|---:|---:|
| Display title badge | 1146 | 142 | 120 | 30 |
| Display type text area | 1266 | 142 | 172 | 30 |
| Display screen rect | 1146 | 172 | 292 | 146 |
| Display footer | 1146 | 318 | 292 | 38 |
| Meter badge | 1146 | 402 | 94 | 38 |
| Meter lamp bezel | 1310 | 408 | 36 | 26 |
| Meter input slot | 1146 | 440 | 98 | 314 |
| Meter output slot | 1258 | 440 | 98 | 314 |
| Meter footer | 1146 | 754 | 210 | 66 |
| Output fader label | 1384 | 390 | 66 | 20 |
| Output fader bounds | 1384 | 414 | 66 | 418 |

## 8. What should remain code-drawn

Keep these live in JUCE unless there is a strong reason to bake them into assets:

- all control labels
- status text
- preset names
- type selector text
- rotary pointer lines
- rotary tick arcs
- combo-box arrows
- meter fill and overload behavior
- room field wireframe display content
- display footer readout

Reasoning:

- they are already dynamic in the current editor
- live drawing will stay sharper than AI-generated text or baked graphics
- leaving them live reduces future maintenance cost

## 9. Recommended asset-generation order

Generate in this order for the fastest useful first pass:

1. `ui_bg_full`
2. `display_frame`
3. `meter_frame`
4. `fader_lane_bg`
5. the five strip backgrounds
6. `module_nameplate_bg`
7. combo boxes, nav buttons, and freeze toggle states
8. the reusable knob family
9. optional glass, slot, and lamp overlay assets

## 10. Integration notes

- The asset pass should skin the current editor, not replace the layout.
- Asset prompts should always describe the job of the asset, not only the overall style.
- When an output feels too polished, reduce the "premium product render" language and reinforce studio patina, subtle wear, and softened sheen.
- The target is "used and maintained for years in a professional studio," not "factory new" and not "distressed."
