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
3. **Countryball charm, serious depth** [Decided]: cute style, real drama (war, betrayal, loss).
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

---

## 5. Time & Calendar

- **Accurate real calendar that advances**, RDR2-style [Decided]
- **Day/night ≈ 48 real minutes per day** while playing (GTA V/RDR2-like) [Decided]
- Time moves faster through sleep, waiting, long travel and command actions [Proposed]
- **Seasons and weather** are dynamic and affect the world [Decided]
- **Story chapters can jump the calendar** (no waiting literal years) [Decided]
- A playthrough covers a few in-game years; no fixed end date [Decided]

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
- **You basically never die**: losing a big battle means capture (ransom, escape, bargaining, losses) instead of game over. Details [Open]
- **No inheritor/succession system** (probably) [Decided]

---

## 8. Characters & Art (Countryballs)

- **All humans are countryballs**: coat of arms/livery on the ball, **white eyes, no mouth** [Decided]
- **Expressive eyes** (normal, happy, sad, angry, scared, tired, suspicious, dead ×_×…); reference: u/tengam15 "Big Chart o' Expressions" (inspiration only, draw our own set) [Decided]
- **Floating hands** hold weapons and tools [Proposed]
- **Body variation:** randomized size (99% in a normal range, ~1% very tall or very small); proportions: longer, fatter, thinner, shorter [Decided]
- **Hair and beards** with different hair colors [Decided]
- **Aging is visible:** graying hair, growing beards, scars from battles, weathered colors [Proposed]
- **Animals:** very simple and stylized (horses and sheep most important). Possible source: Quaternius (free, commercial OK) [Decided]
- **Real historical figures** have their real heraldry and source-based personalities [Proposed]

---

## 9. Gameplay Systems (outline)

- Travel the world as your character; horses [Decided]
- **Command panel:** decide which units go where, prepare for invasions [Decided]
- **Small armies**, train units, fight [Decided]
- Towns, cities, villages: take and manage them [Decided]
- **Limited but nice building** mechanic [Decided]
- **Basic resources and economy** [Decided]; realistic money (£/s/d), upkeep that scales with size, AI plays by the same rules, to avoid AoH's runaway economy [Proposed]
- Battles: **"command, don't control"**, fewer and simpler fights than Bannerlord [Proposed]
- News and orders travel at messenger speed [Proposed]
- Living chronicle of your playthrough [Proposed]

---

## 10. Technical

- **Engine:** Unreal Engine 5.8, C++ gameplay code (Blueprints only for small visual hookups)
- **Data-driven world:** provinces, settlements, houses, characters, units, events and quests live in data files, so DLC regions = new data + new art [Decided]
- **Platform:** PC first; keep lighting/UI scalable for a possible mobile port [Decided]
- **Tools:** Unreal MCP, Blender MCP, Git + GitHub (LFS)

---

## 11. First Playable Slice [Proposed]

Yorkshire & the North, spring 1455:
- A small stretch of stylized terrain with day/night and weather
- 1 city (York), 1 castle, 2–3 tiny villages
- The player ball: walk, ride a horse, emotions
- York vs Lancaster balls with real heraldry
- Recruit a small army, basic money/resources
- One small quest and one skirmish

---

## 12. Open Questions

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
