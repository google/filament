struct modf_result_f32 {
  float fract;
  float whole;
};
struct frexp_result_f32 {
  float fract;
  int exp;
};
void deref_modf() {
  modf_result_f32 a = {0.5f, 1.0f};
  float fract = a.fract;
  float whole = a.whole;
}

void no_deref_modf() {
  modf_result_f32 a = {0.5f, 1.0f};
  float fract = a.fract;
  float whole = a.whole;
}

void deref_frexp() {
  frexp_result_f32 a = {0.75f, 1};
  float fract = a.fract;
  int exp = a.exp;
}

void no_deref_frexp() {
  frexp_result_f32 a = {0.75f, 1};
  float fract = a.fract;
  int exp = a.exp;
}

[numthreads(1, 1, 1)]
void main() {
  deref_modf();
  no_deref_modf();
  deref_frexp();
  no_deref_frexp();
  return;
}
