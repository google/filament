SKIP: INVALID

struct Inner {
  int scalar_i32;
  float scalar_f32;
  float16_t scalar_f16;
};


RWByteAddressBuffer sb : register(u0);
void v(uint offset, Inner obj) {
  sb.Store((offset + 0u), asuint(obj.scalar_i32));
  sb.Store((offset + 4u), asuint(obj.scalar_f32));
  sb.Store<float16_t>((offset + 8u), obj.scalar_f16);
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
      v((offset + (v_3 * 12u)), v_4);
      {
        v_2 = (v_3 + 1u);
      }
      continue;
    }
  }
}

void v_5(uint offset, matrix<float16_t, 4, 2> obj) {
  sb.Store<vector<float16_t, 2> >((offset + 0u), obj[0u]);
  sb.Store<vector<float16_t, 2> >((offset + 4u), obj[1u]);
  sb.Store<vector<float16_t, 2> >((offset + 8u), obj[2u]);
  sb.Store<vector<float16_t, 2> >((offset + 12u), obj[3u]);
}

void v_6(uint offset, matrix<float16_t, 4, 2> obj[2]) {
  {
    uint v_7 = 0u;
    v_7 = 0u;
    while(true) {
      uint v_8 = v_7;
      if ((v_8 >= 2u)) {
        break;
      }
      v_5((offset + (v_8 * 16u)), obj[v_8]);
      {
        v_7 = (v_8 + 1u);
      }
      continue;
    }
  }
}

void v_9(uint offset, float3 obj[2]) {
  {
    uint v_10 = 0u;
    v_10 = 0u;
    while(true) {
      uint v_11 = v_10;
      if ((v_11 >= 2u)) {
        break;
      }
      sb.Store3((offset + (v_11 * 16u)), asuint(obj[v_11]));
      {
        v_10 = (v_11 + 1u);
      }
      continue;
    }
  }
}

void v_12(uint offset, matrix<float16_t, 4, 4> obj) {
  sb.Store<vector<float16_t, 4> >((offset + 0u), obj[0u]);
  sb.Store<vector<float16_t, 4> >((offset + 8u), obj[1u]);
  sb.Store<vector<float16_t, 4> >((offset + 16u), obj[2u]);
  sb.Store<vector<float16_t, 4> >((offset + 24u), obj[3u]);
}

void v_13(uint offset, matrix<float16_t, 4, 3> obj) {
  sb.Store<vector<float16_t, 3> >((offset + 0u), obj[0u]);
  sb.Store<vector<float16_t, 3> >((offset + 8u), obj[1u]);
  sb.Store<vector<float16_t, 3> >((offset + 16u), obj[2u]);
  sb.Store<vector<float16_t, 3> >((offset + 24u), obj[3u]);
}

void v_14(uint offset, matrix<float16_t, 3, 4> obj) {
  sb.Store<vector<float16_t, 4> >((offset + 0u), obj[0u]);
  sb.Store<vector<float16_t, 4> >((offset + 8u), obj[1u]);
  sb.Store<vector<float16_t, 4> >((offset + 16u), obj[2u]);
}

void v_15(uint offset, matrix<float16_t, 3, 3> obj) {
  sb.Store<vector<float16_t, 3> >((offset + 0u), obj[0u]);
  sb.Store<vector<float16_t, 3> >((offset + 8u), obj[1u]);
  sb.Store<vector<float16_t, 3> >((offset + 16u), obj[2u]);
}

void v_16(uint offset, matrix<float16_t, 3, 2> obj) {
  sb.Store<vector<float16_t, 2> >((offset + 0u), obj[0u]);
  sb.Store<vector<float16_t, 2> >((offset + 4u), obj[1u]);
  sb.Store<vector<float16_t, 2> >((offset + 8u), obj[2u]);
}

void v_17(uint offset, matrix<float16_t, 2, 4> obj) {
  sb.Store<vector<float16_t, 4> >((offset + 0u), obj[0u]);
  sb.Store<vector<float16_t, 4> >((offset + 8u), obj[1u]);
}

void v_18(uint offset, matrix<float16_t, 2, 3> obj) {
  sb.Store<vector<float16_t, 3> >((offset + 0u), obj[0u]);
  sb.Store<vector<float16_t, 3> >((offset + 8u), obj[1u]);
}

void v_19(uint offset, matrix<float16_t, 2, 2> obj) {
  sb.Store<vector<float16_t, 2> >((offset + 0u), obj[0u]);
  sb.Store<vector<float16_t, 2> >((offset + 4u), obj[1u]);
}

void v_20(uint offset, float4x4 obj) {
  sb.Store4((offset + 0u), asuint(obj[0u]));
  sb.Store4((offset + 16u), asuint(obj[1u]));
  sb.Store4((offset + 32u), asuint(obj[2u]));
  sb.Store4((offset + 48u), asuint(obj[3u]));
}

void v_21(uint offset, float4x3 obj) {
  sb.Store3((offset + 0u), asuint(obj[0u]));
  sb.Store3((offset + 16u), asuint(obj[1u]));
  sb.Store3((offset + 32u), asuint(obj[2u]));
  sb.Store3((offset + 48u), asuint(obj[3u]));
}

void v_22(uint offset, float4x2 obj) {
  sb.Store2((offset + 0u), asuint(obj[0u]));
  sb.Store2((offset + 8u), asuint(obj[1u]));
  sb.Store2((offset + 16u), asuint(obj[2u]));
  sb.Store2((offset + 24u), asuint(obj[3u]));
}

void v_23(uint offset, float3x4 obj) {
  sb.Store4((offset + 0u), asuint(obj[0u]));
  sb.Store4((offset + 16u), asuint(obj[1u]));
  sb.Store4((offset + 32u), asuint(obj[2u]));
}

void v_24(uint offset, float3x3 obj) {
  sb.Store3((offset + 0u), asuint(obj[0u]));
  sb.Store3((offset + 16u), asuint(obj[1u]));
  sb.Store3((offset + 32u), asuint(obj[2u]));
}

void v_25(uint offset, float3x2 obj) {
  sb.Store2((offset + 0u), asuint(obj[0u]));
  sb.Store2((offset + 8u), asuint(obj[1u]));
  sb.Store2((offset + 16u), asuint(obj[2u]));
}

void v_26(uint offset, float2x4 obj) {
  sb.Store4((offset + 0u), asuint(obj[0u]));
  sb.Store4((offset + 16u), asuint(obj[1u]));
}

void v_27(uint offset, float2x3 obj) {
  sb.Store3((offset + 0u), asuint(obj[0u]));
  sb.Store3((offset + 16u), asuint(obj[1u]));
}

void v_28(uint offset, float2x2 obj) {
  sb.Store2((offset + 0u), asuint(obj[0u]));
  sb.Store2((offset + 8u), asuint(obj[1u]));
}

[numthreads(1, 1, 1)]
void main() {
  sb.Store(0u, asuint(0.0f));
  sb.Store(4u, asuint(int(0)));
  sb.Store(8u, 0u);
  sb.Store<float16_t>(12u, float16_t(0.0h));
  sb.Store2(16u, asuint((0.0f).xx));
  sb.Store2(24u, asuint(int2((int(0)).xx)));
  sb.Store2(32u, (0u).xx);
  sb.Store<vector<float16_t, 2> >(40u, (float16_t(0.0h)).xx);
  sb.Store3(48u, asuint((0.0f).xxx));
  sb.Store3(64u, asuint(int3((int(0)).xxx)));
  sb.Store3(80u, (0u).xxx);
  sb.Store<vector<float16_t, 3> >(96u, (float16_t(0.0h)).xxx);
  sb.Store4(112u, asuint((0.0f).xxxx));
  sb.Store4(128u, asuint(int4((int(0)).xxxx)));
  sb.Store4(144u, (0u).xxxx);
  sb.Store<vector<float16_t, 4> >(160u, (float16_t(0.0h)).xxxx);
  v_28(168u, float2x2((0.0f).xx, (0.0f).xx));
  v_27(192u, float2x3((0.0f).xxx, (0.0f).xxx));
  v_26(224u, float2x4((0.0f).xxxx, (0.0f).xxxx));
  v_25(256u, float3x2((0.0f).xx, (0.0f).xx, (0.0f).xx));
  v_24(288u, float3x3((0.0f).xxx, (0.0f).xxx, (0.0f).xxx));
  v_23(336u, float3x4((0.0f).xxxx, (0.0f).xxxx, (0.0f).xxxx));
  v_22(384u, float4x2((0.0f).xx, (0.0f).xx, (0.0f).xx, (0.0f).xx));
  v_21(416u, float4x3((0.0f).xxx, (0.0f).xxx, (0.0f).xxx, (0.0f).xxx));
  v_20(480u, float4x4((0.0f).xxxx, (0.0f).xxxx, (0.0f).xxxx, (0.0f).xxxx));
  v_19(544u, matrix<float16_t, 2, 2>((float16_t(0.0h)).xx, (float16_t(0.0h)).xx));
  v_18(552u, matrix<float16_t, 2, 3>((float16_t(0.0h)).xxx, (float16_t(0.0h)).xxx));
  v_17(568u, matrix<float16_t, 2, 4>((float16_t(0.0h)).xxxx, (float16_t(0.0h)).xxxx));
  v_16(584u, matrix<float16_t, 3, 2>((float16_t(0.0h)).xx, (float16_t(0.0h)).xx, (float16_t(0.0h)).xx));
  v_15(600u, matrix<float16_t, 3, 3>((float16_t(0.0h)).xxx, (float16_t(0.0h)).xxx, (float16_t(0.0h)).xxx));
  v_14(624u, matrix<float16_t, 3, 4>((float16_t(0.0h)).xxxx, (float16_t(0.0h)).xxxx, (float16_t(0.0h)).xxxx));
  v_5(648u, matrix<float16_t, 4, 2>((float16_t(0.0h)).xx, (float16_t(0.0h)).xx, (float16_t(0.0h)).xx, (float16_t(0.0h)).xx));
  v_13(664u, matrix<float16_t, 4, 3>((float16_t(0.0h)).xxx, (float16_t(0.0h)).xxx, (float16_t(0.0h)).xxx, (float16_t(0.0h)).xxx));
  v_12(696u, matrix<float16_t, 4, 4>((float16_t(0.0h)).xxxx, (float16_t(0.0h)).xxxx, (float16_t(0.0h)).xxxx, (float16_t(0.0h)).xxxx));
  float3 v_29[2] = (float3[2])0;
  v_9(736u, v_29);
  matrix<float16_t, 4, 2> v_30[2] = (matrix<float16_t, 4, 2>[2])0;
  v_6(768u, v_30);
  Inner v_31 = (Inner)0;
  v(800u, v_31);
  Inner v_32[4] = (Inner[4])0;
  v_1(812u, v_32);
}

FXC validation failure:
<scrubbed_path>(4,3-11): error X3000: unrecognized identifier 'float16_t'


tint executable returned error: exit status 1
