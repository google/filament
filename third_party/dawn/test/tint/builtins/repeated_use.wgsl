// Check that for backends that generate builtin helpers, repeated use of the
// same builtin overload results in single helper being generated.
@compute @workgroup_size(1)
fn main() {
    let va = vec4<f32>();
    let a = degrees(va);
    let vb = vec4<f32>(1.);
    let b = degrees(vb);
    let vc = vec4<f32>(1., 2., 3., 4.);
    let c = degrees(vc);

    let vd = vec3<f32>();
    let d = degrees(vd);
    let ve = vec3<f32>(1.);
    let e = degrees(ve);
    let vf = vec3<f32>(1., 2., 3.);
    let f = degrees(vf);

    let vg = vec2<f32>();
    let g = degrees(vg);
    let vh = vec2<f32>(1.);
    let h = degrees(vh);
    let vi = vec2<f32>(1., 2.);
    let i = degrees(vi);

    let vj = 1.;
    let j = degrees(vj);
    let vk = 2.;
    let k = degrees(vk);
    let vl = 3.;
    let l = degrees(vl);
}
