// flags: --overrides o=0,j=1
override o : bool = bool();
override j : bool = bool();

@compute @workgroup_size(1)
fn main() {
    if o && j {
        _ = o;
    }
}
