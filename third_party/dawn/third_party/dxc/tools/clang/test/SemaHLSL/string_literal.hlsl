// RUN: %dxc -T cs_6_0 -verify %s

template<typename T>
void take_string(T s) {
}

[numthreads(1, 1, 1)]
void main() {
  take_string("toto"); /* expected-error {{string literals parameters are not supported in HLSL}} */

  // Fine.
  printf("toto");
}
