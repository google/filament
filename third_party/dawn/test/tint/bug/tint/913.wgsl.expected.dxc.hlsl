Texture2D<float4> src : register(t0);
Texture2D<float4> tint_symbol : register(t1);
RWByteAddressBuffer output : register(u2);
cbuffer cbuffer_uniforms : register(b3) {
  uint4 uniforms[2];
};

bool aboutEqual(float value, float expect) {
  return (abs((value - expect)) < 0.00100000004749745131f);
}

struct tint_symbol_8 {
  uint3 GlobalInvocationID : SV_DispatchThreadID;
};

void main_inner(uint3 GlobalInvocationID) {
  uint2 tint_tmp;
  src.GetDimensions(tint_tmp.x, tint_tmp.y);
  uint2 srcSize = tint_tmp;
  uint2 tint_tmp_1;
  tint_symbol.GetDimensions(tint_tmp_1.x, tint_tmp_1.y);
  uint2 dstSize = tint_tmp_1;
  uint2 dstTexCoord = uint2(GlobalInvocationID.xy);
  float4 nonCoveredColor = float4(0.0f, 1.0f, 0.0f, 1.0f);
  bool success = true;
  bool tint_tmp_4 = (dstTexCoord.x < uniforms[1].x);
  if (!tint_tmp_4) {
    tint_tmp_4 = (dstTexCoord.y < uniforms[1].y);
  }
  bool tint_tmp_3 = (tint_tmp_4);
  if (!tint_tmp_3) {
    tint_tmp_3 = (dstTexCoord.x >= (uniforms[1].x + uniforms[1].z));
  }
  bool tint_tmp_2 = (tint_tmp_3);
  if (!tint_tmp_2) {
    tint_tmp_2 = (dstTexCoord.y >= (uniforms[1].y + uniforms[1].w));
  }
  if ((tint_tmp_2)) {
    bool tint_tmp_5 = success;
    if (tint_tmp_5) {
      tint_tmp_5 = all((tint_symbol.Load(int3(int2(dstTexCoord), 0)) == nonCoveredColor));
    }
    success = (tint_tmp_5);
  } else {
    uint2 srcTexCoord = ((dstTexCoord - uniforms[1].xy) + uniforms[0].zw);
    if ((uniforms[0].x == 1u)) {
      srcTexCoord.y = ((srcSize.y - srcTexCoord.y) - 1u);
    }
    float4 srcColor = src.Load(int3(int2(srcTexCoord), 0));
    float4 dstColor = tint_symbol.Load(int3(int2(dstTexCoord), 0));
    if ((uniforms[0].y == 2u)) {
      bool tint_symbol_2 = success;
      if (tint_symbol_2) {
        tint_symbol_2 = aboutEqual(dstColor.r, srcColor.r);
      }
      bool tint_symbol_1 = tint_symbol_2;
      if (tint_symbol_1) {
        tint_symbol_1 = aboutEqual(dstColor.g, srcColor.g);
      }
      success = tint_symbol_1;
    } else {
      bool tint_symbol_6 = success;
      if (tint_symbol_6) {
        tint_symbol_6 = aboutEqual(dstColor.r, srcColor.r);
      }
      bool tint_symbol_5 = tint_symbol_6;
      if (tint_symbol_5) {
        tint_symbol_5 = aboutEqual(dstColor.g, srcColor.g);
      }
      bool tint_symbol_4 = tint_symbol_5;
      if (tint_symbol_4) {
        tint_symbol_4 = aboutEqual(dstColor.b, srcColor.b);
      }
      bool tint_symbol_3 = tint_symbol_4;
      if (tint_symbol_3) {
        tint_symbol_3 = aboutEqual(dstColor.a, srcColor.a);
      }
      success = tint_symbol_3;
    }
  }
  uint outputIndex = ((GlobalInvocationID.y * dstSize.x) + GlobalInvocationID.x);
  if (success) {
    uint tint_symbol_10 = 0u;
    output.GetDimensions(tint_symbol_10);
    uint tint_symbol_11 = ((tint_symbol_10 - 0u) / 4u);
    output.Store((4u * min(outputIndex, (tint_symbol_11 - 1u))), asuint(1u));
  } else {
    uint tint_symbol_12 = 0u;
    output.GetDimensions(tint_symbol_12);
    uint tint_symbol_13 = ((tint_symbol_12 - 0u) / 4u);
    output.Store((4u * min(outputIndex, (tint_symbol_13 - 1u))), asuint(0u));
  }
}

[numthreads(1, 1, 1)]
void main(tint_symbol_8 tint_symbol_7) {
  main_inner(tint_symbol_7.GlobalInvocationID);
  return;
}
