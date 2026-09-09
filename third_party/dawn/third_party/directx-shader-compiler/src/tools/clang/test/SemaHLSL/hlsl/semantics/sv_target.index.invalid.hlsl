// RUN: %dxc -T ps_6_0 -verify %s

float4 bad() : SV_Target8 { /* expected-error{{'SV_Target8' is defined with semantic index 8, but only values 0 through 7 are supported}} */
    return float4(0, 1, 2, 3);
}
