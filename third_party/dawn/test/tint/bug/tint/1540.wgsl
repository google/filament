struct S {
    e: bool,
}

@compute
@workgroup_size(1)
fn main() {
    var b : bool;
    var v = S(true & b);
}
