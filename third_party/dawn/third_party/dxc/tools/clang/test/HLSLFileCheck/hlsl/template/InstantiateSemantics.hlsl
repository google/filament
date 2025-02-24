// RUN: %dxc -T ps_6_5 -HV 2021 %s -ast-dump %s | FileCheck %s

template<typename C>
struct Output {
  C col : SV_Target;
};

template<typename P>
struct Input {
  int id : SV_InstanceID;
  P pos : SV_Position;
};

SamplerState g_samp : register(s0);
Texture2D<float4> g_tex : register(t0);

void main(Input<float2> I, out Output<float4> O) {
  O.col = g_tex.Sample(g_samp, I.pos);
}

/// This test verifies that the HLSL semantics applied to struct fields are
/// propagated during instantiation. The test verifies the SemanticDecl on both
/// template-sepcialized fields (col & pos), and a non-specialized field (id).

//CHECK:      |-ClassTemplateDecl 0x{{[0-9a-zA-Z]+}} <{{.*}}> line:4:8 Output
//CHECK-NEXT: | |-TemplateTypeParmDecl 0x{{[0-9a-zA-Z]+}} <line:3:10, col:19> col:19 referenced typename C

//CHECK:      | | `-FieldDecl 0x{{[0-9a-zA-Z]+}} <line:5:3, col:5> col:5 col 'C'
//CHECK-NEXT: | |   `-SemanticDecl 0x{{[0-9a-zA-Z]+}} <col:11> "SV_Target"
//CHECK-NEXT: | `-ClassTemplateSpecializationDecl 0x{{[0-9a-zA-Z]+}} <line:3:1, line:6:1> line:4:8 struct Output definition
//CHECK-NEXT: |   |-TemplateArgument type 'vector<float, 4>'

//CHECK:      |   `-FieldDecl 0x{{[0-9a-zA-Z]+}} <line:5:3, col:5> col:5 referenced col 'vector<float, 4>':'vector<float, 4>'
//CHECK-NEXT: |     `-SemanticDecl 0x{{[0-9a-zA-Z]+}} <col:11> "SV_Target"

//CHECK:      |-ClassTemplateDecl 0x{{[0-9a-zA-Z]+}} <line:8:1, line:12:1> line:9:8 Input
//CHECK-NEXT: | |-TemplateTypeParmDecl 0x{{[0-9a-zA-Z]+}} <line:8:10, col:19> col:19 referenced typename P

//CHECK:      | | |-FieldDecl 0x{{[0-9a-zA-Z]+}} <line:10:3, col:7> col:7 id 'int'
//CHECK-NEXT: | | | `-SemanticDecl 0x{{[0-9a-zA-Z]+}} <col:12> "SV_InstanceID"
//CHECK-NEXT: | | `-FieldDecl 0x{{[0-9a-zA-Z]+}} <line:11:3, col:5> col:5 pos 'P'
//CHECK-NEXT: | |   `-SemanticDecl 0x{{[0-9a-zA-Z]+}} <col:11> "SV_Position"
//CHECK-NEXT: | `-ClassTemplateSpecializationDecl 0x{{[0-9a-zA-Z]+}} <line:8:1, line:12:1> line:9:8 struct Input definition
//CHECK-NEXT: |   |-TemplateArgument type 'vector<float, 2>'

//CHECK:      |   |-FieldDecl 0x{{[0-9a-zA-Z]+}} <line:10:3, col:7> col:7 id 'int'
//CHECK-NEXT: |   | `-SemanticDecl 0x{{[0-9a-zA-Z]+}} <col:12> "SV_InstanceID"
//CHECK-NEXT: |   `-FieldDecl 0x{{[0-9a-zA-Z]+}} <line:11:3, col:5> col:5 referenced pos 'vector<float, 2>':'vector<float, 2>'
//CHECK-NEXT: |     `-SemanticDecl 0x{{[0-9a-zA-Z]+}} <col:11> "SV_Position"
