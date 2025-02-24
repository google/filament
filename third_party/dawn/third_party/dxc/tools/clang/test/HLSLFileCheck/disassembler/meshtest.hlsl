// RUN: %dxc /T ms_6_6 /E MSMain1 %s | FileCheck %s -check-prefix=CHECK1
// CHECK1: Mesh Shader
// CHECK1: MeshOutputTopology=triangle
// CHECK1: NumThreads=(2,3,1)

// RUN: %dxc /T ms_6_6 /E MSMain2 %s | FileCheck %s -check-prefix=CHECK2
// CHECK2: Mesh Shader
// CHECK2: MeshOutputTopology=line
// CHECK2: NumThreads=(2,2,1)

// RUN: %dxc /T ms_6_6 /E MSMain3 %s | FileCheck %s -check-prefix=CHECK3
// CHECK3: Mesh Shader
// CHECK3: MeshOutputTopology=undefined
// CHECK3: NumThreads=(2,8,1)

// RUN: %dxc /T ms_6_6 /E MSMain4 %s | FileCheck %s -check-prefix=CHECK4
// CHECK4: Mesh Shader
// CHECK4: MeshOutputTopology=undefined
// CHECK4: NumThreads=(2,8,2)

// RUN: %dxc /T ms_6_6 /E MSMain5 %s | FileCheck %s -check-prefix=CHECK5
// CHECK5: Mesh Shader
// CHECK5: MeshOutputTopology=undefined
// CHECK5: NumThreads=(2,8,2)



[NumThreads(2,3,1)]
[OutputTopology("triangle")] 
void  MSMain1() {
  int x = 2;
}

[NumThreads(2,2,1)]
[OutputTopology("line")] 
void  MSMain2() {
  int x = 2;
}

[NumThreads(2,8,1)]
[OutputTopology("point")] 
void  MSMain3() {
  int x = 2;
}

[NumThreads(2,8,2)]
[OutputTopology("triangle_cw")] 
void  MSMain4() {
  int x = 2;
}

[NumThreads(2,8,2)]
[OutputTopology("triangle_ccw")] 
void  MSMain5() {
  int x = 2;
}



