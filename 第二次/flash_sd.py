"""Write only the official boot image region after checking the USB SD device."""
from pathlib import Path
import argparse
import datetime
import hashlib
import json
import os
import stat
import subprocess

ROOT = Path(__file__).resolve().parent
p = argparse.ArgumentParser()
p.add_argument('variant', choices=['asm_on', 'asm_off', 'c_500', 'c_3000'])
p.add_argument('device')
p.add_argument('--write', action='store_true', required=True)
a = p.parse_args()
dev = Path(a.device).resolve(strict=True)
assert stat.S_ISBLK(dev.stat().st_mode), 'Not a block device'
sys = Path('/sys/class/block') / dev.name
assert not (sys / 'partition').exists(), 'Select the whole SD disk'
assert 'usb' in str(sys.resolve()).lower(), 'Not a USB disk'
assert (sys / 'device/model').read_text().strip() == 'SD Card Reader', 'Unexpected device model'
assert int((sys / 'size').read_text()) * 512 == 31270109184, 'Unexpected SD capacity'
serial = subprocess.check_output(['lsblk', '-dn', '-o', 'SERIAL', str(dev)], text=True).strip()
assert serial == 'ABCDEF0123456789AB', 'Unexpected reader serial'
info = json.loads(subprocess.check_output(['lsblk', '-J', '-o', 'NAME,MOUNTPOINTS', str(dev)], text=True))
def check_unmounted(node):
    assert not any(node.get('mountpoints') or []), 'A disk partition is mounted'
    for child in node.get('children', []):
        check_unmounted(child)
for node in info['blockdevices']:
    check_unmounted(node)
payload = (ROOT / '工作工程' / a.variant / 'load.imx').read_bytes()
assert payload[0] == 0xd1 and len(payload) < 1024 * 1024
evidence = ROOT / '实测记录'
evidence.mkdir(exist_ok=True)
backup = evidence / 'sd_original_first4MiB.bin'
with dev.open('rb', buffering=0) as f:
    before = f.read(4 * 1024 * 1024)
assert len(before) == 4 * 1024 * 1024
if not backup.exists():
    with backup.open('xb') as f:
        f.write(before)
offset = 1024
with dev.open('r+b', buffering=0) as f:
    f.seek(offset)
    assert f.write(payload) == len(payload)
    os.fsync(f.fileno())
subprocess.run(['sync'], check=True)
with dev.open('rb', buffering=0) as f:
    after = f.read(len(before))
assert after[offset:offset + len(payload)] == payload, 'Readback differs'
assert before[:offset] == after[:offset], 'Disk partition table changed'
assert before[offset + len(payload):] == after[offset + len(payload):], 'Unexpected write outside image'
record = {'time': datetime.datetime.now(datetime.timezone.utc).isoformat(), 'device': str(dev), 'variant': a.variant, 'offset': offset, 'bytes_written': len(payload), 'sha256': hashlib.sha256(payload).hexdigest(), 'readback_verified': True, 'other_bytes_in_first4MiB_unchanged': True, 'hardware_effect': 'pending physical boot and observation'}
with (evidence / 'flash_log.jsonl').open('a') as f:
    f.write(json.dumps(record) + '\n')
print(json.dumps(record, indent=2))
