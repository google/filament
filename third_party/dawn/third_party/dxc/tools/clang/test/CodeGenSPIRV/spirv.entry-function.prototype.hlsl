// RUN: %dxc -T cs_6_6 -E main -spirv %s | FileCheck %s

void main();

// CHECK:     OpEntryPoint GLCompute %main "main"
[numthreads(1, 1, 1)]
void main() {
}
