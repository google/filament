// flags:  --hlsl-shader-model 62
enable f16;
@compute @workgroup_size(1)
fn f() {
    let a = 4.h;
    let b = vec3<f16>(0.h, 2.h, 0.h);
    let r : vec3<f16> = a / b;
}
