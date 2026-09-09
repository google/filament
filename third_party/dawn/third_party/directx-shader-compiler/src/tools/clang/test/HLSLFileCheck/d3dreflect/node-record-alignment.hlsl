// RUN: %dxilver 1.8 | %dxc -T lib_6_8 %s | %D3DReflect %s | %FileCheck %s -check-prefixes=RDAT

// Ensure alignment field set correctly in RDAT for various node records

RWByteAddressBuffer BAB : register(u1, space0);

#define GLUE2(x, y) x##y
#define GLUE(x, y) GLUE2(x, y)

#define TEST_TYPE(EntryName, CompType) \
  struct GLUE(EntryName, _record) { \
    vector<CompType, 3> x; \
  }; \
  [shader("node")] \
  [NodeLaunch("broadcasting")] \
  [NodeDispatchGrid(1, 1, 1)] \
  [NumThreads(1,1,1)] \
  void EntryName(DispatchNodeInputRecord<GLUE(EntryName, _record)> Input) { \
    BAB.Store(0, (uint)Input.Get().x.x); \
  }

// RDAT: FunctionTable[{{.*}}] = {

// RDAT-LABEL: UnmangledName: "node_half"
// RDAT: RecordAlignmentInBytes: 4
TEST_TYPE(node_half, half)

// RDAT-LABEL: UnmangledName: "node_float"
// RDAT: RecordAlignmentInBytes: 4
TEST_TYPE(node_float, float)

// RDAT-LABEL: UnmangledName: "node_double"
// RDAT: RecordAlignmentInBytes: 8
TEST_TYPE(node_double, double)

// RDAT-LABEL: UnmangledName: "node_int"
// RDAT: RecordAlignmentInBytes: 4
TEST_TYPE(node_int, int)

// RDAT-LABEL: UnmangledName: "node_uint64"
// RDAT: RecordAlignmentInBytes: 8
TEST_TYPE(node_uint64, uint64_t)

// Min-precision types are still 4 bytes in storage.

// RDAT-LABEL: UnmangledName: "node_min16float"
// RDAT: RecordAlignmentInBytes: 4
TEST_TYPE(node_min16float, min16float)

// RDAT-LABEL: UnmangledName: "node_min16int"
// RDAT: RecordAlignmentInBytes: 4
TEST_TYPE(node_min16int, min16int)

// Test alignment preserved for unused input record:
// RDAT-LABEL: UnmangledName: "node_input_unused_double"
// RDAT: RecordAlignmentInBytes: 8
struct UnusedDoubleInputRecord {
  vector<double, 3> x;
};
[shader("node")]
[NodeLaunch("broadcasting")]
[NodeDispatchGrid(1, 1, 1)]
[NumThreads(1,1,1)]
void node_input_unused_double(
    DispatchNodeInputRecord<UnusedDoubleInputRecord> Input) {
  BAB.Store(0, 0);
}

// Test alignment preserved for unused output record:
// RDAT-LABEL: UnmangledName: "node_output_unused_double"
// RDAT: RecordAlignmentInBytes: 8
struct UnusedDoubleOutputRecord {
  vector<double, 3> x;
};
[shader("node")]
[NodeLaunch("broadcasting")]
[NodeDispatchGrid(1, 1, 1)]
[NumThreads(1,1,1)]
void node_output_unused_double(
    NodeOutput<UnusedDoubleOutputRecord> Output) {
  BAB.Store(0, 0);
}
