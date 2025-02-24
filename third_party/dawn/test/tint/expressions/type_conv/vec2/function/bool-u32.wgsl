var<private> t : bool;
fn m() -> vec2<bool> {
    t = true;
    return vec2<bool>(t);
}
fn f() {
    var v : vec2<u32> = vec2<u32>(m());
}