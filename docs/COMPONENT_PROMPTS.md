# BloomVerb Component Asset Prompts

Prompts for assets after the knob family. Use **greenscreen background** (#00FF00)—run `scripts/green_to_transparent.py` after generation.

| Asset | 1x size | Generate at |
|-------|---------|-------------|
| module_nameplate_bg | 176×24 | 352×48 |
| type_combo_bg | 192×22 | 384×44 |
| preset_combo_bg | 244×30 | 488×60 |
| nav_button_off | 30×30 | 60×60 |
| nav_button_hover | 30×30 | 60×60 |
| freeze_toggle_off | 88×30 | 176×60 |
| freeze_toggle_on | 88×30 | 176×60 |
| display_title_badge_bg | 120×30 | 240×60 |
| status_lamp_bezel | 36×26 | 72×52 |
| fader_thumb | 46×16 | 92×32 |

---

## module_nameplate_bg (176×24 → 352×48)

Empty nameplate background behind strip labels (Space, Bloom, Character, etc.).

```
High-end analog console plugin UI, British large-format recording console, AMEK-9099-adjacent, front-facing orthographic, dark graphite chassis, champagne aluminum trim, well-maintained hardware with light patina, subtle edge wear, no perspective distortion, no environment, no hands, no cables, solid greenscreen background (#00FF00). Asset role: empty nameplate background for strip module labels. Composition: single horizontal rectangular plate, recessed or flush, metallic. Must include: dark metallic or champagne-trim plate, subtle wear, screw holes or mounting detail at ends. Keep empty: entire center area where JUCE will draw "Space", "Bloom", "Character", etc. Do not include: text, labels, numbers. Material emphasis: dark anodized or champagne plate, subtle wear. Exact canvas: 352 x 48. Transparent or greenscreen background.
```

---

## type_combo_bg (192×22 → 384×44)

Type selector dropdown shell.

```
High-end analog console plugin UI, British large-format recording console, AMEK-9099-adjacent, front-facing orthographic, dark graphite chassis, champagne trim, well-maintained patina, no perspective distortion, solid greenscreen (#00FF00). Asset role: dropdown selector shell for "Type" control. Composition: single horizontal rectangular recess or well, narrow (short height). Must include: recessed dark well, metallic trim, subtle wear. Keep empty: center where JUCE draws type text and arrow. Do not include: text, arrow, dropdown graphic. Material emphasis: dark recessed well, champagne edge. Exact canvas: 384 x 44. Greenscreen background.
```

---

## preset_combo_bg (244×30 → 488×60)

Preset selector dropdown shell.

```
High-end analog console plugin UI, British large-format recording console, AMEK-9099-adjacent, front-facing orthographic, dark graphite chassis, champagne trim, well-maintained patina, no perspective distortion, solid greenscreen (#00FF00). Asset role: dropdown selector shell for preset name. Composition: single horizontal rectangular recess or well, slightly taller than type combo. Must include: recessed dark well, metallic trim, subtle wear. Keep empty: center where JUCE draws preset name and arrow. Do not include: text, arrow, dropdown graphic. Material emphasis: dark recessed well, champagne edge. Exact canvas: 488 x 60. Greenscreen background.
```

---

## nav_button_off (30×30 → 60×60)

Header navigation button, off state (prev/next preset).

```
High-end analog console plugin UI, British large-format recording console, AMEK-9099-adjacent, front-facing orthographic, dark graphite chassis, champagne trim, well-maintained patina, no perspective distortion, solid greenscreen (#00FF00). Asset role: small square navigation button, off/unpressed state. Composition: single square recessed button or raised pad. Must include: dark metallic button surface, subtle recess or raised edge, screw or mounting detail. Keep empty: center where JUCE may draw arrow. Do not include: arrow, text, icon. Material emphasis: dark charcoal button, subtle wear. Exact canvas: 60 x 60. Greenscreen background.
```

---

## nav_button_hover (30×30 → 60×60)

Header navigation button, hover/pressed state.

```
Same as nav_button_off, but: Material emphasis: slightly brighter or champagne-tinted surface, pressed-in or active state, subtle highlight as if engaged.
```

---

## freeze_toggle_off (88×30 → 176×60)

Utility switch, off state.

```
High-end analog console plugin UI, British large-format recording console, AMEK-9099-adjacent, front-facing orthographic, dark graphite chassis, champagne trim, well-maintained patina, no perspective distortion, solid greenscreen (#00FF00). Asset role: horizontal toggle switch, off state. Composition: single horizontal rectangular switch body, rocker or toggle style. Must include: dark metallic switch housing, recessed or neutral position, subtle wear. Keep empty: area where JUCE draws "Freeze" label. Do not include: text, on/off indicator. Material emphasis: dark switch body, neutral/off position. Exact canvas: 176 x 60. Greenscreen background.
```

---

## freeze_toggle_on (88×30 → 176×60)

Utility switch, on state.

```
Same as freeze_toggle_off, but: Material emphasis: switch in engaged/on position, slight champagne or accent highlight, active state.
```

---

## Optional overlays

### display_title_badge_bg (120×30 → 240×60)

Empty badge behind "BloomVerb" or display title.

```
High-end analog console plugin UI, British large-format console, front-facing orthographic, dark graphite, champagne trim, patina, greenscreen (#00FF00). Asset role: empty badge plate for display title. Composition: small horizontal rectangular badge. Must include: dark metallic badge, subtle trim. Keep empty: center for JUCE text. Do not include: text. Exact canvas: 240 x 60. Greenscreen background.
```

### status_lamp_bezel (36×26 → 72×52)

Lamp housing for status indicator.

```
High-end analog console plugin UI, British large-format console, front-facing orthographic, dark graphite, champagne trim, patina, greenscreen (#00FF00). Asset role: small circular or rectangular lamp bezel/housing. Composition: single lamp housing, recessed for LED. Must include: metallic bezel, recessed well for light. Keep empty: center where JUCE draws status color. Do not include: lit LED, text. Exact canvas: 72 x 52. Greenscreen background.
```

### fader_thumb (46×16 → 92×32)

Output fader cap/slider.

```
High-end analog console plugin UI, British large-format console, front-facing orthographic, dark graphite, champagne trim, patina, greenscreen (#00FF00). Asset role: fader cap/slider thumb for output level. Composition: single horizontal rectangular cap, slightly domed. Must include: metallic cap, grip texture or ridges. Do not include: track, scale. Material emphasis: dark metallic or champagne cap. Exact canvas: 92 x 32. Greenscreen background.
```

---

## Negative prompt (use for all)

```
Angled camera, 3/4 perspective, visible room/background, desk, cables, hands, logos, text, numbers, brand-new showroom finish, mirror-polished metal, excessive gloss, pristine surfaces, rust, dirt, damage.
```
