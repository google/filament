// flags:  --hlsl-shader-model 62
enable f16;
@compute @workgroup_size(1)
fn f() {
    var a = 1.h;
    var b = 0.h;
    let r : f16 = a % (b + b);
}
