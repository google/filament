// RUN: %dxc -Zi -E main -T ps_6_0 %s | FileCheck %s -check-prefix=CHK_DB
// RUN: %dxc -E main -T ps_6_0 %s | FileCheck %s -check-prefix=CHK_NODB

// Regression test for a crash when lowering unsupported intrinsics

// CHK_DB: 10:50: error: Unsupported intrinsic
// CHK_NODB: 10:50: error: Unsupported intrinsic

sampler TextureSampler;
float4 main(float2 uv	: UV) : SV_Target { return tex2D(TextureSampler, uv); }
