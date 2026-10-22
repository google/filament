// RUN: %dxc -E main -T ps_6_0 -HV 2021 %s | FileCheck %s
// CHECK: error: variadic templates are not supported in HLSL

template<typename T>
T summation (T n) {
  return n;
};

template<typename T, typename... Args>
T summation(T n, Args... args) {
  return n + summation(args...);
};

int main(int a:A) : SV_Target {
   return summation(1,3, 11, 7, a);
}
