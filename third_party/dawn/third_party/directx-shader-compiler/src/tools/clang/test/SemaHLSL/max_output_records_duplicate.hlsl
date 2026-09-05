// RUN: %dxc -Tlib_6_8 -verify %s

// Duplicate MaxRecords info with mismatching limits

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
void DuplicateMax1(
  RWThreadNodeInputRecord<rec0> InputyMcInputFace,
  [MaxRecords(5)] NodeOutput<rec1> Output1,
  [MaxRecords(5)] [MaxRecordsSharedWith(Output1)] NodeOutput<rec1> Output2, /* expected-error {{only one of MaxRecords or MaxRecordsSharedWith may be specified to the same parameter.}} */ /* expected-note {{conflicting attribute is here}} */
  [MaxRecords(5)] [MaxRecordsSharedWith(Output1)] NodeOutput<rec1> Output3, /* expected-error {{only one of MaxRecords or MaxRecordsSharedWith may be specified to the same parameter.}} */ /* expected-note {{conflicting attribute is here}} */
  [MaxRecords(7)] [MaxRecordsSharedWith(Output6)] NodeOutput<rec1> Output4, /* expected-error {{only one of MaxRecords or MaxRecordsSharedWith may be specified to the same parameter.}} */ /* expected-note {{conflicting attribute is here}} */
  [MaxRecords(7)] [MaxRecordsSharedWith(Output6)] NodeOutput<rec1> Output5, /* expected-error {{only one of MaxRecords or MaxRecordsSharedWith may be specified to the same parameter.}} */ /* expected-note {{conflicting attribute is here}} */
  [MaxRecords(7)] NodeOutput<rec1> Output6)
{
}

[Shader("node")]
[NodeLaunch("thread")]
void DuplicateMax2(
  RWThreadNodeInputRecord<rec0> InputyMcInputFace,
  [MaxRecords(1)] NodeOutput<rec1> Output1,
  [MaxRecordsSharedWith(Output1)] [MaxRecords(2)] NodeOutput<rec1> Output2, /* expected-error {{only one of MaxRecords or MaxRecordsSharedWith may be specified to the same parameter.}} */ /* expected-note {{conflicting attribute is here}} */
  [MaxRecords(3)] [MaxRecordsSharedWith(Output1)] NodeOutput<rec1> Output3, /* expected-error {{only one of MaxRecords or MaxRecordsSharedWith may be specified to the same parameter.}} */ /* expected-note {{conflicting attribute is here}} */
  [MaxRecordsSharedWith(Output6)] [MaxRecords(4)] NodeOutput<rec1> Output4, /* expected-error {{only one of MaxRecords or MaxRecordsSharedWith may be specified to the same parameter.}} */ /* expected-note {{conflicting attribute is here}} */
  [MaxRecords(5)] [MaxRecordsSharedWith(Output2)] NodeOutput<rec1> Output5) /* expected-error {{only one of MaxRecords or MaxRecordsSharedWith may be specified to the same parameter.}} */ /* expected-note {{conflicting attribute is here}} */
{
}


void DuplicateMaxSameAttr(
[MaxRecords(6)] [MaxRecords(3)] NodeOutput<rec1> Output1, /* expected-error {{only one of MaxRecords or MaxRecordsSharedWith may be specified to the same parameter.}} */ /* expected-note {{conflicting attribute is here}} */
  [MaxRecordsSharedWith(Output1)] [MaxRecordsSharedWith(Output2)] NodeOutput<rec1> Output3  /* expected-error {{only one of MaxRecords or MaxRecordsSharedWith may be specified to the same parameter.}} */ /* expected-note {{conflicting attribute is here}} */
  )
{    
}

struct INPUT_RECORD
{
  uint value;
};

struct OUTPUT_RECORD
{
  uint num;
};

[Shader("node")]
[NodeLaunch("broadcasting")]
[NodeDispatchGrid(256,1,1)]
[NumThreads(1,1,1)]
[NodeIsProgramEntry]
void node092_maxoutputrecords_maxoutputrecordssharedwith(DispatchNodeInputRecord<INPUT_RECORD> input,
                                                         [MaxRecords(5)] NodeOutput<OUTPUT_RECORD> firstOut,
                                                         [MaxRecords(5)][MaxRecordsSharedWith(firstOut)] NodeOutput<OUTPUT_RECORD> secondOut) /* expected-error {{only one of MaxRecords or MaxRecordsSharedWith may be specified to the same parameter.}} */ /* expected-note {{conflicting attribute is here}} */
{
}

void SelfReference(
[MaxRecordsSharedWith(Output1)] NodeOutput<rec1> Output1) /* expected-error {{attribute MaxRecordsSharedWith must not reference the same parameter it is applied to.}} */
{    
}
