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
- **You basically never die**: losing a big battle means capture (ransom, escape, bargaining, losses) instead of game over. Details [Open]
- **No inheritor/succession system** (probably) [Decided]

---

## 8. Characters & Art (Countryballs)

- **All humans are countryballs**: coat of arms/livery on the ball, **white eyes, no mouth** [Decided]
- **Expressive eyes** (normal, happy, sad, angry, scared, tired, suspicious, dead ×_×…); reference: u/tengam15 "Big Chart o' Expressions" (inspiration only, draw our own set) [Decided]
- **Rayman-style floating hands AND feet** (hands hold weapons/tools; feet wear boots/sabatons and make walking/riding read clearly) [Decided]
- **Body variation:** randomized size (99% in a normal range, ~1% very tall or very small); proportions: longer, fatter, thinner, shorter [Decided]
- **Hair and beards** with different hair colors [Decided]
- **Aging is visible:** graying hair, growing beards, scars from battles, weathered colors [Proposed]
- **Animals:** very simple and stylized (horses and sheep most important). Possible source: Quaternius (free, commercial OK) [Decided]
- **Real historical figures** have their real heraldry and source-based personalities [Proposed]

---

## 9. Gameplay Systems (outline)

- Travel the world as your character; horses [Decided]
- **Equipment & inventory** [Decided]:
  - Equipment menu with gear slots: head, body, hands, feet, weapons (sword/shield/spear/bow… whatever you want to use)
  - **Layered clothing & armor, KCD-style** [Decided]: head = arming cap/coif + helmet (+ crest); body = undergarments (shirt/doublet) + padding (gambeson/aketon) + armor (mail/brigandine/plate) + optional over-layer (tabard/surcoat/livery jacket); hands = gloves/gauntlets; feet = shoes/boots/sabatons; plus cloak
  - **Visible carried gear** [Decided]: shield, spear or **bow** on your back, dagger/sword at your side, arrows in an arrow bag or belt, etc. Balls wear a **belt around the middle** for side-carried weapons. Accuracy note: English longbowmen often carried arrows in arrow bags or tucked in the belt, and stuck them in the ground before battle, more than in back quivers [Proposed]
  - **Longbows carried unstrung** and **strung before use** (small animation) [Decided]
  - **Horse shown on the equipment screen** standing nearby (like KCD2, without the rotate-model gimmick), with **horse equipment** (saddle, bridle, saddlebags, caparison, barding)
  - **Carry capacity** on your character and on your horse
  - **Storage** in your house/estates; if an estate is attacked and you lose, **part** of the stored items can be looted (not all)
  - **Hidden stashes** that raiders can't find
- **Heraldry over armor:** identity shows through a **tabard/surcoat with your arms, livery badges, painted shield and helmet crest** when the ball is armored [Decided]
- **Travel & mounts** (historically grounded) [Proposed]:
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
- Battles: **"command, don't control"**, fewer and simpler fights than Bannerlord [Proposed]
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
- [Decided] Local info can be **outdated or wrong** (rumors, exaggeration, lies from enemies); directions from locals get drawn onto your M map
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

## 11b. Marketing & Trailer

- **Trailer concept** [Decided]: start **chill** (peaceful countryside, village life, calm music) → shift to **adventurous music** → **sieges and destruction**, with **fun moments** mixed in
- Possible beats [Proposed]: sheep and a farmer ball at dawn → a messenger ball galloping in → armies marching with banners → bombard fires, wall crumbles, balls bouncing → a big emotional/epic final shot → title
- Distribution through the CountriesIRL network (~300k followers, ~35 creators) [Proposed]

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
