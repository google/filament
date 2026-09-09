// RUN: %dxc -E main -T ps_6_0 %s | FileCheck %s
// CHECK:call %dx.types.Dimensions @dx.op.getDimensions
// CHECK:extractvalue %dx.types.Dimensions %{{.*}}, 0
// CHECK:extractvalue %dx.types.Dimensions %{{.*}}, 1
// CHECK:extractvalue %dx.types.Dimensions %{{.*}}, 3

TextureCube<float4> T;

float main() : SV_Target {
  uint iMips = (uint)(0);
    uint2 Dims = (uint2)(0);
    (T.GetDimensions((uint(0u)), (Dims)[0], (Dims)[1], (iMips)));
  return iMips + Dims.x + Dims.y;
}
