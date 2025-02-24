// flags:  --hlsl-shader-model 62
enable f16;
@compute @workgroup_size(1)
fn f() {
    let a = 1.h;
    let b = 2.h;
    let r : f16 = a * b;
}
