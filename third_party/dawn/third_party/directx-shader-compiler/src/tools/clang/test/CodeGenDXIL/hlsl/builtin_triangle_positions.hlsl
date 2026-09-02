// REQUIRES: dxil-1-10
// RUN: %dxc -T lib_6_10 %s | FileCheck %s

struct [raypayload] Payload {
    float sum : write(caller, closesthit, miss) : read(caller);
};

// Global TriangleObjectPositions - closesthit
// CHECK-LABEL: define void {{.*}}ClosestHit
// CHECK:   %[[TP:[^ ]+]] = call <9 x float> @dx.op.triangleObjectPosition.f32(i32 -2147483641) ; TriangleObjectPosition()
// CHECK: %[[E0:[^ ]+]] = extractelement <9 x float> %[[TP]], i32 0
// CHECK: %{{[^ ]+}} = fmul fast float %[[E0]], 2.000000e+00
// CHECK: %[[E1:[^ ]+]] = extractelement <9 x float> %[[TP]], i32 1
// CHECK: %{{[^ ]+}} = fmul fast float %[[E1]], 3.000000e+00
// CHECK: %[[E2:[^ ]+]] = extractelement <9 x float> %[[TP]], i32 2
// CHECK: %{{[^ ]+}} = fmul fast float %[[E2]], 4.000000e+00
// CHECK: %[[E4:[^ ]+]] = extractelement <9 x float> %[[TP]], i32 4
// CHECK: %{{[^ ]+}} = fmul fast float %[[E4]], 5.000000e+00
// CHECK: %[[E5:[^ ]+]] = extractelement <9 x float> %[[TP]], i32 5
// CHECK: %{{[^ ]+}} = fmul fast float %[[E5]], 6.000000e+00
// CHECK: %[[E3:[^ ]+]] = extractelement <9 x float> %[[TP]], i32 3
// CHECK: %{{[^ ]+}} = fmul fast float %[[E3]], 7.000000e+00
// CHECK: %[[E6:[^ ]+]] = extractelement <9 x float> %[[TP]], i32 6
// CHECK: %{{[^ ]+}} = fmul fast float %[[E6]], 8.000000e+00
// CHECK: %[[E7:[^ ]+]] = extractelement <9 x float> %[[TP]], i32 7
// CHECK: %{{[^ ]+}} = fmul fast float %[[E7]], 9.000000e+00
// CHECK: %[[E8:[^ ]+]] = extractelement <9 x float> %[[TP]], i32 8
// CHECK: %{{[^ ]+}} = fmul fast float %[[E8]], 1.000000e+01
  
[shader("closesthit")]
void ClosestHit(inout Payload payload, in BuiltInTriangleIntersectionAttributes attr) {
    BuiltInTrianglePositions positions = TriangleObjectPositions();
    payload.sum = 2.0f * positions.p0.x + 3.0f * positions.p0.y + 4.0f * positions.p0.z +
            5.0f * positions.p1.y + 6.0f * positions.p1.z + 7.0f * positions.p1.x +
            8.0f * positions.p2.x + 9.0f * positions.p2.y + 10.0f * positions.p2.z;
}

