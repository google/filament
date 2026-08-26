// RUN: %dxc /T vs_6_0 /E main /Zpc -ast-dump %s | FileCheck %s

// Test that the declarations in the ast get annotated with row/column major as expected

void main()
{
    // CHECK: rm_Zpc 'row_major int2x2
    row_major int2x2 rm_Zpc;
    // CHECK: cm_Zpc 'column_major int2x2
    column_major int2x2 cm_Zpc;
    // CHECK: def_Zpc 'int2x2
    int2x2 def_Zpc; // Default to column_major from (implicit) /Zpc

    #pragma pack_matrix(row_major)
    // CHECK: rm_prm 'row_major int2x2
    row_major int2x2 rm_prm;
    // CHECK: cm_prm 'column_major int2x2
    column_major int2x2 cm_prm;
    // CHECK: def_prm 'row_major int2x2
    int2x2 def_prm; // Default to row_major from #pragma

    #pragma pack_matrix(column_major)
    // CHECK: rm_pcm 'row_major int2x2
    row_major int2x2 rm_pcm;
    // CHECK: cm_pcm 'column_major int2x2
    column_major int2x2 cm_pcm;
    // CHECK: def_pcm 'column_major int2x2
    int2x2 def_pcm; // Default to column_major from #pragma
}