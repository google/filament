// RUN: %dxc -E main -T vs_6_2 %s | FileCheck %s

// Regression test for a bug where matrix array arguments would get expanded
// into elementwise copies to respect pass-by-value semantics in case orientation
// changes were needed, but the pattern would not be cleaned up by later optimization passes.

const int1x1 cb_matrices[64];
const row_major int1x1 cb_matrices_rm[64];
const int cb_index;

int get_cm(column_major int1x1 matrices[64]) { return matrices[cb_index]; }
int get_rm(row_major int1x1 matrices[64]) { return matrices[cb_index]; }

int2 main() : OUT
{
    // There should be no dynamic GEP of an array,
    // we should be dynamically indexing the constant buffer directly
    // CHECK-NOT: getelementptr
    return int2(
        // CHECK: call %dx.types.CBufRet.i32 @dx.op.cbufferLoadLegacy.i32(i32 59, %dx.types.Handle %{{.*}}, i32 %{{.*}})
        get_cm(cb_matrices), // Test implicit column major to explicit column major (no conversion needed)
        // CHECK: call %dx.types.CBufRet.i32 @dx.op.cbufferLoadLegacy.i32(i32 59, %dx.types.Handle %{{.*}}, i32 %{{.*}})
        get_rm(cb_matrices_rm)); // Test explicit row major to explicit row major (no conversion needed)
}