struct PixelLocal {
  uint a;
};

struct Out {
  float4 x;
  float4 y;
  float4 z;
};

struct f_outputs {
  float4 Out_x : SV_Target0;
  float4 Out_y : SV_Target2;
  float4 Out_z : SV_Target3;
};

struct f_inputs {
  float4 pos : SV_Position;
};


static PixelLocal P = (PixelLocal)0;
RasterizerOrderedTexture2D<uint4> pixel_local_a : register(u1);
Out f_inner() {
  P.a = (P.a + 42u);
  Out v = {(10.0f).xxxx, (20.0f).xxxx, (30.0f).xxxx};
  return v;
}

f_outputs f(f_inputs inputs) {
  uint2 v_1 = uint2(inputs.pos.xy);
  P.a = pixel_local_a.Load(v_1).x;
  Out v_2 = f_inner();
  f_outputs v_3 = {v_2.x, v_2.y, v_2.z};
  pixel_local_a[v_1] = uint4((P.a).xxxx);
  return v_3;
}

