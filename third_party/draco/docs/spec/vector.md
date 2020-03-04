## Vectors

### Dot()

~~~~~
void Dot(vec_x, vec_y, dot) {
  dot = 0;
  for (i = 0; i < vec_x.size(); ++i) {
    dot += vec_x[i] * vec_y[i];
  }
}
~~~~~
{:.draco-syntax}


### AbsSum()

~~~~~
void AbsSum(vec, abs_sum) {
  result = 0;
  for (i = 0; i < vec.size(); ++i) {
    result += Abs(vec[i]);
  }
  abs_sum = result;
}
~~~~~
{:.draco-syntax}


### MultiplyScalar()

~~~~~
void MultiplyScalar(vec, value, out) {
  for (i = 0; i < vec.size(); ++i) {
    out.push_back(vec[i] * value);
  }
}
~~~~~
{:.draco-syntax}


### DivideScalar()

~~~~~
void DivideScalar(vec, value, out) {
  for (i = 0; i < vec.size(); ++i) {
    out.push_back(vec[i] / value);
  }
}
~~~~~
{:.draco-syntax}


### AddVectors()

~~~~~
void AddVectors(a, b, c) {
  for (i = 0; i < a.size(); ++i) {
    c.push_back(a[i] + b[i]);
  }
}
~~~~~
{:.draco-syntax}


### SubtractVectors()

~~~~~
void SubtractVectors(a, b, c) {
  for (i = 0; i < a.size(); ++i) {
    c.push_back(a[i] - b[i]);
  }
}
~~~~~
{:.draco-syntax}


### CrossProduct()

~~~~~
void CrossProduct(u, v, r) {
  r[0] = (u[1] * v[2]) - (u[2] * v[1]);
  r[1] = (u[2] * v[0]) - (u[0] * v[2]);
  r[2] = (u[0] * v[1]) - (u[1] * v[0]);
}
~~~~~
{:.draco-syntax}
