// RUN: %dxc -T lib_6_3 -fnew-inlining-behavior %s | FileCheck %s -check-prefixes=NORMAL,CHECK
// RUN: %dxc -T lib_6_3 -fnew-inlining-behavior -DNOINLINE %s | FileCheck %s -check-prefixes=NOINLINE,CHECK

// This test verifies a trivial case for disabling the always-inline behavior
// under optimzations. It tests both that a trivial call gets inlined and that
// the [noinline] attribute is correctly respected.
const RWBuffer<int> In;
RWBuffer<int> Out;

// CHECK: target datalayout

// NOINLINE: define internal {{.*}} i32 @"\01?add{{[@$?.A-Za-z0-9_]+}}"(i32 %X, i32 %Y) [[Attr:#[0-9]+]]
// NORMAL-NOT: define {{.*}} i32 @"\01?add{{[@$?.A-Za-z0-9_]+}}"(i32 %X, i32 %Y)
#ifdef NOINLINE
[noinline]
#endif
int add(int X, int Y) {
  return X + Y;
}

// CHECK: define void @CSMain()
// NOINLINE: call {{.*}}i32 @"\01?add{{[@$?.A-Za-z0-9_]+}}"(i32 
// NORMAL-NOT: call {{.*}}i32 @"\01?add{{[@$?.A-Za-z0-9_]+}}"(i32
[shader("compute")]
[numthreads(1,1,1)]
void CSMain(uint GI : SV_GroupIndex) {
  Out[GI] = add(In[GI], In[GI+1]);
}

// NOINLINE: attributes [[Attr]] = { noinline nounwind readnone }
