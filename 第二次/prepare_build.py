from pathlib import Path
import hashlib
import json
import shutil
import subprocess

ROOT = Path(__file__).resolve().parent
OFFICIAL = ROOT / '官方裸机例程'
OUT = ROOT / '工作工程'
OUT.mkdir(exist_ok=True)
records = []

def run(args, cwd):
    result = subprocess.run(args, cwd=cwd, text=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
    print(result.stdout, end='', flush=True)
    with (cwd / 'build.log').open('a', encoding='utf-8') as f:
        f.write('$ ' + ' '.join(map(str, args)) + '\n' + result.stdout)
    result.check_returncode()

for name, source, delay in [('asm_on', '1_leds', None), ('asm_off', '1_leds', None), ('c_500', '2_ledc', 500), ('c_3000', '2_ledc', 3000)]:
    dest = OUT / name
    dest.mkdir(exist_ok=True)
    files = ['led.s'] if delay is None else ['start.S', 'main.c', 'main.h', 'imx6ul.lds']
    for filename in files + ['Makefile', 'imxdownload']:
        shutil.copy2(OFFICIAL / source / filename, dest / filename)
    if name == 'asm_off':
        p = dest / 'led.s'
        text = p.read_text()
        assert text.count('ldr r1, =0\t') == 1
        p.write_text(text.replace('ldr r1, =0\t', 'ldr r1, =8\t'))
    if delay == 3000:
        p = dest / 'main.c'
        text = p.read_text()
        assert text.count('delay(500)') == 2
        p.write_text(text.replace('delay(500)', 'delay(3000)').replace('500ms', '3000ms'))
    # Ubuntu GCC defaults differ from the older course compiler: explicitly use ARM
    # instructions and static, freestanding code for the ROM's ARM entry point.
    p = dest / 'Makefile'
    text = p.read_text().replace('arm-linux-gnueabihf-gcc ', 'arm-linux-gnueabihf-gcc -mcpu=cortex-a7 -marm -fno-pie -ffreestanding ')
    if delay is None:
        text = text.replace('-Ttext 0X87800000', '-e _start -Ttext 0X87800000')
    p.write_text(text)
    (dest / 'build.log').write_text('')
    run(['make', 'clean'], dest)
    run(['make'], dest)
    elf = 'led.elf' if delay is None else 'ledc.elf'
    binary = 'led.bin' if delay is None else 'ledc.bin'
    run(['arm-linux-gnueabihf-readelf', '-h', elf], dest)
    # Build a regular-file SD image first. No physical disk is opened here.
    (dest / 'imxdownload').chmod(0o755)
    run(['./imxdownload', binary, 'sd-image.img', '-512m'], dest)
    load = (dest / 'load.imx').read_bytes()
    image = (dest / 'sd-image.img').read_bytes()
    assert image[1024:] == load, 'SD image layout mismatch'
    assert load[:1] == b'\xd1', 'Missing i.MX IVT header'
    records.append({'variant': name, 'delay': delay, 'binary_bytes': (dest / binary).stat().st_size, 'image_bytes': len(image), 'sha256': hashlib.sha256(image).hexdigest()})

(ROOT / 'build_results.json').write_text(json.dumps(records, ensure_ascii=False, indent=2))
print(json.dumps(records, ensure_ascii=False, indent=2))
