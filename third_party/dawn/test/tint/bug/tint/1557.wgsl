@group(0)
@binding(0)
var<uniform> u: i32;

fn f() -> i32 {
    return 0;
}

fn g() {
    var j = 0;
    loop {
        if (j >= 1) { break; }
        j += 1;
        var k = f();
    }
}

@compute
@workgroup_size(1)
fn main() {
    switch (u) {
        case 0: {
            switch (u) {
                case 0: {}
                default: {
                    g();
                }
            }
        }
        default: {}
    }
}
