struct main_outputs {
  float4 tint_symbol : SV_Position;
};

struct main_inputs {
  uint gl_VertexIndex : SV_VertexID;
};


cbuffer cbuffer_x_20 : register(b0) {
  uint4 x_20[1];
};
cbuffer cbuffer_x_26 : register(b0, space1) {
  uint4 x_26[1];
};
float2x2 v(uint start_byte_offset) {
  uint4 v_1 = x_26[(start_byte_offset / 16u)];
  float2 v_2 = asfloat((((((start_byte_offset % 16u) / 4u) == 2u)) ? (v_1.zw) : (v_1.xy)));
  uint4 v_3 = x_26[((8u + start_byte_offset) / 16u)];
  return float2x2(v_2, asfloat(((((((8u + start_byte_offset) % 16u) / 4u) == 2u)) ? (v_3.zw) : (v_3.xy))));
}

float2x2 v_4(uint start_byte_offset) {
  uint4 v_5 = x_20[(start_byte_offset / 16u)];
  float2 v_6 = asfloat((((((start_byte_offset % 16u) / 4u) == 2u)) ? (v_5.zw) : (v_5.xy)));
  uint4 v_7 = x_20[((8u + start_byte_offset) / 16u)];
  return float2x2(v_6, asfloat(((((((8u + start_byte_offset) % 16u) / 4u) == 2u)) ? (v_7.zw) : (v_7.xy))));
}

float4 main_inner(uint gl_VertexIndex) {
  float2 indexable[3] = (float2[3])0;
  float2x2 x_23 = v_4(0u);
  float2x2 x_28 = v(0u);
  uint x_46 = gl_VertexIndex;
  float2 v_8[3] = {float2(-1.0f, 1.0f), (1.0f).xx, (-1.0f).xx};
  indexable = v_8;
  float2 x_51 = indexable[min(x_46, 2u)];
  float2 x_52 = mul(x_51, float2x2((x_23[0u] + x_28[0u]), (x_23[1u] + x_28[1u])));
  return float4(x_52.x, x_52.y, 0.0f, 1.0f);
}

main_outputs main(main_inputs inputs) {
  main_outputs v_9 = {main_inner(inputs.gl_VertexIndex)};
  return v_9;
}

