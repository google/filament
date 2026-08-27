#!/usr/bin/env python3
"""
gen_golden.py - generate golden reference values for the tocio validation harness.

Loads each real ACES *.ocio config with PyOpenColorIO (the C++ reference engine)
and applies a fixed set of input RGB samples through:

  * colorspace conversions   <colorspace>  <->  scene_linear role
  * display/view transforms  scene_linear  ->   (display, view)

The reference outputs are written as a TSV that tocio's C harness
(tests/toc_validate.c) replays: it builds the same transform with tocio, applies
the same inputs, and compares against these golden values within tolerance.

This script is the ONLY part of the harness that needs PyOpenColorIO; the golden
TSV it produces is committed, so `make tocio-validate` runs oracle-free.

Usage:
    # PyOpenColorIO must be importable. The fetch script installs it isolated under
    # ref/.pyoracle; activate it like:
    SP=$(echo ref/.pyoracle/lib/python*/site-packages)
    PYTHONPATH="$SP" python3 scripts/gen_golden.py

    # options:
    python3 scripts/gen_golden.py --configs a.ocio b.ocio --out tests/golden/aces_golden.tsv
"""
import argparse
import glob
import math
import os
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
TOCIO = os.path.dirname(HERE)  # sandbox/tocio

# Representative input RGB samples: mid-grays, neutral ramp, an HDR highlight,
# a near-primary, and a slightly out-of-gamut (negative) value.
SAMPLES = [
    (0.18, 0.18, 0.18),   # ACES mid gray
    (0.0, 0.0, 0.0),      # black
    (1.0, 1.0, 1.0),      # white
    (0.5, 0.4, 0.3),      # arbitrary neutral-ish
    (0.04, 0.04, 0.04),   # dark
    (4.0, 2.0, 1.0),      # HDR highlight (>1)
    (0.35, 0.08, 0.06),   # saturated red-ish
    (-0.05, 0.12, 0.22),  # slight out-of-gamut negative
]

DEFAULT_CONFIGS = [
    "cg-config-v4.0.0_aces-v2.0_ocio-v2.5.ocio",
    "reference-config-v4.0.0_aces-v2.0_ocio-v2.5.ocio",
    "studio-config-v4.0.0_aces-v2.0_ocio-v2.5.ocio",
]


def finite(v):
    return all(math.isfinite(x) for x in v)


def main():
    ap = argparse.ArgumentParser(description=__doc__,
                                 formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--configs", nargs="*", default=None,
                    help="config files (paths or basenames under ref/configs)")
    ap.add_argument("--out", default=os.path.join(TOCIO, "tests", "golden", "aces_golden.tsv"))
    ap.add_argument("--configs-dir", default=os.path.join(TOCIO, "ref", "configs"))
    args = ap.parse_args()

    try:
        import PyOpenColorIO as ocio
        import numpy as np
    except ImportError as e:
        sys.exit("error: %s\n  PyOpenColorIO + numpy required. See the header of this "
                 "file for the isolated install under ref/.pyoracle." % e)

    # Resolve config list to absolute paths.
    names = args.configs
    if not names:
        names = DEFAULT_CONFIGS
    paths = []
    for n in names:
        if os.path.isfile(n):
            paths.append(n)
        elif os.path.isfile(os.path.join(args.configs_dir, n)):
            paths.append(os.path.join(args.configs_dir, n))
        else:
            hit = glob.glob(os.path.join(args.configs_dir, n))
            if hit:
                paths.extend(hit)
            else:
                print("warn: config not found, skipping: %s" % n, file=sys.stderr)
    if not paths:
        sys.exit("error: no configs found under %s (run scripts/fetch_ocio_ref.sh)"
                 % args.configs_dir)

    samp = np.array(SAMPLES, dtype=np.float32)

    def apply_proc(proc):
        cpu = proc.getDefaultCPUProcessor()
        out = samp.copy()
        cpu.applyRGB(out)
        return out

    os.makedirs(os.path.dirname(args.out), exist_ok=True)
    rows = []
    stats = {"cs": 0, "dv": 0, "skip_nonfinite": 0, "skip_err": 0}

    # Column order is consumed positionally by tests/toc_validate.c.
    header = ["config", "kind", "src", "dst", "display", "view",
              "in_r", "in_g", "in_b", "out_r", "out_g", "out_b"]

    def emit(config, kind, src, dst, display, view, inrgb, outrgb):
        rows.append("\t".join([
            config, kind, src, dst, display, view,
            "%.9g" % inrgb[0], "%.9g" % inrgb[1], "%.9g" % inrgb[2],
            "%.9g" % outrgb[0], "%.9g" % outrgb[1], "%.9g" % outrgb[2],
        ]))

    for path in paths:
        cfgname = os.path.basename(path)
        cfg = ocio.Config.CreateFromFile(path)
        sl = cfg.getColorSpace(ocio.ROLE_SCENE_LINEAR)
        if sl is None:
            print("warn: %s has no scene_linear role, skipping" % cfgname, file=sys.stderr)
            continue
        sl = sl.getName()
        print("# %s  (scene_linear=%s)" % (cfgname, sl), file=sys.stderr)

        # --- colorspace <-> scene_linear (both directions) ---
        for cs in cfg.getColorSpaces():
            name = cs.getName()
            if name == sl:
                continue
            if cs.isData():
                continue
            for src, dst in ((name, sl), (sl, name)):
                try:
                    out = apply_proc(cfg.getProcessor(src, dst))
                except Exception:
                    stats["skip_err"] += 1
                    continue
                for i in range(len(SAMPLES)):
                    o = out[i]
                    if not finite(o):
                        stats["skip_nonfinite"] += 1
                        continue
                    emit(cfgname, "cs", src, dst, "", "", SAMPLES[i], o)
                    stats["cs"] += 1

        # --- scene_linear -> (display, view) ---
        for display in cfg.getDisplays():
            for view in cfg.getViews(display):
                try:
                    dt = ocio.DisplayViewTransform(src=sl, display=display, view=view)
                    out = apply_proc(cfg.getProcessor(dt))
                except Exception:
                    stats["skip_err"] += 1
                    continue
                for i in range(len(SAMPLES)):
                    o = out[i]
                    if not finite(o):
                        stats["skip_nonfinite"] += 1
                        continue
                    emit(cfgname, "dv", sl, "", display, view, SAMPLES[i], o)
                    stats["dv"] += 1

    with open(args.out, "w") as f:
        f.write("# tocio golden reference values - generated by scripts/gen_golden.py\n")
        f.write("# oracle: PyOpenColorIO %s\n" % ocio.__version__)
        f.write("# DO NOT EDIT BY HAND. Regenerate after changing configs/samples.\n")
        f.write("\t".join(header) + "\n")
        f.write("\n".join(rows) + "\n")

    print("wrote %d rows -> %s" % (len(rows), args.out), file=sys.stderr)
    print("  colorspace rows: %d  display/view rows: %d" % (stats["cs"], stats["dv"]),
          file=sys.stderr)
    print("  skipped: %d non-finite, %d build errors"
          % (stats["skip_nonfinite"], stats["skip_err"]), file=sys.stderr)


if __name__ == "__main__":
    main()
