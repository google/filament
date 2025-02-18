[numthreads(1, 1, 1)]
void main() {
  float low = 1.0f;
  float high = 0.0f;
  float x_val = 0.5f;
  float res = smoothstep(low, high, x_val);
  return;
}
