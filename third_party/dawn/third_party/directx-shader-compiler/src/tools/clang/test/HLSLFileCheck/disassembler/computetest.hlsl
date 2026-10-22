// RUN: %dxc /T cs_6_0 /E main %s | FileCheck %s
// CHECK: Compute Shader
// CHECK: NumThreads=(2,2,1)

[NumThreads(2,2,1)]
void  main() {
  int x = 2;
}

[NumThreads(2,4,1)]
void  ASMain() {
  int x = 2;
}

[NumThreads(2,3,1)]
void  MSMain() {
  int x = 2;
}


