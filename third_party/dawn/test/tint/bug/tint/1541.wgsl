@compute
@workgroup_size(1)
fn main() {
    let a = true;
    var v = select(a & true, true, false);
}
