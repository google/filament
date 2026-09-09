// RUN: not %dxc -T cs_6_3 -E main -fspv-target-env=vulkan1.2 -fcgl  %s -spirv  2>&1 | FileCheck %s

void Fun() {
// CHECK: error: cannot initialize a variable of type 'RayQuery<0>' with an rvalue of type 'literal int'
  RayQuery<0> RayQ = 0;
}

struct SomeStruct {
  void DummyMethod() {};
};

[numthreads(1, 1, 1)]
void main() {
  SomeStruct Payload;
  Payload.DummyMethod();

// CHECK: error: type mismatch
  RayQuery<0> RayQ = {0};
  Fun();
}
