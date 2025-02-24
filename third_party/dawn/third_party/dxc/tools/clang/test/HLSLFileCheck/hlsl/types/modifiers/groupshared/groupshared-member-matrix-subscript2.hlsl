// RUN: %dxc -E main -T cs_6_0 -Zpr %s | FileCheck %s

// Make sure non-const gep/addrspace cast in codegen is translated properly

// CHECK: @[[obj:[^,]+]] = addrspace(3) global [8 x float] undef

// CHECK: %[[_6:[^ ]+]] = getelementptr [8 x float], [8 x float] addrspace(3)* @[[obj]], i32 0, i32 %{{.+}}
// CHECK: store float %{{.+}}, float addrspace(3)* %[[_6]], align 16

// Skip next three stores to get to loads
// CHECK: store
// CHECK: store
// CHECK: store

// CHECK: %[[_23:[^ ]+]] = getelementptr [8 x float], [8 x float] addrspace(3)* @[[obj]], i32 0, i32 %{{.+}}
// CHECK: %{{.+}} = load float, float addrspace(3)* %[[_23]], align 8


float4 rows[2];

void set_row(inout float2 row, uint i) {
  row = rows[i];
}

class Obj {
  float2x2 mat;
  void set() {
    set_row(mat[0], 0);
    set_row(mat[1], 1);
  }
};

RWByteAddressBuffer RWBuf;

// Dynamic index array to generate non-const gep/addrspace cast
groupshared Obj obj[2];

[numthreads(2, 1, 1)]
void main(uint3 groupThreadID: SV_GroupThreadID) {
  obj[groupThreadID.x].set();
  GroupMemoryBarrierWithGroupSync();
  float2 row = obj[1 - groupThreadID.x].mat[groupThreadID.x];
  uint addr = groupThreadID.x * 8;
  RWBuf.Store2(addr, uint2(asuint(row.x), asuint(row.y)));
}
