struct main_inputs {
  uint3 GlobalInvocationID : SV_DispatchThreadID;
};


Texture2D<float4> src : register(t0);
Texture2D<float4> v : register(t1);
RWByteAddressBuffer output : register(u2);
cbuffer cbuffer_uniforms : register(b3) {
  uint4 uniforms[1];
};
uint ConvertToFp16FloatValue(float fp32) {
  return 1u;
}

uint4 tint_v4f32_to_v4u32(float4 value) {
  return (((value <= (4294967040.0f).xxxx)) ? ((((value >= (0.0f).xxxx)) ? (uint4(value)) : ((0u).xxxx))) : ((4294967295u).xxxx));
}

void main_inner(uint3 GlobalInvocationID) {
  uint2 v_1 = (0u).xx;
  src.GetDimensions(v_1.x, v_1.y);
  uint2 size = v_1;
  uint2 dstTexCoord = GlobalInvocationID.xy;
  uint2 srcTexCoord = dstTexCoord;
  if ((uniforms[0u].x == 1u)) {
    srcTexCoord.y = ((size.y - dstTexCoord.y) - 1u);
  }
  float4 srcColor = src.Load(int3(int2(srcTexCoord), int(0)));
  float4 dstColor = v.Load(int3(int2(dstTexCoord), int(0)));
  bool success = true;
  uint4 srcColorBits = (0u).xxxx;
  uint4 dstColorBits = tint_v4f32_to_v4u32(dstColor);
  {
    uint2 tint_loop_idx = (0u).xx;
    uint i = 0u;
    while(true) {
      if (all((tint_loop_idx == (4294967295u).xx))) {
        break;
      }
      if ((i < uniforms[0u].w)) {
      } else {
        break;
      }
      uint v_2 = i;
      srcColorBits[min(v_2, 3u)] = ConvertToFp16FloatValue(srcColor[min(i, 3u)]);
      bool v_3 = false;
      if (success) {
        v_3 = (srcColorBits[min(i, 3u)] == dstColorBits[min(i, 3u)]);
      } else {
        v_3 = false;
      }
      success = v_3;
      {
        uint tint_low_inc = (tint_loop_idx.x + 1u);
        tint_loop_idx.x = tint_low_inc;
        uint tint_carry = uint((tint_low_inc == 0u));
        tint_loop_idx.y = (tint_loop_idx.y + tint_carry);
        i = (i + 1u);
      }
      continue;
    }
  }
  uint outputIndex = ((GlobalInvocationID.y * uint(size.x)) + GlobalInvocationID.x);
  if (success) {
    uint v_4 = 0u;
    output.GetDimensions(v_4);
    output.Store((0u + (min(outputIndex, ((v_4 / 4u) - 1u)) * 4u)), 1u);
  } else {
    uint v_5 = 0u;
    output.GetDimensions(v_5);
    output.Store((0u + (min(outputIndex, ((v_5 / 4u) - 1u)) * 4u)), 0u);
  }
}

[numthreads(1, 1, 1)]
void main(main_inputs inputs) {
  main_inner(inputs.GlobalInvocationID);
}

