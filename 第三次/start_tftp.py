from pathlib import Path
import logging
import sys

base = Path(__file__).resolve().parent
sys.path.insert(0, str(base / 'tools' / 'python_packages'))
import tftpy

logging.basicConfig(filename=base / '实测记录' / 'Windows_TFTP.log', level=logging.INFO,
                    format='%(asctime)s %(levelname)s %(message)s')
tftpy.TftpServer(str(base / 'tftp')).listen('192.168.1.100', 69)
