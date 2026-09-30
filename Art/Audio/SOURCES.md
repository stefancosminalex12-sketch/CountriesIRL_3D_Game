# Audio sources and licences

Every sound file in the game, where it came from and its licence. Only CC0, CC-BY, Pixabay and Sonniss GDC
licences are allowed (never NonCommercial). CC-BY authors must appear in the game's credits.
Originals are kept unchanged in `Source/`; the game uses the edited versions (loops, levels) in the other folders.

| Game file | Original | Source / author | Licence | Edits |
|---|---|---|---|---|
| `Ambience/amb_title_river.wav` (title screen) | `Source/freesound_community-river-frome-59101.mp3` (River Frome, England) | Pixabay, uploader "freesound_community" (re-posts Freesound recordings), sound id 59101 | Pixabay Content License (free, commercial use, no credit required) | Seamless loop from 34–125 s (skips the fade-in/out and a loud moment at 29–32 s), 4 s crossfade, levelled |
| `Music/mus_theme_title.wav` (title screen theme) | `Source/theme_song1_user.mp3` ("Theme song1") | The user (the game's own composer) | Own work | Cut from 16 s to the end (164 s), 10 ms edge fades, not levelled |
| `Music/mus_tavern_song1.wav` (main menu playlist) | `Source/tavern_song1_user.mp3` ("Tavern song1") | The user (the game's own composer) | Own work | Full length, 10 ms edge fades, not levelled |
| `Music/mus_tavern_song2.wav` (main menu playlist) | `Source/tavern_song2_user.mp3` ("Tavern song2") | The user (the game's own composer) | Own work | Full length, 10 ms edge fades, not levelled |
| `Music/mus_tavern_song3.wav` (main menu playlist) | `Source/tavern_song3_user.mp3` ("Tavern song3") | The user (the game's own composer) | Own work | Full length, 10 ms edge fades, not levelled |
| `Music/mus_ambience_song1.wav` (first in-game track) | `Source/ambience_song1_user.mp3` ("Ambience song 1") | The user (the game's own composer) | Own work | Full length (240 s), 10 ms edge fades, not levelled |
| `Music/mus_ambience_song2.wav` (in-game playlist) | `Source/ambience_song2_user.mp3` ("Ambience song 2") | The user (the game's own composer) | Own work | Full length, 10 ms edge fades, not levelled |
| `Music/mus_tavern_song4.wav` (main menu playlist) | `Source/tavern_song4_user.mp3` ("Tavern song4") | The user (the game's own composer) | Own work | Full length, 10 ms edge fades, not levelled |
| `Sfx/sfx_death_bells.wav` (plays when the player dies) | `Source/unternehmenxy-glockengelaut-nr-2-435473.mp3` ("Glockengeläut Nr. 2", church bells) | Pixabay, uploader "UnternehmenXY", sound id 435473 | Pixabay Content License (free, commercial use, no credit required) | Cut to 28 s (last 2 s were silence), lowered to 80% (the recording touches full scale), 10 ms edge fades |
| `Sfx/sfx_horse_walk.wav, sfx_horse_trot.wav, sfx_horse_canter.wav, sfx_horse_gallop.wav (hoofbeat loops)` | `Source/Horse walk.mp3, horse trot 2.mp3, Horse canter 1.mp3, Horse gallop 5-11.mp3 (Horse trot.mp3 unused)` | Provided by the user (2026-09-30); **source and licence to confirm** | ? | Loops cut and crossfaded by Tools/prepare_sfx.py, levelled |
| `Sfx/sfx_horse_neigh_1..5.wav` | `Source/Horse neigh 1.mp3, Horse neigh 2.mp3, horse neigh 3.mp3, horse neigh 4.mp3, Horse neigh 5.mp3` | Provided by the user (2026-09-30); **source and licence to confirm** | ? | Trimmed, peaks at 90%, 10 ms fades |
| `Sfx/sfx_horse_snort_1..5.wav, sfx_horse_snort_deep.wav` | `Source/Horse snort 1.mp3 (split into 4), Horse snort 2.mp3, horse deep snort 1.mp3` | Provided by the user (2026-09-30); **source and licence to confirm** | ? | Trimmed, peaks at 90%, 10 ms fades |
| `Sfx/sfx_deer_snort.wav (not used yet)` | `Source/deer snort 1.mp3` | Provided by the user (2026-09-30); **source and licence to confirm** | ? | Trimmed, peaks at 90%, 10 ms fades |
| `Sfx/sfx_steps_sand_1.wav (footsteps on sand)` | `Source/sand walk 1.mp3` | Provided by the user (2026-09-30); **source and licence to confirm** | ? | Loop cut and crossfaded, levelled |
| `Sfx/sfx_steps_dirt_1..2, gravel_1..3, rock_1, leaves_1, dry_leaves_1, mud_1 (footstep loops)` | `Source/dirt walk 1-2.mp3, gravel walk 1-3.mp3, rocky trail walk.mp3, leaves walk 1.mp3, dry leaves walk 1.mp3, wet steps on liquit and dirt 1.mp3` | Provided by the user (2026-09-30); **source and licence to confirm** | ? | Mono, loops cut and crossfaded (long ones kept to 60 s), levelled on the steps |
| `Sfx/sfx_land_dirt_1.wav (landing)` | `Source/fall on dirt 1.mp3` | Provided by the user (2026-09-30); **source and licence to confirm** | ? | Trimmed, levelled, 10 ms fades |
