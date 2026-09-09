// RUN: %dxc /Tps_6_2 /Emain  %s | FileCheck %s
// CHECK: define void @main()
// CHECK: %{{[a-z0-9]+}} = call i16 @dx.op.loadInput.i16(i32 4, i32 4, i32 0, i8 0, i32 undef)
// CHECK: %{{[a-z0-9]+}} = call i16 @dx.op.loadInput.i16(i32 4, i32 3, i32 0, i8 0, i32 undef)
// CHECK: %{{[a-z0-9]+}} = call i16 @dx.op.loadInput.i16(i32 4, i32 2, i32 0, i8 0, i32 undef)
// CHECK: %{{[a-z0-9]+}} = call i32 @dx.op.loadInput.i32(i32 4, i32 1, i32 0, i8 0, i32 undef)
// CHECK: %{{[a-z0-9]+}} = call i32 @dx.op.loadInput.i32(i32 4, i32 0, i32 0, i8 0, i32 undef)
// CHECK: entry

int4 foo(int v0, int v1, int v2, int v3) { return int4(v0, v0 * v1, v0 * v1 * v2, v0 * v1 * v2 * v3); }
uint4 foo(uint v0, uint v1, uint v2, uint v3) { return uint4(v0, v0 * v1, v0 * v1 * v2, v0 * v1 * v2 * v3); }
min16int4 foo(min16int v0, min16int v1, min16int v2, min16int v3) { return min16int4(v0, v0 * v1, v0 * v1 * v2, v0 * v1 * v2 * v3); }
min12int4 foo(min12int v0, min12int v1, min12int v2, min12int v3) { return min12int4(v0, v0 * v1, v0 * v1 * v2, v0 * v1 * v2 * v3); }
min16uint4 foo(min16uint v0, min16uint v1, min16uint v2, min16uint v3) { return min16uint4(v0, v0 * v1, v0 * v1 * v2, v0 * v1 * v2 * v3); }

float4 main(int vi
            : A, uint vui
            : B, min16int vm16i
            : C, min12int vm12i
            : D, min16uint vm16ui
            : E) : SV_Target {
  return foo(vi, vi, vi, vi) + foo(vui, vui, vui, vui) + foo(vm16i, vm16i, vm16i, vm16i) + foo(vm12i, vm12i, vm12i, vm12i) + foo(vm16ui, vm16ui, vm16ui, vm16ui);
}