// RUN: %dxc /T ps_6_5 -fcgl %s | FileCheck %s

// CHECK-NOT: null operand
// CHECK: define void @main()
// CHECK: ret void

void main() {
  while (true) {
    if (false) {
    } else {
      break;
    }
    break;
  }
}
