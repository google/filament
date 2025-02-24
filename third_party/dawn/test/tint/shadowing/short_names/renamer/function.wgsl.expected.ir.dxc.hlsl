struct tint_struct {
  float4 tint_member : SV_Position;
};

struct tint_struct_1 {
  uint tint_member_1 : SV_VertexID;
};


int v() {
  return int(0);
}

float v_1(int v_2) {
  return float(v_2);
}

bool v_3(float v_4) {
  return bool(v_4);
}

float4 v_5(uint v_6) {
  return ((v_3(v_1(v()))) ? ((1.0f).xxxx) : ((0.0f).xxxx));
}

tint_struct tint_entry_point(tint_struct_1 v_8) {
  tint_struct v_9 = {v_5(v_8.tint_member_1)};
  return v_9;
}

