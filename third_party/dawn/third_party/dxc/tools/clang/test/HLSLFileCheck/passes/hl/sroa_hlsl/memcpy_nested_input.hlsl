// RUN: %dxc -E main -T vs_6_0 %s | FileCheck %s

// Regression test for GitHub #2277, where parameter SROA
// would misbehave on memcpys and end up causing an undef
// value to be returned.

struct Inner { int x, y; };
struct Outer { Inner inner; };
void bug(inout Inner inner) {}
int main(Outer outer : IN) : OUT
{
  // CHECK: loadInput
  // CHECK: storeOutput
  bug(outer.inner);
  return outer.inner.x;
}