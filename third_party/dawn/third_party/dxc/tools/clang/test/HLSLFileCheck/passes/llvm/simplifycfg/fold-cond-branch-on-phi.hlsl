// RUN: %dxc /Zi -E main -T ps_6_0 %s  | FileCheck %s

// the FoldCondBranchOnPhi transformation in simplify cfg
// often creates unstructured flow. This test makes sure
// that transformation doesn't happen.

// CHECK: %[[cond:.+]] = phi i1
// CHECK-SAME: [ false
// CHECK: br i1 %[[cond]]

cbuffer cb : register(b0) {
  uint a,b,c,d,e,f,g,h,i,j,k,l,m,n;
  float nums[10];
};

bool foo() {
  [branch]
  if (a) {
    return false;
  }

  [branch]
  if (b & c) {

    [branch]
    if (g && h) {
      return true;
    }

    [branch]
    if (e && f) {
      return true;
    }
    return false;
  }

  return true;
}

[RootSignature("CBV(b0)")]
float main(uint ia : IA) : SV_Target {
  float ret = 0;
  if (foo()) {
    int i = 0;
    [loop]
    do {
      ret += sin(nums[i]);
      i++;
    }
    while(i < ia);
  }
  return ret;
}


