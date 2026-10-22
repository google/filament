// RUN: %dxc -T ps_6_0 -E main -fcgl  %s -spirv | FileCheck %s

namespace n {
    using f = float;
    namespace n2 {
       f foo(f a) { return sin(a); }
    }

}

using namespace n;
using f32 = n::f;


using n2::foo;

// CHECK: %src_main = OpFunction %float
// CHECK-NEXT: %a = OpFunctionParameter %_ptr_Function_float
f32 main(f32 a:A) : SV_Target {

  // CHECK: OpFunctionCall %float %n__n2__foo %param_var_a_0
  return foo(a);
}
