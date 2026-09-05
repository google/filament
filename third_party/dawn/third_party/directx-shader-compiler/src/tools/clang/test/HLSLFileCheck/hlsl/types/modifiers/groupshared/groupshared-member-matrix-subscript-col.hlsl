// RUN: %dxc -E main -T cs_6_0 -Zpc %s | FileCheck %s

// CHECK: %[[cb0:[^ ]+]] = call %dx.types.CBufRet.f32 @dx.op.cbufferLoadLegacy.f32(i32 59, %dx.types.Handle %{{.*}}, i32 0)
// CHECK: %[[cb0x:[^ ]+]] = extractvalue %dx.types.CBufRet.f32 %[[cb0]], 0
// CHECK: store float %[[cb0x]], float addrspace(3)* getelementptr inbounds ([4 x float], [4 x float] addrspace(3)* @[[obj:[^,]+]], i32 0, i32 0), align 4

// CHECK: %[[cb1:[^ ]+]] = call %dx.types.CBufRet.f32 @dx.op.cbufferLoadLegacy.f32(i32 59, %dx.types.Handle %{{.*}}, i32 1)
// CHECK: %[[cb1x:[^ ]+]] = extractvalue %dx.types.CBufRet.f32 %[[cb1]], 0
// CHECK: store float %[[cb1x]], float addrspace(3)* getelementptr inbounds ([4 x float], [4 x float] addrspace(3)* @[[obj]], i32 0, i32 1), align 4

// CHECK: %[[_25:[^ ]+]] = getelementptr [4 x float], [4 x float] addrspace(3)* @[[obj]], i32 0, i32 %{{.+}}
// CHECK: %[[_26:[^ ]+]] = load float, float addrspace(3)* %[[_25]], align 4
// CHECK: %[[_27:[^ ]+]] = getelementptr [4 x float], [4 x float] addrspace(3)* @[[obj]], i32 0, i32 %{{.+}}
// CHECK: %[[_28:[^ ]+]] = load float, float addrspace(3)* %[[_27]], align 4

// CHECK: %[[_33:[^ ]+]] = bitcast float %[[_26]] to i32
// CHECK: %[[_34:[^ ]+]] = bitcast float %[[_28]] to i32

// CHECK: call void @dx.op.bufferStore.i32(i32 69, %dx.types.Handle %{{[^,]+}}, i32 %{{.+}}, i32 undef, i32 %[[_33]], i32 %[[_34]], i32 undef, i32 undef, i8 3)

float2 rows[2];

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
groupshared Obj obj;

[numthreads(2, 1, 1)]
void main(uint3 groupThreadID: SV_GroupThreadID) {
  if (groupThreadID.x == 0) {
    obj.set();
  }
  GroupMemoryBarrierWithGroupSync();
  float2 row = obj.mat[groupThreadID.x];
  uint addr = groupThreadID.x * 8;
  RWBuf.Store2(addr, uint2(asuint(row.x), asuint(row.y)));
}
