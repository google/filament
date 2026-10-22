// RUN: %dxc /T vs_6_0 /E main  %s | FileCheck %s

// Tests the exact place at which #pragma pack_matrix takes effect.

#pragma pack_matrix(column_major)

typedef
#pragma pack_matrix(row_major)
int2x2
// With FXC, we could place the #pragma pack_matrix(column_major) here
// and still get the type be row_major. This not easy to replicate
// in DXC because the parser looks ahead one token to see if the
// type is followed by '::', which causes the execution of a #pragma
// following the 'int2x2' one token early, but it is highly unlikely that this
// would be a backwards compatibility issue.
i22
#pragma pack_matrix(column_major)
;

void main(out i22 mat : OUT)
{
    // CHECK: call void @dx.op.storeOutput.i32(i32 5, i32 0, i32 0, i8 0, i32 11)
    // CHECK: call void @dx.op.storeOutput.i32(i32 5, i32 0, i32 0, i8 1, i32 12)
    // CHECK: call void @dx.op.storeOutput.i32(i32 5, i32 0, i32 1, i8 0, i32 21)
    // CHECK: call void @dx.op.storeOutput.i32(i32 5, i32 0, i32 1, i8 1, i32 22)
    mat = i22(11, 12, 21, 22);

    // FXC output, for reference:
    // mov o0.xy, l(11,12,0,0)
    // mov o1.xy, l(21,22,0,0)
}
