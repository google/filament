// RUN: %dxc -E RS -T rootsig_1_0 %s
// Test root signature compilation from expanded macro.

#define YYY "DescriptorTable" "(SRV(t3))"
#define RS YYY
