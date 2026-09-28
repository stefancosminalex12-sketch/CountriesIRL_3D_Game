# CountriesIRL 3D Game: Master File

> Working design document. Living file: updated as ideas are discussed and the game is built.
> Status legend: **[Decided]** agreed · **[Proposed]** suggested, not confirmed · **[Open]** needs a decision

---

## 1. Vision

A stylized 3D open-world medieval game set in real history, where every person is a **countryball**.
Charming to look at, serious and deep underneath: a real, breathing world without micromanagement.

**One-liner:** Live through the Wars of the Roses as your own countryball noble, in an accurate, living, stylized England.

**Why it's different** (vs Age of History, Europa Universalis, Hearts of Iron, Crusader Kings, Total War, Bannerlord):
- Those games either lack "soul" (AoH/EU/HoI: abstract maps, no characters, no atmosphere) or lack depth and history (Bannerlord: fictional world, shallow politics, separate campaign map).
- This game: one continuous living world, real history and people, expressive characters, atmosphere (music, sound, weather, seasons).

**Brand fit:** CountriesIRL (a creator network of country accounts, ~300k followers, focused on history, geography and geopolitics).

---

## 2. Pillars

1. **Real history, accurately portrayed** [Decided]: real places, people, heraldry and events at the start; the canon main story follows history.
2. **A breathing world** [Decided]: day/night, seasons, weather, random events, people living their lives.
3. **Countryball charm, serious depth** [Decided]: cute style, real drama (war, betrayal, loss). **Not serious all the time:** fun, humor and spectacle moments too (reference: *The Angry Birds Movie* energy) [Decided]
4. **Freedom** [Decided]: play how you want (Skyrim/KCD-style); your role emerges from what you do.
5. **Realism without micromanagement** [Decided].

---

## 3. Setting

- **Base game:** England, **1455**, the **Wars of the Roses** (House of York vs House of Lancaster) [Decided]
- **Suggested start:** a few weeks before the First Battle of St Albans (22 May 1455) [Proposed]
- **Expansion model:** ETS2/ATS-style DLCs that add **genuinely new land** to the same continuous map, e.g. Wales/Scotland → Normandy/France → Denmark… [Decided]
- **Tone:** charming but serious [Decided]

---

## 4. World

- **One continuous open world.** No separate campaign map / scene split like Bannerlord [Decided]
- **Size:** large but simple and nice (~500 km²+ acceptable) [Decided]
- **Compression, Assassin's Creed-style:** key cities and strongholds plus the relevant fields, villages and landmarks around them; empty countryside is cut heavily [Decided]
- **Real elevation** data, real rivers, roads, and relative positions of places [Decided]
- **Villages are tiny** (a few houses plus the relevant businesses: church, mill, smithy, alehouse…); medieval scale is small [Decided]
- **Build approach:** hand-built cities/castles/villages, procedurally generated countryside (Unreal PCG), World Partition streaming [Proposed]
- **Far-away armies/lords** are simulated abstractly as data, and become real in-world when near the player [Proposed]
- **Visual style:** stylized, cozy diorama look. Reference: StylArts "Stylized Fantasy Provençal" pack (saved in Fab library; Provençal architecture suits a future France DLC, while England needs timber-frame/thatch/grey-stone in the same style) [Decided]
  - **Realistic proportions, stylized surface (user, 2026-09-28):** everything keeps **true-to-life proportions and scale** (buildings, props, weapons, trees, animals); the stylization is in simpler shapes, softer painterly textures and colour, never in exaggerated or distorted proportions (no chunky oversized doors, bent fantasy roofs, giant weapons). "Realistic, even if a bit cartoonish looking." Only the countryball characters are deliberately non-realistic [Decided]
  - **Target look (user, 2026-09-28): painterly hand-painted stylized.** Key references, all in the Fab library: LapaModels **"Free Stylized Bridge"** hero shot (lush grass and wildflowers, chunky painterly rocks and stonework, soft sunlight, deep valley: *this is what the world should look like*), UpDraft Art "Stylized Artisan Forge Kit", LapaModels **"FREE Windmill & Village Buildings Pack"** (user: "looks amazing"), Agustin Honnun "Stylized House - 1 Material", Black Cloud9 "Scalable Stone House", Iinmost "Stylized Door Dungeon", ElectraStylized "Metal Sword", Daria Borovleva food, Forge of Fantasy "Stylized Environment Pack", StyleHex "Free Stylized Foliage Pack" [Decided]
  - **Realistic, not photorealistic (user, 2026-09-28):** take the **shapes** from the forge kit (chunky, clean, simple forms), the **painted surfaces** from the bridge, and **realistic colour**: colourful where real life is (heraldry, banners, flowers, dyed noble clothes), muted where it isn't (stone, weathered wood, mud, peasant wool). The references are about graphics style, not specific assets to use [Decided]
  - **Reworking assets to match the style (2026-09-28):** assets may be reworked (both CC-BY and Fab Standard License allow changes) [Decided]. Default = **recolour/restyle in Unreal** through a shared master material (tint, saturation, brightness sliders), no image editing. **AI image tools** for **tiling textures** (stone, plaster, thatch, planks), where they work well; for **unwrapped model textures** only when needed, since AI tends to shift pixels off the UV layout. **Only put a Fab asset into an AI tool if its listing says "Allows usage with AI: Yes"** [Decided]
  - **Animals: painterly, true proportions**, at the detail of Charlie catling's **"Game ready crow"** or slightly more. **Replaces the earlier faceted low-poly decision** (section 8) [Decided]
  - **Period check on assets:** no New World foods in 1455 England (no potatoes, peppers, pumpkins, maize, tomatoes); no modern boat rigs. Such parts of packs are dropped or swapped [Decided]
  - **Asset pool:** the user saved ~350 free Fab items (CC-BY or Standard Personal licence) to pick from; modern and most fantasy items are ignored, some fantasy kept only for reusable parts [Noted]

---

## 5. Time & Calendar

- **Accurate real calendar that advances**, RDR2-style [Decided]
- **Day/night ≈ 48 real minutes per day** while playing (GTA V/RDR2-like) [Decided]
- Time moves faster through sleep, waiting, long travel and command actions [Proposed]
- **Seasons and weather** are dynamic and affect the world [Decided]
- **Story chapters can jump the calendar** (no waiting literal years) [Decided]
- A playthrough covers a few in-game years; no fixed end date [Decided]
- **Implementation (v0.1)** [Built]:
  - **Start: Thursday 1 May 1455, 07:00** (a few weeks before St Albans, 22 May); set in Project Settings > CountriesIRL World, so DLC regions can change it
  - **Julian calendar**, as England used until 1752: dates show as contemporaries wrote them; sun, moon and weekdays use the real astronomy (Julian + 9 days in the 1400s). Check: 22 May 1455 comes out as a Thursday, matching the historical record. The English year officially began on 25 March (Lady Day); the game shows the modern year number [Proposed]
  - Clock = **local solar time** (medieval hours followed the sun); **real sun path for York (54°N)**: long summer days, short winter days; **moon with real phases** (1 May 1455 was near full moon)
  - Nights are dark blue and moonlit but playable; new-moon nights keep a little starlight. Stars in the sky: later polish
  - World orientation convention: **+X = north, +Y = east**
- **Seasons (v0.1)** [Built]:
  - Region **climate profile** (data asset `DA_Climate_Yorkshire`, DLC regions bring their own): per month the mean temperature, day/night range, how leafy the trees are, leaf colour and grass colour. Yorkshire 1450s = modern York averages ~0.5 °C colder (Little Ice Age). Nature follows the astronomical date, not the Julian calendar on the wall
  - **Temperature** changes through the day (coldest ~3:00, warmest ~15:00) and from day to day (warmer and colder spells)
  - **Trees**: bare Dec-Mar, buds breaking late April, fresh green May, full dark canopy June-Sept, gold/orange in October, leaves falling through November
  - **Grass**: dull in winter, lush green May-June, hay-coloured in the dry weeks of late summer (haymaking, harvest)
  - **Frost** on freezing nights (ground frost forms with the air a few degrees above 0 °C); it lingers in the morning and only melts once the sun is on it (slowly under the low winter sun). **Morning mist** on cool mornings, mostly autumn and winter (thicker fog)
  - Everything goes into one material parameter set (`MPC_Season`: LeafAmount, LeafTint, GrassTint, Frost, Mist, Temperature), so any tree, grass or ground material, including free Fab assets, can follow the seasons by reading it (`M_Seasonal_Ground`, `M_Seasonal_Leaves` are the first two)
  - **Medieval calendar** on the dev clock: feast days (fixed ones like Lady Day, May Day, Midsummer, Lammas, Michaelmas, St Crispin's, Martinmas, Christmas; moveable ones from **Easter computed the medieval Julian way**: Shrove Tuesday, Ash Wednesday, Good Friday, Easter, Ascension, Whitsun, Corpus Christi with the York mystery plays; Plough Monday), church seasons (Advent, Christmastide, Lent, Eastertide) and the farm work of the month ("labours of the months"). Check: Easter 1455 = Sunday 6 April. Later these drive NPC life (no work on feast days, fasting in Lent, markets)
  - Test garden in `L_DevSandbox` (south side): simple placeholder trees and a grass patch using the seasonal materials; real trees/grass will come from Fab
- **Weather (v0.1)** [Built] (user: "do actual England weather", keep regions and the 1450s in mind):
  - **Realistic British pattern**: weather fronts pass every day or two (rain comes in many shortish spells, rarely all day), **afternoon showers** in spring and summer with sunny spells between, grey winters. Prevailing **south-westerly wind** that swings around and strengthens as fronts pass
  - **Calibrated to real data** for the Vale of York (Met Office averages, Linton-on-Ouse, 15 km from York): days with rain per month (8-12), cloudiness from sunshine hours (only ~31 h of sun in December, ~186 h in July), share of rain falling as showers, wind. A 15-year simulation matches: rain ~6-10% of all hours, monthly cloudiness within ~0.02 of the data, summer rain days on target (winter slightly low, tunable)
  - **Temperature reacts to the sky**: sunny days warm up more, cloudy nights stay milder (clear nights bring frost), rain cools; **1.5 C per 230 m colder on higher ground** (-0.65 C/100 m), so later it can snow on the Pennines while it rains in the vale
  - **Snow** when precipitation falls below ~1.5 C; it **settles** (the ground turns white) and **melts** with warmth, rain and sun. **Wet ground** after rain dries over half a day to a day (faster when warm, sunny, windy). **Fog/mist** only on still, clear, cool mornings (radiation fog). **Thunderstorms** from heavy summer showers
  - **The year matters**: the Little Ice Age (already ~0.5 C colder) plus **cold summers after 1453**: tree-ring studies show about 15 years of cold Northern Hemisphere summers starting 1453 (-2.5 C network mean in 1453 to -0.5 C by 1468; English oak panels have abnormally narrow rings 1453-1455), linked to a huge volcanic eruption whose identity/date is debated (Kuwae 1452/53 vs 1458). The game uses gentle estimates for England: -1.5 C in 1453 easing to -0.2 C by 1468, felt fully in summer and half in winter (data: `YearAnomalies` in the climate profile)
  - Like corpses and seasons, weather is **worked out from the date and time**, so saves only need the clock; after time jumps the last week is replayed to get wet ground and lying snow right
  - Looks: clouds build up and clear gradually; overcast skies block most direct sun (soft shadows, the view adapts like eyes do so grey days read grey, not dark); rain and heavy cloud darken the scene and thicken the haze; **rain streaks slanted by the wind and fluttering snowflakes** around the viewer; grass darkens and shines when wet and turns white under snow. Everything goes into `MPC_Season` (CloudCover, Rain, Snowfall, Wetness, SnowCover, Wind, Daylight) so free Fab materials can react too
  - Dev: `DevWeather Clear|Fair|Cloudy|Overcast|Showers|Rain|HeavyRain|Storm|Snow|Fog|Auto`; the dev clock shows e.g. "Overcast, light rain, wind SW 5 m/s"
  - Later: rain/snow stop under roofs (not yet), puddles, sounds (rain, wind, thunder), lightning flashes, NPCs sheltering, Niagara effects instead of the simple drops
- **Regional climates (for the map)** [Proposed]: each region gets its own climate profile (same data format); the player's region decides the weather:
  - **Vale of York / eastern lowlands**: in the rain shadow of the Pennines, fairly dry (~600-770 mm/yr), sheltered
  - **Pennines and the Dales**: much wetter (1,000-2,000 mm), windier, colder (height), far more snow and longer-lying snow, low cloud and hill fog
  - **North York Moors**: wet and exposed, snowy winters
  - **East coast (Scarborough, Whitby)**: cold easterly winds off the North Sea, sea fog (**"haar"**) in spring and early summer, milder winters by the sea
  - **West (Lancashire, Cheshire)**: the wettest lowlands, mild, frequent drizzle
  - **South-east / London**: warmest and driest, sunniest summers
  - Mountains and moors later also get their altitude from the terrain (-0.65 C/100 m already built in)

---

## 6. Story & History

- **Main story = main quests** following the canon historical beats (KCD/Skyrim model) [Decided]
- Quests can require prerequisites: a smaller quest, a level/reputation, or a date [Decided]
- Some player actions can **accelerate** the main story [Decided]
- **No alternate-history fantasy**: main beats stay canon; smaller things play out differently, and you can still win or lose depending on your choices [Decided]
- **Random events**, RDR2-style: skirmishes, bandits, messengers, feuds, fires, hunts… [Decided]
- **Open-world activities:** take villages, sieges, loot money and weapons, cut enemy supplies, "literally anything" [Decided]

---

## 7. Player Character

- **A fictional character you create** (simple creator) in the real history; a "kingmaker"-style rise [Decided]
- **Choose a real house to serve** [Decided]
- **No archetype/class selection**: your role emerges from how you play; skills grow by doing [Decided]
- **Character creator** (before the game starts): a few simple looks options; **type any name or randomize** a real English name of the period [Decided]
- **Choose your house**: sets your spawn area, starting resources, troops, allies and enemies [Decided]
- **Origins = social class you start in** [Decided]:
  - **Commoner:** peasant/yeoman; enlist (e.g. as a longbowman), almost nothing to start
  - **Gentry:** esquire/knight's son; horse, some gear, small household
  - **Noble:** a lesser member of a real house (younger son, cousin) or a fictional relative; lands, money, retainers
  - Each origin has its own prologue leading into the main story. The first slice ships with **one** origin [Proposed]
- **Presets:** optionally play a real but historically obscure (less relevant) family member [Decided]
- **Famous figures stay NPCs** (York, Warwick, Henry VI, Margaret of Anjou, Edward IV…) so the canon holds; you meet, serve and influence them [Decided]
- **Spawn choice:** pick your house **and/or the town or village where you spawn**, so every playthrough can start where you like [Decided]
- **Noble start:** you already have men you can **summon** (they physically travel to you, which takes time), **estate(s) to administer**, **income**, and you can **send men** between places or keep them with you [Decided]
- **Other classes** have the same panels (estates, men, income) but they are **empty/locked ("N/A")** until you earn them through progression [Decided]
- **Estate management stays light and realistic:** your places are run by other people (steward/reeve); you only change a few things: **production, export/import, moving or calling units** [Decided]
- **Orders take time to arrive** (sent by messenger), **unless you are physically at that estate**, in which case they apply immediately [Decided]
- **Rank ladder as progression:** peasant → yeoman → esquire → knight → baron → earl → duke…, climbed through deeds, service, marriage and politics [Decided]
- **Rising from the bottom:** enlist, fight skirmishes, loot and resell, do work/jobs, buy better gear, meet influential people [Decided]
- **Main quest, side quests and treasures** [Decided]
- **Main quest hook:** fairly early, a **local lord recruits you**, which introduces you to the main quest (KCD-style) [Decided]
- **Main quest unlocks at a certain level:** you receive a **letter** telling you to go somewhere, and the main story starts. Since you can be anyone, anywhere, the letter adapts: a **knight is summoned to fight**; a **noble is called to the battlefield** [Decided]
- **Only historically accurate participants:** research which real nobles actually fought (even once) and include only those as fighting nobles [Decided]
- **Death & saving** [Decided]: you *can* die (e.g. if you just stand there taking damage), which means reloading. **Frequent autosaves**, and an **autosave at the start of every random battle**
- **Defeat** [Decided]: you can **retreat/run away** before the fight is lost. If you lose and can't escape, you are **captured along with a few others** → cutscene → taken to the enemy's town/castle, followed by a consequence (see historical options in the capture section) [consequence details: Open]
- **Advisor** [Decided]: hire an advisor who gives recommendations before and during battles
- **Capture: historical reality in the Wars of the Roses** [Decided]:
  - **Commoners** were usually spared ("spare the commons"), often stripped of gear/money and sent home, or absorbed into the winning side
  - **Knights/gentry** could be ransomed, pardoned, or switch allegiance
  - **Nobles** were in real danger: many captured lords were **executed** as traitors (e.g. Salisbury after Wakefield 1460, Owen Tudor after Mortimer's Cross 1461, Somerset after Tewkesbury 1471). Ransom was more typical of the Hundred Years' War than of this civil war
  - **Attainder:** Parliament could confiscate a defeated lord's lands and titles (Act of Attainder), and they could later be **restored** (reversal of attainder)
  - Forced labour and being "sold" were **not** English practices then
  - Game version [Decided]: the player never gets executed; instead: imprisonment and escape quest, ransom paid by your lord/family, swearing to the other side, **losing lands via attainder** (restorable later), losing gear/money
  - **Consequences depend on your *current* social rank, not your origin** [Decided]: a peasant who rose to knight or noble is treated as a knight or noble when captured (and the same goes for how NPCs, letters and quests treat you)
- **No inheritor/succession system** (probably) [Decided]

---

## 8. Characters & Art (Countryballs)

- **All humans are countryballs**: coat of arms/livery on the ball, **white eyes, no mouth** [Decided]
  - **Size and shape (user, 2026-09-27):** the balls looked small next to the horses, and perfect spheres read like toy balls. Now the ball is **slightly taller than wide (egg, 1.2x)**, 104 cm wide (still fits doors) and **~1.57 m tall with the feet** (was 1.25 m), closer to human scale so Fab buildings, furniture and saddles fit [Superseded 2026-09-28, see below]. The **eyes keep their original round shape** on the egg (user: the stretch had made them taller); the eye shader undoes the shell's stretch
  - **Back to a perfect sphere (community poll, 2026-09-28):** the user polled three shapes (A sphere, B near-sphere, C tall ball); **~87% chose the perfect sphere**, most of the rest the near-sphere. The body is a **perfect sphere** again, with the floating boots below it [Decided]
  - **Height = historical English averages (user, 2026-09-28)** [Decided]. Height is measured from the top of the ball to the soles of the boots. Averages follow skeletal studies of late-medieval England (e.g. the Towton grave, 1461): **men ~1.71 m, women ~1.59 m**. Each character's height is rolled on a bell curve; the ranges are the user's first proposal (1.80 m average, 1.65–1.95, extremes 1.40–1.50 / 2.00–2.10) scaled down proportionally to these averages:

    | | Men | Women |
    |---|---|---|
    | Average | **1.71 m** | **1.59 m** |
    | Most people (~95%) | 1.62–1.80 m | 1.50–1.68 m |
    | Normal range (~99%) | 1.57–1.85 m | 1.46–1.72 m |
    | Extreme short (≤0.5%) | 1.33–1.43 m | 1.24–1.33 m |
    | Extreme tall (≤0.5%) | 1.90–2.00 m | 1.77–1.86 m |

    (Bell curve: standard deviation ~4.5 cm for both, clamped to the normal range; a separate ≤1% roll picks an extreme instead.)
  - **Body proportions: smaller ball, bigger gap to the floating boots (user, 2026-09-28)** [Decided]. Every character is the same shape scaled uniformly (ball, hands, boots and gap all scale with height). Proportions of total height: **ball 58%, gap 29%, boots 13%**. The ball's widest point is its diameter.

    | | Height | Ball width | Gap under ball | Boots |
    |---|---|---|---|---|
    | Average man | 1.71 m | **1.00 m** | 0.50 m | 0.22 m |
    | Average woman | 1.59 m | 0.92 m | 0.46 m | 0.21 m |
    | Tallest normal man | 1.85 m | 1.07 m | 0.54 m | 0.24 m |
    | Shortest normal woman | 1.46 m | 0.85 m | 0.42 m | 0.19 m |
    | Extreme tall man | 2.00 m | 1.16 m | 0.58 m | 0.26 m |
    | Extreme short woman | 1.24 m | 0.72 m | 0.36 m | 0.16 m |

    Hands float at ball mid-height, just outside the ball; palm-to-fingertip ~0.19 m for an average man (scales too).
  - **Doors** [Decided, user 2026-09-28]: real medieval cottage doors were only ~0.8–0.9 m wide × ~1.8 m tall, narrower than even an average ball, so doors are the one thing not built to strict real size: **ordinary doors ~1.15 m wide × ~1.95 m tall** (fits everyone in the normal range, max ball 1.07 m); halls, churches and castle gates are big enough for anyone anyway. **Extreme-tall characters (≤1%) don't fit ordinary cottage doors**, as a fun real-life quirk (tall people ducked; a ball can't)
  - Replaces the 2026-09-27 egg (1.04 m wide, ~1.57 m tall) and the first 1.80 m proposal. Built in the game code on 2026-09-28 for the average man (see roadmap step A) [Built]
  - **Confirmed by community vote (2026-09-27):** the user polled their community on the body question: after ~20 minutes, **48 votes for pure countryballs**, 7 for a simplified body, 7 for a complex body (~77% countryballs). Pure countryballs stay the game's look; the humanoid body prototype (B key, Quaternius outfit, retargeter) was **removed** at the user's request; it stays in git history (commits 1f445f9–6533936) if ever needed [Decided]
- **Expressive eyes** (normal, happy, sad, angry, scared, tired, suspicious, dead ×_×…); reference: u/tengam15 "Big Chart o' Expressions" (inspiration only, draw our own set) [Decided]
- **Standing on each other** (user, 2026-09-27): you can land on top of another countryball and stay there (the engine used to bounce characters off each other) [Built]
- **Rayman-style floating hands AND feet** (hands hold weapons/tools; feet wear boots/sabatons and make walking/riding read clearly) [Decided]
  - **Hands have fingers** [Decided, user]: a palm with **four cylinder-shaped fingers and a thumb** (rounded tips), needed for holding swords/tools and punching. Poses: relaxed, **fist** (punching, guard), later **grip** for weapons [Built as simple shapes; art pass can replace with Blender meshes]
  - **Feet are boot-shaped** [Decided, user]: sole, foot and ankle shaft, so they read as shoes/boots; gear will swap footwear later (turnshoes, ankle boots, riding boots, sabatons) [Built as simple shapes]
- **Body variation:** randomized size (99% in a normal range, ~1% very tall or very small); proportions: longer, fatter, thinner, shorter [Decided]
- **Hair and beards** with different hair colors [Decided]
- **Aging is visible:** graying hair, growing beards, scars from battles, weathered colors [Proposed]
- **Animals:** very simple and stylized (horses and sheep most important). Possible source: Quaternius (free, commercial OK) [Decided]
  - **Art style for all animals (user, 2026-09-28): painterly hand-painted with true proportions**, at the detail level of Charlie catling's **"Game ready crow"** on Fab, or slightly more [Decided]. *Superseded:* faceted low-poly like LucySail's "Stylized lowpoly HORSE" (2026-09-27)
    - **Now placeholders (2026-09-28):** the Quaternius animals below stay in the game (riding already works with them) until painterly replacements are found or made; they must be rigged and animated (walk, gallop, idle, death at minimum) [Planned]
    - Previous animated source: **Quaternius Ultimate Animated Animal Pack** (CC0, free; 12 animals: horse, donkey, cow, bull, deer, stag, alpaca, fox, wolf, husky, shiba; 12+ animations each incl. walk, gallop, jump, attack, death) has the same faceted look. **Chosen as the animal source (user, 2026-09-27)** [Decided]. **Imported (2026-09-27)** into `Content/CountriesIRL/Animals`, scaled to real life (medieval breeds on the small side): horse (brown + white) and donkey 2.0/1.7 m long (horse back ~1.3 m, a small medieval horse of ~13 hands; shrunk after the user found them too big next to the balls), cattle 1.3 m tall, red deer hind and stag, wolf, fox; husky and shiba stand in as village dogs until we have period dogs; alpaca left out (not in medieval England). Each has its animations (idle, walk, gallop, jump, eating, attack, hit reactions, death); all stand in a lineup in `L_DevSandbox`. Riding comes in roadmap step 5. Sheep, pigs and chickens still needed (another free pack or our own in Blender, same style) [Proposed]
- **Real historical figures** have their real heraldry and source-based personalities [Proposed]

---

## 9. Gameplay Systems (outline)

- Travel the world as your character; horses [Decided]
- **Stamina** [Decided]: you can't run or jump indefinitely. Running drains stamina, each jump costs a chunk; it refills after a short rest. Running it to zero leaves you **exhausted** (can't run/jump) until it recovers partway. A small bar shows at the bottom of the screen only while stamina is not full (clean screen). Same component will serve NPCs, horses and later combat actions
- **Health (HP)** [Decided]: **100 HP base**. Damage lowers it; at 0 the ball dies (x_x eyes, stops). For the player this leads to the reload flow in §7 (Death & saving) once saving exists. Whether HP regenerates on its own or only through rest/food/treatment: [Open]
- **Unarmed punch damage** [Decided, user asked; Claude's recommendation]: random **5–8** base × hit zone: **head/face ×1.5** (8–12), **chest ×1** (5–8), **lower ×0.7** (4–6). ~9 head punches or ~15 body punches to beat 100 HP; weapons hit much harder later. All values tunable [Built: left click jabs, alternating hands; hits what you aim at within arm's reach; the face and everything above counts as head; costs 8 stamina, 0.45 s cooldown, small knockback; walls block punches; a dev message shows zone and damage]
- **Guard** [Decided, user]: **hold right mouse button** to raise the guard (fists up in front of the face, boxer-style); you can punch straight out of it. While guarding you move slower (65%) and can't run, the guard **slowly drains stamina (1.5/s, user asked ~1/s)** and stamina doesn't refill; hits from the front are **blocked (35% of the damage gets through)** at 6 stamina per blocked hit; with no stamina left the guard drops. In third-person the ball faces where you look while guarding (fighting stance) [Built]
- **Damage flash** [Decided]: a ball flashes **red** for a moment while taking damage (Minecraft-mob style) [Built]
- **Death & corpses** [Decided] (user idea): dead balls get x_x eyes, topple onto their back and **stay in the world and decay over in-game time**, so you can see the outcome of a recent battle. Only the moment of death is stored and the look is always worked out from the world clock, so sleeping, waiting, travel or returning days later all show the right state (and saving is one timestamp). Stage table is data, tunable per creature (horses too) [Built]
  - Stages (in-game time; 1 day = 48 real min): fresh → **pale** 6 h (~12 min) → **flies** 12 h (~24 min) → **decomposing** (greenish, bloated) 1 day → **rotting** 2 days → **bones** 3 days (bone-white, hollow eye sockets) → gone after 7 days. Colors blend gradually between stages
  - **Corpses are solid** (user, 2026-09-27; was: walk through them): a fresh body is a box the size of the lying ball that you bump into and can stand on; bones are low (you step up onto the ribs and over the skull). Traces still hit them (for looting/interaction later)
  - Only the body and hands decay; **boots (feet) stay as they are** and are left lying with the bones
  - **Bones = a cartoon ball-skeleton** (user design): big round skull with hollow eye sockets, thick neck (two chunky vertebrae, thinner than the chest), the **thickest bone in the center** (spine/breastbone) with **3 thick ribs** with knobby cartoon ends, a small pelvis; bone hands and boots lie beside it. Built from simple shapes for now; a Blender model can replace it in the art pass. Fixed (user bug report, 2026-09-27): the skeleton lay the wrong way round (skull where the boots are) and was bigger than the ball; now the skull lies where the top of the fallen ball was, the pelvis toward the boots, and it fits inside the ball's outline. Second pass (user): bone hands now lie beside the ribs and the boots past the pelvis (they were still at the skull end), and hands lie flat on the ground instead of hovering
  - **Scavengers** [Planned, needs bird art]: **ravens and crows** (common across medieval England; ravens and **red kites** even scavenged town streets) come to pick at bodies from the decomposing stage on. Flag is in the stage table already
  - Historical notes [Proposed]: after Wars of the Roses battles the dead were **stripped** of armour, weapons and valuables (by the victors, camp followers and locals) and usually **buried in grave pits** near the field within days (e.g. the Towton grave pit, 1461); nobles' bodies were often taken for burial in churches, and heralds counted the dead. Ideas: burial parties clear battlefields after a few days; you (and others) can **loot** corpses; sound of flies; crows cawing on old battlefields
  - Later: cap on how many corpses stay loaded; decomposing smell/fly sound; bone piles as proper art
- **HUD bars** [Decided]: **bottom-left**, HP bar (red) with the stamina bar (parchment yellow) under it; bars are a bit larger and scale with resolution. HP always shown, stamina only while not full
- **Equipment & inventory** [Decided]:
  - Equipment menu with gear slots: head, body, hands, feet, weapons (sword/shield/spear/bow… whatever you want to use)
  - **Layered clothing & armor, KCD-style** [Decided]: head = arming cap/coif + helmet (+ crest); body = undergarments (shirt/doublet) + padding (gambeson/aketon) + armor (mail/brigandine/plate) + optional over-layer (tabard/surcoat/livery jacket); hands = gloves/gauntlets; feet = shoes/boots/sabatons; plus cloak
  - **Icons vs worn models (user question, 2026-09-28)** [Decided]: **inventory icons show the real, historically accurate object** (players recognise it and learn from it; icons from the user's ChatGPT art in `Art/AI/Icons`). The **3D version worn by a ball is adapted to the sphere**: helmets become a cap over the upper part of the ball, sized to it; body armour wraps the lower half; gloves/boots fit the floating hands and feet. **The eyes and the flag always stay visible**
  - **Visored helmets (sallet, armet)** [Proposed]: visor up while walking (face visible); snaps down in combat, with the eyes showing through the eye slit
  - **Visible carried gear** [Decided]: shield, spear or **bow** on your back, dagger/sword at your side, arrows in an arrow bag or belt, etc. Balls wear a **belt around the middle** for side-carried weapons. Accuracy note: English longbowmen often carried arrows in arrow bags or tucked in the belt, and stuck them in the ground before battle, more than in back quivers [Proposed]
  - **Longbows carried unstrung** and **strung before use** (small animation) [Decided]
  - **Horse shown on the equipment screen** standing nearby (like KCD2, without the rotate-model gimmick), with **horse equipment** (saddle, bridle, saddlebags, caparison, barding)
  - **Carry capacity** on your character and on your horse
  - **Storage** in your house/estates; if an estate is attacked and you lose, **part** of the stored items can be looted (not all)
  - **Hidden stashes** that raiders can't find
- **Heraldry over armor:** identity shows through a **tabard/surcoat with your arms, livery badges, painted shield and helmet crest** when the ball is armored [Decided]
- **Travel & mounts** (historically grounded) [Decided]:
  - **On foot** is the default for commoners (~15–20 miles/day historically)
  - **Horse tiers:** affer/stott (farm workhorse) → hackney (hired/ordinary riding horse) → rouncey → palfrey → courser → destrier (warhorse, a status symbol). Better horses are faster, carry more, have more endurance, and change how NPCs treat you
  - **Renting horses ("hackneys")** at inns and towns, returned at another inn on the same road (historically real, e.g. regulated hire on the London–Dover road in the late 1300s)
  - **Mules/donkeys are rare** in England (common in southern Europe); **packhorses** carry goods (the wool trade)
  - **River boats and ferries** (Thames, Ouse, Trent) for travel and trade
  - Exact historical prices to be verified from sources when designing the economy
- **Command panel:** decide which units go where, prepare for invasions [Decided]
- **Small armies**, train units, fight [Decided]
- Towns, cities, villages: take and manage them [Decided]
- **Limited but nice building** mechanic [Decided]
- **Basic resources and economy** [Decided]; realistic money (£/s/d), upkeep that scales with size, AI plays by the same rules, to avoid AoH's runaway economy [Proposed]
- **Combat side markers** (a marker above each fighter), **shown only during combat**, hidden otherwise [Decided]:
  - **Blue:** your own troops/squad (under your control). If you hit them they **never fight back**, they just take damage/die
  - **Green:** allied houses. They **can turn hostile** if you attack or betray them
  - **Red:** enemies
  - **Orange** (or yellow): the enemy's allies. Claude suggests orange, since yellow can be confused with quest markers
  - **Grey:** neutrals and civilians [Decided]
  - **Colorblind support:** different marker shapes per side plus a colorblind palette option [Decided]
  - **Marker setting on/off**; off = identify sides by heraldry and livery [Decided]
  - **Hitting your own troops lowers morale and loyalty**; badly treated men can **desert** (run away) [Decided]
- **Artillery mishaps** [Decided]: cannons (and other gear) can go wrong and blow up your own side, for fun moments. Historically real: early bombards sometimes burst; King James II of Scotland was killed in 1460 at the siege of Roxburgh when a cannon exploded beside him
- **Camera** [Decided]: **first-person by default**, switchable to third-person
- **Personal combat is simple, with hit zones** [Decided]: strikes to the **head/face** do the most damage, **chest** is high, lower areas do less. Hit zones to be defined
- **Commanding troops** [Decided]: **fight alongside your troops or just watch**. Commands similar to Bannerlord but simpler and smaller scale: **formations, fall back, attack, split, encircle**, etc.
- **Progression** [Decided]: **levels and stats** (Skyrim/KCD-like). Gain **XP** from fighting, trading, communicating and period skills; **level up and assign skill points**. Less complex than KCD2/Skyrim; exact skill list [Open]
- **Economy** [Decided]:
  - A **limited list of relevant commodities**, plus **building materials** (stone, timber) for rebuilding, plus **food** (what armies, villages and everything run on), plus **money** (what you do everything with)
  - **Every village produces a couple of things** (mining, fishing, timber…), and all can have multiple outputs including food
  - Keep it **simple, only what is needed**; no micromanagement; only relevant if you own something
  - Historically accurate commodity candidates for 1455 England [Proposed]: **wool** (England's great export) and **cloth**, grain, livestock, fish, **salt**, timber, stone, iron, **lead**, **tin** (Cornwall/Devon), sea-coal, hides/leather, ale, imported **wine**; gold/silver mainly as coin and plate (oil is not a period commodity)
- News and orders travel at messenger speed [Proposed]
- Living chronicle of your playthrough [Proposed]

- **Destruction** [Decided]: walls, gates, houses and other structures can be damaged/destroyed (Unreal Chaos Destruction, stylized and pre-fractured on key structures for performance) [tech: Proposed]
- **Fire** [Decided]: buildings can be burned down (mostly wooden/thatched structures); **whole villages can be burned**
- **Rebuilding & resettlement** [Decided]:
  - Partly destroyed places can be **rebuilt**; takes **time**, **funds**, **resources** and **manpower** (townsfolk or soldiers); slower if you lack funds/resources
  - Fully destroyed villages must be **resettled**; for your own village you can **ask a lord to send settlers from a city** and try rebuilding
- **Destroyed places on the maps** [Decided]: a **red X** on the small (M) map; on the big (Esc) map the label reads **"VillageName (Destroyed)"**
- **Place statuses** in the same style: (Burning), (Under Siege), (Rebuilding), (Abandoned), (Plundered) [Decided]
- **Maps show only what you know** [Decided]: map information updates only when **your scouts, troops, allies (or you yourself)** learn about it; until then it shows the last known state
- **Ask the locals** [Decided]: you can ask people in an area for news, rumors and directions; this information is dynamic and changes over time
- [Decided] **Reliability depends on who you ask:** only **peasants/commoners** can give outdated or wrong info; **nobles, knights and scholars always give correct details**
- [Decided] Peasant info can be **outdated or wrong** (rumors, exaggeration, lies from enemies); directions from locals get drawn onto your M map
- Fire details [Decided]: fire **spreads** between close wooden/thatched buildings; **weather matters** (rain slows it, dry summers spread it); stone buildings resist; villagers form **bucket chains** to fight fires. Historical note: burning and plundering was a real tactic, e.g. Queen Margaret's northern army plundering on its march south in early 1461
- **Siege equipment of the period** [Decided]. Accuracy notes [Proposed]:
  - By 1455 England, **gunpowder artillery** (bombards, serpentines, handgonnes) was the main wall-breaker; e.g. Bamburgh (1464) fell to Warwick's cannon
  - **Trebuchets were largely obsolete by then**: use them rarely (old or improvised), or in other regions/eras via DLC
  - Also accurate: scaling ladders, battering rams, mantlets (mobile shields), mining/sapping, siege camps, starvation/blockade
- **Physics fun:** balls get knocked around and bounce during explosions and impacts (Angry Birds-like energy in action moments) [Proposed]

---

## 9a. Music & Audio

- **Period songs** (public-domain melodies and lyrics from the 1400s/1500s) **re-arranged and recorded by the user and friends** in their DAW, plus **original soundtrack** compositions [Decided]
- Prefer **15th-century** songs for accuracy (e.g. Agincourt Carol, Ritson and Fayrfax manuscripts); 1500s pieces (e.g. *Pastime with Good Company*) used sparingly [Proposed]
- **Rights checklist** [Proposed]: work from original manuscripts/facsimiles/public-domain editions (IMSLP), not modern copyrighted editions; never sample others' recordings; use original-language lyrics or our own translations; **written agreements with every friend who performs**; keep sources and project files to dispute YouTube Content ID false claims; lawyer check before commercial release
- Period songs in taverns, churches and on the march; original score for exploration, battles and story moments [Proposed]
- **Sound sources (2026-09-28)** [Decided]: free sounds from **Freesound.org** (only **CC0** or **CC-BY**, never NC), the **Sonniss GDC Game Audio Bundle** (free, commercial, no credit), **Pixabay** sound effects and free Fab audio. The user downloads (accounts are theirs) and sends each file with its source link; every file's source and licence is logged, and CC-BY authors go in the credits. WAV preferred
- **Soundscapes should sound like England** [Decided]: evening = blackbird and song thrush, rooks/crows, wind in the trees, a stream, an occasional distant church bell or sheep; only light grasshopper sounds in summer (a loud cricket chorus sounds Mediterranean/American)
- **Title screen ambience** [Built 2026-09-28]: a recording of the **River Frome** (England; user's pick from Pixabay), cut into a seamless 87 s loop, fading in over 2.5 s with the picture and out on New Game. Birdsong/rooks/wind layers can be added later. Sources and licences are logged in `Art/Audio/SOURCES.md`; originals kept in `Art/Audio/Source`, `Tools/make_audio_loop.py` makes seamless loops, `Tools/Unreal/import_audio.py` imports
- **The user produces music in FL Studio** (2026-09-28): an original soundtrack may come from them later ("especially for soundtracks"); basic sound design from free libraries for now

---

## 9b. UI & HUD

- **Compass bar at the top** (N/S/E/W) showing quest markers, settlements, your army and nearby threats. No minimap by default; keep the screen clean so players navigate by the world itself [Proposed]
- **Full map (M):** stylized parchment map of England in medieval cartography style; doubles as the **command panel** (lands, armies, orders) [Proposed]
- **Compass bar is the default; minimap can be switched on** in settings [Decided]
- **Map:** open anytime; place checkpoints/waypoints, pins and custom markers [Decided]
- **Two maps** [Decided]:
  - **Esc = full map + main menu:** pauses; a **stylized, high-quality and geographically accurate** map of all of England, **also in the Gough Map visual style** (polished and accurate, while the M map is the rougher period-authentic version) with menu tabs at the top (proposed tabs: Map · Quests · Inventory · Character · Realm/Armies · Chronicle · Settings); planning, waypoints, orders.
  - **M = small regional travel map:** a **period-authentic** map, the kind people of the time would actually use (style reference: the **Gough Map**, the earliest surviving road map of Britain, late 1300s–1400s); shows only the current region, with the route to your waypoint **drawn in ink along the real roads**.
  - Idea: the ball physically pulls out the M map in-world with its floating hands and the game keeps running, so you can check it while riding but stay vulnerable (Far Cry 2/Firewatch-style) [Decided]
- **Route guidance that feels part of the world, NOT a glowing GPS path** (like Ghost of Tsushima's Guiding Wind in spirit, not in form; unlike AC Shadows' highlighted path) [Decided]. Proposed approach:
  - **Horse follows the real road** to your waypoint when you hold a key (RDR2/KCD-style) [Proposed]
  - **Crossroads guidance:** at junctions the compass and an in-world marker (a wayside cross or boundary stone catching the light; no signposts, which are anachronistic) show which road to take [Proposed]
  - **Route drawn on the parchment map only**, not in the world [Proposed]
  - Optional: **your house pennant flutters toward the destination** [Proposed]
  - Accessibility setting: full highlighted path for players who want it [Proposed]
- **Menu look (user mockup, 2026-09-28)** [Decided]: dark charcoal panels with a thin **worn-gold border and gold highlights**, a serif period-feel font, tabs across the top switched with **Q / E**, key hints bottom right (F Unequip, R Inspect). The **Equipment tab is a paper doll**: the character's live 3D model in the middle, slots arranged around it, and an **item card** on the right (name, type, short historical description, then stats such as Defense, Weight, Durability)
  - **Slot layout (user, 2026-09-28, final)** [Decided]: **8 small boxes** around the character: 1 at the top, 3 on each side, 1 at the bottom. **Every box is split in half**, and each half is its own slot (16 slots). Concept image: `Docs/Concept/billman_ui_concept_v2.webp` (final layout; `billman_ui_concept.webp` is the earlier version).
    - **Top:** [Helmet | Coif]
    - **Left, top to bottom:** [Weapon Main | Weapon Off] · [Back | Cloak] · [Belt 1 | Belt 2]
    - **Right, top to bottom:** [Gambeson | Tunic] · [Mail | Plate] · [Ring | Necklace]
    - **Bottom:** [Gloves | Boots]
    - A **two-handed weapon (bill, poleaxe, longbow in hand) occupies both weapon halves**: the Off half greys out and shows "Used by <weapon>"
    - **Back:** shield, bow, etc. **Cloak:** warmth and rain cover (ties into the weather and temperature system) and looks (plain wool vs fur-lined)
    - **Belt halves are general-purpose:** any belt item fits either half (dagger, torch, arrow bag, sword in scabbard, purse, lantern)
    - **Armour is two layers, like real 15th-century soldiers:** **Mail** (inner: mail shirt/haubergeon, mail skirt) | **Plate** (outer: brigandine, jack of plates, breastplate, full harness). Either or both can be worn
    - **Jewellery has meaning:** **Necklace = livery collar** (Lancastrian **SS collar**, Yorkist **collar of suns and roses**: shows whose side you're on, a status item for lords and retainers); **Ring = signet ring** (nobles seal letters and orders with it; may be required for sending them)
    - **Stats line under the character:** **Weight** (carried / max) · **Protection** · **Warmth**
    - The shirt/undergarment is included automatically (not visible on a ball); gauntlets go in Gloves, sabatons in Boots; a bevor comes as part of a helmet item ("sallet with bevor")
    - **The horse gets its own equipment page later** (saddle, saddlebags, barding, tack) [Planned]
  - **Controls** [Decided]: every half is its own cell on the grid, so there is **no "open the box" step**.
    - **Mouse:** hover a half to highlight it; click opens its **item picker**; the wheel scrolls the picker list
    - **Keyboard (WASD/arrows) and controller (d-pad/stick):** move straight from half to half (A/D steps Helmet → Coif, then on to the next box; W/S moves up and down the columns). **Enter/Space** (controller A) opens the item picker; W/S chooses an item; Enter equips; **Esc** (controller B) goes back. F unequips and R inspects the highlighted slot
    - **Item picker:** the right-hand card turns into a list of the items you own that fit that slot, each showing its stat changes against what you're wearing (green better, red worse)
    - **Equipped** halves have a thin gold border; the **highlighted** one has a brighter glow
  - Tabs merge with the Esc menu's tab list (Map · Quests · Inventory/Equipment · Character · Realm/Armies · Chronicle · Settings) [Proposed]
  - Item descriptions must be historically right (e.g. the kettle hat was common among archers and billmen; men-at-arms mostly wore a sallet with a bevor) [Decided]

---

## 10. Technical

- **Engine:** Unreal Engine 5.8, C++ gameplay code (Blueprints only for small visual hookups)
- **Data-driven world:** provinces, settlements, houses, characters, units, events and quests live in data files, so DLC regions = new data + new art [Decided]
- **Platform:** PC first; keep lighting/UI scalable for a possible mobile port [Decided]
- **Tools:** Unreal MCP, Blender MCP, Git + GitHub (LFS)

---

## 11a. Build Roadmap [Decided]

Start with the **commoner origin**; build what's designed so far and add features as we progress.

- **v0.1 "Does it feel good?"**: small stylized Yorkshire countryside with day/night and weather; one tiny village (houses, church, smithy, alehouse); player ball (walk/run, first/third-person, floating hands and feet, eye emotions); rideable horse; simple hit-zone combat vs a bandit or two; compass bar
- **v0.2 "A slice of life"**: commoner origin + simple character creator (name + looks); NPC balls living in the village; talking/asking directions; buying/selling, money; first quest; M map (Gough style)
- **v0.3 "Going to war"**: recruitment letter from the local lord; small skirmish with your troops, simple commands, side markers; XP and leveling

**v0.1 progress** (build order; one tested, committed step at a time):
1. [x] Player ball: C++ `ABallCharacter` (base for every person) + `APlayerBallCharacter`; procedural floating hands/feet (walk cycle, bob, lean, jump tuck); 8 eye emotions (Neutral, Happy, Sad, Angry, Scared, Tired, Suspicious, Dead) drawn by the `M_BallEyes` shader with blinking; walk/run; first-person default, third-person toggle. Placeholder shapes until the Blender art pass
   - Playtest tuning (user): walk 300→220, run 600→400, jump 450→340 ("a bit better now", fine for now)
   - Stamina for running/jumping (see Gameplay Systems)
   - Fixed: feet crossing each other when strafing (A/D) → side-steps now; hands/feet going through objects → hands stop at walls, feet plant on the real ground (slopes/steps)
   - Playtest round 2 (user): hands confirmed fixed. Feet still overlapped a bit sideways → **sideways (A/D) movement in first-person is slower (70%)**, stance widens while side-stepping and feet keep an edge-to-edge gap. Stamina bar moved bottom-left and enlarged; **HP bar added** (100 base)
   - Playtest (user, 2026-09-27): **each foot finds its own ground height** where it actually lands (steps, slopes, edges, bodies), not just under its resting spot
   - Playtest round 3 (user): stopping (especially after walking sideways) looked abrupt because feet slid back to the idle pose → **feet now really step**: each foot stays planted until the body moves too far, then steps in an arc; stopping ends with small settling steps, turning on the spot steps too; hands swing with the opposite foot, the body bobs with the steps
   - Playtest round 4 (user): thumb and index finger passed through each other → thumb moved to the side edge of the palm; in a fist it tucks on the front of the fist
   - Playtest round 5 (user): one-piece thumb looked wrong in a fist, and the punch looked like the hand just sliding forward → **every finger now has two segments with a joint** (fingers roll into a real fist, the **thumb wraps over the front** of the curled fingers); **the punch has phases**: short wind-up, a snap forward that accelerates into the hit and **corkscrews** from a vertical fist to palm-down, then the recoil; the **body twists into the punch** (punching side forward), leans and lunges; the **other fist comes up to guard the face**; in first-person the **view nudges forward** with each punch. Parts of one color share one material (cheaper to draw)
2. [x] Day/night (48 min) + weather
   - 2a [x] World clock + calendar and `DayNightSky` (sun, moon, atmosphere, clouds, fog, exposure) following it
   - 2b [x] Weather (clear/cloudy/overcast/rain/snow) and seasons
     - [x] **Seasons** (2026-09-27, user asked for seasons first): see Time & Calendar > Seasons
     - [x] **Weather** (2026-09-27): see Time & Calendar > Weather
3. [ ] Yorkshire test landscape
4. [ ] Village (free Fab assets, approved by the user first)
5. [x] Rideable horse (first version, 2026-09-27; user asked for it before weather)
   - **E** near a horse gets on (prompt "E  Get on the horse" appears when close), **E** again gets off (you land beside it, left side like a real rider, the right if blocked). E is the general interact key (later doors, talking, looting)
   - **WASD steer relative to where you look**; the horse turns gradually (wide turns at a gallop, turns around at a slow walk if you ask it to go backwards), never slides sideways, and builds up / loses speed smoothly. **Shift = gallop** (~32 km/h; walk ~7 km/h), draining the **horse's own stamina** (brown bar above your health while riding). **Space = jump** (costs horse stamina, plays the gallop-jump)
   - **Stamina (user, 2026-09-27):** the horse has its **own, much bigger stamina (400 vs a person's 100**, about a minute of gallop; same bar length as ours, it just empties more slowly) and **its own health (100)**: while riding, the horse's health and stamina bars sit above yours. A horse at 0 health falls dead (death animation, stays lying) and throws the rider off. **Galloping also tires the rider slowly** (1.5/s, the same rate as holding the guard); an exhausted rider can't push the horse into a gallop. Stamina always starts full
   - The ball sits in the saddle, boots hanging at the horse's sides, fists on the reins; you can still look around freely. Third-person camera pulls back to show the whole horse
   - Horses are data (`MountDefinition`: model, animations, speeds, turn rates, stamina costs, saddle point): the design's tiers (affer, hackney, rouncey, palfrey, courser, destrier) become more data assets. `DA_Mount_Horse` (brown) and `DA_Mount_HorseWhite` stand near the start of `L_DevSandbox`
   - Animation: idle, walk and gallop blend by speed and play faster as the horse speeds up (no hoof sliding); the Quaternius pack has no trot, so a canter uses the gallop played slower
   - Known limits (later): getting on/off is instant (no climb animation yet); only the chest has collision (the horse stops before its head hits a wall; the rump can still clip); no saddle/bridle model yet; riderless horses just stand (grazing/wandering AI later); no combat from horseback tuning yet
6. [ ] Hit-zone combat + bandits (**together with the item system and filling the Equipment screen**: bill, dagger, kettle hat, jack, mail… with real damage, protection and weight)
7. [ ] Compass bar

**Inserted before step 3 (user, 2026-09-28)** [Decided]:
- A. [x] **Ball resize** (2026-09-28): perfect sphere + the new height/proportion rules (Characters & Art > Height, Body proportions). Every ball is now the average man: **1.00 m sphere, 49 cm empty gap, 22 cm boots (taller ankle shaft), 1.71 m total** (was a 1.04 × 1.25 m egg with a 32 cm gap). Face shell and eye shader need no stretch any more. NPCs roll their own height (and women's sizes) when NPCs arrive [Built, waiting for the user's playtest]
- B. [ ] **UI style + Esc pause menu + Equipment tab shell:** B1 [x] done 2026-09-28: shared style (`UI/CIRLUIStyle`: colours, Cinzel/EB Garamond, bordered boxes), the menu (`UI/SCIRLGameMenu`, pure Slate): blurred paused game behind a fixed-size panel, tabs Map · Quests · Equipment · Character · Game with Q/E, gold underline on the open tab, key hints; the **Game tab** has Resume, Quit to Desktop, the version and the controls list; other tabs say what's coming. **Esc** opens it on Game, **Tab / I** on Equipment (Esc in the editor's Play mode stops the game, so use Tab there); WASD, arrows and controller move between buttons; console `DevMenu <Tab>` for testing. B1b [x] **Title screen** (2026-09-28, user asked for it before the Equipment tab): the game now starts in `L_MainMenu` (GameMode `CIRLTitleGameMode`), showing the user's village painting with a dark fade on the left, the title, "The Wars of the Roses · England, 1455", New Game / Continue (greyed: no saves yet) / Settings (greyed: coming) / Quit, the version, and a fade in from black. New Game shows one of the 4 loading paintings with a true historical fact while the world loads (Project Settings > CountriesIRL World > Maps: `TitleMap`, `NewGameMap`). The Esc menu's Game tab got **Main Menu**. Playing `L_DevSandbox` in the editor still starts straight in the world. A proper streaming loading screen (MoviePlayer) comes when the real map is big enough to need it. B2a [x] **Equipment tab layout** (2026-09-28): `UI/SCIRLEquipmentPage` with the 8 split boxes (16 slot buttons), labels, the item card (slot name, "Nothing equipped", big faint picture, "What goes here") and the Weight / Protection / Warmth line (zeros until items exist). Empty slots show a faint silhouette of the user's icon for that slot (so far helmet and coif); icons are trimmed and centred by `Tools/prepare_icons.py`. Mouse, WASD/arrows and controller move slot to slot and the card follows; Enter/F/R hints shown (they act once items exist). Slot list is data (`Items/EquipmentSlot`: `ECIRLEquipSlot` with name, what it holds, silhouette icon) for the item system to reuse. B2b [x] **Live 3D character** (2026-09-28): `UI/CIRLPaperDollStage` is a little photo studio 2 km below the world with a copy of the ball (the "doll"), three studio lights on lighting channel 1 (only the doll is lit by them, so it looks the same day or night) and a camera filming it every frame while the Equipment tab is open (captures don't update on their own while paused). `M_UI_PaperDoll` shows the picture with the right transparency and colours. **Drag with the mouse to turn the ball, double-click to face it forward.** The doll keeps blinking while the game is paused. For now it's the default ball; the player's flag, livery and worn gear get copied onto it once they exist. Needs the project setting Alpha Output (`r.PostProcessing.PropagateAlpha`). Original plan: the mockup's look as reusable pieces (dark panels, worn-gold border, serif font, Q/E tabs, key hints, hover glow); pause menu (Resume, Settings, Controls, Quit) so test builds have a menu; Equipment tab with the live 3D ball and the 8 split boxes, navigation working, slots empty until step 6
- **Art from the user (ChatGPT):** list, sizes and folders in `Art/AI/README.md` (menu background, panel texture, ~45 item icons, optional model sheets). Free fonts: Cinzel (titles) + EB Garamond (text) from Google Fonts (OFL); small UI glyphs from game-icons.net (CC BY 3.0, credit needed)

**Controls (v0.1):** WASD move · mouse look · Shift run · Space jump · **Left click punch** · **hold right click guard** · V first/third-person · **E get on/off a horse** · T cycle emotion (debug). Dev console: `Emotion Angry`, `ToggleCamera`, `DevWalk <forward> <right> <seconds> <run 0/1>` (fakes held movement keys for testing), `DevDamage 25`, `DevHeal 25`, `DevTime 21.5` (jump to a time), `DevTimeSpeed 60` (fast-forward; 1 = normal), `DevClock` (show date/time), `DevAdvance 24` (skip hours), `DevHitNearest 20` (damage nearest ball; 1000 kills), `DevPunch`, `DevGuard` (toggle guard), `DevDate 25 12` (jump to a day of the year, shows the clock), `DevYear 1461`, `DevInteract` (press E), `DevJump`, `DevHitHorse 35` (damage your/the nearest horse), `DevWeather Rain` (Clear/Fair/Cloudy/Overcast/Showers/Rain/HeavyRain/Storm/Snow/Fog/Auto). Dev test level: `L_DevSandbox`.

**Test builds (sharing the game)** [Built, 2026-09-27]:
- `powershell -ExecutionPolicy Bypass -File Tools\package_game.ps1` (editor closed) builds a Windows **Shipping** copy into `C:\Dev\CountriesIRL_Builds\Windows` and zips it (`CountriesIRL_v0.1.0_test.zip`, **~430 MB**, ~730 MB unzipped; about 4 minutes, longer the first time because of shaders). `Docs/HOW TO PLAY.txt` (controls, Windows "More info > Run anyway", requirements) goes next to the .exe
- Only our map is included (template demo levels left out); everything under `/Game/CountriesIRL` is always included because some of it is loaded by code
- Sharing: the user shares it (WeTransfer etc.); later itch.io (free, private pages) and Steam ($100, Playtest) [Proposed]
- **Windows Smart App Control** (2026-09-28): on the dev PC it blocked a freshly built, unsigned game DLL ("Bad Image 0xc0e90002", CodeIntegrity log: Smart App Control Block). Developers normally turn it off (Windows Security > App & browser control); only the user can change that. Testers with it on may also get the packaged game blocked: long-term fix is code signing or distributing via Steam/itch.io [Noted]
- In release builds the Dev console commands are off. **Esc menu and title screen built 2026-09-28** (Resume, Main Menu, Quit, controls list, version; title screen with New Game/Quit). Still to come: save/Continue, settings (graphics, resolution, mouse, FOV), credits [Proposed]
- Project display name "CountriesIRL" (working title, the real title is still open), version 0.1.0

**Budget rule** [Decided]: use **free assets and tools as much as possible**, plus our own creations; only consider paid assets when nothing free works.

**Asset workflow** [Decided]: Claude searches Fab for free assets that fit, **shows them to the user and asks for approval**; the user claims them with their Epic account and adds them to the project; then Claude integrates them.

---

## 11. First Playable Slice (original proposal)

Yorkshire & the North, spring 1455:
- A small stretch of stylized terrain with day/night and weather
- 1 city (York), 1 castle, 2–3 tiny villages
- The player ball: walk, ride a horse, emotions
- York vs Lancaster balls with real heraldry
- Recruit a small army, basic money/resources
- One small quest and one skirmish

---

## 11b. Marketing & Trailer

- **Trailer concept** [Decided]: start **chill** (peaceful countryside, village life, calm music) → shift to **adventurous music** → **sieges and destruction**, with **fun moments** mixed in
- Possible beats [Proposed]: sheep and a farmer ball at dawn → a messenger ball galloping in → armies marching with banners → bombard fires, wall crumbles, balls bouncing → a big emotional/epic final shot → title
- Distribution through the CountriesIRL network (~300k followers, ~35 creators) [Proposed]

---

## 12. Open Questions

- ~~**Character body: pure countryball vs. countryball head on a small cartoon body**~~ **Resolved 2026-09-27: pure countryballs** (community vote 48 / 7 simplified / 7 complex; see section 8). History kept below. (user idea, 2026-09-27, reference: a chibi skeleton with a big head and small body). Keep the ball head with the flag and eyes either way.
  - Research (Sept 2026): countryballs are still big across ages: ~16.6M TikTok posts (#countryballs), ~10M for countryball animations, r/polandball ~690k members, several well-reviewed countryball games in 2025 (e.g. Countryballs at War 94%, Countryballs Conquest 88%), growing "kidult" plush market (adults >20% of plush buyers). Almost all countryball games are 2D strategy/map games; a 3D open-world countryball game is an open niche
  - For a body: the planned KCD-style layered clothing/armor, belts, back-carried bows and tabards need a body; clearer melee and riding; free CC0 humanoid animations exist (Quaternius Universal Animation Library 1+2, 250+ animations incl. melee and farming) and can be retargeted in Unreal
  - For pure balls: instantly recognizable brand; the traditional countryball rules say no arms/legs; procedural animation already works with no rig; cheaper
  - Plan [Proposed]: build one prototype of the hybrid (Blender model + rig + a few free animations) next to the current countryball in the sandbox, play both, then decide
  - User's view (2026-09-27): leaning toward a body. A body lets the game use normal human-scale assets (and reuse them), is simpler, and the user thinks it looks better; unsure countryballs alone are relevant enough to carry the game; wants it to be unique (no 3D countryball game like this exists)
  - Claude's view: agree on the hybrid (countryball head with flag + eyes, small cartoon body): keeps the recognizable signature while unlocking human-scale Fab assets (furniture, tools, saddles), free animations (Unreal template anims already in the project, Quaternius CC0) and layered clothing/armor. (Correction noted: the current 1.25 m ball already fits human-scale doors; buildings would not need to be weird either way.) Prototype approach: Blender body rigged to Unreal's standard mannequin skeleton so existing animations work immediately
  - **Prototype built (2026-09-27):** press **B** to switch your character between the countryball and the **ball head on a small body** (Unreal's free template mannequin at 55% size, its head hidden, our countryball head with eyes/emotions on the neck; free template walk/jog/jump animations, attack animations for punches, ragdoll on death). Console `DevBodyAll` switches every ball. Grey mannequin = shape test only; next: a free cartoon body (Quaternius CC0 / Fab free, links sent to the user first) or our own Blender body
  - Tuning (2026-09-27): side-by-side tests showed the plain 55% mannequin looked like a thin robot with the head on a stick. **New default: chunky chibi proportions** (body scaled 0.8 wide × 0.5 tall, **72 cm head**, head sitting low on the shoulders), close to the user's skeleton reference (superseded by the Quaternius outfit sizes below)
  - Free body candidates found (Fab **Personal license is free** for individuals/teams under $100k revenue/year; Professional is ~5 lei): **Synty Sidekick FREE Starter Pack** (modular incl. fantasy knight parts, body sliders, fully compatible with Unreal's mannequin skeleton so our animations work directly; needs the free Sidekick plugin), **Quaternius Modular Character Outfits – Fantasy + Universal Base Characters** (CC0, 12 medieval/fantasy outfits, needs retargeting; quaternius.com / itch.io), **PolyOne Free Pack – Cartoon Skeleton (Rigged)** (likely the user's skeleton reference; could be the bones stage for bodied characters)
  - **Quaternius outfits in (2026-09-27):** the user downloaded *Modular Character Outfits – Fantasy [Standard]* (CC0, free: Male/Female Peasant, Male/Female Ranger). The body mode now wears the **Male Peasant** outfit (shirt, trousers, arms, boots) instead of the grey mannequin
    - How it works: the mannequin still plays all animations (walk, jump, punch montages, ragdoll) but is invisible; the outfit copies its pose every frame through an **IK Retargeter** (the two skeletons are posed differently: A-pose vs T-pose). No extra animation work per outfit; Quaternius' free Universal Animation Library uses this same outfit rig, so its 250+ CC0 animations fit later
    - Data-driven: an outfit is a **CharacterOutfit** data asset (list of mesh parts + retargeter + neck bone), e.g. `DA_Outfit_MalePeasant`. Villager/soldier/DLC outfits = new data assets
    - User's call: with these more realistic bodies the head should be **smaller** (the first idea was more animated/anatomically incorrect). Lineup tests at 38/44/50/56/72 cm → **new default: 52 cm head** (bigger than a real head, smaller than the pure countryball), **body 0.85 wide × 0.75 tall** (a bit stocky), head sitting on top of the neck instead of sunk into the shoulders
    - Still open: which body style to keep overall; next candidates are the Female Peasant/Rangers outfits for variety and skin/flag colors on the arms
  - **User feedback on the Quaternius body (2026-09-27): too detailed.** Wanted: low poly but much more simplified, not a full human body; "a shape or two", cartoonish and simple, with its own style. Quaternius outfit is parked (kept in the project, not the direction)
    - Animations clarified: we did not make our own body animations. The body uses Unreal's free template animations; the Quaternius rig only needed the retargeter to translate them. Any simple shape body pinned to the (invisible) mannequin skeleton gets the same free animations
    - Options shown to the user: A) bean tunic with tube arms and stubby legs, B) tunic/bell body with floating hands and boots (Rayman-like, closest to the current countryball), C) blocky toy (trapezoid torso, block arms and legs). The tunic front can carry the flag/heraldry


- Capture system details (ransom, escape, consequences)
- How the command panel works in practice
- Battle presentation and how much direct control
- Building mechanic scope
- Economy and resource list
- Game title (note: "Kingmaker" is taken by a 1974 Wars of the Roses board game and by *Pathfinder: Kingmaker*)

---

## 13. Idea Backlog

_(User's ideas to be added here as we discuss them.)_

---

## Archive

- *Pecado City*: an earlier noir detective game design by the user (2D point-and-click). Shelved.
