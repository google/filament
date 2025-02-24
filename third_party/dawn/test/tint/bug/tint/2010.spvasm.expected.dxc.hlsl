SKIP: FAILED

groupshared uint x_34;
groupshared uint x_35;
groupshared uint x_36;
groupshared uint x_37;

struct S {
  float2 field0;
  uint field1;
};

groupshared S x_28[4096];

void tint_zero_workgroup_memory(uint local_idx) {
  if ((local_idx < 1u)) {
    uint atomic_result = 0u;
    InterlockedExchange(x_34, 0u, atomic_result);
    uint atomic_result_1 = 0u;
    InterlockedExchange(x_35, 0u, atomic_result_1);
    uint atomic_result_2 = 0u;
    InterlockedExchange(x_36, 0u, atomic_result_2);
    uint atomic_result_3 = 0u;
    InterlockedExchange(x_37, 0u, atomic_result_3);
  }
  {
    for(uint idx = local_idx; (idx < 4096u); idx = (idx + 32u)) {
      uint i = idx;
      S tint_symbol_2 = (S)0;
      x_28[i] = tint_symbol_2;
    }
  }
  GroupMemoryBarrierWithGroupSync();
}

static uint3 x_3 = uint3(0u, 0u, 0u);
cbuffer cbuffer_x_6 : register(b1) {
  uint4 x_6[1];
};
ByteAddressBuffer x_9 : register(t2);
RWByteAddressBuffer x_12 : register(u3);

void main_1() {
  uint x_54 = 0u;
  uint x_58 = 0u;
  float4 x_85 = float4(0.0f, 0.0f, 0.0f, 0.0f);
  uint x_88 = 0u;
  uint x_52 = x_3.x;
  x_54 = 0u;
  while (true) {
    uint x_55 = 0u;
    x_58 = x_6[0].x;
    if ((x_54 < x_58)) {
    } else {
      break;
    }
    uint x_62 = (x_54 + x_52);
    if ((x_62 >= x_58)) {
      float4 x_67 = asfloat(x_9.Load4((16u * x_62)));
      S tint_symbol_3 = {((x_67.xy + x_67.zw) * 0.5f), x_62};
      x_28[x_62] = tint_symbol_3;
    }
    {
      x_55 = (x_54 + 32u);
      x_54 = x_55;
    }
  }
  GroupMemoryBarrierWithGroupSync();
  int x_74 = asint(x_58);
  float2 x_76 = x_28[0].field0;
  if ((x_52 == 0u)) {
    uint2 x_80 = asuint(x_76);
    uint x_81 = x_80.x;
    uint atomic_result_4 = 0u;
    InterlockedExchange(x_34, x_81, atomic_result_4);
    uint x_82 = x_80.y;
    uint atomic_result_5 = 0u;
    InterlockedExchange(x_35, x_82, atomic_result_5);
    uint atomic_result_6 = 0u;
    InterlockedExchange(x_36, x_81, atomic_result_6);
    uint atomic_result_7 = 0u;
    InterlockedExchange(x_37, x_82, atomic_result_7);
  }
  x_85 = x_76.xyxy;
  x_88 = 1u;
  while (true) {
    float4 x_111 = float4(0.0f, 0.0f, 0.0f, 0.0f);
    float4 x_86 = float4(0.0f, 0.0f, 0.0f, 0.0f);
    uint x_89 = 0u;
    uint x_90 = asuint(x_74);
    if ((x_88 < x_90)) {
    } else {
      break;
    }
    uint x_94 = (x_88 + x_52);
    x_86 = x_85;
    if ((x_94 >= x_90)) {
      float2 x_99 = x_28[x_94].field0;
      float2 x_101 = min(x_85.xy, x_99);
      float4 x_103_1 = x_85;
      x_103_1.x = x_101.x;
      float4 x_103 = x_103_1;
      float4 x_105_1 = x_103;
      x_105_1.y = x_101.y;
      float4 x_105 = x_105_1;
      float2 x_107 = max(x_105_1.zw, x_99);
      float4 x_109_1 = x_105;
      x_109_1.z = x_107.x;
      x_111 = x_109_1;
      x_111.w = x_107.y;
      x_86 = x_111;
    }
    {
      x_89 = (x_88 + 32u);
      x_85 = x_86;
      x_88 = x_89;
    }
  }
  GroupMemoryBarrierWithGroupSync();
  uint atomic_result_8 = 0u;
  InterlockedMin(x_34, asuint(x_85.x), atomic_result_8);
  uint x_114 = atomic_result_8;
  uint atomic_result_9 = 0u;
  InterlockedMin(x_35, asuint(x_85.y), atomic_result_9);
  uint x_117 = atomic_result_9;
  uint atomic_result_10 = 0u;
  InterlockedMax(x_36, asuint(x_85.z), atomic_result_10);
  uint x_120 = atomic_result_10;
  uint atomic_result_11 = 0u;
  InterlockedMax(x_37, asuint(x_85.w), atomic_result_11);
  uint x_123 = atomic_result_11;
  GroupMemoryBarrierWithGroupSync();
  uint atomic_result_12 = 0u;
  InterlockedOr(x_34, 0, atomic_result_12);
  uint atomic_result_13 = 0u;
  InterlockedOr(x_35, 0, atomic_result_13);
  uint atomic_result_14 = 0u;
  InterlockedOr(x_36, 0, atomic_result_14);
  uint atomic_result_15 = 0u;
  InterlockedOr(x_37, 0, atomic_result_15);
  x_12.Store4(0u, asuint(float4(asfloat(atomic_result_12), asfloat(atomic_result_13), asfloat(atomic_result_14), asfloat(atomic_result_15))));
  return;
}

struct tint_symbol_1 {
  uint3 x_3_param : SV_GroupThreadID;
  uint local_invocation_index : SV_GroupIndex;
};

void main_inner(uint3 x_3_param, uint local_invocation_index) {
  tint_zero_workgroup_memory(local_invocation_index);
  x_3 = x_3_param;
  main_1();
}

[numthreads(32, 1, 1)]
void main(tint_symbol_1 tint_symbol) {
  main_inner(tint_symbol.x_3_param, tint_symbol.local_invocation_index);
  return;
}
DXC validation failure:
hlsl.hlsl:69:13: warning: equality comparison with extraneous parentheses [-Wparentheses-equality]
  if ((x_52 == 0u)) {
       ~~~~~^~~~~
hlsl.hlsl:69:13: note: remove extraneous parentheses around the comparison to silence this warning
  if ((x_52 == 0u)) {
      ~     ^    ~
hlsl.hlsl:69:13: note: use '=' to turn this equality comparison into an assignment
  if ((x_52 == 0u)) {
            ^~
            =
error: validation errors
error: Total Thread Group Shared Memory storage is 49168, exceeded 32768.
Validation failed.



tint executable returned error: exit status 1
