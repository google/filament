// RUN: %dxc %s -T lib_6_3 -Fo lib0
// RUN: %dxl %s -T lib_6_3 lib0 -exports foo | FileCheck %s

// CHECK: @llvm.global_ctors = appending global [1 x { i32, void ()*, i8* }] [{ i32, void ()*, i8* } { i32 65535, void ()* @{{.+}}, i8* null }]

// This is a regression test for a crash in the linker where when:
//   1. There are constructors for global variables
//   2. There are exports
//
// The bug assumes that functions not on the export list must have no users. This is not the case when
// the functions are constructors for global variables.

Texture1D<float> t0 : register(t0);
static float get_val(int i) {
  return t0[i];
}
struct My_Glob {
  float a, b, c;
};
// This constructor has real code that has to run and cannot be removed.
static My_Glob glob = { get_val(0), get_val(1), get_val(2) };

export float foo() {
  return (++glob.a) + (++glob.b) + (++glob.c);
}
