// RUN: %dxc -Tlib_6_8 -verify %s

// Test maxoutputrecordssharedwith with invalid references
// copied from tools\clang\test\HLSLFileCheck\shader_targets\nodes\max_output_records_invalidref.hlsl

struct rec0
{
    int i0;
    float f0;
};

struct rec1
{
    float f1;
    int i1;
};

[Shader("node")]
[NodeLaunch("thread")]
void InvalidRef(
  RWThreadNodeInputRecord<rec0> InputyMcInputFace,
  // MaxRecordsSharedWith referencing non-existant parameter
  [MaxRecordsSharedWith(Output7)] NodeOutput<rec1> Output1,  /* expected-error {{attribute MaxRecordsSharedWith must reference a valid ouput parameter name.}} */ 
  // MaxRecordsSharedWith referencing an input parameter
  [MaxRecordsSharedWith(InputyMcInputFace)] NodeOutput<rec1> Output2, /* expected-error {{attribute MaxRecordsSharedWith must reference a valid ouput parameter name.}} */
  // MaxRecordsSharedWith referencing its own parameter
  [MaxRecordsSharedWith(Output3)] NodeOutput<rec1> Output3) /* expected-error {{attribute MaxRecordsSharedWith must not reference the same parameter it is applied to.}} */  
{
}
