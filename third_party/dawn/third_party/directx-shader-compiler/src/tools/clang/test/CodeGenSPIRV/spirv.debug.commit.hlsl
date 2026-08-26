// RUN: %dxc -T vs_6_0 -E main -fspv-target-env=vulkan1.1 -Zi -fcgl  %s -spirv | FileCheck %s

// CHECK: OpModuleProcessed "dxc-commit-hash: {{[0-9a-zA-Z]+}}"

void main() { }
