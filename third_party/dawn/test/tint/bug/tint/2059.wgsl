alias Mat = mat3x3<f32>;

// Struct of matrix
struct S { m : Mat }

// Struct of array of matrix
struct S2 { m : array<Mat, 1> }

// Struct of struct of matrix
struct S3 { s : S }

// Struct of array of struct of matrix
struct S4 { s : array<S, 1> }

@group(0) @binding(0) var<storage, read_write> buffer0 : Mat;
@group(0) @binding(1) var<storage, read_write> buffer1 : S;
@group(0) @binding(2) var<storage, read_write> buffer2 : S2;
@group(0) @binding(3) var<storage, read_write> buffer3 : S3;
@group(0) @binding(4) var<storage, read_write> buffer4 : S4;
@group(0) @binding(5) var<storage, read_write> buffer5 : array<Mat, 1>;
@group(0) @binding(6) var<storage, read_write> buffer6 : array<S, 1>;
@group(0) @binding(7) var<storage, read_write> buffer7 : array<S2, 1>;

@compute @workgroup_size(1)
fn main() {
  var m : Mat;
  for (var c = 0u; c < 3; c++) {
    m[c] = vec3(f32(c*3 + 1), f32(c*3 + 2), f32(c*3 + 3));
  }
  
  // copy of matrix
  {
    let a = m;
    buffer0 = a;
  }

  // struct of matrix
  {
    let a = S(m);
    buffer1 = a; 
  }

  // struct of array of matrix
  {
    let a = S2(array<Mat, 1>(m));
    buffer2 = a;
  }
  
  // struct of struct of matrix
  {
    let a = S3(S(m));
    buffer3 = a;
  }

  // struct of array of struct of matrix
  {
    let a = S4(array<S, 1>(S(m)));
    buffer4 = a;
  }

  // array of matrix
  {
    let a = array<Mat, 1>(m);
    buffer5 = a;
  }

  // array of struct of matrix
  {
    let a = array<S, 1>(S(m));
    buffer6 = a;
  }

  // array of struct of array of matrix
  {
    let a = array<S2, 1>(S2(array<Mat, 1>(m)));
    buffer7 = a;
  }
}
