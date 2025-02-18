struct modf_result_f32 {
  float fract;
  float whole;
};


void foo() {
  modf_result_f32 s1 = (modf_result_f32)0;
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

