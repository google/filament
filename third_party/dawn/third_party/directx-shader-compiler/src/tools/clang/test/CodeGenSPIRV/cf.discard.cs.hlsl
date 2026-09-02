// RUN: not %dxc -T cs_6_0 -E main -fcgl  %s -spirv  2>&1 | FileCheck %s

// According to the HLS spec, discard can only be called from a pixel shader.


[numthreads(32, 1, 1)]
void main(uint3 id: SV_DispatchThreadID) {
  int a, b;
  bool cond = true;
  while(cond) {
    if(a==b) {
// CHECK: :13:7: error: discard statement may only be used in pixel shaders
      discard;
      break;
    } else {
      ++a;
// CHECK: :18:7: error: discard statement may only be used in pixel shaders
      discard;
      continue;
      --b;
    }
  }
}
