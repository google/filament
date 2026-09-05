// RUN: not %dxc -E main -T ps_6_2 -denorm bad -spirv %s 2>&1 | FileCheck %s

// CHECK: dxc failed : Unsupported value 'bad' for denorm option.

float4 main(float4 col : COL) : SV_Target {
    return col;
}
