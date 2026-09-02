// RUN: %dxc -E main -T cs_6_0 -HV 2021 -ast-dump %s | FileCheck %s
template<typename T>
void apply(in T X, inout T Y, out T Z);

template<>
void apply<int>(in int X, inout int Y, out int Z){
  Z = (Y += X);
}

template<>
void apply<float3>(in float3 X, inout float3 Y, out float3 Z) {
  Y += X;
  Z = Y + X;
}

[numthreads(1,1,1)]
void main() {
  int X = 1, Y = 2;
  int Z;
  apply(X, Y, Z);

  float3 V = {0.0, 0.0, 0.0};
  float3 W = {0.0, 1.0, 1.0};
  float3 T;
  apply(V, W, T);
}


// CHECK:      FunctionTemplateDecl
// CHECK-NEXT: | |-TemplateTypeParmDecl 0x{{[0-9a-fA-F]+}} <line:2:10, col:19> col:19 referenced typename T
// CHECK-NEXT: | |-FunctionDecl 0x{{[0-9a-fA-F]+}} <line:3:1, col:38> col:6 apply 'void (T, T &__restrict, T &__restrict)'
// CHECK-NEXT: | | |-ParmVarDecl 0x{{[0-9a-fA-F]+}} <col:12, col:17> col:17 X 'T'
// CHECK-NEXT: | | | `-HLSLInAttr 0x{{[0-9a-fA-F]+}} <col:12>
// CHECK-NEXT: | | |-ParmVarDecl 0x{{[0-9a-fA-F]+}} <col:20, col:28> col:28 Y 'T &__restrict'
// CHECK-NEXT: | | | `-HLSLInOutAttr 0x{{[0-9a-fA-F]+}} <col:20>
// CHECK-NEXT: | | `-ParmVarDecl 0x{{[0-9a-fA-F]+}} <col:31, col:37> col:37 Z 'T &__restrict'
// CHECK-NEXT: | |   `-HLSLOutAttr 0x{{[0-9a-fA-F]+}} <col:31>
// CHECK-NEXT: | |-Function 0x{{[0-9a-fA-F]+}} 'apply' 'void (int, int &__restrict, int &__restrict)'
// CHECK-NEXT: | `-Function 0x{{[0-9a-fA-F]+}} 'apply' 'void (float3, float3 &__restrict, float3 &__restrict)'
// CHECK-NEXT: |-FunctionDecl 0x{{[0-9a-fA-F]+}} prev 0x{{[0-9a-fA-F]+}} <line:5:1, line:8:1> line:6:6 used apply 'void (int, int &__restrict, int &__restrict)'
// CHECK-NEXT: | |-TemplateArgument type 'int'
// CHECK-NEXT: | |-ParmVarDecl 0x{{[0-9a-fA-F]+}} <col:17, col:24> col:24 used X 'int'
// CHECK-NEXT: | | `-HLSLInAttr 0x{{[0-9a-fA-F]+}} <col:17>
// CHECK-NEXT: | |-ParmVarDecl 0x{{[0-9a-fA-F]+}} <col:27, col:37> col:37 used Y 'int &__restrict'
// CHECK-NEXT: | | `-HLSLInOutAttr 0x{{[0-9a-fA-F]+}} <col:27>
// CHECK-NEXT: | |-ParmVarDecl 0x{{[0-9a-fA-F]+}} <col:40, col:48> col:48 used Z 'int &__restrict'
// CHECK-NEXT: | | `-HLSLOutAttr 0x{{[0-9a-fA-F]+}} <col:40>

// CHECK:      |-FunctionDecl 0x{{[0-9a-fA-F]+}} prev 0x{{[0-9a-fA-F]+}} <line:10:1, line:14:1> line:11:6 used apply 'void (float3, float3 &__restrict, float3 &__restrict)'
// CHECK-NEXT: | |-TemplateArgument type 'vector<float, 3>'
// CHECK-NEXT: | |-ParmVarDecl 0x{{[0-9a-fA-F]+}} <col:20, col:30> col:30 used X 'float3':'vector<float, 3>'
// CHECK-NEXT: | | `-HLSLInAttr 0x{{[0-9a-fA-F]+}} <col:20>
// CHECK-NEXT: | |-ParmVarDecl 0x{{[0-9a-fA-F]+}} <col:33, col:46> col:46 used Y 'float3 &__restrict'
// CHECK-NEXT: | | `-HLSLInOutAttr 0x{{[0-9a-fA-F]+}} <col:33>
// CHECK-NEXT: | |-ParmVarDecl 0x{{[0-9a-fA-F]+}} <col:49, col:60> col:60 used Z 'float3 &__restrict'
// CHECK-NEXT: | | `-HLSLOutAttr 0x{{[0-9a-fA-F]+}} <col:49>

// CHECK:      | |-CallExpr 0x{{[0-9a-fA-F]+}} <line:20:3, col:16> 'void'
// CHECK-NEXT: | | |-ImplicitCastExpr 0x{{[0-9a-fA-F]+}} <col:3> 'void (*)(int, int &__restrict, int &__restrict)' <FunctionToPointerDecay>
// CHECK:      | `-CallExpr 0x{{[0-9a-fA-F]+}} <line:25:3, col:16> 'void'
// CHECK-NEXT: |   |-ImplicitCastExpr 0x{{[0-9a-fA-F]+}} <col:3> 'void (*)(float3, float3 &__restrict, float3 &__restrict)' <FunctionToPointerDecay>
