// RUN: %dxc -T lib_6_6 %s | FileCheck %s

// A diagnostic is generated for an integer literal that is too large to be
// represented by any integer type - an argument indicates whether the  text
// contains "signed". That argument was missing in HLSL specific code within
// Sema::ActOnNumericConstant() which resulted in an assert being raised if
// the diagnostic was generated in an assert enabled DXC and a random string
// being inserted in a non-assert enabled DXC.

// CHECK: integer literal is too large to be represented in any integer type
int a = 98765432109876543210;

// CHECK: integer literal is too large to be represented in any integer type
uint b = 98765432109876543210U;
