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

void DuplicateMaxSameAttr(
[MaxRecordsSharedWith(Output1)] [MaxRecordsSharedWith(Output1)] NodeOutput<rec1> Output2,  /* expected-no-diagnostics */
[MaxRecords(3)] [MaxRecords(3)] NodeOutput<rec1> Output3 /* expected-no-diagnostics */
  )
{    
}
