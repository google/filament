@group(0) @binding(0)
var<uniform> b: i32;

fn func_3() -> bool {
    for (var i = 0; i < b; i++) {
        for (var j = -1; j == 1; j++) {
            return false;
        }
    }
    return false;
}

@compute @workgroup_size(1)
fn main() {
    func_3();
}
