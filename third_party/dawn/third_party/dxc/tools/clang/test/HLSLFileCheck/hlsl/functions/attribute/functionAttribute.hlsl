// RUN: %dxilver 1.2 | %dxc -E main -T ps_6_2 -denorm ftz %s | FileCheck %s

// CHECK: @main
// CHECK: attributes #{{.*}} = { "fp32-denorm-mode"="ftz" }
float4 main(float4 col : COL) : SV_Target {
    return col;
}
