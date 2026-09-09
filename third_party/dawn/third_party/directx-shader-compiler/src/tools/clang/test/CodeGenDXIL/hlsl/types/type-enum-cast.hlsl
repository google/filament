// RUN: %dxc  -T vs_6_5 %s | FileCheck %s

enum UE : uint {
  UA = 1
};

enum SE : int {
  SA = 3
};

struct Output {
  float u;
  float s;
};

Output main(UE u : A, SE s : B) : C {
// CHECK-DAG: %[[U:.+]] = call i32 @dx.op.loadInput.i32(i32 4, i32 0, i32 0, i8 0, i32 undef)
// CHECK-DAG: %[[S:.+]] = call i32 @dx.op.loadInput.i32(i32 4, i32 1, i32 0, i8 0, i32 undef)

  Output o;

  o.u = u;
// CHECK-DAG: %[[FU:.+]] = uitofp i32 %[[U]] to float
// CHECK-DAG: call void @dx.op.storeOutput.f32(i32 5, i32 0, i32 0, i8 0, float %[[FU]])

  o.s = s;
// CHECK-DAG: %[[FS:.+]] = sitofp i32 %[[S]] to float
// CHECK-DAG: call void @dx.op.storeOutput.f32(i32 5, i32 1, i32 0, i8 0, float %[[FS]])

  return o;
}
