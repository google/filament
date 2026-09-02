// RUN: %dxc -Tlib_6_3 -Wno-unused-value -verify %s
// RUN: %dxc -Tps_6_0 -Wno-unused-value -verify %s

string globalRs = "CBV(b0)";
string localRs = "UAV(u0, visibility = SHADER_VISIBILITY_GEOMETRY)";

GlobalRootSignature grs1_1 = {"CBV(b0)"};
GlobalRootSignature grs1_2 = { globalRs };

GlobalRootSignature grs2_2 = globalRs;                      /* expected-error {{cannot initialize a variable of type 'GlobalRootSignature' with an lvalue of type 'string'}} */
GlobalRootSignature grs2_3 = "CBV(b0)";                     /* expected-error {{cannot initialize a variable of type 'GlobalRootSignature' with an lvalue of type 'literal string'}} */
GlobalRootSignature grs2_4 = 10;                            /* expected-error {{cannot initialize a variable of type 'GlobalRootSignature' with an rvalue of type 'literal int'}} */
GlobalRootSignature grs2_5 = {"CBV(b0)", 78};;              /* expected-error {{too many elements in subobject initialization (expected 1 element, have 2)}} */
GlobalRootSignature grs2_6 = {""};;  /* TODO: add error here */

StateObjectConfig soc1_1 = { STATE_OBJECT_FLAGS_ALLOW_LOCAL_DEPENDENCIES_ON_EXTERNAL_DEFINITONS };
StateObjectConfig soc1_2 = { STATE_OBJECT_FLAGS_ALLOW_LOCAL_DEPENDENCIES_ON_EXTERNAL_DEFINITONS | STATE_OBJECT_FLAGS_ALLOW_EXTERNAL_DEPENDENCIES_ON_LOCAL_DEFINITIONS };
StateObjectConfig soc1_3 = { 0xFF };
StateObjectConfig soc1_4 = { 1 };
StateObjectConfig soc1_5 = { 0.1f };                        /* expected-warning {{implicit conversion from 'float' to 'unsigned int' changes value from 0.1 to 0}} */

StateObjectConfig soc2_2 = STATE_OBJECT_FLAGS_ALLOW_LOCAL_DEPENDENCIES_ON_EXTERNAL_DEFINITONS;    /* expected-error {{cannot initialize a variable of type 'StateObjectConfig' with an lvalue of type 'const unsigned int'}} */
StateObjectConfig soc2_3 = 0x1;                             /* expected-error {{cannot initialize a variable of type 'StateObjectConfig' with an rvalue of type 'literal int'}} */
StateObjectConfig soc2_4 = "none";                          /* expected-error {{cannot initialize a variable of type 'StateObjectConfig' with an lvalue of type 'literal string'}} */
StateObjectConfig soc2_5 = { STATE_OBJECT_FLAGS_ALLOW_LOCAL_DEPENDENCIES_ON_EXTERNAL_DEFINITONS, STATE_OBJECT_FLAGS_ALLOW_EXTERNAL_DEPENDENCIES_ON_LOCAL_DEFINITIONS };    /* expected-error {{too many elements in subobject initialization (expected 1 element, have 2)}} */
StateObjectConfig soc2_6 = { 0x1, 0x2 };                    /* expected-error {{too many elements in subobject initialization (expected 1 element, have 2)}} */
StateObjectConfig soc2_7 = 1.5f;                            /* expected-error {{cannot initialize a variable of type 'StateObjectConfig' with an rvalue of type 'float'}} */

LocalRootSignature lrs1_1 = {"UAV(u0, visibility = SHADER_VISIBILITY_GEOMETRY)"};
LocalRootSignature lrs1_2 = { localRs };

LocalRootSignature lrs2_2 = localRs;                        /* expected-error {{cannot initialize a variable of type 'LocalRootSignature' with an lvalue of type 'string'}} */
LocalRootSignature lrs2_3 = "UAV(u0, visibility = SHADER_VISIBILITY_GEOMETRY)";     /* expected-error {{cannot initialize a variable of type 'LocalRootSignature' with an lvalue of type 'literal string'}} */
LocalRootSignature lrs2_4 = 10;                             /* expected-error {{cannot initialize a variable of type 'LocalRootSignature' with an rvalue of type 'literal int'}} */
LocalRootSignature lrs2_5 = { 1.7f, "UAV(u0, visibility = SHADER_VISIBILITY_GEOMETRY)" };;     /* expected-error {{too many elements in subobject initialization (expected 1 element, have 2)}} */
LocalRootSignature lrs2_6 = {""};;  /* TODO: add error here */

string s1 = "abc";
string s2 = "edf";
string s3 = s1;
string s4 = s1 + s2;                                                                           /* expected-error {{scalar, vector, or matrix expected}} */

SubobjectToExportsAssociation sea1_1 = { "grs", "a;b;foo;c" };
SubobjectToExportsAssociation sea1_2 = { "grs", ";;;" };
SubobjectToExportsAssociation sea1_4 = { s1, s2 };
SubobjectToExportsAssociation sea1_5 = { "a", s3 };
SubobjectToExportsAssociation sea1_6 = { "b", s4 };

SubobjectToExportsAssociation sea2_4 = { 15, 0.01f };       /* expected-error {{type mismatch}} */
SubobjectToExportsAssociation sea2_5 = { s1, s2 + ';' + s2 };    /* expected-error {{scalar, vector, or matrix expected}} */
SubobjectToExportsAssociation sea2_6 = { 51 };              /* expected-error {{too few elements in subobject initialization (expected 2 elements, have 1)}} */
SubobjectToExportsAssociation sea2_7 = "foo";               /* expected-error {{cannot initialize a variable of type 'SubobjectToExportsAssociation' with an lvalue of type 'literal string'}} */
SubobjectToExportsAssociation sea2_8 = 65412;               /* expected-error {{cannot initialize a variable of type 'SubobjectToExportsAssociation' with an rvalue of type 'literal int'}} */

int i1 = 10, i2 = 156;

RaytracingShaderConfig rsc1_1 = { 128, 64 };
RaytracingShaderConfig rsc1_2 = { int2(128, 64) };
RaytracingShaderConfig rsc1_3 = { i1, i1 + i2 };

RaytracingShaderConfig rsc2_2 = int2(128, 64);              /* expected-error {{cannot initialize a variable of type 'RaytracingShaderConfig' with an rvalue of type 'int2'}} */
RaytracingShaderConfig rsc2_3 = { 128, 64, 32 };            /* expected-error {{too many elements in subobject initialization (expected 2 elements, have 3)}} */
RaytracingShaderConfig rsc2_4 = { 128 };                    /* expected-error {{too few elements in subobject initialization (expected 2 elements, have 1)}} */
RaytracingShaderConfig rsc2_5 = { 128f, 64f };              /* expected-error {{invalid digit 'f' in decimal constant}} expected-error {{invalid digit 'f' in decimal constant}} */
RaytracingShaderConfig rsc2_6 = { "foo" };                  /* expected-error {{too few elements in subobject initialization (expected 2 elements, have 1)}} */
RaytracingShaderConfig rsc2_7 = "foo";                      /* expected-error {{cannot initialize a variable of type 'RaytracingShaderConfig' with an lvalue of type 'literal string'}} */
RaytracingShaderConfig rsc2_8 = 128;                        /* expected-error {{cannot initialize a variable of type 'RaytracingShaderConfig' with an rvalue of type 'literal int'}} */

RaytracingPipelineConfig rpc1_1 = { 512 };
RaytracingPipelineConfig rpc1_2 = { 512.15f };                 /* expected-warning {{implicit conversion from 'float' to 'unsigned int' changes value from 512.15002 to 512}} */

RaytracingPipelineConfig rpc2_2 = { 512, 128 };             /* expected-error {{too many elements in subobject initialization (expected 1 element, have 2)}} */
RaytracingPipelineConfig rpc2_3 = 512;                      /* expected-error {{cannot initialize a variable of type 'RaytracingPipelineConfig' with an rvalue of type 'literal int'}} */
RaytracingPipelineConfig rpc2_4 = 51.1f;                    /* expected-error {{cannot initialize a variable of type 'RaytracingPipelineConfig' with an rvalue of type 'float'}} */
RaytracingPipelineConfig rpc2_5 = "foo";                    /* expected-error {{cannot initialize a variable of type 'RaytracingPipelineConfig' with an lvalue of type 'literal string'}} */

TriangleHitGroup trHitGt1_1 = { "a", "b" };
TriangleHitGroup trHitGt1_2 = { s1, s2 };
TriangleHitGroup trHitGt1_3 = { "", "" };

TriangleHitGroup trHitGt2_2 = { "a", "b", "c"};               /* expected-error {{too many elements in subobject initialization (expected 2 elements, have 3)}} */
TriangleHitGroup trHitGt2_3 = { "a", 10 };                    /* expected-error {{type mismatch}} */
TriangleHitGroup trHitGt2_4 = { 1, 2 };                       /* expected-error {{type mismatch}} */
TriangleHitGroup trHitGt2_5 = "foo";                          /* expected-error {{cannot initialize a variable of type 'TriangleHitGroup' with an lvalue of type 'literal string'}} */
TriangleHitGroup trHitGt2_6 = 115;                            /* expected-error {{cannot initialize a variable of type 'TriangleHitGroup' with an rvalue of type 'literal int'}} */
TriangleHitGroup trHitGt2_7 = s2;                             /* expected-error {{cannot initialize a variable of type 'TriangleHitGroup' with an lvalue of type 'string'}} */

ProceduralPrimitiveHitGroup ppHitGt1_1 = { "a", "b", "c"};
ProceduralPrimitiveHitGroup ppHitGt1_2 = { s1, s2, s3 };
ProceduralPrimitiveHitGroup ppHitGt1_3 = { "", "", "c" };

ProceduralPrimitiveHitGroup ppHitGt2_2 = { "a", "b"};         /* expected-error {{too few elements in subobject initialization (expected 3 elements, have 2)}} */
ProceduralPrimitiveHitGroup ppHitGt2_3 = { "a", "b", 10 };    /* expected-error {{type mismatch}} */
ProceduralPrimitiveHitGroup ppHitGt2_4 = { 1, 2, 3 };         /* expected-error {{type mismatch}} */
ProceduralPrimitiveHitGroup ppHitGt2_5 = "foo";               /* expected-error {{cannot initialize a variable of type 'ProceduralPrimitiveHitGroup' with an lvalue of type 'literal string'}} */
ProceduralPrimitiveHitGroup ppHitGt2_6 = 115;                 /* expected-error {{cannot initialize a variable of type 'ProceduralPrimitiveHitGroup' with an rvalue of type 'literal int'}} */
ProceduralPrimitiveHitGroup ppHitGt2_7 = s2;                  /* expected-error {{cannot initialize a variable of type 'ProceduralPrimitiveHitGroup' with an lvalue of type 'string'}} */

TriangleHitGroup trHitGt2_8 = { s1, s4 };
ProceduralPrimitiveHitGroup ppHitGt2_8 = { s1, "", s4 };
ProceduralPrimitiveHitGroup ppHitGt2_9 = { "a", "b", ""};

[shader("pixel")]
int main(int i : INDEX) : SV_Target {
  return 1;
}
