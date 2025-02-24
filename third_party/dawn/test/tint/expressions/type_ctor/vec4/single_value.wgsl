fn v() -> vec4<f32> {
    return vec4(0.0);
}

fn f() {
    let a = vec4(1.0);
    let b = vec4(a);
    let c = vec4(v());
    let d = vec4(a * 2);
}
