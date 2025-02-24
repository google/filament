// RUN: %dxc -Zi -E main -T ps_6_0 %s | FileCheck %s -check-prefix=CHK_DB
// RUN: %dxc -E main -T ps_6_0 %s | FileCheck %s -check-prefix=CHK_NODB

// CHK_DB: 9:1: error: i64(type for I) cannot be used as shader inputs or outputs.
// CHK_DB: 9:1: error: double(type for SV_Target) cannot be used as shader inputs or outputs.
// CHK_NODB: 9:1: error: i64(type for I) cannot be used as shader inputs or outputs.
// CHK_NODB: 9:1: error: double(type for SV_Target) cannot be used as shader inputs or outputs.

double main(uint64_t i:I) : SV_Target {
    return 1;
}
