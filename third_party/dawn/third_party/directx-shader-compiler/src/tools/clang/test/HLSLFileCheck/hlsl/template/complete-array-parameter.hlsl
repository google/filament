// RUN: %dxc -T lib_6_3 -ast-dump %s | FileCheck %s
// RUN: %dxc -T lib_6_3 %s | FileCheck %s -check-prefix=IR
float4 Val(Texture2D f[2]) {
  return float4(0,0,0,0) + f[0].Load(0);
}

Texture2D a[2];

export float foo() {
  return Val(a);
}

// CHECK: FunctionDecl 0x{{[0-9a-fA-F]+}} <{{.*}}, line:5:1> line:3:8 used Val 'float4 (Texture2D [2])'
// CHECK-NEXT: ParmVarDecl 0x{{[0-9a-fA-F]+}} <col:12, col:25> col:22 used f 'Texture2D [2]'

// IR:!{i32 0, [2 x %"class.Texture2D<vector<float, 4> >"]* @"\01?a@@3PAV?$Texture2D@V?$vector@M$03@@@@A", !"a", i32 -1, i32 -1, i32 2, i32 2, i32 0, ![[EXTRA:[0-9]+]]}
// IR:![[EXTRA]] = !{i32 0, i32 9}