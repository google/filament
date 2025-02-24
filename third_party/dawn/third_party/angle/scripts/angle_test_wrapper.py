#! /usr/bin/env python3
import os
import sys

TEMPLATE = '''#!/bin/bash
cd "$(dirname "$0")"
python3 ../../{script} "$@"
'''

output_path, script = sys.argv[1:]

with open(output_path, 'w') as f:
    f.write(TEMPLATE.format(script=script))

os.chmod(output_path, 0o750)
