SKIP: INVALID

struct main_inputs {
  uint idx : SV_GroupIndex;
};


RWByteAddressBuffer sb : register(u0);
void v(uint offset, matrix<float16_t, 4, 2> obj) {
  sb.Store<vector<float16_t, 2> >((offset + 0u), obj[0u]);
  sb.Store<vector<float16_t, 2> >((offset + 4u), obj[1u]);
  sb.Store<vector<float16_t, 2> >((offset + 8u), obj[2u]);
  sb.Store<vector<float16_t, 2> >((offset + 12u), obj[3u]);
}

void v_1(uint offset, matrix<float16_t, 4, 2> obj[2]) {
  {
    uint v_2 = 0u;
    v_2 = 0u;
    while(true) {
      uint v_3 = v_2;
      if ((v_3 >= 2u)) {
        break;
      }
      v((offset + (v_3 * 16u)), obj[v_3]);
      {
        v_2 = (v_3 + 1u);
      }
      continue;
    }
  }
}

void v_4(uint offset, float3 obj[2]) {
  {
    uint v_5 = 0u;
    v_5 = 0u;
    while(true) {
      uint v_6 = v_5;
      if ((v_6 >= 2u)) {
        break;
      }
      sb.Store3((offset + (v_6 * 16u)), asuint(obj[v_6]));
      {
        v_5 = (v_6 + 1u);
      }
      continue;
    }
  }
}

void v_7(uint offset, matrix<float16_t, 4, 4> obj) {
  sb.Store<vector<float16_t, 4> >((offset + 0u), obj[0u]);
  sb.Store<vector<float16_t, 4> >((offset + 8u), obj[1u]);
  sb.Store<vector<float16_t, 4> >((offset + 16u), obj[2u]);
  sb.Store<vector<float16_t, 4> >((offset + 24u), obj[3u]);
}

void v_8(uint offset, matrix<float16_t, 4, 3> obj) {
  sb.Store<vector<float16_t, 3> >((offset + 0u), obj[0u]);
  sb.Store<vector<float16_t, 3> >((offset + 8u), obj[1u]);
  sb.Store<vector<float16_t, 3> >((offset + 16u), obj[2u]);
  sb.Store<vector<float16_t, 3> >((offset + 24u), obj[3u]);
}

void v_9(uint offset, matrix<float16_t, 3, 4> obj) {
  sb.Store<vector<float16_t, 4> >((offset + 0u), obj[0u]);
  sb.Store<vector<float16_t, 4> >((offset + 8u), obj[1u]);
  sb.Store<vector<float16_t, 4> >((offset + 16u), obj[2u]);
}

void v_10(uint offset, matrix<float16_t, 3, 3> obj) {
  sb.Store<vector<float16_t, 3> >((offset + 0u), obj[0u]);
  sb.Store<vector<float16_t, 3> >((offset + 8u), obj[1u]);
  sb.Store<vector<float16_t, 3> >((offset + 16u), obj[2u]);
}

void v_11(uint offset, matrix<float16_t, 3, 2> obj) {
  sb.Store<vector<float16_t, 2> >((offset + 0u), obj[0u]);
  sb.Store<vector<float16_t, 2> >((offset + 4u), obj[1u]);
  sb.Store<vector<float16_t, 2> >((offset + 8u), obj[2u]);
}

void v_12(uint offset, matrix<float16_t, 2, 4> obj) {
  sb.Store<vector<float16_t, 4> >((offset + 0u), obj[0u]);
  sb.Store<vector<float16_t, 4> >((offset + 8u), obj[1u]);
}

void v_13(uint offset, matrix<float16_t, 2, 3> obj) {
  sb.Store<vector<float16_t, 3> >((offset + 0u), obj[0u]);
  sb.Store<vector<float16_t, 3> >((offset + 8u), obj[1u]);
}

void v_14(uint offset, matrix<float16_t, 2, 2> obj) {
  sb.Store<vector<float16_t, 2> >((offset + 0u), obj[0u]);
  sb.Store<vector<float16_t, 2> >((offset + 4u), obj[1u]);
}

void v_15(uint offset, float4x4 obj) {
  sb.Store4((offset + 0u), asuint(obj[0u]));
  sb.Store4((offset + 16u), asuint(obj[1u]));
  sb.Store4((offset + 32u), asuint(obj[2u]));
  sb.Store4((offset + 48u), asuint(obj[3u]));
}

void v_16(uint offset, float4x3 obj) {
  sb.Store3((offset + 0u), asuint(obj[0u]));
  sb.Store3((offset + 16u), asuint(obj[1u]));
  sb.Store3((offset + 32u), asuint(obj[2u]));
  sb.Store3((offset + 48u), asuint(obj[3u]));
}

void v_17(uint offset, float4x2 obj) {
  sb.Store2((offset + 0u), asuint(obj[0u]));
  sb.Store2((offset + 8u), asuint(obj[1u]));
  sb.Store2((offset + 16u), asuint(obj[2u]));
  sb.Store2((offset + 24u), asuint(obj[3u]));
}

void v_18(uint offset, float3x4 obj) {
  sb.Store4((offset + 0u), asuint(obj[0u]));
  sb.Store4((offset + 16u), asuint(obj[1u]));
  sb.Store4((offset + 32u), asuint(obj[2u]));
}

void v_19(uint offset, float3x3 obj) {
  sb.Store3((offset + 0u), asuint(obj[0u]));
  sb.Store3((offset + 16u), asuint(obj[1u]));
  sb.Store3((offset + 32u), asuint(obj[2u]));
}

void v_20(uint offset, float3x2 obj) {
  sb.Store2((offset + 0u), asuint(obj[0u]));
  sb.Store2((offset + 8u), asuint(obj[1u]));
  sb.Store2((offset + 16u), asuint(obj[2u]));
}

void v_21(uint offset, float2x4 obj) {
  sb.Store4((offset + 0u), asuint(obj[0u]));
  sb.Store4((offset + 16u), asuint(obj[1u]));
}

void v_22(uint offset, float2x3 obj) {
  sb.Store3((offset + 0u), asuint(obj[0u]));
  sb.Store3((offset + 16u), asuint(obj[1u]));
}

void v_23(uint offset, float2x2 obj) {
  sb.Store2((offset + 0u), asuint(obj[0u]));
  sb.Store2((offset + 8u), asuint(obj[1u]));
}

void main_inner(uint idx) {
  uint v_24 = (0u + (uint(idx) * 800u));
  sb.Store(v_24, asuint(0.0f));
  uint v_25 = (4u + (uint(idx) * 800u));
  sb.Store(v_25, asuint(int(0)));
  sb.Store((8u + (uint(idx) * 800u)), 0u);
  sb.Store<float16_t>((12u + (uint(idx) * 800u)), float16_t(0.0h));
  uint v_26 = (16u + (uint(idx) * 800u));
  sb.Store2(v_26, asuint((0.0f).xx));
  uint v_27 = (24u + (uint(idx) * 800u));
  sb.Store2(v_27, asuint(int2((int(0)).xx)));
  sb.Store2((32u + (uint(idx) * 800u)), (0u).xx);
  sb.Store<vector<float16_t, 2> >((40u + (uint(idx) * 800u)), (float16_t(0.0h)).xx);
  uint v_28 = (48u + (uint(idx) * 800u));
  sb.Store3(v_28, asuint((0.0f).xxx));
  uint v_29 = (64u + (uint(idx) * 800u));
  sb.Store3(v_29, asuint(int3((int(0)).xxx)));
  sb.Store3((80u + (uint(idx) * 800u)), (0u).xxx);
  sb.Store<vector<float16_t, 3> >((96u + (uint(idx) * 800u)), (float16_t(0.0h)).xxx);
  uint v_30 = (112u + (uint(idx) * 800u));
  sb.Store4(v_30, asuint((0.0f).xxxx));
  uint v_31 = (128u + (uint(idx) * 800u));
  sb.Store4(v_31, asuint(int4((int(0)).xxxx)));
  sb.Store4((144u + (uint(idx) * 800u)), (0u).xxxx);
  sb.Store<vector<float16_t, 4> >((160u + (uint(idx) * 800u)), (float16_t(0.0h)).xxxx);
  v_23((168u + (uint(idx) * 800u)), float2x2((0.0f).xx, (0.0f).xx));
  v_22((192u + (uint(idx) * 800u)), float2x3((0.0f).xxx, (0.0f).xxx));
  v_21((224u + (uint(idx) * 800u)), float2x4((0.0f).xxxx, (0.0f).xxxx));
  v_20((256u + (uint(idx) * 800u)), float3x2((0.0f).xx, (0.0f).xx, (0.0f).xx));
  v_19((288u + (uint(idx) * 800u)), float3x3((0.0f).xxx, (0.0f).xxx, (0.0f).xxx));
  v_18((336u + (uint(idx) * 800u)), float3x4((0.0f).xxxx, (0.0f).xxxx, (0.0f).xxxx));
  v_17((384u + (uint(idx) * 800u)), float4x2((0.0f).xx, (0.0f).xx, (0.0f).xx, (0.0f).xx));
  v_16((416u + (uint(idx) * 800u)), float4x3((0.0f).xxx, (0.0f).xxx, (0.0f).xxx, (0.0f).xxx));
  v_15((480u + (uint(idx) * 800u)), float4x4((0.0f).xxxx, (0.0f).xxxx, (0.0f).xxxx, (0.0f).xxxx));
  v_14((544u + (uint(idx) * 800u)), matrix<float16_t, 2, 2>((float16_t(0.0h)).xx, (float16_t(0.0h)).xx));
  v_13((552u + (uint(idx) * 800u)), matrix<float16_t, 2, 3>((float16_t(0.0h)).xxx, (float16_t(0.0h)).xxx));
  v_12((568u + (uint(idx) * 800u)), matrix<float16_t, 2, 4>((float16_t(0.0h)).xxxx, (float16_t(0.0h)).xxxx));
  v_11((584u + (uint(idx) * 800u)), matrix<float16_t, 3, 2>((float16_t(0.0h)).xx, (float16_t(0.0h)).xx, (float16_t(0.0h)).xx));
  v_10((600u + (uint(idx) * 800u)), matrix<float16_t, 3, 3>((float16_t(0.0h)).xxx, (float16_t(0.0h)).xxx, (float16_t(0.0h)).xxx));
  v_9((624u + (uint(idx) * 800u)), matrix<float16_t, 3, 4>((float16_t(0.0h)).xxxx, (float16_t(0.0h)).xxxx, (float16_t(0.0h)).xxxx));
  v((648u + (uint(idx) * 800u)), matrix<float16_t, 4, 2>((float16_t(0.0h)).xx, (float16_t(0.0h)).xx, (float16_t(0.0h)).xx, (float16_t(0.0h)).xx));
  v_8((664u + (uint(idx) * 800u)), matrix<float16_t, 4, 3>((float16_t(0.0h)).xxx, (float16_t(0.0h)).xxx, (float16_t(0.0h)).xxx, (float16_t(0.0h)).xxx));
  v_7((696u + (uint(idx) * 800u)), matrix<float16_t, 4, 4>((float16_t(0.0h)).xxxx, (float16_t(0.0h)).xxxx, (float16_t(0.0h)).xxxx, (float16_t(0.0h)).xxxx));
  float3 v_32[2] = (float3[2])0;
  v_4((736u + (uint(idx) * 800u)), v_32);
  matrix<float16_t, 4, 2> v_33[2] = (matrix<float16_t, 4, 2>[2])0;
  v_1((768u + (uint(idx) * 800u)), v_33);
}

[numthreads(1, 1, 1)]
void main(main_inputs inputs) {
  main_inner(inputs.idx);
}

FXC validation failure:
<scrubbed_path>(7,28-36): error X3000: syntax error: unexpected token 'float16_t'
<scrubbed_path>(8,3-10): error X3018: invalid subscript 'Store'


tint executable returned error: exit status 1
