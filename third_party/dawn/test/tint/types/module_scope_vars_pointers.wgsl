var<private> p : f32;
var<workgroup> w : f32;

@compute @workgroup_size(1) fn main() {
    let p_ptr : ptr<private, f32> = &p;
    let w_ptr : ptr<workgroup, f32> = &w;
    let x : f32 = *p_ptr + *w_ptr;
    *p_ptr = x;
}
