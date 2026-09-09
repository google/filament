#define ENABLE_PRINT 0

#define TERMINATE -40
#define IGNORE 0
#define ACCEPT 40
#define OPAQUE 41

#define LEAF_DONE 90
#define LEAF_INST 91
#define LEAF_TRIS 92
#define LEAF_CUSTOM 93

#define RAYGEN 190
#define INTERSECT 290
#define ANYHIT 390
#define CLOSESTHIT 490
#define MISS 590
#define TRACERAY 999

#ifndef HLSL
typedef unsigned uint;

#define HIT_KIND_TRIANGLE_FRONT_FACE 0xFE
#define HIT_KIND_TRIANGLE_BACK_FACE 0xFF

#define RAY_FLAG_NONE 0x00
#define RAY_FLAG_FORCE_OPAQUE 0x01
#define RAY_FLAG_FORCE_NON_OPAQUE 0x02
#define RAY_FLAG_TERMINATE_ON_FIRST_HIT 0x04
#define RAY_FLAG_SKIP_CLOSEST_HIT_SHADER 0x08
#define RAY_FLAG_CULL_BACK_FACING_TRIANGLES 0x10
#define RAY_FLAG_CULL_FRONT_FACING_TRIANGLES 0x20
#define RAY_FLAG_CULL_OPAQUE 0x40
#define RAY_FLAG_CULL_NON_OPAQUE 0x80

#define INSTANCE_FLAG_NONE 0x0
#define INSTANCE_FLAG_TRIANGLE_CULL_DISABLE 0x1
#define INSTANCE_FLAG_TRIANGLE_FRONT_COUNTERCLOCKWISE 0x2
#define INSTANCE_FLAG_FORCE_OPAQUE 0x4
#define INSTANCE_FLAG_FORCE_NON_OPAQUE 0x8

#endif

static int pack(int primIdx, int geomIdx, bool opaque) {
  return (opaque ? 0x80000000 : 0) | (geomIdx << 16) | (primIdx);
}

#ifdef HLSL
static void unpack(int val, out uint primIdx, out uint geomIdx,
                   out bool opaque) {
  opaque = val & 0x80000000;
  geomIdx = (val >> 16) & 0xFFFF;
  primIdx = val & 0xFFFF;
}
#endif

static bool isOpaque(bool geomOpaque, uint instanceFlags, uint rayFlags) {
  bool opaque = geomOpaque;

  if (instanceFlags & INSTANCE_FLAG_FORCE_OPAQUE)
    opaque = true;
  else if (instanceFlags & INSTANCE_FLAG_FORCE_NON_OPAQUE)
    opaque = false;

  if (rayFlags & RAY_FLAG_FORCE_OPAQUE)
    opaque = true;
  else if (rayFlags & RAY_FLAG_FORCE_NON_OPAQUE)
    opaque = false;

  return opaque;
}

static float computeCullFaceDir(uint instanceFlags, uint rayFlags) {
  float cullFaceDir = 0;
  if (rayFlags & RAY_FLAG_CULL_FRONT_FACING_TRIANGLES)
    cullFaceDir = 1;
  else if (rayFlags & RAY_FLAG_CULL_BACK_FACING_TRIANGLES)
    cullFaceDir = -1;
  if (instanceFlags & INSTANCE_FLAG_TRIANGLE_CULL_DISABLE)
    cullFaceDir = 0;

  return cullFaceDir;
}

static bool cull(bool opaque, uint rayFlags) {
  return (opaque && (rayFlags & RAY_FLAG_CULL_OPAQUE)) ||
         (!opaque && (rayFlags & RAY_FLAG_CULL_NON_OPAQUE));
}
