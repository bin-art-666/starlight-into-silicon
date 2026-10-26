"""Verify the serial archive against board hashes and inspect recorded PCM."""
import array
import hashlib
import io
import json
import math
from pathlib import Path
import sys
import tarfile
import wave
import zlib

root = Path(__file__).resolve().parents[1]
expected = {
    'mic_left1.wav': 'e45fc18a5320d7a9595e949d639b3e7d3f0348af306380ae2f5ffbab68a243af',
    'mixer-before.state': '27e59c7068f7dd0a7f4b2f3f4ba4f68fcf184010469c4026f9d7ea9ef19d7bb6',
    'mixer-lab5.state': 'f5c4c8f29744546dddf9d6047ace384837a7ed509bfbef9d8fa6dcbf3d26bfe6',
}
hashes = {}
with tarfile.open(fileobj=io.BytesIO(zlib.decompress(
        (root / '实测记录' / 'evidence.zlib').read_bytes())), mode='r:') as archive:
    for name, digest in expected.items():
        member = archive.getmember(name)
        assert member.isfile()
        data = archive.extractfile(member).read()
        actual = hashlib.sha256(data).hexdigest()
        if actual != digest:
            raise ValueError('SHA256 mismatch: ' + name)
        destination = root / ('音频' if name.endswith('.wav') else '实测记录') / name
        destination.write_bytes(data)
        hashes[name] = {'sha256': actual, 'bytes': len(data), 'match': True}

with wave.open(str(root / '音频' / 'mic_left1.wav'), 'rb') as recording:
    channels, width, rate, frames = (recording.getnchannels(), recording.getsampwidth(),
                                   recording.getframerate(), recording.getnframes())
    assert width == 2
    samples = array.array('h', recording.readframes(frames))
    if sys.byteorder != 'little':
        samples.byteswap()
    stats = []
    for channel in range(channels):
        values = samples[channel::channels]
        rms = math.sqrt(sum(v * v for v in values) / len(values))
        stats.append({'channel': channel + 1, 'peak': max(abs(v) for v in values),
                      'rms': round(rms, 3),
                      'rms_dbfs': round(20 * math.log10(rms / 32768), 3) if rms else None,
                      'nonzero_samples': sum(v != 0 for v in values),
                      'clipped_samples': sum(abs(v) >= 32767 for v in values)})
    result = {'hashes': hashes, 'recording': {'channels': channels, 'sample_width': width,
              'rate': rate, 'frames': frames, 'duration_seconds': frames / rate,
              'channels_identical': samples[0::channels] == samples[1::channels],
              'channel_stats': stats}}
output = json.dumps(result, ensure_ascii=False, indent=2)
(root / '实测记录' / '音频校验.json').write_text(output, encoding='utf-8')
print(output)
