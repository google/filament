// RUN: %dxilver 1.6 | %dxc -T lib_6_3 -Wno-return-type %s | FileCheck %s

// disable return-type warning (that defaults to error) to catch
// validation case that would crash before HLSL Change to ilist_node
// preventing deref of Prev nullptr.

// CHECK: error: Instructions must be of an allowed type.
// CHECK: note: at 'unreachable' in block
// CHECK-SAME: of function
// CHECK-SAME: no_return

export float4 no_return() {
  float4 f = 1.0;
  f += 2.0;
}
