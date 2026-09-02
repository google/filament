// RUN: %dxc -E main -T vs_6_0 %s | FileCheck %s

// CHECK: use of undeclared identifier

float x;
float a = (x < b) ? c : d;
