"""Generate an original PCM melody; compatible with board Python 3.5."""
import math
import struct
import wave

rate = 44100
notes = [60, 64, 67, 72, 71, 67, 65, 62, 60, 64, 67, 69, 65, 62, 60, 60]
with wave.open('lab5_music.wav', 'wb') as out:
    out.setparams((2, 2, rate, 0, 'NONE', 'not compressed'))
    for note in notes:
        frequency = 440.0 * 2 ** ((note - 69) / 12.0)
        samples = bytearray()
        length = int(rate * 0.4)
        for i in range(length):
            t = i / float(rate)
            envelope = min(1.0, i / 441.0, (length - 1 - i) / 2205.0)
            value = int(7000 * envelope * (math.sin(2 * math.pi * frequency * t)
                        + 0.2 * math.sin(4 * math.pi * frequency * t)))
            samples.extend(struct.pack('<hh', value, value))
        out.writeframes(samples)
print('MUSIC_CREATED: 6.4 seconds, 44100 Hz, S16_LE, stereo')
