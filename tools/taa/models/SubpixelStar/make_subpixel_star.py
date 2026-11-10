#!/usr/bin/env python3
"""Generate SubpixelStar.glb -- a synthetic scene for TAA sub-pixel flicker testing.

An unlit white backdrop with an unlit black Siemens star in front of it. A star is the
canonical target for this: wedge width falls off linearly toward the centre, so one static
frame contains a continuous sweep from comfortably-resolved features down to far below one
pixel, at every orientation. Measuring flicker in rings around the centre turns that into a
curve of flicker against feature width.

Unlit so contrast is exactly 1.0 and independent of the harness's IBL/sun. The star sits
slightly in front of the backdrop, so every wedge edge is a real depth discontinuity rather
than a texture. A band of straight bars along the bottom gives a uniform, easily-measured
control region at a known pitch.

The whole thing is rotated to face the taa_harness camera (azimuth 60 degrees).
"""
import json, struct, sys, math

SPOKES     = 256     # black wedges; wedge angular width is pi/SPOKES
STAR_R     = 1.4
PANEL_W    = 4.0
PANEL_H    = 3.4
BAR_BAND_H = 0.5     # straight-bar control strip along the bottom
BAR_COUNT  = 220
BAR_DUTY   = 0.5
FRONT_Z    = 0.01    # how far the star sits in front of the backdrop; see --front-z

def main(out):
    pos, nrm, idx = [], [], []
    def tri(a, b, c):
        base = len(pos)
        for p in (a, b, c):
            pos.append(p); nrm.append((0.0, 0.0, 1.0))
        idx.extend([base, base + 1, base + 2])
    def quad(c0, c1, c2, c3):
        tri(c0, c1, c2); tri(c0, c2, c3)

    hw, hh = PANEL_W * 0.5, PANEL_H * 0.5
    quad((-hw,-hh,0.0), (hw,-hh,0.0), (hw,hh,0.0), (-hw,hh,0.0))
    backdropCount = len(idx)

    starCy = -hh + BAR_BAND_H + STAR_R + 0.05
    for i in range(SPOKES):
        a0 = (i * 2.0 * math.pi) / SPOKES
        a1 = a0 + math.pi / SPOKES
        tri((0.0, starCy, FRONT_Z),
            (STAR_R * math.cos(a0), starCy + STAR_R * math.sin(a0), FRONT_Z),
            (STAR_R * math.cos(a1), starCy + STAR_R * math.sin(a1), FRONT_Z))

    period = PANEL_W / BAR_COUNT
    w = period * BAR_DUTY
    y0, y1 = -hh, -hh + BAR_BAND_H
    for i in range(BAR_COUNT):
        x = -hw + i * period
        quad((x, y0, FRONT_Z), (x + w, y0, FRONT_Z), (x + w, y1, FRONT_Z), (x, y1, FRONT_Z))

    posB = b"".join(struct.pack("<3f", *p) for p in pos)
    nrmB = b"".join(struct.pack("<3f", *n) for n in nrm)
    idxB = b"".join(struct.pack("<I", i) for i in idx)
    blob = posB + nrmB + idxB
    pmin = [min(p[k] for p in pos) for k in range(3)]
    pmax = [max(p[k] for p in pos) for k in range(3)]
    unlit = {"KHR_materials_unlit": {}}
    g = {
      "asset": {"version": "2.0", "generator": "make_star.py"},
      "extensionsUsed": ["KHR_materials_unlit"],
      "scene": 0, "scenes": [{"nodes": [0]}],
      "nodes": [{"mesh": 0, "rotation": [0.0, 0.5, 0.0, 0.8660254037844387]}],
      "materials": [
        {"name": "white", "pbrMetallicRoughness": {"baseColorFactor": [1,1,1,1],
            "metallicFactor": 0.0, "roughnessFactor": 1.0}, "extensions": unlit},
        {"name": "black", "pbrMetallicRoughness": {"baseColorFactor": [0,0,0,1],
            "metallicFactor": 0.0, "roughnessFactor": 1.0}, "extensions": unlit}],
      "meshes": [{"name": "star", "primitives": [
         {"attributes": {"POSITION": 0, "NORMAL": 1}, "indices": 2, "material": 0},
         {"attributes": {"POSITION": 0, "NORMAL": 1}, "indices": 3, "material": 1}]}],
      "accessors": [
        {"bufferView": 0, "componentType": 5126, "count": len(pos), "type": "VEC3",
         "min": pmin, "max": pmax},
        {"bufferView": 1, "componentType": 5126, "count": len(nrm), "type": "VEC3"},
        {"bufferView": 2, "componentType": 5125, "count": backdropCount, "type": "SCALAR",
         "byteOffset": 0},
        {"bufferView": 2, "componentType": 5125, "count": len(idx)-backdropCount,
         "type": "SCALAR", "byteOffset": backdropCount*4}],
      "bufferViews": [
        {"buffer": 0, "byteOffset": 0, "byteLength": len(posB), "target": 34962},
        {"buffer": 0, "byteOffset": len(posB), "byteLength": len(nrmB), "target": 34962},
        {"buffer": 0, "byteOffset": len(posB)+len(nrmB), "byteLength": len(idxB), "target": 34963}],
      "buffers": [{"byteLength": len(blob)}]}
    js = json.dumps(g, separators=(",", ":")).encode()
    while len(js) % 4: js += b" "
    total = 12 + 8 + len(js) + 8 + len(blob)
    with open(out, "wb") as f:
        f.write(struct.pack("<4sII", b"glTF", 2, total))
        f.write(struct.pack("<II", len(js), 0x4E4F534A)); f.write(js)
        f.write(struct.pack("<II", len(blob), 0x004E4942)); f.write(blob)
    print(f"{out}: {len(pos)} verts, {len(idx)//3} tris, {total} bytes, {SPOKES} spokes")

# The depth gap is the one parameter worth varying from the command line. At the 0.01 default
# a sub-texel feature carries almost no depth contrast, which isolates colour aliasing; at a
# real distance the same geometry carries a full depth jump, which is what exercises anything
# reading the depth buffer (depthDisocclusion). Both are checked in.
if "--front-z" in sys.argv:
    i = sys.argv.index("--front-z")
    globals()["FRONT_Z"] = float(sys.argv[i + 1])
    del sys.argv[i:i + 2]
main(sys.argv[1] if len(sys.argv) > 1 else "SubpixelStar.glb")
