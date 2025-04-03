// RUN: %dxc -T lib_6_8 %s -verify

// expected-no-diagnostics
using namespace dx;

[shader("raygeneration")]
void main() {
}
