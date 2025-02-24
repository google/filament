struct main_inputs {
  uint idx : SV_GroupIndex;
};


RWByteAddressBuffer sb : register(u0);
void v(uint offset, float3 obj[2]) {
  {
    uint v_1 = 0u;
    v_1 = 0u;
    while(true) {
      uint v_2 = v_1;
      if ((v_2 >= 2u)) {
        break;
      }
      sb.Store3((offset + (v_2 * 16u)), asuint(obj[v_2]));
      {
        v_1 = (v_2 + 1u);
      }
      continue;
    }
  }
}

void v_3(uint offset, float4x4 obj) {
  sb.Store4((offset + 0u), asuint(obj[0u]));
  sb.Store4((offset + 16u), asuint(obj[1u]));
  sb.Store4((offset + 32u), asuint(obj[2u]));
  sb.Store4((offset + 48u), asuint(obj[3u]));
}

void v_4(uint offset, float4x3 obj) {
  sb.Store3((offset + 0u), asuint(obj[0u]));
  sb.Store3((offset + 16u), asuint(obj[1u]));
  sb.Store3((offset + 32u), asuint(obj[2u]));
  sb.Store3((offset + 48u), asuint(obj[3u]));
}

void v_5(uint offset, float4x2 obj) {
  sb.Store2((offset + 0u), asuint(obj[0u]));
  sb.Store2((offset + 8u), asuint(obj[1u]));
  sb.Store2((offset + 16u), asuint(obj[2u]));
  sb.Store2((offset + 24u), asuint(obj[3u]));
}

void v_6(uint offset, float3x4 obj) {
  sb.Store4((offset + 0u), asuint(obj[0u]));
  sb.Store4((offset + 16u), asuint(obj[1u]));
  sb.Store4((offset + 32u), asuint(obj[2u]));
}

void v_7(uint offset, float3x3 obj) {
  sb.Store3((offset + 0u), asuint(obj[0u]));
  sb.Store3((offset + 16u), asuint(obj[1u]));
  sb.Store3((offset + 32u), asuint(obj[2u]));
}

void v_8(uint offset, float3x2 obj) {
  sb.Store2((offset + 0u), asuint(obj[0u]));
  sb.Store2((offset + 8u), asuint(obj[1u]));
  sb.Store2((offset + 16u), asuint(obj[2u]));
}

void v_9(uint offset, float2x4 obj) {
  sb.Store4((offset + 0u), asuint(obj[0u]));
  sb.Store4((offset + 16u), asuint(obj[1u]));
}

void v_10(uint offset, float2x3 obj) {
  sb.Store3((offset + 0u), asuint(obj[0u]));
  sb.Store3((offset + 16u), asuint(obj[1u]));
}

void v_11(uint offset, float2x2 obj) {
  sb.Store2((offset + 0u), asuint(obj[0u]));
  sb.Store2((offset + 8u), asuint(obj[1u]));
}

void main_inner(uint idx) {
  uint v_12 = 0u;
  sb.GetDimensions(v_12);
  sb.Store((0u + (min(idx, ((v_12 / 544u) - 1u)) * 544u)), asuint(0.0f));
  uint v_13 = 0u;
  sb.GetDimensions(v_13);
  sb.Store((4u + (min(idx, ((v_13 / 544u) - 1u)) * 544u)), asuint(int(0)));
  uint v_14 = 0u;
  sb.GetDimensions(v_14);
  sb.Store((8u + (min(idx, ((v_14 / 544u) - 1u)) * 544u)), 0u);
  uint v_15 = 0u;
  sb.GetDimensions(v_15);
  sb.Store2((16u + (min(idx, ((v_15 / 544u) - 1u)) * 544u)), asuint((0.0f).xx));
  uint v_16 = 0u;
  sb.GetDimensions(v_16);
  uint v_17 = (24u + (min(idx, ((v_16 / 544u) - 1u)) * 544u));
  sb.Store2(v_17, asuint(int2((int(0)).xx)));
  uint v_18 = 0u;
  sb.GetDimensions(v_18);
  sb.Store2((32u + (min(idx, ((v_18 / 544u) - 1u)) * 544u)), (0u).xx);
  uint v_19 = 0u;
  sb.GetDimensions(v_19);
  sb.Store3((48u + (min(idx, ((v_19 / 544u) - 1u)) * 544u)), asuint((0.0f).xxx));
  uint v_20 = 0u;
  sb.GetDimensions(v_20);
  uint v_21 = (64u + (min(idx, ((v_20 / 544u) - 1u)) * 544u));
  sb.Store3(v_21, asuint(int3((int(0)).xxx)));
  uint v_22 = 0u;
  sb.GetDimensions(v_22);
  sb.Store3((80u + (min(idx, ((v_22 / 544u) - 1u)) * 544u)), (0u).xxx);
  uint v_23 = 0u;
  sb.GetDimensions(v_23);
  sb.Store4((96u + (min(idx, ((v_23 / 544u) - 1u)) * 544u)), asuint((0.0f).xxxx));
  uint v_24 = 0u;
  sb.GetDimensions(v_24);
  uint v_25 = (112u + (min(idx, ((v_24 / 544u) - 1u)) * 544u));
  sb.Store4(v_25, asuint(int4((int(0)).xxxx)));
  uint v_26 = 0u;
  sb.GetDimensions(v_26);
  sb.Store4((128u + (min(idx, ((v_26 / 544u) - 1u)) * 544u)), (0u).xxxx);
  uint v_27 = 0u;
  sb.GetDimensions(v_27);
  v_11((144u + (min(idx, ((v_27 / 544u) - 1u)) * 544u)), float2x2((0.0f).xx, (0.0f).xx));
  uint v_28 = 0u;
  sb.GetDimensions(v_28);
  v_10((160u + (min(idx, ((v_28 / 544u) - 1u)) * 544u)), float2x3((0.0f).xxx, (0.0f).xxx));
  uint v_29 = 0u;
  sb.GetDimensions(v_29);
  v_9((192u + (min(idx, ((v_29 / 544u) - 1u)) * 544u)), float2x4((0.0f).xxxx, (0.0f).xxxx));
  uint v_30 = 0u;
  sb.GetDimensions(v_30);
  v_8((224u + (min(idx, ((v_30 / 544u) - 1u)) * 544u)), float3x2((0.0f).xx, (0.0f).xx, (0.0f).xx));
  uint v_31 = 0u;
  sb.GetDimensions(v_31);
  v_7((256u + (min(idx, ((v_31 / 544u) - 1u)) * 544u)), float3x3((0.0f).xxx, (0.0f).xxx, (0.0f).xxx));
  uint v_32 = 0u;
  sb.GetDimensions(v_32);
  v_6((304u + (min(idx, ((v_32 / 544u) - 1u)) * 544u)), float3x4((0.0f).xxxx, (0.0f).xxxx, (0.0f).xxxx));
  uint v_33 = 0u;
  sb.GetDimensions(v_33);
  v_5((352u + (min(idx, ((v_33 / 544u) - 1u)) * 544u)), float4x2((0.0f).xx, (0.0f).xx, (0.0f).xx, (0.0f).xx));
  uint v_34 = 0u;
  sb.GetDimensions(v_34);
  v_4((384u + (min(idx, ((v_34 / 544u) - 1u)) * 544u)), float4x3((0.0f).xxx, (0.0f).xxx, (0.0f).xxx, (0.0f).xxx));
  uint v_35 = 0u;
  sb.GetDimensions(v_35);
  v_3((448u + (min(idx, ((v_35 / 544u) - 1u)) * 544u)), float4x4((0.0f).xxxx, (0.0f).xxxx, (0.0f).xxxx, (0.0f).xxxx));
  uint v_36 = 0u;
  sb.GetDimensions(v_36);
  float3 v_37[2] = (float3[2])0;
  v((512u + (min(idx, ((v_36 / 544u) - 1u)) * 544u)), v_37);
}

[numthreads(1, 1, 1)]
void main(main_inputs inputs) {
  main_inner(inputs.idx);
}

