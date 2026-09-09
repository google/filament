// RUN: %dxc -E main -T ps_6_0 %s -HV 2021  | FileCheck -check-prefix=HV2021 %s

// RUN: %dxc -E main -T ps_6_0 %s -HV 2018 | FileCheck %s

// CHECK:using.hlsl:14:5: warning: keyword 'using' is a HLSL 2021 feature, and is available in older versions as a non-portable extension.
// CHECK:using.hlsl:21:1: warning: keyword 'using' is a HLSL 2021 feature, and is available in older versions as a non-portable extension.
// CHECK:using.hlsl:22:1: warning: keyword 'using' is a HLSL 2021 feature, and is available in older versions as a non-portable extension.

// CHECK:error: control reaches end of non-void function

// HV2021-NOT:keyword 'using' is a HLSL 2021 feature, and is available in older versions as a non-portable extension
// HV2021:error: control reaches end of non-void function
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

  f32 r = foo(a);
}
