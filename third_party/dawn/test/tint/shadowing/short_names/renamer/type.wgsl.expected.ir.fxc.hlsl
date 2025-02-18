struct tint_struct {
  int tint_member;
};

struct tint_struct_1 {
  float4 tint_member_1 : SV_Position;
};

struct tint_struct_2 {
  uint tint_member_2 : SV_VertexID;
};


float4 v(uint v_1) {
  tint_struct v_2 = {int(1)};
  float v_3 = float(v_2.tint_member);
  bool v_4 = bool(v_3);
  return ((v_4) ? ((1.0f).xxxx) : ((0.0f).xxxx));
}

tint_struct_1 tint_entry_point(tint_struct_2 v_6) {
  tint_struct_1 v_7 = {v(v_6.tint_member_2)};
  return v_7;
}

