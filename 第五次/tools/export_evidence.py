"""Prepare exact evidence for transfer over the already connected serial port."""
import base64
import hashlib
import json
import os
import tarfile
import zlib

files = ['mic_left1.wav', 'mixer-before.state', 'mixer-lab5.state']
with tarfile.open('evidence.tar', 'w') as archive:
    for name in files:
        archive.add(name, arcname=name)
raw = open('evidence.tar', 'rb').read()
packed = base64.b64encode(zlib.compress(raw, 6))
open('evidence.b64', 'wb').write(packed)
print(json.dumps({'files': {p: {'bytes': os.path.getsize(p),
      'sha256': hashlib.sha256(open(p, 'rb').read()).hexdigest()} for p in files},
      'transfer_bytes': len(packed)}))
