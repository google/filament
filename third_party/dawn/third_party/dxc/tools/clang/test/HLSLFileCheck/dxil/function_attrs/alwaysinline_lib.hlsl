// RUN: %dxc -T lib_6_3 -fcgl %s | FileCheck %s -check-prefixes=ALWAYS,CHECK
// RUN: %dxc -T lib_6_3 -fnew-inlining-behavior -fcgl %s | FileCheck %s -check-prefixes=NORMAL,CHECK

// CHECK: define internal void @"\01?fn1{{[@$?.A-Za-z0-9_]+}}"() [[Fn1:#[0-9]+]]
void fn1() {}

// CHECK: define internal void @"\01?fn2{{[@$?.A-Za-z0-9_]+}}"() [[Fn2:#[0-9]+]]
void fn2() {
  fn1();
}

// ALWAYS: attributes [[Fn1]] = { alwaysinline nounwind }
// ALWAYS: attributes [[Fn2]] = { nounwind }

// In the normal inlining mode, the two functions share the same attributes so
// their attribute sets will be merged.

// NORMAL: attributes [[Fn1]] = { nounwind }
// NORMAL-NOT: attributes [[Fn2]] = 

