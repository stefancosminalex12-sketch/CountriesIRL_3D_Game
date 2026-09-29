"""
Prepare a music track for the game: cut it from a start time (and optionally to an end time) and save it as WAV.
    python Tools/make_music_track.py <source.mp3|wav|flac> <out.wav> <start s> [end s]
The music itself is left as the composer mixed it (no levelling); only 10 ms fades at the cut points, so the track
doesn't click when it starts or loops. Output: 16-bit WAV at the source's sample rate and channels.
"""
import sys
import wave

import miniaudio
import numpy as np

source, out = sys.argv[1], sys.argv[2]
start = float(sys.argv[3])
end = float(sys.argv[4]) if len(sys.argv) > 4 else None

decoded = miniaudio.decode_file(source, output_format=miniaudio.SampleFormat.FLOAT32)
rate, channels = decoded.sample_rate, decoded.nchannels
audio = np.frombuffer(decoded.samples, dtype=np.float32).reshape(-1, channels).copy()

a = int(start * rate)
b = int(end * rate) if end else len(audio)
track = audio[a:b].copy()
edge = int(0.01 * rate)
ramp = np.linspace(0.0, 1.0, edge, dtype=np.float32)[:, None]
track[:edge] *= ramp
track[-edge:] *= ramp[::-1]
track = np.clip(track, -1.0, 1.0)

with wave.open(out, "wb") as w:
    w.setnchannels(channels)
    w.setsampwidth(2)
    w.setframerate(rate)
    w.writeframes((track * 32767).astype("<i2").tobytes())
tail = track[-rate:] if len(track) > rate else track
print("track %s: %.1f s (from %.1f s of %.1f s), %d Hz, %d ch, peak %.2f, last second rms %.4f"
      % (out, len(track) / rate, start, len(audio) / rate, rate, channels, float(np.abs(track).max()),
         float(np.sqrt((tail ** 2).mean()))))
