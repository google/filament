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
  uint v_24 = 0u;
  sb.GetDimensions(v_24);
  sb.Store((0u + (min(idx, ((v_24 / 800u) - 1u)) * 800u)), asuint(0.0f));
  uint v_25 = 0u;
  sb.GetDimensions(v_25);
  sb.Store((4u + (min(idx, ((v_25 / 800u) - 1u)) * 800u)), asuint(int(0)));
  uint v_26 = 0u;
  sb.GetDimensions(v_26);
  sb.Store((8u + (min(idx, ((v_26 / 800u) - 1u)) * 800u)), 0u);
  uint v_27 = 0u;
  sb.GetDimensions(v_27);
  sb.Store<float16_t>((12u + (min(idx, ((v_27 / 800u) - 1u)) * 800u)), float16_t(0.0h));
  uint v_28 = 0u;
  sb.GetDimensions(v_28);
  sb.Store2((16u + (min(idx, ((v_28 / 800u) - 1u)) * 800u)), asuint((0.0f).xx));
  uint v_29 = 0u;
  sb.GetDimensions(v_29);
  uint v_30 = (24u + (min(idx, ((v_29 / 800u) - 1u)) * 800u));
  sb.Store2(v_30, asuint(int2((int(0)).xx)));
  uint v_31 = 0u;
  sb.GetDimensions(v_31);
  sb.Store2((32u + (min(idx, ((v_31 / 800u) - 1u)) * 800u)), (0u).xx);
  uint v_32 = 0u;
  sb.GetDimensions(v_32);
  sb.Store<vector<float16_t, 2> >((40u + (min(idx, ((v_32 / 800u) - 1u)) * 800u)), (float16_t(0.0h)).xx);
  uint v_33 = 0u;
  sb.GetDimensions(v_33);
  sb.Store3((48u + (min(idx, ((v_33 / 800u) - 1u)) * 800u)), asuint((0.0f).xxx));
  uint v_34 = 0u;
  sb.GetDimensions(v_34);
  uint v_35 = (64u + (min(idx, ((v_34 / 800u) - 1u)) * 800u));
  sb.Store3(v_35, asuint(int3((int(0)).xxx)));
  uint v_36 = 0u;
  sb.GetDimensions(v_36);
  sb.Store3((80u + (min(idx, ((v_36 / 800u) - 1u)) * 800u)), (0u).xxx);
  uint v_37 = 0u;
  sb.GetDimensions(v_37);
  sb.Store<vector<float16_t, 3> >((96u + (min(idx, ((v_37 / 800u) - 1u)) * 800u)), (float16_t(0.0h)).xxx);
  uint v_38 = 0u;
  sb.GetDimensions(v_38);
  sb.Store4((112u + (min(idx, ((v_38 / 800u) - 1u)) * 800u)), asuint((0.0f).xxxx));
  uint v_39 = 0u;
  sb.GetDimensions(v_39);
  uint v_40 = (128u + (min(idx, ((v_39 / 800u) - 1u)) * 800u));
  sb.Store4(v_40, asuint(int4((int(0)).xxxx)));
  uint v_41 = 0u;
  sb.GetDimensions(v_41);
  sb.Store4((144u + (min(idx, ((v_41 / 800u) - 1u)) * 800u)), (0u).xxxx);
  uint v_42 = 0u;
  sb.GetDimensions(v_42);
  sb.Store<vector<float16_t, 4> >((160u + (min(idx, ((v_42 / 800u) - 1u)) * 800u)), (float16_t(0.0h)).xxxx);
  uint v_43 = 0u;
  sb.GetDimensions(v_43);
  v_23((168u + (min(idx, ((v_43 / 800u) - 1u)) * 800u)), float2x2((0.0f).xx, (0.0f).xx));
  uint v_44 = 0u;
  sb.GetDimensions(v_44);
  v_22((192u + (min(idx, ((v_44 / 800u) - 1u)) * 800u)), float2x3((0.0f).xxx, (0.0f).xxx));
  uint v_45 = 0u;
  sb.GetDimensions(v_45);
  v_21((224u + (min(idx, ((v_45 / 800u) - 1u)) * 800u)), float2x4((0.0f).xxxx, (0.0f).xxxx));
  uint v_46 = 0u;
  sb.GetDimensions(v_46);
  v_20((256u + (min(idx, ((v_46 / 800u) - 1u)) * 800u)), float3x2((0.0f).xx, (0.0f).xx, (0.0f).xx));
  uint v_47 = 0u;
  sb.GetDimensions(v_47);
  v_19((288u + (min(idx, ((v_47 / 800u) - 1u)) * 800u)), float3x3((0.0f).xxx, (0.0f).xxx, (0.0f).xxx));
  uint v_48 = 0u;
  sb.GetDimensions(v_48);
  v_18((336u + (min(idx, ((v_48 / 800u) - 1u)) * 800u)), float3x4((0.0f).xxxx, (0.0f).xxxx, (0.0f).xxxx));
  uint v_49 = 0u;
  sb.GetDimensions(v_49);
  v_17((384u + (min(idx, ((v_49 / 800u) - 1u)) * 800u)), float4x2((0.0f).xx, (0.0f).xx, (0.0f).xx, (0.0f).xx));
  uint v_50 = 0u;
  sb.GetDimensions(v_50);
  v_16((416u + (min(idx, ((v_50 / 800u) - 1u)) * 800u)), float4x3((0.0f).xxx, (0.0f).xxx, (0.0f).xxx, (0.0f).xxx));
  uint v_51 = 0u;
  sb.GetDimensions(v_51);
  v_15((480u + (min(idx, ((v_51 / 800u) - 1u)) * 800u)), float4x4((0.0f).xxxx, (0.0f).xxxx, (0.0f).xxxx, (0.0f).xxxx));
  uint v_52 = 0u;
  sb.GetDimensions(v_52);
  v_14((544u + (min(idx, ((v_52 / 800u) - 1u)) * 800u)), matrix<float16_t, 2, 2>((float16_t(0.0h)).xx, (float16_t(0.0h)).xx));
  uint v_53 = 0u;
  sb.GetDimensions(v_53);
  v_13((552u + (min(idx, ((v_53 / 800u) - 1u)) * 800u)), matrix<float16_t, 2, 3>((float16_t(0.0h)).xxx, (float16_t(0.0h)).xxx));
  uint v_54 = 0u;
  sb.GetDimensions(v_54);
  v_12((568u + (min(idx, ((v_54 / 800u) - 1u)) * 800u)), matrix<float16_t, 2, 4>((float16_t(0.0h)).xxxx, (float16_t(0.0h)).xxxx));
  uint v_55 = 0u;
  sb.GetDimensions(v_55);
  v_11((584u + (min(idx, ((v_55 / 800u) - 1u)) * 800u)), matrix<float16_t, 3, 2>((float16_t(0.0h)).xx, (float16_t(0.0h)).xx, (float16_t(0.0h)).xx));
  uint v_56 = 0u;
  sb.GetDimensions(v_56);
  v_10((600u + (min(idx, ((v_56 / 800u) - 1u)) * 800u)), matrix<float16_t, 3, 3>((float16_t(0.0h)).xxx, (float16_t(0.0h)).xxx, (float16_t(0.0h)).xxx));
  uint v_57 = 0u;
  sb.GetDimensions(v_57);
  v_9((624u + (min(idx, ((v_57 / 800u) - 1u)) * 800u)), matrix<float16_t, 3, 4>((float16_t(0.0h)).xxxx, (float16_t(0.0h)).xxxx, (float16_t(0.0h)).xxxx));
  uint v_58 = 0u;
  sb.GetDimensions(v_58);
  v((648u + (min(idx, ((v_58 / 800u) - 1u)) * 800u)), matrix<float16_t, 4, 2>((float16_t(0.0h)).xx, (float16_t(0.0h)).xx, (float16_t(0.0h)).xx, (float16_t(0.0h)).xx));
  uint v_59 = 0u;
  sb.GetDimensions(v_59);
  v_8((664u + (min(idx, ((v_59 / 800u) - 1u)) * 800u)), matrix<float16_t, 4, 3>((float16_t(0.0h)).xxx, (float16_t(0.0h)).xxx, (float16_t(0.0h)).xxx, (float16_t(0.0h)).xxx));
  uint v_60 = 0u;
  sb.GetDimensions(v_60);
  v_7((696u + (min(idx, ((v_60 / 800u) - 1u)) * 800u)), matrix<float16_t, 4, 4>((float16_t(0.0h)).xxxx, (float16_t(0.0h)).xxxx, (float16_t(0.0h)).xxxx, (float16_t(0.0h)).xxxx));
  uint v_61 = 0u;
  sb.GetDimensions(v_61);
  float3 v_62[2] = (float3[2])0;
  v_4((736u + (min(idx, ((v_61 / 800u) - 1u)) * 800u)), v_62);
  uint v_63 = 0u;
  sb.GetDimensions(v_63);
  matrix<float16_t, 4, 2> v_64[2] = (matrix<float16_t, 4, 2>[2])0;
  v_1((768u + (min(idx, ((v_63 / 800u) - 1u)) * 800u)), v_64);
}

[numthreads(1, 1, 1)]
void main(main_inputs inputs) {
  main_inner(inputs.idx);
}

