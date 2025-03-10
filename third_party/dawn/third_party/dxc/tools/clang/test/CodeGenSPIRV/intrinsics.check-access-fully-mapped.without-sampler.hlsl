// RUN: %dxc -T ps_6_0 -E main -fcgl  %s -spirv | FileCheck %s

// This test checks if capability visitor emits SparseResidency capability
// correctly when OpImageSparseTexelsResident is used.

// CHECK: OpCapability SparseResidency

void main() {
  uint status;

// CHECK: [[residency_code:%[0-9]+]] = OpLoad %uint %status
// CHECK:         [[result:%[0-9]+]] = OpImageSparseTexelsResident %bool [[residency_code]]
// CHECK:                           OpStore %result [[result]]
  bool result = CheckAccessFullyMapped(status);
}
