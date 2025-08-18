// RUN: %dxc -T as_6_6 -E main -fspv-target-env=universal1.5 -fcgl  %s -spirv | FileCheck %s

// CHECK: OpCapability Linkage
// CHECK: OpDecorate %external_function LinkageAttributes "external_function" Export
export int external_function() {
  return 1;
}

[numthreads(8, 8, 1)]
void main() {
  external_function();
	return;
}
