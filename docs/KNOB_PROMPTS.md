# BloomVerb Knob Family – Image Generation Prompts

Use these prompts to generate the 7 knob variants. Generate at **184 x 184** (2x). Output filenames: `knob_blue.png`, `knob_teal.png`, etc.

**Reference design:** Two-part knob—(1) central smooth colored disc with white indicator, (2) outer silver-gray metallic rim with grooved grips. Clean, functional, not bulky. No concentric rings on the face. **Taller profile**—raised/domed, strong shading for depth, not flat. **Use greenscreen background** (#00FF00 or bright saturated green)—AI doesn't generate transparency correctly; key out green in post (e.g. run `scripts/green_to_transparent.py` on the output). **If knobs aren't perfectly round:** run `scripts/center_knob_for_rotation.py` to place the rotation center (hub) at the image center—required for correct JUCE rotation. Use `center_knob_for_rotation.py [cx] [cy]` to specify the hub center manually if auto-centering is wrong.

---

## knob_gray (base / neutral)

```
Analog console rotary knob, front-facing orthographic, solid bright greenscreen background (#00FF00, chroma key green). Two-part design: (1) Central disc: smooth, slightly glossy surface, neutral dark gray color, no texture or concentric rings, raised or domed to feel taller—not flat. (2) Outer rim: silver-gray brushed metallic, grooved grips (radial teeth-like indentations) around the circumference, taller profile so the knob has depth and height. Thin darker ring separates disc from rim. Prominent solid white vertical indicator line at 12 o'clock on the central disc, extending nearly edge to edge—makes it read as a potentiometer, not an encoder. Strong shading and highlights to emphasize height and 3D form—knob should feel raised, tactile, not flat. Well-maintained hardware with light patina. No perspective distortion, no environment, no hands, no cables. Do not include: concentric rings on face, thick bezel, stepped base, tick marks, numbers, logos. Material emphasis: dark gray central disc, silver-gray metallic rim, subtle wear. Exact canvas: 184 x 184. Solid greenscreen background for chroma key.
```

---

## knob_blue

Same as knob_gray, but change material emphasis to:

```
Material emphasis: light blue or pale cyan central disc, silver-gray metallic rim, subtle wear.
```

---

## knob_teal

Same as knob_gray, but change material emphasis to:

```
Material emphasis: teal or cyan central disc, silver-gray metallic rim, subtle wear.
```

---

## knob_mint

Same as knob_gray, but change material emphasis to:

```
Material emphasis: pale mint green central disc, silver-gray metallic rim, subtle wear.
```

---

## knob_red

Same as knob_gray, but change material emphasis to:

```
Material emphasis: muted red central disc, silver-gray metallic rim, subtle wear.
```

---

## knob_bronze

Same as knob_gray, but change material emphasis to:

```
Material emphasis: warm bronze or cream central disc, silver-gray metallic rim, subtle wear.
```

---

## knob_dark_master

Same as knob_gray, but change material emphasis to:

```
Material emphasis: darker charcoal central disc, silver-gray metallic rim, subdued, subtle wear.
```

---

## Negative prompt (use for all)

```
Angled camera, 3/4 perspective, warped circles, blurry text, tick marks or numbers baked in, visible room/background, logos, brand-new showroom finish, mirror-polished metal, excessive gloss, toy-like plastic shine, encoder without indicator, thick bezel, stepped base, concentric housing rings, bulky framing, flat, pancake.
```
