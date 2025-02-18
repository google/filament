var<private> t : f32;
fn m() -> vec4<f32> {
    t = 1.0f;
    return vec4<f32>(t);
}
fn f() {
    var v : vec4<bool> = vec4<bool>(m());
}