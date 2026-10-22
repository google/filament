// RUN: %dxc /T vs_6_0 /E main %s | FileCheck %s

// Test that the entry point must be in the global namespace.

// CHECK: error: missing entry point definition

namespace foo { void main() {} }
struct bar { static void main() {} };
