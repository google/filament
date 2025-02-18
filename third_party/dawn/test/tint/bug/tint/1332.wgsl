@compute @workgroup_size(1)
fn compute_main() {
	let a = 1.24;
	var b = max(a, 1.17549435e-38);
}
