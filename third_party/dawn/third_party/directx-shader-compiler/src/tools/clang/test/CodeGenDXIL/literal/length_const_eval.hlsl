// RUN: %dxc -T vs_6_0 %s -E main | FileCheck %s
// RUN: not %dxc -T vs_6_0 %s -E main -DNO_FOLD 2>&1 | FileCheck %s --check-prefixes=NO_FOLD

// The code path generates invalid overload.  The invalid overload will either be
// constant folded away, or caught by the validator.

// Ensure length is constant evaluated during codegen, or dxil const eval
// TODO: handle fp specials properly, tracked with https://github.com/microsoft/DirectXShaderCompiler/issues/6567

RWBuffer<float4> results : register(u0);

[shader("vertex")]
void main(bool b : B) {
    uint i = 0;

    // Literal float
    // CHECK: call void @dx.op.bufferStore.f32(i32 69, %dx.types.Handle %{{.+}}, i32 0, i32 undef, float 0x3FE6A09E60000000, float 0x4004C8DC20000000, float 0.000000e+00, float 0.000000e+00, i8 15)
    results[i++] = float4(length(0.5.xx),
                          length(-1.5.xxx),
                          length(0.0.xxxx),
                          length(-0.0.xxxx));

    // Explicit float
    // CHECK: call void @dx.op.bufferStore.f32(i32 69, %dx.types.Handle %{{.+}}, i32 1, i32 undef, float 0x3FE6A09E60000000, float 0x4004C8DC20000000, float 0.000000e+00, float 0.000000e+00, i8 15)
    results[i++] = float4(length(0.5F.xx),
                          length(-1.5F.xxx),
                          length(0.0F.xxxx),
                          length(-0.0F.xxxx));

#ifdef NO_FOLD
    // Currently, we rely on constant folding of DXIL ops to get rid of illegal
    // double overloads. If this doesn't happen, we expect a validation error.
    // Ternary operator can return literal type, while not being foldable due
    // non-constant condition.
    // NO_FOLD: error: validation errors
    // NO_FOLD: error: DXIL intrinsic overload must be valid.
    float result = length((b ? 1.5 : 0.5).xxx);
    results[i++] = float4(result, 0, 0, 0);
#endif // NO_FOLD
}
