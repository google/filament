// RUN: %dxc -E main -T ps_6_0 %s | FileCheck %s

// CHECK: @main
float a;
float4 main() : SV_Target {
   return a;
}


[RootSignature("SRV(t0)")]
float4 main2() : SV_Target {
   return a;
}

