var<private> arr = array<array<f32, 2>, 2>(array<f32, 2>(1f, 2f),
                                           array<f32, 2>(3f, 4f));

fn f() {
    var v = arr;
}
