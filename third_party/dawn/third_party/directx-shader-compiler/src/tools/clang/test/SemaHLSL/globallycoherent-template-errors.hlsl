// RUN: %dxc -Tlib_6_3 -HV 2021 -verify %s
// RUN: %dxc -Tcs_6_0 -HV 2021 -verify %s

template <typename T> void doSomething(uint pos) {
  globallycoherent RWTexture2D<T> output;
  // expected-error@+2 {{'globallycoherent' is not a valid modifier for a declaration of type 'Buffer<float>'}}
  // expected-note@+1 {{'globallycoherent' can only be applied to UAV or RWDispatchNodeInputRecord objects}}
  globallycoherent Buffer<T> nonUAV;
  // expected-error@+2 {{'globallycoherent' is not a valid modifier for a declaration of type 'float'}}
  // expected-note@+1 {{'globallycoherent' can only be applied to UAV or RWDispatchNodeInputRecord objects}}
  globallycoherent T ThisShouldBreak = 2.0;
  output[uint2(pos, pos)] = 0;
}

void doSomething2(uint pos) {
  globallycoherent RWTexture2D<float> output;
  // expected-error@+2 {{'globallycoherent' is not a valid modifier for a declaration of type 'float'}}
  // expected-note@+1 {{'globallycoherent' can only be applied to UAV or RWDispatchNodeInputRecord objects}}
  globallycoherent float ThisShouldBreak = 2.0;
}

[shader("compute")]
[numthreads(8, 8, 1)] void main(uint threadId
                                : SV_DispatchThreadID) {
  // expected-note@+1 {{in instantiation of function template specialization 'doSomething<float>' requested here}}
  doSomething<float>(threadId);
  doSomething2(threadId);
}

template <typename T>
void doSomething3(T data) {
  T local1 = data;

  // expected-warning@+1 {{implicit conversion from 'RWDispatchNodeInputRecord<Record>' to 'globallycoherent RWDispatchNodeInputRecord<Record>' adds globallycoherent annotation}}
  globallycoherent T local2 = data;
}

template <typename T>
void doSomething4(globallycoherent T data) {
  // expected-warning@+1 {{implicit conversion from 'globallycoherent RWDispatchNodeInputRecord<Record>' to 'RWDispatchNodeInputRecord<Record>' loses globallycoherent annotation}}
  doSomething3(data);

  // expected-warning@+1 {{implicit conversion from 'globallycoherent RWDispatchNodeInputRecord<Record>' to 'RWDispatchNodeInputRecord<Record>' loses globallycoherent annotation}}
  T local1 = data;

  globallycoherent T local2 = data;

  doSomething3(local1);
}

// TBD: Support node types in templates.
//  Currently, these produce:
//  'T' cannot be used as a type parameter where a struct/class is required
//  Is this #5729?
//template <typename T>
//void doSomething5() {
//  RWDispatchNodeInputRecord<T> local1;
//  globallycoherent RWDispatchNodeInputRecord<T> local2;
//  local1 = local2;
//}
//template <typename T>
//void doSomething6(RWDispatchNodeInputRecord<T> data) {
//  globallycoherent RWDispatchNodeInputRecord<T> local = data;
//}
//template <typename T>
//void doSomething7(globallycoherent RWDispatchNodeInputRecord<T> data) {
//  RWDispatchNodeInputRecord<T> local = data;
//}

struct Record { uint index; };

[Shader("node")]
[NodeLaunch("broadcasting")]
[NodeDispatchGrid(2, 1, 1)]
[numthreads(1, 1, 1)]
void main2(globallycoherent RWDispatchNodeInputRecord<Record> inputData) {
  // expected-warning@+2 {{implicit conversion from 'globallycoherent RWDispatchNodeInputRecord<Record>' to 'RWDispatchNodeInputRecord<Record>' loses globallycoherent annotation}}
  // expected-note@+1 {{in instantiation of function template specialization 'doSomething3<RWDispatchNodeInputRecord<Record> >' requested here}}
  doSomething3(inputData);
  // expected-note@+1 {{in instantiation of function template specialization 'doSomething4<RWDispatchNodeInputRecord<Record> >' requested here}}
  doSomething4(inputData);
  // expected-warning@+1 {{implicit conversion from 'globallycoherent RWDispatchNodeInputRecord<Record>' to 'RWDispatchNodeInputRecord<Record>' loses globallycoherent annotation}}
  RWDispatchNodeInputRecord<Record> local = inputData;
  // expected-warning@+1 {{implicit conversion from 'RWDispatchNodeInputRecord<Record>' to 'globallycoherent RWDispatchNodeInputRecord<Record>' adds globallycoherent annotation}}
  doSomething4(local);
  //doSomething5<Record>();
  //doSomething6(inputData);
  //doSomething7(inputData);
}
