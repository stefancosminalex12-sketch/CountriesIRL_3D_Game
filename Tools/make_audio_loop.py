"""
Cut a seamless loop out of a longer recording (for ambiences).
    python Tools/make_audio_loop.py <source.mp3|wav> <out.wav> <start s> <end s> [crossfade s] [target RMS]
The end of the chosen section is blended into its start, so the loop has no click or jump when it repeats.
Output is 16-bit WAV at the source's sample rate, levelled to the target RMS (default 0.06).
"""
import sys
import wave
import miniaudio
import numpy as np

source, out = sys.argv[1], sys.argv[2]
start, end = float(sys.argv[3]), float(sys.argv[4])
fade = float(sys.argv[5]) if len(sys.argv) > 5 else 4.0
target_rms = float(sys.argv[6]) if len(sys.argv) > 6 else 0.06

decoded = miniaudio.decode_file(source, output_format=miniaudio.SampleFormat.FLOAT32)
rate, channels = decoded.sample_rate, decoded.nchannels
audio = np.frombuffer(decoded.samples, dtype=np.float32).reshape(-1, channels).copy()

a, b, f = int(start * rate), int(end * rate), int(fade * rate)
body = audio[a:b - f].copy()
tail = audio[b - f:b]
# Equal-power crossfade: the section's last seconds fade out over its first seconds fading in
t = np.linspace(0.0, 1.0, f, dtype=np.float32)[:, None]
body[:f] = body[:f] * np.sin(t * np.pi / 2) + tail * np.cos(t * np.pi / 2)

body *= target_rms / max(float(np.sqrt((body ** 2).mean())), 1e-9)
body = np.clip(body, -1.0, 1.0)

with wave.open(out, "wb") as w:
    w.setnchannels(channels)
    w.setsampwidth(2)
    w.setframerate(rate)
    w.writeframes((body * 32767).astype("<i2").tobytes())
print("loop %s: %.1f s, %d Hz, %d ch, peak %.2f" % (out, len(body) / rate, rate, channels, float(np.abs(body).max())))
