
[numthreads(1, 1, 1)]
void f() {
  float a = 1.0f;
  float b = 0.0f;
  float v = (a / b);
  float r = (a - ((((v < 0.0f)) ? (ceil(v)) : (floor(v))) * b));
}

