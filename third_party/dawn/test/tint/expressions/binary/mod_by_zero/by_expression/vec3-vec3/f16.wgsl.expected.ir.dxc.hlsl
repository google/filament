
[numthreads(1, 1, 1)]
void f() {
  vector<float16_t, 3> a = vector<float16_t, 3>(float16_t(1.0h), float16_t(2.0h), float16_t(3.0h));
  vector<float16_t, 3> b = vector<float16_t, 3>(float16_t(0.0h), float16_t(5.0h), float16_t(0.0h));
  vector<float16_t, 3> v = a;
  vector<float16_t, 3> v_1 = (b + b);
  vector<float16_t, 3> v_2 = (v / v_1);
  vector<float16_t, 3> r = (v - ((((v_2 < (float16_t(0.0h)).xxx)) ? (ceil(v_2)) : (floor(v_2))) * v_1));
}

