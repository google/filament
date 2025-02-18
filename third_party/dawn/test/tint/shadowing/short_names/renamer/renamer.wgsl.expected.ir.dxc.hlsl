struct tint_struct {
  float4 tint_member : SV_Position;
};

struct tint_struct_1 {
  uint tint_member_1 : SV_VertexID;
};


float4 v(uint v_1) {
  return float4(0.0f, 0.0f, 0.0f, 1.0f);
}

tint_struct tint_entry_point(tint_struct_1 v_3) {
  tint_struct v_4 = {v(v_3.tint_member_1)};
  return v_4;
}

