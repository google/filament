// RUN: %dxc -E main -no-warnings -T ps_6_0 %s | FileCheck %s
// CHECK-NOT: warning: attribute 'branch' can only be applied to 'if' and 'switch' statements

[branch] // this should generate a warning that is used for testing
float4 main() : SV_TARGET
{
    // no return stmt should cause error which is needed to direct errors
	// and warnings to stdin instead of dxil output.
    // return float4(1, 0, 0, 1);
}
