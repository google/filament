// flags: --overrides o=0,j=1
override o : bool = true;
override j : bool = false;

@compute @workgroup_size(1)
fn main() {
    if o && j {
        _ = o;
    }
}
