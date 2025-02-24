var<private> t : u32;
fn m() -> vec4<u32> {
    t = 1u;
    return vec4<u32>(t);
}
fn f() {
    var v : vec4<bool> = vec4<bool>(m());
}