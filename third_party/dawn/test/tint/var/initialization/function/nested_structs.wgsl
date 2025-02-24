struct S1 {
    i : i32,
}
struct S2 {
    s1 : S1,
}
struct S3 {
    s2 : S2,
}

fn f(s3 : S3) -> i32 { return s3.s2.s1.i; }

@group(0) @binding(0) var<storage, read_write> out : i32;

@compute @workgroup_size(1)
fn main() {
    const C = 42;
    out = f(S3(S2(S1(C))));
}
