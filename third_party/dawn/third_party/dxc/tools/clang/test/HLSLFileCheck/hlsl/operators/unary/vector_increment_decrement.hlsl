// RUN: %dxc /T vs_6_0 /E main %s | FileCheck %s

// Check that pre/post increment/decrement operators on
// matrices have the intended semantics for both the original
// variable and the returned value.

AppendStructuredBuffer<int2> results;

void main()
{
  int1 variable, result;
  
  // Post-increment
  // CHECK: i32 11, i32 10
  variable = int1(10);
  result = variable++;
  results.Append(int2(variable.x, result.x));
  
  // Post-decrement
  // CHECK: i32 9, i32 10
  variable = int1(10);
  result = variable--;
  results.Append(int2(variable.x, result.x));
  
  // Pre-increment
  // CHECK: i32 11, i32 11
  variable = int1(10);
  result = ++variable;
  results.Append(int2(variable.x, result.x));
  
  // Pre-decrement
  // CHECK: i32 9, i32 9
  variable = int1(10);
  result = --variable;
  results.Append(int2(variable.x, result.x));
}