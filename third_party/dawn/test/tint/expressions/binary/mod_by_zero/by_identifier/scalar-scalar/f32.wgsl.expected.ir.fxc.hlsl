
[numthreads(1, 1, 1)]
void f() {
  float a = 1.0f;
  float b = 0.0f;
  float v = a;
  float v_1 = b;
  float v_2 = (v / v_1);
  float r = (v - ((((v_2 < 0.0f)) ? (ceil(v_2)) : (floor(v_2))) * v_1));
}

