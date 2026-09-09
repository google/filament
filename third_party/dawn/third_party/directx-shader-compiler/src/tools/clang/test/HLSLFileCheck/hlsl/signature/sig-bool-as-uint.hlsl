// RUN: %dxilver 1.5 | %dxc -E main -T ps_6_0 %s | FileCheck %s

// make sure bool maps to uint in signature
// CHECK: ; BOOL                     0   x           0     NONE    uint

float4 main(bool b : BOOL) : SV_Target
{
	return b ? 1.0 : 0.0;
}
