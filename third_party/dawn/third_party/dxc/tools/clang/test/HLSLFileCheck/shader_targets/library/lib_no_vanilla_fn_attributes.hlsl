// RUN: %dxc -T lib_6_3 %s | FileCheck %s

// Make sure a bunch of vanilla function attributes are not added to the functions which are not inlined. 

// CHECK: define float @"\01?foo{{[@$?.A-Za-z0-9_]+}}"(float %a)
// CHECK: nounwind
// CHECK: readnone
// CHECK-NOT: disable-tail-calls
// CHECK-NOT: less-precise-fpmad
// CHECK-NOT: no-frame-pointer-elim
// CHECK-NOT: no-infs-fp-math
// CHECK-NOT: no-nans-fp-math
// CHECK-NOT: no-realign-stack 
// CHECK-NOT: stack-protector-buffer-size
// CHECK-NOT: unsafe-fp-math
// CHECK-NOT: use-soft-float


export float foo(float a) {
    return a*a;
}
