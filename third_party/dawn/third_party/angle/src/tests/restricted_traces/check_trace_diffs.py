#! /usr/bin/env python3

import json
import os
from pathlib import Path

path1Traces = Path("C:/src/angle/src/tests/restricted_traces")
path2Traces = Path("C:/src/angle2/src/tests/restricted_traces")


def main():
    print("Trace Name, original GLES version, new GLES version, Required Extensions")
    for f in path2Traces.iterdir():
        if not f.is_dir():
            continue
        trace1 = path1Traces / f.name
        trace2 = path2Traces / f.name

        traceJSON1 = trace1 / f"{f.name}.json"
        traceJSON2 = trace2 / f"{f.name}.json"

        contents1 = readJSON(traceJSON1)
        contents2 = readJSON(traceJSON2)

        gles1 = (contents1["TraceMetadata"]["ContextClientMajorVersion"],
                 contents1["TraceMetadata"]["ContextClientMinorVersion"])
        gles2 = (contents2["TraceMetadata"]["ContextClientMajorVersion"],
                 contents2["TraceMetadata"]["ContextClientMinorVersion"])

        if "RequiredExtensions" in contents1:
            requiredExts = contents1["RequiredExtensions"]
            print(f"\"{f.name}\", \"{gles2}\", \"{gles1}\", \"{requiredExts}\"")
        else:
            print(f"\"{f.name}\", \"{gles2}\", \"{gles1}\", \"TRACE NOT RE-EVALUATED\"")


def readJSON(path):
    with open(path, 'r') as f:
        return json.load(f)


if __name__ == "__main__":
    main()
