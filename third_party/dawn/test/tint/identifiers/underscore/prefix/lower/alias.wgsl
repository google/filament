@group(0) @binding(0) var<storage, read_write> s: i32;

alias a = i32;
alias _a = i32;
alias b = a;
alias _b = _a;

@compute @workgroup_size(1)
fn f() {
    var c : b;
    var d : _b;

    s = c + d;
}
