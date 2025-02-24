// RUN: %dxc -Tlib_6_3  -HV 2021 -verify %s

// expected-no-diagnostics
template<typename T, int Sz = 4>
vector<T, Sz> someVec() {}
