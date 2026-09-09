// RUN: %dxc /T vs_6_0 /E main /Zpc %s | FileCheck %s

// Test effective matrix orientations with every combination
// of default and explicit matrix orientations.

struct S1 { row_major int2x2 mat; };
struct S2 { int2x2 mat; }; // Default to column_major from /Zpc

#pragma pack_matrix(row_major)
struct S3 { column_major int2x2 mat; };
struct S4 { int2x2 mat; }; // Default to row_major from #pragma

#pragma pack_matrix(column_major)
struct S5 { row_major int2x2 mat; };
struct S6 { int2x2 mat; }; // Default to column_major from #pragma

RWStructuredBuffer<S1> sb1;
RWStructuredBuffer<S2> sb2;
RWStructuredBuffer<S3> sb3;
RWStructuredBuffer<S4> sb4;
RWStructuredBuffer<S5> sb5;
RWStructuredBuffer<S6> sb6;

void main()
{
    // CHECK: i32 11, i32 12, i32 21, i32 22
    sb1[0].mat = int2x2(11, 12, 21, 22);
    // CHECK: i32 11, i32 21, i32 12, i32 22
    sb2[0].mat = int2x2(11, 12, 21, 22);
    // CHECK: i32 11, i32 21, i32 12, i32 22
    sb3[0].mat = int2x2(11, 12, 21, 22);
    // CHECK: i32 11, i32 12, i32 21, i32 22
    sb4[0].mat = int2x2(11, 12, 21, 22);
    // CHECK: i32 11, i32 12, i32 21, i32 22
    sb5[0].mat = int2x2(11, 12, 21, 22);
    // CHECK: i32 11, i32 21, i32 12, i32 22
    sb6[0].mat = int2x2(11, 12, 21, 22);
}