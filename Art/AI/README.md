# AI art to make (ChatGPT)

Drop finished images into the matching folder here; Claude imports them into Unreal.
Save as **PNG**. Name files like the list says (lowercase, underscores). If you make a few versions, add `_v2`, `_v3`.

ChatGPT only makes three sizes, so ask for these:
- **Landscape 3:2** (1536×1024): backgrounds, loading screens, model sheets
- **Square 1:1** (1024×1024): icons, textures

Do everything in **one ChatGPT conversation** so the style stays the same, and paste the style block at the start of each request.

---

## 1. Menu (folder `Menu/`) — needed first

| File | Size | What |
|---|---|---|
| `menu_background.png` | 3:2 | Main menu painting. A Yorkshire village at golden hour seen from a low hill: timber-framed and grey-stone cottages with thatch, a small stone church, fields, a road, a stone bridge, hazy hills. **Left third calmer and darker** (menu buttons go there). No text, no characters. Make 2–3 versions |
| `panel_texture.png` | 1:1 | **Seamless tileable** dark charcoal texture (worn leather or dark parchment), very subtle, low contrast. Used behind all menu panels |
| `corner_ornament.png` | 1:1 | One worn-gold filigree **corner** ornament (top-left corner shape), **transparent background**, thin lines. Optional |
| `loading_01.png` … `loading_05.png` | 3:2 | Optional, later: loading screens (village market, St Albans street fight 1455, Middleham Castle, army camp at dawn, alehouse interior). Countryball characters welcome here |

**Style block for menu art:**
> Painterly hand-painted game art, stylized but realistic proportions (not cartoon, not photoreal), 15th-century England (1455), warm natural colours, soft light, no text, no logos, no watermark.

---

## 2. Item icons (folder `Icons/`) — ~45, make over time

Every icon: **1:1, transparent background**, one item, centered, filling ~80% of the frame, **three-quarter view**, light from the top-left, no text, no border, no ground shadow.
The greyed "empty slot" silhouettes are made in code from these, so no separate silhouettes are needed.

**Style block for icons:**
> Game inventory icon, painterly hand-painted style, realistic proportions and historically accurate for England in 1455, soft top-left light, three-quarter view, centered, transparent background, no text, no border.

| Slot | Files |
|---|---|
| Helmet | `helmet_kettle_hat`, `helmet_sallet`, `helmet_sallet_bevor` (sallet with chin/throat guard), `helmet_armet`, `hat_straw`, `hat_felt` |
| Coif | `coif_linen`, `coif_arming_cap` (padded), `hood_wool` |
| Weapon (main) | `weapon_bill` (English bill: billhook-shaped blade with a concave edge, top spike, back spike; NOT a halberd), `weapon_poleaxe`, `weapon_longsword`, `weapon_arming_sword`, `weapon_falchion`, `weapon_war_hammer`, `weapon_mace`, `weapon_spear`, `weapon_longbow`, `weapon_crossbow`, `weapon_wood_axe`, `weapon_cudgel`, `weapon_quarterstaff` |
| Weapon (off) | `offhand_buckler`, `offhand_lantern` (horn lantern) |
| Back | `back_pavise` (large crossbowman's shield), `back_heater_shield` |
| Cloak | `cloak_wool`, `cloak_hooded`, `cloak_fur_lined` (noble) |
| Belt | `belt_rondel_dagger`, `belt_ballock_dagger`, `belt_knife`, `belt_arrow_bag` (canvas arrow bag), `belt_purse`, `belt_torch` |
| Gambeson | `gambeson_padded_jack`, `gambeson_arming_doublet` |
| Tunic | `tunic_plain_wool` (undyed commoner), `tunic_livery_neville` (red, white saltire), `tunic_tabard` |
| Mail | `mail_shirt` (haubergeon), `mail_skirt` |
| Plate | `plate_brigandine`, `plate_jack_of_plates`, `plate_breastplate`, `plate_full_harness` |
| Ring | `ring_signet`, `ring_gold` |
| Necklace | `necklace_ss_collar` (Lancastrian SS collar), `necklace_suns_roses` (Yorkist collar of suns and roses), `necklace_pendant_cross` |
| Gloves | `gloves_leather`, `gloves_gauntlets` (plate) |
| Boots | `boots_ankle` (flat leather sole, no heel, side-laced), `boots_riding` (tall), `boots_sabatons` (plate) |

**Worn gear is drawn for a countryball** (helmets, hats, coifs, hoods, cloaks, gambesons, tunics, mail, plate, collars, gloves, boots). Paste this before the icon style block:
> Made to be worn by a countryball character: a perfect sphere about 1.35 m wide with two big white eyes on its upper front. Show the item on its own, empty, no ball and no person, but shaped for that sphere: helmets are wide, round domes that sit on top of the sphere like a cap and leave the eyes free; body clothing and armour are a rounded band that wraps the lower half of the sphere (no sleeves, no neck hole, no shoulders); cloaks drape over the back of the sphere; gloves fit big cartoon hands with four fingers and a thumb; boots are short and chunky.

Carried items (weapons, shields, bucklers, pavises, lanterns, torches, daggers, knives, purses, arrow bags, rings) stay drawn as the real object.

**Avoid** (wrong for 1455 England): halberds, horned/winged helmets, mail coifs as everyday wear, heels or tread on shoes, anything fantasy-glowing.

---

## 3. Model sheets (folder `ModelSheets/`) — optional, later

For items we'll model in Blender: **3:2**, plain light-grey background, the item shown **front, side and top** in flat orthographic view (no perspective), with a real-size ruler in metres. Start with: `sheet_bill`, `sheet_kettle_hat`, `sheet_sallet`, `sheet_rondel_dagger`, `sheet_boots_ankle`.

---

## 4. Map icons

- `MapIcons/` — **simple** icons for the whole-England planning map (`Tools/world/draw_plan_map.py` uses any `map_*.png` found there). Still to make: less detailed versions of the ones below (prompts in `MapIcon_Prompts.txt`).
- `RegionalMapIcons/` — the user's **detailed** hand-painted set (2026-09-29), kept for the **regional maps** (England split into a few regions later), where icons are drawn bigger. Same `map_*` names as the planning map, so the same code can use them: `map_city` (walled city), `map_town` (market town), `map_village` (cottage), `map_castle_major`, `map_castle_minor` (tower), `map_cathedral`, `map_abbey`, `map_battle` (crossed swords), `map_landmark` (Stonehenge), `map_nature` (mountain), plus `map_title_scroll` (title banner) and `map_compass_rose`.
