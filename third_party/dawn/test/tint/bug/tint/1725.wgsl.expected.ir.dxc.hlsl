struct tint_struct {
  uint tint_member : SV_GroupIndex;
};


ByteAddressBuffer v : register(t0);
void v_1(uint v_2) {
  int v_3 = int(0);
  int v_4 = int(0);
  int v_5 = int(0);
  uint v_6 = 0u;
  v.GetDimensions(v_6);
  uint v_7 = v.Load((0u + (min(v_2, ((v_6 / 4u) - 1u)) * 4u)));
}

[numthreads(1, 1, 1)]
void tint_entry_point(tint_struct v_9) {
  v_1(v_9.tint_member);
}

