// RUN: %dxc -T lib_6_6 -HV 2021 -ast-dump %s | FileCheck %s


template<typename U, typename FV>
struct Test {
  U col : SV_Target;
  FV pos : SV_Position;
  U vid : SV_VertexID;
  FV clip : SV_ClipDistance;
  FV cull : SV_CullDistance;
  U gid : SV_GroupID;
  U tid : SV_GroupThreadID;
};

Test<uint, float4> Val;

//CHECK:      FieldDecl 0x{{[0-9a-zA-Z]+}} <line:6:3, col:5> col:5 col 'U'
//CHECK-NEXT: `-SemanticDecl 0x{{[0-9a-zA-Z]+}} <col:11> "SV_Target"
//CHECK-NEXT: FieldDecl 0x{{[0-9a-zA-Z]+}} <line:7:3, col:6> col:6 pos 'FV'
//CHECK-NEXT: `-SemanticDecl 0x{{[0-9a-zA-Z]+}} <col:12> "SV_Position"
//CHECK-NEXT: FieldDecl 0x{{[0-9a-zA-Z]+}} <line:8:3, col:5> col:5 vid 'U'
//CHECK-NEXT: `-SemanticDecl 0x{{[0-9a-zA-Z]+}} <col:11> "SV_VertexID"
//CHECK-NEXT: FieldDecl 0x{{[0-9a-zA-Z]+}} <line:9:3, col:6> col:6 clip 'FV'
//CHECK-NEXT: `-SemanticDecl 0x{{[0-9a-zA-Z]+}} <col:13> "SV_ClipDistance"
//CHECK-NEXT: FieldDecl 0x{{[0-9a-zA-Z]+}} <line:10:3, col:6> col:6 cull 'FV'
//CHECK-NEXT: `-SemanticDecl 0x{{[0-9a-zA-Z]+}} <col:13> "SV_CullDistance"
//CHECK-NEXT: FieldDecl 0x{{[0-9a-zA-Z]+}} <line:11:3, col:5> col:5 gid 'U'
//CHECK-NEXT: `-SemanticDecl 0x{{[0-9a-zA-Z]+}} <col:11> "SV_GroupID"
//CHECK-NEXT: FieldDecl 0x{{[0-9a-zA-Z]+}} <line:12:3, col:5> col:5 tid 'U'
//CHECK-NEXT: `-SemanticDecl 0x{{[0-9a-zA-Z]+}} <col:11> "SV_GroupThreadID"
