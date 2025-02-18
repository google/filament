struct FragmentOutput {
  float4 color;
};

struct FragmentInput {
  float2 vUv;
};

struct main_outputs {
  float4 FragmentOutput_color : SV_Target0;
};

struct main_inputs {
  float2 FragmentInput_vUv : TEXCOORD2;
};


Texture2D depthMap : register(t5, space1);
SamplerState texSampler : register(s3, space1);
FragmentOutput main_inner(FragmentInput fIn) {
  float v = depthMap.Sample(texSampler, fIn.vUv).x;
  float3 color = float3(v, v, v);
  FragmentOutput fOut = (FragmentOutput)0;
  fOut.color = float4(color, 1.0f);
  FragmentOutput v_1 = fOut;
  return v_1;
}

main_outputs main(main_inputs inputs) {
  FragmentInput v_2 = {inputs.FragmentInput_vUv};
  FragmentOutput v_3 = main_inner(v_2);
  main_outputs v_4 = {v_3.color};
  return v_4;
}

