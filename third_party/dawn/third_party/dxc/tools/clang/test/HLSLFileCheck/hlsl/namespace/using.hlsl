// RUN: %dxc -E main -T ps_6_0 %s | FileCheck %s

// CHECK:call float @dx.op.unary.f32(i32 13

namespace n {
    using f = float;
    namespace n2 {
       f foo(f a) { return sin(a); }
    }

}

using namespace n;
using f32 = n::f;


using n2::foo;

f32 main(f32 a:A) : SV_Target {

  return foo(a);
}