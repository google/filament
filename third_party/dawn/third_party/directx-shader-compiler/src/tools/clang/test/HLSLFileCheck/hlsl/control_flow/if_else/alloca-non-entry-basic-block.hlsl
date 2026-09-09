// RUN: %dxc /Tps_6_0 /Emain  %s | FileCheck %s
// CHECK: define void @main()
// CHECK: entry

float2 foo() {
  return float2(1.0f, -1.0f);
}

[RootSignature("")]
float main() : SV_Target {
  for (int c = 0; c < 2; ++c) {
    if (foo()[c] >= 1.0f) {
      return foo()[c];
    }
  }
  return 1.0f;
}