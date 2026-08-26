// RUN: %dxc -E main -T lib_6_3 %s | FileCheck %s

// Make sure all pointer operand of foo is from alloca.
// CHECK:%[[A0:.*]] = alloca i32
// CHECK:%[[A1:.*]] = alloca i32
// CHECK:%[[A2:.*]] = alloca i32
// CHECK:%[[A3:.*]] = alloca i32
// CHECK:%[[A4:.*]] = alloca i32
// CHECK:%[[A5:.*]] = alloca i32
// CHECK:%[[A6:.*]] = alloca i32

// CHECK:call void @"\01?foo{{[@$?.A-Za-z0-9_]+}}"(i32 0, i32* nonnull dereferenceable(4) %[[A0]], i32* nonnull dereferenceable(4) %[[A5]], i32* nonnull dereferenceable(4) %[[A6]])
// CHECK:call void @"\01?foo{{[@$?.A-Za-z0-9_]+}}"(i32 0, i32* nonnull dereferenceable(4) %[[A4]], i32* nonnull dereferenceable(4) %[[A3]], i32* nonnull dereferenceable(4) %[[A6]])
// CHECK:call void @"\01?foo{{[@$?.A-Za-z0-9_]+}}"(i32 0, i32* nonnull dereferenceable(4) %[[A2]], i32* nonnull dereferenceable(4) %[[A1]], i32* nonnull dereferenceable(4) %[[A6]])

struct DimStruct {
  uint2 Dims;
};
RWStructuredBuffer<DimStruct> SB;
groupshared uint2 gs_Dims;

void foo(uint i, out uint, out uint, out uint);

[shader("compute")]
[numthreads(1,1,1)]
void main() {
  uint iMips = (uint)(0);
  uint2 Dims = (uint2)(0);
  (foo((uint(0u)), (Dims)[0], (Dims)[1], (iMips)));
  SB[0].Dims = Dims;
  (foo((uint(0u)), (SB[1].Dims)[0], (SB[1].Dims)[1], (iMips)));
  (foo((uint(0u)), (gs_Dims)[0], (gs_Dims)[1], (iMips)));
  SB[2].Dims = gs_Dims;
}
