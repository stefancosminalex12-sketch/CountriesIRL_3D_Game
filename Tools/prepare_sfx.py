"""
Turn the user's sound recordings (Art/Audio/Source) into the game's sound effects (Art/Audio/Sfx/*.wav):
loops for continuous sounds (a horse's gaits, footsteps) with the end blended into the start, and one-shots
(neighs, snorts) trimmed to the sound itself with short fades. Levels are evened out so nothing jumps out.
    python Tools/prepare_sfx.py
Then import: UnrealEditor-Cmd.exe <uproject> -run=pythonscript -script="Tools/Unreal/import_audio.py sfx_"
"""
import os
import subprocess
import sys
import wave
import miniaudio
import numpy as np

SRC, OUT = "Art/Audio/Source", "Art/Audio/Sfx"

# Loops: (game name, source file, start s, end s, crossfade s, loudness)
LOOPS = [
    ("sfx_horse_walk", "Horse walk.mp3", 0.5, 17.5, 1.0, 0.05),
    ("sfx_horse_trot", "horse trot 2.mp3", 0.02, 1.84, 0.12, 0.06),
    ("sfx_horse_canter", "Horse canter 1.mp3", 0.3, 7.3, 0.5, 0.07),
    ("sfx_horse_gallop", "Horse gallop 5-11.mp3", 0.2, 14.5, 0.8, 0.09),
    ("sfx_steps_sand", "sand walk 1.mp3", 0.1, 6.1, 0.4, 0.04),
]
# One-shots: (game name, source file, start s, end s)
SHOTS = [
    ("sfx_horse_neigh_1", "Horse neigh 1.mp3", 0.05, 2.3),
    ("sfx_horse_neigh_2", "Horse neigh 2.mp3", 0.7, 7.4),
    ("sfx_horse_neigh_3", "horse neigh 3.mp3", 0.0, 1.3),
    ("sfx_horse_neigh_4", "horse neigh 4.mp3", 0.04, 1.5),
    ("sfx_horse_neigh_5", "Horse neigh 5.mp3", 0.0, 2.1),
    ("sfx_horse_snort_1", "Horse snort 1.mp3", 0.16, 0.85),
    ("sfx_horse_snort_2", "Horse snort 1.mp3", 1.55, 2.65),
    ("sfx_horse_snort_3", "Horse snort 1.mp3", 4.7, 5.75),
    ("sfx_horse_snort_4", "Horse snort 1.mp3", 7.75, 9.9),
    ("sfx_horse_snort_5", "Horse snort 2.mp3", 0.3, 0.95),
    ("sfx_horse_snort_deep", "horse deep snort 1.mp3", 0.05, 1.6),
    ("sfx_deer_snort", "deer snort 1.mp3", 0.3, 0.9),
]


def read(name):
    d = miniaudio.decode_file(os.path.join(SRC, name), output_format=miniaudio.SampleFormat.FLOAT32)
    return np.frombuffer(d.samples, dtype=np.float32).reshape(-1, d.nchannels).copy(), d.sample_rate


def write(name, audio, rate):
    pcm = (np.clip(audio, -1, 1) * 32767).astype(np.int16)
    with wave.open(os.path.join(OUT, name + ".wav"), "wb") as w:
        w.setnchannels(audio.shape[1]); w.setsampwidth(2); w.setframerate(rate); w.writeframes(pcm.tobytes())


os.makedirs(OUT, exist_ok=True)
for name, source, start, end, fade, rms in LOOPS:
    subprocess.run([sys.executable, "Tools/make_audio_loop.py", os.path.join(SRC, source), os.path.join(OUT, name + ".wav"),
                    str(start), str(end), str(fade), str(rms)], check=True, capture_output=True)
    print("loop ", name)
for name, source, start, end in SHOTS:
    audio, rate = read(source)
    audio = audio[int(start * rate):int(end * rate)]
    # Peaks at 90%, 10 ms fades so nothing clicks
    audio *= 0.9 / max(float(np.abs(audio).max()), 1e-6)
    f = int(0.01 * rate)
    ramp = np.linspace(0, 1, f, dtype=np.float32)[:, None]
    audio[:f] *= ramp
    audio[-f:] *= ramp[::-1]
    write(name, audio, rate)
    print("shot ", name, "%.2fs" % (len(audio) / rate))
