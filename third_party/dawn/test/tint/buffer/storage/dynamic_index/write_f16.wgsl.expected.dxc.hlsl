RWByteAddressBuffer sb : register(u0);

struct tint_symbol_1 {
  uint idx : SV_GroupIndex;
};

void sb_store_16(uint offset, float2x2 value) {
  sb.Store2((offset + 0u), asuint(value[0u]));
  sb.Store2((offset + 8u), asuint(value[1u]));
}

void sb_store_17(uint offset, float2x3 value) {
  sb.Store3((offset + 0u), asuint(value[0u]));
  sb.Store3((offset + 16u), asuint(value[1u]));
}

void sb_store_18(uint offset, float2x4 value) {
  sb.Store4((offset + 0u), asuint(value[0u]));
  sb.Store4((offset + 16u), asuint(value[1u]));
}

void sb_store_19(uint offset, float3x2 value) {
  sb.Store2((offset + 0u), asuint(value[0u]));
  sb.Store2((offset + 8u), asuint(value[1u]));
  sb.Store2((offset + 16u), asuint(value[2u]));
}

void sb_store_20(uint offset, float3x3 value) {
  sb.Store3((offset + 0u), asuint(value[0u]));
  sb.Store3((offset + 16u), asuint(value[1u]));
  sb.Store3((offset + 32u), asuint(value[2u]));
}

void sb_store_21(uint offset, float3x4 value) {
  sb.Store4((offset + 0u), asuint(value[0u]));
  sb.Store4((offset + 16u), asuint(value[1u]));
  sb.Store4((offset + 32u), asuint(value[2u]));
}

void sb_store_22(uint offset, float4x2 value) {
  sb.Store2((offset + 0u), asuint(value[0u]));
  sb.Store2((offset + 8u), asuint(value[1u]));
  sb.Store2((offset + 16u), asuint(value[2u]));
  sb.Store2((offset + 24u), asuint(value[3u]));
}

void sb_store_23(uint offset, float4x3 value) {
  sb.Store3((offset + 0u), asuint(value[0u]));
  sb.Store3((offset + 16u), asuint(value[1u]));
  sb.Store3((offset + 32u), asuint(value[2u]));
  sb.Store3((offset + 48u), asuint(value[3u]));
}

void sb_store_24(uint offset, float4x4 value) {
  sb.Store4((offset + 0u), asuint(value[0u]));
  sb.Store4((offset + 16u), asuint(value[1u]));
  sb.Store4((offset + 32u), asuint(value[2u]));
  sb.Store4((offset + 48u), asuint(value[3u]));
}

void sb_store_25(uint offset, matrix<float16_t, 2, 2> value) {
  sb.Store<vector<float16_t, 2> >((offset + 0u), value[0u]);
  sb.Store<vector<float16_t, 2> >((offset + 4u), value[1u]);
}

void sb_store_26(uint offset, matrix<float16_t, 2, 3> value) {
  sb.Store<vector<float16_t, 3> >((offset + 0u), value[0u]);
  sb.Store<vector<float16_t, 3> >((offset + 8u), value[1u]);
}

void sb_store_27(uint offset, matrix<float16_t, 2, 4> value) {
  sb.Store<vector<float16_t, 4> >((offset + 0u), value[0u]);
  sb.Store<vector<float16_t, 4> >((offset + 8u), value[1u]);
}

void sb_store_28(uint offset, matrix<float16_t, 3, 2> value) {
  sb.Store<vector<float16_t, 2> >((offset + 0u), value[0u]);
  sb.Store<vector<float16_t, 2> >((offset + 4u), value[1u]);
  sb.Store<vector<float16_t, 2> >((offset + 8u), value[2u]);
}

void sb_store_29(uint offset, matrix<float16_t, 3, 3> value) {
  sb.Store<vector<float16_t, 3> >((offset + 0u), value[0u]);
  sb.Store<vector<float16_t, 3> >((offset + 8u), value[1u]);
  sb.Store<vector<float16_t, 3> >((offset + 16u), value[2u]);
}

void sb_store_30(uint offset, matrix<float16_t, 3, 4> value) {
  sb.Store<vector<float16_t, 4> >((offset + 0u), value[0u]);
  sb.Store<vector<float16_t, 4> >((offset + 8u), value[1u]);
  sb.Store<vector<float16_t, 4> >((offset + 16u), value[2u]);
}

void sb_store_31(uint offset, matrix<float16_t, 4, 2> value) {
  sb.Store<vector<float16_t, 2> >((offset + 0u), value[0u]);
  sb.Store<vector<float16_t, 2> >((offset + 4u), value[1u]);
  sb.Store<vector<float16_t, 2> >((offset + 8u), value[2u]);
  sb.Store<vector<float16_t, 2> >((offset + 12u), value[3u]);
}

void sb_store_32(uint offset, matrix<float16_t, 4, 3> value) {
  sb.Store<vector<float16_t, 3> >((offset + 0u), value[0u]);
  sb.Store<vector<float16_t, 3> >((offset + 8u), value[1u]);
  sb.Store<vector<float16_t, 3> >((offset + 16u), value[2u]);
  sb.Store<vector<float16_t, 3> >((offset + 24u), value[3u]);
}

void sb_store_33(uint offset, matrix<float16_t, 4, 4> value) {
  sb.Store<vector<float16_t, 4> >((offset + 0u), value[0u]);
  sb.Store<vector<float16_t, 4> >((offset + 8u), value[1u]);
  sb.Store<vector<float16_t, 4> >((offset + 16u), value[2u]);
  sb.Store<vector<float16_t, 4> >((offset + 24u), value[3u]);
}

void sb_store_34(uint offset, float3 value[2]) {
  float3 array_1[2] = value;
  {
    for(uint i = 0u; (i < 2u); i = (i + 1u)) {
      sb.Store3((offset + (i * 16u)), asuint(array_1[i]));
    }
  }
}

void sb_store_35(uint offset, matrix<float16_t, 4, 2> value[2]) {
  matrix<float16_t, 4, 2> array_2[2] = value;
  {
    for(uint i_1 = 0u; (i_1 < 2u); i_1 = (i_1 + 1u)) {
      sb_store_31((offset + (i_1 * 16u)), array_2[i_1]);
    }
  }
}

void main_inner(uint idx) {
  uint tint_symbol_3 = 0u;
  sb.GetDimensions(tint_symbol_3);
  uint tint_symbol_4 = ((tint_symbol_3 - 0u) / 800u);
  sb.Store((800u * min(idx, (tint_symbol_4 - 1u))), asuint(0.0f));
  sb.Store(((800u * min(idx, (tint_symbol_4 - 1u))) + 4u), asuint(0));
  sb.Store(((800u * min(idx, (tint_symbol_4 - 1u))) + 8u), asuint(0u));
  sb.Store<float16_t>(((800u * min(idx, (tint_symbol_4 - 1u))) + 12u), float16_t(0.0h));
  sb.Store2(((800u * min(idx, (tint_symbol_4 - 1u))) + 16u), asuint((0.0f).xx));
  sb.Store2(((800u * min(idx, (tint_symbol_4 - 1u))) + 24u), asuint(int2((0).xx)));
  sb.Store2(((800u * min(idx, (tint_symbol_4 - 1u))) + 32u), asuint((0u).xx));
  sb.Store<vector<float16_t, 2> >(((800u * min(idx, (tint_symbol_4 - 1u))) + 40u), (float16_t(0.0h)).xx);
  sb.Store3(((800u * min(idx, (tint_symbol_4 - 1u))) + 48u), asuint((0.0f).xxx));
  sb.Store3(((800u * min(idx, (tint_symbol_4 - 1u))) + 64u), asuint(int3((0).xxx)));
  sb.Store3(((800u * min(idx, (tint_symbol_4 - 1u))) + 80u), asuint((0u).xxx));
  sb.Store<vector<float16_t, 3> >(((800u * min(idx, (tint_symbol_4 - 1u))) + 96u), (float16_t(0.0h)).xxx);
  sb.Store4(((800u * min(idx, (tint_symbol_4 - 1u))) + 112u), asuint((0.0f).xxxx));
  sb.Store4(((800u * min(idx, (tint_symbol_4 - 1u))) + 128u), asuint(int4((0).xxxx)));
  sb.Store4(((800u * min(idx, (tint_symbol_4 - 1u))) + 144u), asuint((0u).xxxx));
  sb.Store<vector<float16_t, 4> >(((800u * min(idx, (tint_symbol_4 - 1u))) + 160u), (float16_t(0.0h)).xxxx);
  sb_store_16(((800u * min(idx, (tint_symbol_4 - 1u))) + 168u), float2x2((0.0f).xx, (0.0f).xx));
  sb_store_17(((800u * min(idx, (tint_symbol_4 - 1u))) + 192u), float2x3((0.0f).xxx, (0.0f).xxx));
  sb_store_18(((800u * min(idx, (tint_symbol_4 - 1u))) + 224u), float2x4((0.0f).xxxx, (0.0f).xxxx));
  sb_store_19(((800u * min(idx, (tint_symbol_4 - 1u))) + 256u), float3x2((0.0f).xx, (0.0f).xx, (0.0f).xx));
  sb_store_20(((800u * min(idx, (tint_symbol_4 - 1u))) + 288u), float3x3((0.0f).xxx, (0.0f).xxx, (0.0f).xxx));
  sb_store_21(((800u * min(idx, (tint_symbol_4 - 1u))) + 336u), float3x4((0.0f).xxxx, (0.0f).xxxx, (0.0f).xxxx));
  sb_store_22(((800u * min(idx, (tint_symbol_4 - 1u))) + 384u), float4x2((0.0f).xx, (0.0f).xx, (0.0f).xx, (0.0f).xx));
  sb_store_23(((800u * min(idx, (tint_symbol_4 - 1u))) + 416u), float4x3((0.0f).xxx, (0.0f).xxx, (0.0f).xxx, (0.0f).xxx));
  sb_store_24(((800u * min(idx, (tint_symbol_4 - 1u))) + 480u), float4x4((0.0f).xxxx, (0.0f).xxxx, (0.0f).xxxx, (0.0f).xxxx));
  sb_store_25(((800u * min(idx, (tint_symbol_4 - 1u))) + 544u), matrix<float16_t, 2, 2>((float16_t(0.0h)).xx, (float16_t(0.0h)).xx));
  sb_store_26(((800u * min(idx, (tint_symbol_4 - 1u))) + 552u), matrix<float16_t, 2, 3>((float16_t(0.0h)).xxx, (float16_t(0.0h)).xxx));
  sb_store_27(((800u * min(idx, (tint_symbol_4 - 1u))) + 568u), matrix<float16_t, 2, 4>((float16_t(0.0h)).xxxx, (float16_t(0.0h)).xxxx));
  sb_store_28(((800u * min(idx, (tint_symbol_4 - 1u))) + 584u), matrix<float16_t, 3, 2>((float16_t(0.0h)).xx, (float16_t(0.0h)).xx, (float16_t(0.0h)).xx));
  sb_store_29(((800u * min(idx, (tint_symbol_4 - 1u))) + 600u), matrix<float16_t, 3, 3>((float16_t(0.0h)).xxx, (float16_t(0.0h)).xxx, (float16_t(0.0h)).xxx));
  sb_store_30(((800u * min(idx, (tint_symbol_4 - 1u))) + 624u), matrix<float16_t, 3, 4>((float16_t(0.0h)).xxxx, (float16_t(0.0h)).xxxx, (float16_t(0.0h)).xxxx));
  sb_store_31(((800u * min(idx, (tint_symbol_4 - 1u))) + 648u), matrix<float16_t, 4, 2>((float16_t(0.0h)).xx, (float16_t(0.0h)).xx, (float16_t(0.0h)).xx, (float16_t(0.0h)).xx));
  sb_store_32(((800u * min(idx, (tint_symbol_4 - 1u))) + 664u), matrix<float16_t, 4, 3>((float16_t(0.0h)).xxx, (float16_t(0.0h)).xxx, (float16_t(0.0h)).xxx, (float16_t(0.0h)).xxx));
  sb_store_33(((800u * min(idx, (tint_symbol_4 - 1u))) + 696u), matrix<float16_t, 4, 4>((float16_t(0.0h)).xxxx, (float16_t(0.0h)).xxxx, (float16_t(0.0h)).xxxx, (float16_t(0.0h)).xxxx));
  float3 tint_symbol_5[2] = (float3[2])0;
  sb_store_34(((800u * min(idx, (tint_symbol_4 - 1u))) + 736u), tint_symbol_5);
  matrix<float16_t, 4, 2> tint_symbol_6[2] = (matrix<float16_t, 4, 2>[2])0;
  sb_store_35(((800u * min(idx, (tint_symbol_4 - 1u))) + 768u), tint_symbol_6);
}

[numthreads(1, 1, 1)]
void main(tint_symbol_1 tint_symbol) {
  main_inner(tint_symbol.idx);
  return;
}
