// RUN: %dxc -Emain -Tvs_6_0 %s | %opt -S -hlsl-dxil-eliminate-output-dynamic | %FileCheck %s

// CHECK-NOT: storeOutput.f32(i32 5, i32 0, i32 %i
// CHECK-NOT: storeOutput.f32(i32 5, i32 1, i32 %i
// CHECK-NOT: storeOutput.f32(i32 5, i32 2, i32 %i
// CHECK: storeOutput.f32(i32 5, i32 2, i32 0, i8 0
// CHECK: storeOutput.f32(i32 5, i32 2, i32 1, i8 0
// CHECK: storeOutput.f32(i32 5, i32 3, i32 0, i8 0
// CHECK: storeOutput.f32(i32 5, i32 3, i32 1, i8 0
// CHECK: storeOutput.f32(i32 5, i32 1, i32 0, i8 0
// CHECK: storeOutput.f32(i32 5, i32 1, i32 1, i8 0

int  count;
float4 c[16];

struct T {
   float a;
};

struct T2 : T {
  struct T t2;
  float b;
};

float4 main(out T2 o[2] : I, float4 pos: POS) : SV_POSITION {

    for (uint i=0;i<count;i++) {
        o[i].a = c[i].x;
        o[i].b = c[i].y;
        o[i].t2.a = c[i].z;
    }
    return pos;
}

