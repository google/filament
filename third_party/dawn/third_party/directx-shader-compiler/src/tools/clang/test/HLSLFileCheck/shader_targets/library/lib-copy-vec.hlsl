// RUN: %dxc -T lib_6_3 -D INIT=array[0],array[1] -D ACCUM=array[0]+array[1] %s | FileCheck %s -check-prefixes=CHECK,VEC
// RUN: %dxc -T lib_6_3 -D INIT=s_array[0],s_array[1] -D ACCUM=s_array[0]+s_array[1] %s | FileCheck %s -check-prefixes=CHECK,VEC

// RUN: %dxc -T lib_6_3 -D INIT=f0,f1 -D ACCUM=f0+f1 %s | FileCheck %s -check-prefixes=CHECK,VEC
// RUN: %dxc -T lib_6_3 -D INIT=s_f0,s_f1 -D ACCUM=s_f0+s_f1 %s | FileCheck %s -check-prefixes=CHECK,VEC

// RUN: %dxc -T lib_6_3 -D INIT=agg -D ACCUM=agg.f3 %s | FileCheck %s -check-prefixes=CHECK,AGG
// RUN: %dxc -T lib_6_3 -D INIT=s_agg -D ACCUM=s_agg.f3 %s | FileCheck %s -check-prefixes=CHECK,AGG

// RUN: %dxc -T lib_6_3 -D INIT=agg -D ACCUM=agg.array[0]+agg.array[1] -D ARRAY %s | FileCheck %s -check-prefixes=CHECK,AGG
// RUN: %dxc -T lib_6_3 -D INIT=s_agg -D ACCUM=s_agg.array[0]+s_agg.array[1] -D ARRAY %s | FileCheck %s -check-prefixes=CHECK,AGG

// CHECK: define <3 x float> @"\01?main{{[@$?.A-Za-z0-9_]+}}"()
// VEC: alloca <3 x float>
// VEC: alloca <3 x float>
// AGG: alloca %struct.Agg
// CHECK-NOT: alloca

struct Agg {
#ifdef ARRAY
  float3 array[2];
#else
  float3 f3;
#endif
};

void get(out float3 f0, out float3 f1);
void get(out Agg agg);

static float3 s_array[2];
static float3 s_f0, s_f1;
static Agg s_agg;

export
float3 main() {
  float3 result = 0;

  float3 array[2];
  float3 f0, f1;
  Agg agg;

  get(INIT);
  return ACCUM;
}
