struct main_inputs {
  uint3 GlobalInvocationID : SV_DispatchThreadID;
};


Texture2D<float4> src : register(t0);
Texture2D<float4> v : register(t1);
RWByteAddressBuffer output : register(u2);
cbuffer cbuffer_uniforms : register(b3) {
  uint4 uniforms[2];
};
bool aboutEqual(float value, float expect) {
  return (abs((value - expect)) < 0.00100000004749745131f);
}

void main_inner(uint3 GlobalInvocationID) {
  uint2 v_1 = (0u).xx;
  src.GetDimensions(v_1.x, v_1.y);
  uint2 srcSize = v_1;
  uint2 v_2 = (0u).xx;
  v.GetDimensions(v_2.x, v_2.y);
  uint2 dstSize = v_2;
  uint2 dstTexCoord = uint2(GlobalInvocationID.xy);
  float4 nonCoveredColor = float4(0.0f, 1.0f, 0.0f, 1.0f);
  bool success = true;
  bool v_3 = false;
  if ((dstTexCoord.x < uniforms[1u].x)) {
    v_3 = true;
  } else {
    v_3 = (dstTexCoord.y < uniforms[1u].y);
  }
  bool v_4 = false;
  if (v_3) {
    v_4 = true;
  } else {
    v_4 = (dstTexCoord.x >= (uniforms[1u].x + uniforms[1u].z));
  }
  bool v_5 = false;
  if (v_4) {
    v_5 = true;
  } else {
    v_5 = (dstTexCoord.y >= (uniforms[1u].y + uniforms[1u].w));
  }
  if (v_5) {
    bool v_6 = false;
    if (success) {
      v_6 = all((v.Load(int3(int2(dstTexCoord), int(0))) == nonCoveredColor));
    } else {
      v_6 = false;
    }
    success = v_6;
  } else {
    uint2 srcTexCoord = ((dstTexCoord - uniforms[1u].xy) + uniforms[0u].zw);
    if ((uniforms[0u].x == 1u)) {
      srcTexCoord.y = ((srcSize.y - srcTexCoord.y) - 1u);
    }
    float4 srcColor = src.Load(int3(int2(srcTexCoord), int(0)));
    float4 dstColor = v.Load(int3(int2(dstTexCoord), int(0)));
    if ((uniforms[0u].y == 2u)) {
      bool v_7 = false;
      if (success) {
        v_7 = aboutEqual(dstColor.x, srcColor.x);
      } else {
        v_7 = false;
      }
      bool v_8 = false;
      if (v_7) {
        v_8 = aboutEqual(dstColor.y, srcColor.y);
      } else {
        v_8 = false;
      }
      success = v_8;
    } else {
      bool v_9 = false;
      if (success) {
        v_9 = aboutEqual(dstColor.x, srcColor.x);
      } else {
        v_9 = false;
      }
      bool v_10 = false;
      if (v_9) {
        v_10 = aboutEqual(dstColor.y, srcColor.y);
      } else {
        v_10 = false;
      }
      bool v_11 = false;
      if (v_10) {
        v_11 = aboutEqual(dstColor.z, srcColor.z);
      } else {
        v_11 = false;
      }
      bool v_12 = false;
      if (v_11) {
        v_12 = aboutEqual(dstColor.w, srcColor.w);
      } else {
        v_12 = false;
      }
      success = v_12;
    }
  }
  uint outputIndex = ((GlobalInvocationID.y * dstSize.x) + GlobalInvocationID.x);
  if (success) {
    uint v_13 = 0u;
    output.GetDimensions(v_13);
    output.Store((0u + (min(outputIndex, ((v_13 / 4u) - 1u)) * 4u)), 1u);
  } else {
    uint v_14 = 0u;
    output.GetDimensions(v_14);
    output.Store((0u + (min(outputIndex, ((v_14 / 4u) - 1u)) * 4u)), 0u);
  }
}

[numthreads(1, 1, 1)]
void main(main_inputs inputs) {
  main_inner(inputs.GlobalInvocationID);
}

