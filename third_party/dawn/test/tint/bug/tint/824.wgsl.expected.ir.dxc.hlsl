struct Output {
  float4 Position;
  float4 color;
};

struct main_outputs {
  float4 Output_color : TEXCOORD0;
  float4 Output_Position : SV_Position;
};

struct main_inputs {
  uint VertexIndex : SV_VertexID;
  uint InstanceIndex : SV_InstanceID;
};


Output main_inner(uint VertexIndex, uint InstanceIndex) {
  float2 zv[4] = {(0.20000000298023223877f).xx, (0.30000001192092895508f).xx, (-0.10000000149011611938f).xx, (1.10000002384185791016f).xx};
  float z = zv[min(InstanceIndex, 3u)].x;
  Output output = (Output)0;
  output.Position = float4(0.5f, 0.5f, z, 1.0f);
  float4 colors[4] = {float4(1.0f, 0.0f, 0.0f, 1.0f), float4(0.0f, 1.0f, 0.0f, 1.0f), float4(0.0f, 0.0f, 1.0f, 1.0f), (1.0f).xxxx};
  output.color = colors[min(InstanceIndex, 3u)];
  Output v = output;
  return v;
}

main_outputs main(main_inputs inputs) {
  Output v_1 = main_inner(inputs.VertexIndex, inputs.InstanceIndex);
  main_outputs v_2 = {v_1.color, v_1.Position};
  return v_2;
}

