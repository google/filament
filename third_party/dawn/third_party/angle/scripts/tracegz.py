import gzip
import json
import os
import sys

trace, trace_dir = sys.argv[1:]

with open(os.path.join(trace_dir, trace + '.json'), 'rb') as f:
    trace_json = json.loads(f.read())

# Concatenate trace cpp source into a single gz
with open('gen/tracegz_' + sys.argv[1] + '.gz', 'wb') as f:
    with gzip.GzipFile(fileobj=f, mode='wb', compresslevel=9, mtime=0) as fgz:
        for fn in trace_json['TraceFiles']:
            if fn.endswith('.cpp'):
                with open(os.path.join(trace_dir, fn), 'rb') as fcpp:
                    fgz.write(fcpp.read())
