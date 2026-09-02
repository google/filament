// RUN: %dxc -E main -T ps_6_0 %s | FileCheck %s


// CHECK:call %dx.types.CBufRet.i32 @dx.op.cbufferLoadLegacy.i32
// CHECK: call %dx.types.ResRet.f32 @dx.op.textureLoad.f32

namespace  n {
   using T2D = Texture2D<float>;
}


namespace  n2 {
    using n::T2D;
}

namespace n3 {
   cbuffer A {
    n2::T2D t0;
    int i;
   };
}

using namespace n3;

int main(int a:A) : SV_Target {
  return t0.Load(i);
}