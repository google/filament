struct modf_result_f32 {
  float fract;
  float whole;
};
static const modf_result_f32 c = {0.0f, 1.0f};
static modf_result_f32 v = c;

[numthreads(1, 1, 1)]
void main() {
  return;
}
