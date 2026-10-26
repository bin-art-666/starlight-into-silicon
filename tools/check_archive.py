"""Check publication boundaries and integrity of archived source material."""
from pathlib import Path
import hashlib
import json
import re
import sys

root = Path(__file__).resolve().parents[1]
forbidden = re.compile(r'排版|预览|底稿|render', re.I)
blocked = {'.doc', '.docx', '.pdf', '.zip', '.tar', '.xz', '.bz2', '.zlib', '.pem', '.key'}
errors = []
count = 0
for f in root.rglob('*'):
    if not f.is_file() or '.git' in f.relative_to(root).parts:
        continue
    rel = f.relative_to(root).as_posix()
    count += 1
    if forbidden.search(rel) or f.suffix.lower() in blocked:
        errors.append('Forbidden file: ' + rel)
    data = f.read_bytes()
    if data.startswith(b'PK\x03\x04'):
        errors.append('Unreviewed ZIP container: ' + rel)
    if data.startswith(b'%PDF-'):
        errors.append('PDF content: ' + rel)
for item in json.loads((root / '资料清单.json').read_text(encoding='utf-8')):
    f = root / item['path']
    if not f.is_file():
        errors.append('Missing: ' + item['path'])
    elif hashlib.sha256(f.read_bytes()).hexdigest() != item['sha256']:
        errors.append('Hash mismatch: ' + item['path'])
for n in ['第一次','第二次','第三次','第四次','第五次','第六次','第七次']:
    if not (root / n / 'README.md').is_file():
        errors.append('Missing summary: ' + n)
print(json.dumps({'files':count,'errors':errors}, ensure_ascii=False, indent=2))
sys.exit(bool(errors))
