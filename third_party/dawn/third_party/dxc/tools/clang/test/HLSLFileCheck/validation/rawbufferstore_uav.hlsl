// RUN: %dxilver 1.5 | %dxc -T vs_6_2 -E main %s | FileCheck %s

// CHECK: error: cannot assign to return value because function 'operator[]<const int &>' returns a const value

StructuredBuffer<int> buf;
void main() { buf[0] = 0; }