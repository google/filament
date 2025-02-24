// RUN: %dxc /T vs_6_0 /E main %s | FileCheck %s

// Regression test for a bug where any function named
// like the entry point would get the same mangling,
// regardless of its scope, causing a name clash (GitHub #1848).

// CHECK: define void @main()

namespace foo { void main() {} }
struct bar { static void main() {} };
void main() { foo::main(); bar::main(); }
