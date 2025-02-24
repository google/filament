SKIP: INVALID


[numthreads(1, 1, 1)]
void main() {
  float val = 2.0f;
  float subadd = (WavePrefixSum(val) + val);
  float submul = (WavePrefixProduct(val) * val);
}

FXC validation failure:
<scrubbed_path>(5,19-36): error X3004: undeclared identifier 'WavePrefixSum'


tint executable returned error: exit status 1
