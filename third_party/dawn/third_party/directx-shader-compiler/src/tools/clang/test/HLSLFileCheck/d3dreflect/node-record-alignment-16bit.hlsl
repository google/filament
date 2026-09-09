// RUN: %dxilver 1.8 | %dxc -T lib_6_8 -enable-16bit-types %s | %D3DReflect %s | %FileCheck %s -check-prefixes=RDAT

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
    BAB.Store(0, Input.Get().x.x); \
  }

// RDAT: FunctionTable[{{.*}}] = {

// RDAT-LABEL: UnmangledName: "node_half"
// RDAT: RecordAlignmentInBytes: 2
TEST_TYPE(node_half, half)

// RDAT-LABEL: UnmangledName: "node_float16"
// RDAT: RecordAlignmentInBytes: 2
TEST_TYPE(node_float16, float16_t)

// RDAT-LABEL: UnmangledName: "node_int16"
// RDAT: RecordAlignmentInBytes: 2
TEST_TYPE(node_int16, int16_t)

// min16 types are converted to native 16-bit types.

// RDAT-LABEL: UnmangledName: "node_min16float"
// RDAT: RecordAlignmentInBytes: 2
TEST_TYPE(node_min16float, min16float)

// RDAT-LABEL: UnmangledName: "node_min16int"
// RDAT: RecordAlignmentInBytes: 2
TEST_TYPE(node_min16int, min16int)
