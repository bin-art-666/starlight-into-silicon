import json
import subprocess
from pathlib import Path

cases = [
    ('普通成绩', '80\n90\n60\n50\n100\n', 0,
     'Average: 76.00\nMaximum: 100.00\nPassed: 4/5\n', ''),
    ('边界成绩', '0\n59\n60\n100\n100\n', 0,
     'Average: 63.80\nMaximum: 100.00\nPassed: 3/5\n', ''),
    ('小数成绩', '59.5\n60.5\n80\n90\n100\n', 0,
     'Average: 78.00\nMaximum: 100.00\nPassed: 4/5\n', ''),
    ('负数', '-1\n', 1, '', 'Invalid score at position 1\n'),
    ('超过满分', '101\n', 1, '', 'Invalid score at position 1\n'),
    ('非数字', 'abc\n', 1, '', 'Invalid score at position 1\n'),
    ('非有限数', 'nan\n', 1, '', 'Invalid score at position 1\n'),
    ('输入提前结束', '80\n90\n', 1, '', 'Invalid score at position 3\n'),
    ('尾部非法字符', '80x\n', 1, '', 'Invalid score at position 1\n'),
]
results = []
for name, data, status, expected, error in cases:
    run = subprocess.run(['./scores'], input=data, text=True, capture_output=True)
    ok = (run.returncode == status and
          run.stdout == 'Enter 5 scores, one per line (0-100):\n' + expected and
          run.stderr == error)
    results.append(dict(name=name, input=data, returncode=run.returncode,
                        stdout=run.stdout, stderr=run.stderr, passed=ok))
    print(('PASS' if ok else 'FAIL') + ' ' + name)
Path('test_results.json').write_text(json.dumps(results, ensure_ascii=False, indent=2), encoding='utf-8')
print(f'{sum(r["passed"] for r in results)}/{len(results)} passed')
raise SystemExit(0 if all(r['passed'] for r in results) else 1)
