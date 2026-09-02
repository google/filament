// RUN: %dxc -E main -T ps_6_0 -HV 2018 %s | FileCheck -check-prefix=DISABLED %s
// RUN: %dxc -E main -T ps_6_0 -HV 2021 %s | FileCheck -check-prefix=ENABLED %s

// DISABLED: error: 'template' is a reserved keyword in HLSL
// ENABLED: define void @main()

template<typename T>
T f(T a) {
  return a + 1;
};

int main(int a:A) : SV_Target {
   return f(a);
}
