@group(0) @binding(0) var<storage, read_write> s: i32;

alias a = i32;
alias a__ = i32;
alias b = a;
alias b__ = a__;

@compute @workgroup_size(1)
fn f() {
    var c : b;
    var d : b__;

    s = c + d;
}
