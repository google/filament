struct Inner {
  int scalar_i32;
  float scalar_f32;
};


RWByteAddressBuffer sb : register(u0);
void v(uint offset, Inner obj) {
  sb.Store((offset + 0u), asuint(obj.scalar_i32));
  sb.Store((offset + 4u), asuint(obj.scalar_f32));
}

void v_1(uint offset, Inner obj[4]) {
  {
    uint v_2 = 0u;
    v_2 = 0u;
    while(true) {
      uint v_3 = v_2;
      if ((v_3 >= 4u)) {
        break;
      }
      Inner v_4 = obj[v_3];
      v((offset + (v_3 * 8u)), v_4);
      {
        v_2 = (v_3 + 1u);
      }
      continue;
    }
  }
}

void v_5(uint offset, float3 obj[2]) {
  {
    uint v_6 = 0u;
    v_6 = 0u;
    while(true) {
      uint v_7 = v_6;
      if ((v_7 >= 2u)) {
        break;
      }
      sb.Store3((offset + (v_7 * 16u)), asuint(obj[v_7]));
      {
        v_6 = (v_7 + 1u);
      }
      continue;
    }
  }
}

void v_8(uint offset, float4x4 obj) {
  sb.Store4((offset + 0u), asuint(obj[0u]));
  sb.Store4((offset + 16u), asuint(obj[1u]));
  sb.Store4((offset + 32u), asuint(obj[2u]));
  sb.Store4((offset + 48u), asuint(obj[3u]));
}

void v_9(uint offset, float4x3 obj) {
  sb.Store3((offset + 0u), asuint(obj[0u]));
  sb.Store3((offset + 16u), asuint(obj[1u]));
  sb.Store3((offset + 32u), asuint(obj[2u]));
  sb.Store3((offset + 48u), asuint(obj[3u]));
}

void v_10(uint offset, float4x2 obj) {
  sb.Store2((offset + 0u), asuint(obj[0u]));
  sb.Store2((offset + 8u), asuint(obj[1u]));
  sb.Store2((offset + 16u), asuint(obj[2u]));
  sb.Store2((offset + 24u), asuint(obj[3u]));
}

void v_11(uint offset, float3x4 obj) {
  sb.Store4((offset + 0u), asuint(obj[0u]));
  sb.Store4((offset + 16u), asuint(obj[1u]));
  sb.Store4((offset + 32u), asuint(obj[2u]));
}

void v_12(uint offset, float3x3 obj) {
  sb.Store3((offset + 0u), asuint(obj[0u]));
  sb.Store3((offset + 16u), asuint(obj[1u]));
  sb.Store3((offset + 32u), asuint(obj[2u]));
}

void v_13(uint offset, float3x2 obj) {
  sb.Store2((offset + 0u), asuint(obj[0u]));
  sb.Store2((offset + 8u), asuint(obj[1u]));
  sb.Store2((offset + 16u), asuint(obj[2u]));
}

void v_14(uint offset, float2x4 obj) {
  sb.Store4((offset + 0u), asuint(obj[0u]));
  sb.Store4((offset + 16u), asuint(obj[1u]));
}

void v_15(uint offset, float2x3 obj) {
  sb.Store3((offset + 0u), asuint(obj[0u]));
  sb.Store3((offset + 16u), asuint(obj[1u]));
}

void v_16(uint offset, float2x2 obj) {
  sb.Store2((offset + 0u), asuint(obj[0u]));
  sb.Store2((offset + 8u), asuint(obj[1u]));
}

[numthreads(1, 1, 1)]
void main() {
  sb.Store(0u, asuint(0.0f));
  sb.Store(4u, asuint(int(0)));
  sb.Store(8u, 0u);
  sb.Store2(16u, asuint((0.0f).xx));
  sb.Store2(24u, asuint(int2((int(0)).xx)));
  sb.Store2(32u, (0u).xx);
  sb.Store3(48u, asuint((0.0f).xxx));
  sb.Store3(64u, asuint(int3((int(0)).xxx)));
  sb.Store3(80u, (0u).xxx);
  sb.Store4(96u, asuint((0.0f).xxxx));
  sb.Store4(112u, asuint(int4((int(0)).xxxx)));
  sb.Store4(128u, (0u).xxxx);
  v_16(144u, float2x2((0.0f).xx, (0.0f).xx));
  v_15(160u, float2x3((0.0f).xxx, (0.0f).xxx));
  v_14(192u, float2x4((0.0f).xxxx, (0.0f).xxxx));
  v_13(224u, float3x2((0.0f).xx, (0.0f).xx, (0.0f).xx));
  v_12(256u, float3x3((0.0f).xxx, (0.0f).xxx, (0.0f).xxx));
  v_11(304u, float3x4((0.0f).xxxx, (0.0f).xxxx, (0.0f).xxxx));
  v_10(352u, float4x2((0.0f).xx, (0.0f).xx, (0.0f).xx, (0.0f).xx));
  v_9(384u, float4x3((0.0f).xxx, (0.0f).xxx, (0.0f).xxx, (0.0f).xxx));
  v_8(448u, float4x4((0.0f).xxxx, (0.0f).xxxx, (0.0f).xxxx, (0.0f).xxxx));
  float3 v_17[2] = (float3[2])0;
  v_5(512u, v_17);
  Inner v_18 = (Inner)0;
  v(544u, v_18);
  Inner v_19[4] = (Inner[4])0;
  v_1(552u, v_19);
}

