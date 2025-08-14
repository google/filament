// RUN: %dxc -T lib_6_8 %s | FileCheck %s

// Check that the SV_DispatchGrid DXIL metadata for a node input record is
// generated in cases where:
// node1 - the field with the SV_DispatchGrid semantic is in a nested record
// node2 - the field with the SV_DispatchGrid semantic is in a record field
// node3 - the field with the SV_DispatchGrid semantic is inherited from a base record
// node4 - the field with the SV_DispatchGrid semantic is within a nested record inherited from a base record
// node5 - the field with the SV_DispatchGrid semantic is within a base record of a nested record
// node6 - the field with the SV_DispatchGrid semantic is within a templated base record
// node7 - the field with the SV_DispatchGrid semantic is within a templated base record of a templated record
// node8 - the field with the SV_DispatchGrid semantic has templated type

struct Record1 {
    struct {
      // SV_DispatchGrid is within a nested record
      uint3 grid : SV_DispatchGrid;
    };
};

[Shader("node")]
[NodeMaxDispatchGrid(32,16,1)]
[NumThreads(32,1,1)]
void node1(DispatchNodeInputRecord<Record1> input) {}
// CHECK: {!"node1"
// CHECK: , i32 1, ![[SVDG_1:[0-9]+]]
// CHECK: [[SVDG_1]] = !{i32 0, i32 5, i32 3}

struct Record2a {
  uint u;
  uint2 grid : SV_DispatchGrid;
};

struct Record2 {
  uint a;
  // SV_DispatchGrid is within a record field
  Record2a b;
};

[Shader("node")]
[NodeMaxDispatchGrid(32,16,1)]
[NumThreads(32,1,1)]
void node2(DispatchNodeInputRecord<Record2> input) {}
// CHECK: {!"node2"
// CHECK: , i32 1, ![[SVDG_2:[0-9]+]]
// CHECK: [[SVDG_2]] = !{i32 8, i32 5, i32 2}

struct Record3 : Record2a {
  // SV_DispatchGrid is inherited
  uint4 n;
};

[Shader("node")]
[NodeMaxDispatchGrid(32,16,1)]
[NumThreads(32,1,1)]
void node3(DispatchNodeInputRecord<Record3> input) {}
// CHECK: {!"node3"
// CHECK: , i32 1, ![[SVDG_3:[0-9]+]]
// CHECK: [[SVDG_3]] = !{i32 4, i32 5, i32 2}

struct Record4 : Record2 {
  // SV_DispatchGrid is in a nested field in a base record
  float f;
};

[Shader("node")]
[NodeMaxDispatchGrid(32,16,1)]
[NumThreads(32,1,1)]
void node4(DispatchNodeInputRecord<Record4> input) {}
// CHECK: {!"node4"
// CHECK: , i32 1, ![[SVDG_2]]

struct Record5 {
  uint4 x;
  // SV_DispatchGrid is in a base record of a record field
  Record3 r;
};

[Shader("node")]
[NodeLaunch("broadcasting")]
[NodeMaxDispatchGrid(32,16,1)]
[NumThreads(32,1,1)]
void node5(DispatchNodeInputRecord<Record5> input) {}
// CHECK: {!"node5"
// CHECK: , i32 1, ![[SVDG_5:[0-9]+]]
// CHECK: [[SVDG_5]] = !{i32 20, i32 5, i32 2}

template <typename T>
struct Base {
  T DG : SV_DispatchGrid;
};

struct Derived1 : Base<uint3> {
  int4 x;
};

[Shader("node")]
[NodeLaunch("broadcasting")]
[NodeMaxDispatchGrid(32,16,1)]
[NumThreads(32,1,1)]
void node6(DispatchNodeInputRecord<Derived1 > input) {}
// CHECK: {!"node6"
// CHECK: , i32 1, ![[SVDG_1]]

template <typename T>
struct Derived2 : Base<T> {
  T Y;
};

[Shader("node")]
[NodeLaunch("broadcasting")]
[NodeMaxDispatchGrid(32,16,1)]
[NumThreads(32,1,1)]
void node7(DispatchNodeInputRecord<Derived2<uint2> > input) {}
// CHECK: {!"node7"
// CHECK: , i32 1, ![[SVDG_7:[0-9]+]]
// CHECK: [[SVDG_7]] = !{i32 0, i32 5, i32 2}

template <typename T>
struct Derived3 {
  Derived2<T> V;
};

[Shader("node")]
[NodeLaunch("broadcasting")]
[NodeMaxDispatchGrid(32,16,1)]
[NumThreads(32,1,1)]
void node8(DispatchNodeInputRecord< Derived3 <uint3> > input) {}
// CHECK: {!"node8"
// CHECK: , i32 1, ![[SVDG_1]]
