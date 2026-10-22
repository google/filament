// RUN: %dxc -E main -T lib_6_3 %s  | FileCheck %s

// Make sure phi with undef created.
// CHECK:phi float {{.*}}undef

float foo(float a, float b);

int c;
float a;

RWBuffer<float> buf;
[shader("compute")]
[numthreads(8,8,1)]
void main() {

  float b;
  int x = c;
  while (x>0) {
    b = foo(b, a);
    buf[x] = b;
    x--;
  }

}