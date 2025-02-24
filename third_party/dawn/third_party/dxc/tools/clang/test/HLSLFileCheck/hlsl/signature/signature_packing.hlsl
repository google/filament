// RUN: %dxc -E main -T ps_6_0 %s | FileCheck %s


// CHECK: {{![0-9]+}} = !{i32 0, !"A", i8 8, i8 0, {{![0-9]+}}, i8 2, i32 1, i8 2, i32 0, i8 0, null}
// CHECK: {{![0-9]+}} = !{i32 1, !"B", i8 9, i8 0, {{![0-9]+}}, i8 2, i32 1, i8 2, i32 0, i8 2, null}
// CHECK: {{![0-9]+}} = !{i32 2, !"C", i8 9, i8 0, {{![0-9]+}}, i8 2, i32 1, i8 3, i32 1, i8 0, null}
// CHECK: {{![0-9]+}} = !{i32 3, !"D", i8 9, i8 0, {{![0-9]+}}, i8 2, i32 1, i8 2, i32 2, i8 0, null}
// CHECK: {{![0-9]+}} = !{i32 4, !"E", i8 4, i8 0, {{![0-9]+}}, i8 1, i32 1, i8 1, i32 3, i8 0, null}
// CHECK: {{![0-9]+}} = !{i32 5, !"F", i8 9, i8 0, {{![0-9]+}}, i8 2, i32 1, i8 2, i32 2, i8 2, null}
// CHECK: {{![0-9]+}} = !{i32 6, !"G", i8 9, i8 0, {{![0-9]+}}, i8 2, i32 1, i8 1, i32 1, i8 3, null}

float4 main(min16float2 a : A, float2 b : B, half3 c : C, 
            float2 d : D, int e : E, half2 f : F, half g : G) : SV_Target {
  return 1;
}