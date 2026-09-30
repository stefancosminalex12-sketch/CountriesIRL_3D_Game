"""
Turn the user's sound recordings (Art/Audio/Source) into the game's sound effects (Art/Audio/Sfx/*.wav):
- loops for continuous sounds (a horse's gaits, footsteps): the end is blended into the start so it repeats without
  a click. Long recordings are kept long (up to MAX_LOOP s); the game starts them at a random point, so they never
  sound the same twice. Footsteps are made mono (they are heard as your own steps).
- one-shots (neighs, snorts, landing): trimmed to the sound itself with 10 ms fades.
Every sound is levelled on its loud parts only (the steps, the neigh), not on the silence between, so a quiet
recording and a loud one end up equally loud. LOUDNESS sets how loud each kind is compared with the others.
    python Tools/prepare_sfx.py
Then import: UnrealEditor-Cmd.exe <uproject> -run=pythonscript -script="Tools/Unreal/import_audio.py sfx_"
"""
import os
import wave
import miniaudio
import numpy as np

SRC, OUT = "Art/Audio/Source", "Art/Audio/Sfx"
MAX_LOOP = 60.0      # seconds kept of a long recording (the rest only makes the game bigger)

# How loud each kind of sound is, as the level of its loud parts (0..1)
LOUDNESS = {
    "steps": 0.08, "land": 0.14,
    "horse_walk": 0.09, "horse_trot": 0.12, "horse_canter": 0.13, "horse_gallop": 0.15,
    "neigh": 0.16, "snort": 0.11, "deer": 0.11,
    "swing": 0.12, "armor": 0.17, "flesh": 0.15, "cut": 0.16, "killed": 0.15, "fall": 0.14,
}

# Loops: (game name, source file, start s, end s or None for the end, crossfade s, loudness kind, mono)
LOOPS = [
    ("sfx_horse_walk", "Horse walk.mp3", 0.5, 17.5, 1.0, "horse_walk", False),
    ("sfx_horse_trot", "horse trot 2.mp3", 0.02, 1.84, 0.12, "horse_trot", False),
    ("sfx_horse_canter", "Horse canter 1.mp3", 0.3, 7.3, 0.5, "horse_canter", False),
    # (the first 3 s of the gallop recording have the horse calling out: left out, or it neighs on every loop)
    ("sfx_horse_gallop", "Horse gallop 5-11.mp3", 3.0, 14.5, 0.8, "horse_gallop", False),
    # Footsteps by ground; the number is a variation of the same ground, picked at random
    ("sfx_steps_dirt_1", "dirt walk 1.mp3", 0.05, 1.95, 0.15, "steps", True),
    ("sfx_steps_dirt_2", "dirt walk 2.mp3", 0.05, 14.4, 0.4, "steps", True),
    ("sfx_steps_gravel_1", "gravel walk 1.mp3", 0.4, None, 0.5, "steps", True),
    ("sfx_steps_gravel_2", "gravel walk 2.mp3", 0.2, None, 0.5, "steps", True),
    ("sfx_steps_gravel_3", "gravel walk 3.mp3", 0.1, 6.3, 0.3, "steps", True),
    ("sfx_steps_rock_1", "rocky trail walk.mp3", 0.6, 47.7, 0.5, "steps", True),
    ("sfx_steps_leaves_1", "leaves walk 1.mp3", 0.6, 15.0, 0.4, "steps", True),
    ("sfx_steps_dry_leaves_1", "dry leaves walk 1.mp3", 0.2, None, 0.5, "steps", True),
    ("sfx_steps_mud_1", "wet steps on liquit and dirt 1.mp3", 0.2, None, 0.5, "steps", True),
    ("sfx_steps_sand_1", "sand walk 1.mp3", 0.1, 6.1, 0.4, "steps", True),
]
# One-shots: (game name, source file, start s, end s, loudness kind)
SHOTS = [
    ("sfx_horse_neigh_1", "Horse neigh 1.mp3", 0.05, 2.3, "neigh"),
    ("sfx_horse_neigh_2", "Horse neigh 2.mp3", 0.7, 7.4, "neigh"),
    ("sfx_horse_neigh_3", "horse neigh 3.mp3", 0.0, 1.3, "neigh"),
    ("sfx_horse_neigh_4", "horse neigh 4.mp3", 0.04, 1.5, "neigh"),
    ("sfx_horse_neigh_5", "Horse neigh 5.mp3", 0.0, 2.1, "neigh"),
    ("sfx_horse_snort_1", "Horse snort 1.mp3", 0.16, 0.85, "snort"),
    ("sfx_horse_snort_2", "Horse snort 1.mp3", 1.55, 2.65, "snort"),
    ("sfx_horse_snort_3", "Horse snort 1.mp3", 4.7, 5.75, "snort"),
    ("sfx_horse_snort_4", "Horse snort 1.mp3", 7.75, 9.9, "snort"),
    ("sfx_horse_snort_5", "Horse snort 2.mp3", 0.3, 0.95, "snort"),
    ("sfx_horse_snort_deep", "horse deep snort 1.mp3", 0.05, 1.6, "snort"),
    ("sfx_deer_snort", "deer snort 1.mp3", 0.3, 0.9, "deer"),
    ("sfx_land_dirt_1", "fall on dirt 1.mp3", 0.15, 2.0, "land"),
    # Fighting: swings, blows landing on armour or on flesh, a cut into flesh, a death cry, a body hitting the ground
    ("sfx_sword_slash_1", "sword slash 1.mp3", 0.0, 0.5, "swing"),
    ("sfx_sword_slash_2", "sword slash 2.mp3", 0.0, 0.6, "swing"),
    ("sfx_armor_hit_1", "armor hit 1.mp3", 0.0, 0.8, "armor"),
    ("sfx_armor_hit_2", "armor hit 2.mp3", 0.0, 0.7, "armor"),
    ("sfx_flesh_hit_1", "flesh hit 1.mp3", 0.0, 0.55, "flesh"),
    ("sfx_flesh_hit_2", "flesh hit 2.mp3", 0.04, 0.6, "flesh"),
    ("sfx_axe_slice_flesh_1", "axe slice flesh 1.mp3", 0.0, 0.6, "cut"),
    ("sfx_body_killed_1", "body killed 1.mp3", 0.08, 1.6, "killed"),
    ("sfx_body_killed_2", "body killed 2.mp3", 0.06, 1.4, "killed"),
    ("sfx_body_fall_1", "body fall 1.mp3", 0.12, 1.05, "fall"),
]


def read(name):
    d = miniaudio.decode_file(os.path.join(SRC, name), output_format=miniaudio.SampleFormat.FLOAT32)
    return np.frombuffer(d.samples, dtype=np.float32).reshape(-1, d.nchannels).copy(), d.sample_rate


def level(audio, rate, target):
    """Scales so the loud parts (the louder half of 50 ms windows above a fifth of the loudest) sit at target"""
    w = max(int(0.05 * rate), 1)
    windows = np.array([np.sqrt((audio[i:i + w] ** 2).mean()) for i in range(0, max(len(audio) - w, 1), w)])
    loud = windows[windows > 0.2 * windows.max()]
    gain = target / max(float(np.sqrt((loud ** 2).mean())), 1e-6)
    audio = audio * gain
    # Never louder than 95% at the very peak
    peak = float(np.abs(audio).max())
    return audio * (0.95 / peak) if peak > 0.95 else audio


def write(name, audio, rate):
    pcm = (np.clip(audio, -1, 1) * 32767).astype(np.int16)
    with wave.open(os.path.join(OUT, name + ".wav"), "wb") as w:
        w.setnchannels(audio.shape[1]); w.setsampwidth(2); w.setframerate(rate); w.writeframes(pcm.tobytes())


def fades(audio, rate, seconds=0.01):
    f = int(seconds * rate)
    ramp = np.linspace(0, 1, f, dtype=np.float32)[:, None]
    audio[:f] *= ramp
    audio[-f:] *= ramp[::-1]
    return audio


os.makedirs(OUT, exist_ok=True)
for name, source, start, end, fade, kind, mono in LOOPS:
    audio, rate = read(source)
    if mono:
        audio = audio.mean(axis=1, keepdims=True)
    end = len(audio) / rate if end is None else end
    end = min(end, start + MAX_LOOP)
    a, b, f = int(start * rate), int(end * rate), int(fade * rate)
    body = audio[a:b - f].copy()
    tail = audio[b - f:b]
    # Equal-power crossfade: the section's last moments fade out over its first moments fading in
    t = np.linspace(0.0, 1.0, f, dtype=np.float32)[:, None]
    body[:f] = body[:f] * np.sin(t * np.pi / 2) + tail * np.cos(t * np.pi / 2)
    write(name, level(body, rate, LOUDNESS[kind]), rate)
    print("loop %-24s %5.1fs" % (name, len(body) / rate))
for name, source, start, end, kind in SHOTS:
    audio, rate = read(source)
    audio = audio[int(start * rate):int(end * rate)]
    write(name, fades(level(audio, rate, LOUDNESS[kind]), rate), rate)
    print("shot %-24s %5.1fs" % (name, len(audio) / rate))
