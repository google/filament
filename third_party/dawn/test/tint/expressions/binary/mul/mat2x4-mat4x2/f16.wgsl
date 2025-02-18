// flags:  --hlsl-shader-model 62
enable f16;
@compute @workgroup_size(1)
fn f() {
    let a = mat2x4<f16>(vec4<f16>(1.h, 2.h, 3.h, 4.h), vec4<f16>(5.h, 6.h, 7.h, 8.h));
    let b = mat4x2<f16>(vec2<f16>(-1.h, -2.h), vec2<f16>(-3.h, -4.h), vec2<f16>(-5.h, -6.h), vec2<f16>(-7.h, -8.h));
    let r : mat4x4<f16> = a * b;
}
