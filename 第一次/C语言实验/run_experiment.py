from pathlib import Path
import subprocess

def run(label, args, **kwargs):
    p = subprocess.run(args, text=True, capture_output=True, **kwargs)
    block = f'$ {label}\n' + p.stdout + p.stderr + f'[exit={p.returncode}]\n'
    print(block)
    with open('experiment.log', 'a', encoding='utf-8') as f:
        f.write(block + '\n')
    if p.returncode != 0:
        raise SystemExit(p.returncode)
    return block

Path('experiment.log').write_text('', encoding='utf-8')
run('date -Iseconds', ['date', '-Iseconds'])
run('whoami', ['whoami'])
run('uname -m', ['uname', '-m'])
run('make -B', ['make', '-B'])
run('file scores', ['file', 'scores'])
Path('normal.in').write_text('80\n90\n60\n50\n100\n', encoding='ascii')
run('./scores < normal.in', ['./scores'], input=Path('normal.in').read_text())
run('make test', ['make', 'test'])
gdb = run('gdb -q -batch ... ./scores', ['gdb', '-q', '-batch',
    '-ex', 'break summarize', '-ex', 'run < normal.in',
    '-ex', 'print n', '-ex', 'print scores[0]', '-ex', 'continue', './scores'])
Path('gdb.log').write_text(gdb, encoding='utf-8')
if '$1 = 5' not in gdb or '$2 = 80' not in gdb or 'exited normally' not in gdb:
    raise SystemExit('Unexpected GDB result')
