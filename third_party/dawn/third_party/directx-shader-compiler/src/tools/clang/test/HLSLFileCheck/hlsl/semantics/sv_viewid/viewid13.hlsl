// RUN: %dxilver 1.1 | %dxc -E main -T ps_6_1 %s | FileCheck %s

// CHECK: Buffer Definitions:
// CHECK: Resource Bindings:
// CHECK: Name                                 Type  Format         Dim      ID      HLSL Bind  Count
// CHECK: ------------------------------ ---------- ------- ----------- ------- -------------- ------
// CHECK:target triple = "dxil-ms-dx"

void main()
{
}