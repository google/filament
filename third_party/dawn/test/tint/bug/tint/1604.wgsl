@group(0) @binding(0)
var<uniform> x: i32;

@compute @workgroup_size(1)
fn main() {
    switch (x) {
        case 0: {
            loop {
                return;
            }
        }
        default: {
        }
    }
}
