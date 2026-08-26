// Source file for unaltered known_nodeio_tags.ll
// and the altered unknown_nodeio_tags.ll
// Not intended for indpendent testing

// Run line required in this location, so we'll verify compilation succeeds.
// RUN: %dxc -T lib_6_8 %s | FileCheck %s
// CHECK: define void @main()

struct MY_INPUT_RECORD {
  float foo;
};

struct MY_RECORD {
  float bar;
};

struct MY_MATERIAL_RECORD {
  uint textureIndex;
};

[Shader("node")]
[NodeLaunch("broadcasting")]
[NodeDispatchGrid(2,3,2)]
[NumThreads(1024,1,1)]
[NodeIsProgramEntry]
void main(DispatchNodeInputRecord<MY_INPUT_RECORD> myInput,
              [MaxRecords(7)]
              NodeOutput<MY_RECORD> myFascinatingNode,
              [MaxRecordsSharedWith(myFascinatingNode)]
              [AllowSparseNodes]
              [NodeArraySize(63)] NodeOutputArray<MY_MATERIAL_RECORD> myMaterials)
{
  // Don't really need to do anything
}
