// RUN: %dxc -T cs_6_3 -E cs_main %s -opt-enable aggressive-reassociation  | FileCheck %s -check-prefixes=CHECK,COMMON_FACTOR
// RUN: %dxc -T cs_6_3 -E cs_main %s -opt-disable aggressive-reassociation | FileCheck %s -check-prefixes=CHECK,NO_COMMON_FACTOR

// Make sure DXC recognize the common factor and generate optimized dxils if the enable-aggressive-reassociation is true.

// CHECK: [[FACTOR_SRC1:%.*]] = call i32 @dx.op.flattenedThreadIdInGroup.i32(i32 96)
// CHECK: [[FACTOR_SRC0:%.*]] = call i32 @dx.op.threadIdInGroup.i32(i32 95, i32 0)

// COMMON_FACTOR: [[FACTOR:%.*]] = mul i32 [[FACTOR_SRC0]], [[FACTOR_SRC1]]
// COMMON_FACTOR: mul i32 [[FACTOR]],
// COMMON_FACTOR: mul i32 [[FACTOR]],

// NO_COMMON_FACTOR: [[EXPRESSION_0:%.*]] = mul i32 [[FACTOR_SRC1]],
// NO_COMMON_FACTOR:                        mul i32 [[EXPRESSION_0]], [[FACTOR_SRC0]]
// NO_COMMON_FACTOR: [[EXPRESSION_1:%.*]] = mul i32 [[FACTOR_SRC0]], [[FACTOR_SRC1]]
// NO_COMMON_FACTOR:                        mul i32 [[EXPRESSION_1]],  


RWTexture1D < float2 > outColorBuffer : register ( u0 ) ;

[ numthreads ( 8 , 8 , 1 ) ]
void cs_main ( uint3 GroupID : SV_GroupID , uint GroupIndex : SV_GroupIndex , uint3 GTID : SV_GroupThreadID , uint3 DispatchThreadID : SV_DispatchThreadID )
{
      // DXC should recognize (GroupIndex * GTID.x) is a common factor
      uint a = GroupIndex * GroupID.x;
      uint b = GroupIndex * DispatchThreadID.x;
      uint c = a * GTID.x;
      uint d = b * GTID.x;

      outColorBuffer [ DispatchThreadID.y ] = float2(c, d);
}