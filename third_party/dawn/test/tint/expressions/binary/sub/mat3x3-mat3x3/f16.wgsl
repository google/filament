// flags:  --hlsl-shader-model 62
enable f16;
@compute @workgroup_size(1)
fn f() {
    let a = mat3x3<f16>(vec3<f16>( 1.h,  2.h,  3.h), vec3<f16>( 4.h,  5.h,  6.h), vec3<f16>( 7.h,  8.h,  9.h));
    let b = mat3x3<f16>(vec3<f16>(-1.h, -2.h, -3.h), vec3<f16>(-4.h, -5.h, -6.h), vec3<f16>(-7.h, -8.h, -9.h));
    let r : mat3x3<f16> = a - b;
}
