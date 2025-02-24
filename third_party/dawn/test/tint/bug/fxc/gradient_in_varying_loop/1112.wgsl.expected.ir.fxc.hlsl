struct main_outputs {
  float4 tint_symbol : SV_Target0;
};

struct main_inputs {
  float2 vUV : TEXCOORD0;
};


SamplerState v : register(s0);
Texture2D<float4> randomTexture : register(t1);
float4 main_inner(float2 vUV) {
  float3 random = randomTexture.Sample(v, vUV).xyz;
  int i = int(0);
  {
    uint2 tint_loop_idx = (0u).xx;
    while(true) {
      if (all((tint_loop_idx == (4294967295u).xx))) {
        break;
      }
      if ((i < int(1))) {
      } else {
        break;
      }
      float3 offset = float3((random.x).xxx);
      bool v_1 = false;
      if ((offset.x < 0.0f)) {
        v_1 = true;
      } else {
        v_1 = (offset.y < 0.0f);
      }
      bool v_2 = false;
      if (v_1) {
        v_2 = true;
      } else {
        v_2 = (offset.x > 1.0f);
      }
      bool v_3 = false;
      if (v_2) {
        v_3 = true;
      } else {
        v_3 = (offset.y > 1.0f);
      }
      if (v_3) {
        i = (i + int(1));
        {
          uint tint_low_inc = (tint_loop_idx.x + 1u);
          tint_loop_idx.x = tint_low_inc;
          uint tint_carry = uint((tint_low_inc == 0u));
          tint_loop_idx.y = (tint_loop_idx.y + tint_carry);
        }
        continue;
      }
      float sampleDepth = 0.0f;
      i = (i + int(1));
      {
        uint tint_low_inc = (tint_loop_idx.x + 1u);
        tint_loop_idx.x = tint_low_inc;
        uint tint_carry = uint((tint_low_inc == 0u));
        tint_loop_idx.y = (tint_loop_idx.y + tint_carry);
      }
      continue;
    }
  }
  return (1.0f).xxxx;
}

main_outputs main(main_inputs inputs) {
  main_outputs v_4 = {main_inner(inputs.vUV)};
  return v_4;
}

