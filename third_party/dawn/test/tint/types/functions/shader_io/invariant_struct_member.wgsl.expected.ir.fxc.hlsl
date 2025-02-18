struct Out {
  float4 pos;
};

struct main_outputs {
  precise float4 Out_pos : SV_Position;
};


Out main_inner() {
  Out v = (Out)0;
  return v;
}

main_outputs main() {
  Out v_1 = main_inner();
  main_outputs v_2 = {v_1.pos};
  return v_2;
}

