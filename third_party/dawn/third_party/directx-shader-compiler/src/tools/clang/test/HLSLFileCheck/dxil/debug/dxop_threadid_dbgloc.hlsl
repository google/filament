// RUN: %dxc -T cs_6_5 -E CSMain -O0 -Zi -enable-16bit-types %s | FileCheck %s

// CHECK-LABEL:  @CSMain()
// CHECK: %{{[0-9]+}} =  call i32 @dx.op.threadId.i32(i32 {{[0-9]+}}, i32 {{[0-9]+}}), !dbg [[DBG_MD:![0-9]+]]
// CHECK-DAG: [[SUBPGM:![0-9]+]] = !DISubprogram(name: "CSMain"
// CHECK-DAG: [[DBG_MD]] = !DILocation(line: 21, column: 15, scope: [[SUBPGM]])

RWBuffer<float> u0 : register(u0);
RWBuffer<float> u1 : register(u1);

static float my_var;
static float my_var2;

void foo() {
  my_var2 = my_var * 2;
}

[RootSignature("DescriptorTable(UAV(u0,numDescriptors=2))")]
[numthreads(64,1,1)]
void CSMain(uint3 dtid : SV_DispatchThreadID) {
  my_var = u0[dtid.x];
  foo();
  u1[dtid.x] = my_var2;
}
