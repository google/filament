struct main_inputs {
  uint3 GlobalInvocationID : SV_DispatchThreadID;
};


Texture2DArray<float4> myTexture : register(t1);
RWByteAddressBuffer result : register(u3);
void main_inner(uint3 GlobalInvocationID) {
  uint flatIndex = (((4u * GlobalInvocationID.z) + (2u * GlobalInvocationID.y)) + GlobalInvocationID.x);
  flatIndex = (flatIndex * 1u);
  float4 texel = myTexture.Load(int4(int2(GlobalInvocationID.xy), int(0), int(0)));
  {
    uint i = 0u;
    while(true) {
      if ((i < 1u)) {
      } else {
        break;
      }
      uint v = 0u;
      result.GetDimensions(v);
      result.Store((0u + (min((flatIndex + i), ((v / 4u) - 1u)) * 4u)), asuint(texel.x));
      {
        i = (i + 1u);
      }
      continue;
    }
  }
}

[numthreads(1, 1, 1)]
void main(main_inputs inputs) {
  main_inner(inputs.GlobalInvocationID);
}

