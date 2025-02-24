[numthreads(1, 1, 1)]
void main() {
  float val = 2.0f;
  float subadd = (WavePrefixSum(val) + val);
  float submul = (WavePrefixProduct(val) * val);
  return;
}
