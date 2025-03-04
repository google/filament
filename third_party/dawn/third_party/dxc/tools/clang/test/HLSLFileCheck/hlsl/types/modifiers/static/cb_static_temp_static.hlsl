// RUN: %dxc -E main -T vs_6_0 %s | FileCheck %s

// CHECK:@main
// Make sure no copy left.
// CHECK-NOT:store float

struct ST {
  float4x4 mat[2];
};

cbuffer A {
  float4x4 gmat[2];
  uint index;
};

static const struct {
  float4x4 mat[2];
} GV = {gmat};

ST Get() {
  ST R;
  R.mat = GV.mat;
  return R;
}

static ST SGV;


float4 main() : SV_POSITION {
  SGV = Get();
  return SGV.mat[index][0];
}


