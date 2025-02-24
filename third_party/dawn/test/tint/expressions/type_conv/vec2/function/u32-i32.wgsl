var<private> t : u32;
fn m() -> vec2<u32> {
    t = 1u;
    return vec2<u32>(t);
}
fn f() {
    var v : vec2<i32> = vec2<i32>(m());
}