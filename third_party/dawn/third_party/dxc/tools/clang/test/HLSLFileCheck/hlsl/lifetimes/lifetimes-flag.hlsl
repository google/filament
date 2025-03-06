// This test checks that the lifetime markers flag works correctly when
// multiple instances are given on the command line.
//
// The shader here is not important except that it triggers the insertion
// of lifetime markers when they are enabled.

// Default value is enabled for 6.6.
// RUN: %dxc /Tvs_6_6 %s | FileCheck %s -check-prefix=ENABLE

// Explicitly enabling the flag should enable lifetime markers.
// RUN: %dxc /Tvs_6_6 %s -enable-lifetime-markers | FileCheck %s -check-prefix=ENABLE

// Explicitly disabling the flag should disable lifetime markers.
// RUN: %dxc /Tvs_6_6 %s -disable-lifetime-markers | FileCheck %s -check-prefix=DISABLE

// When both enable and disable flags are given the last occurence should win.
// RUN: %dxc /Tvs_6_6 %s -enable-lifetime-markers -disable-lifetime-markers | FileCheck %s -check-prefix=DISABLE
// RUN: %dxc /Tvs_6_6 %s -disable-lifetime-markers -enable-lifetime-markers | FileCheck %s -check-prefix=ENABLE

// ENABLE: define void @main()
// ENABLE: call void @llvm.lifetime.start

// DISABLE: define void @main()
// DISABLE-NOT: call void @llvm.lifetime.start

void foo() {};

[RootSignature("DescriptorTable(UAV(u0, numDescriptors=10))")]
float main(int N : A, float X : X) : Z {
    float Arr[64];
  
    foo();
    Arr[N] = 0;
    foo();

    return Arr[X];
}