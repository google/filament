// RUN: %dxc -T cs_6_3 -fcgl %s | FileCheck %s
// RUN: %dxc -T cs_6_3 -fnew-inlining-behavior -fcgl %s | FileCheck %s

void fn1() {}

[numthreads(1,1,1)]
void main() {
  fn1();
}

// CHECK: define void @main() [[MainAttr:#[0-9]+]]
// CHECK: define internal void @"\01?fn1{{[@$?.A-Za-z0-9_]+}}"() [[FnAttr:#[0-9]+]]

// CHECK: attributes [[MainAttr]] = { nounwind }
// CHECK: attributes [[FnAttr]] = { alwaysinline nounwind }
