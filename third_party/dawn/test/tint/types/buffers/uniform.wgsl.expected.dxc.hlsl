cbuffer cbuffer_weights : register(b0) {
  uint4 weights[1];
};

void main() {
  float a = asfloat(weights[0].x);
  return;
}
