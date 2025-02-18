var<private> arr = array<array<f32, 2>, 2>(array<f32, 2>(1, 2),
                                           array<f32, 2>(3, 4));

fn f() {
    var v = arr;
}
