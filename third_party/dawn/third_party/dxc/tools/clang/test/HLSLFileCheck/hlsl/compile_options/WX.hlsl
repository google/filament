// RUN: %dxc -E main -T ps_6_0 -WX %s | FileCheck %s

// CHECK: error: 'min10float' is promoted to 'min16float'

float4 main(min10float4 a:A) : SV_Target { // expected-error {{min12int is promoted to min16int}}
  return a;
}
