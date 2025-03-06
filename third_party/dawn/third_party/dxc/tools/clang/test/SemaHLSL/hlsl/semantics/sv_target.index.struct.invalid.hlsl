// RUN: %dxc -T ps_6_0 -verify %s

struct S {
  float4 t : SV_Target12345; /* expected-error{{'SV_Target12345' is defined with semantic index 12345, but only values 0 through 7 are supported}} */
};

S main() {
    return S(float4(0, 1, 2, 3));
}
