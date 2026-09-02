// RUN: %dxc -Tlib_6_8 %s -ast-dump | FileCheck %s

// Make sure NodeOutput attribute works at AST level.

struct MY_INPUT_RECORD
    {
        float value;
        uint data;
    };

    struct MY_RECORD
    {
        uint3 dispatchGrid : SV_DispatchGrid;
        // shader arguments:
        uint foo;
        float bar;
    };
    struct MY_MATERIAL_RECORD
    {
        uint textureIndex;
        float3 normal;
    };

// CHECK:FunctionDecl 0x{{.*}} myFancyNode 'void (GroupNodeInputRecords<MY_INPUT_RECORD>, NodeOutput<MY_RECORD>, NodeOutput<MY_RECORD>, NodeOutputArray<MY_MATERIAL_RECORD>, EmptyNodeOutput)'
// CHECK-NEXT:ParmVarDecl 0x{{.*}} myInput 'GroupNodeInputRecords<MY_INPUT_RECORD>':'GroupNodeInputRecords<MY_INPUT_RECORD>'
// CHECK-NEXT: HLSLMaxRecordsAttr 0x{{.*}} 4
// CHECK-NEXT: ParmVarDecl 0x{{.*}} myFascinatingNode 'NodeOutput<MY_RECORD>':'NodeOutput<MY_RECORD>'
// CHECK-NEXT: HLSLMaxRecordsAttr 0x{{.*}} 4
// CHECK-NEXT: ParmVarDecl 0x{{.*}} myRecords 'NodeOutput<MY_RECORD>':'NodeOutput<MY_RECORD>'
// CHECK-NEXT: HLSLMaxRecordsAttr 0x{{.*}} 4
// CHECK-NEXT: HLSLNodeIdAttr 0x{{.*}} "myNiftyNode" 3
// CHECK-NEXT: ParmVarDecl 0x{{.*}} col:65 myMaterials 'NodeOutputArray<MY_MATERIAL_RECORD>'
// CHECK-NEXT:HLSLNodeArraySizeAttr 0x{{.*}} 63
// CHECK-NEXT:HLSLAllowSparseNodesAttr 0x{{.*}}
// CHECK-NEXT:HLSLMaxRecordsSharedWithAttr 0x{{.*}} myRecords
// CHECK-NEXT:ParmVarDecl 0x{{.*}} myProgressCounter 'EmptyNodeOutput'
// CHECK-NEXT: HLSLMaxRecordsAttr 0x{{.*}} 20
// CHECK-NEXT: CompoundStmt 0x
// CHECK-NEXT: HLSLNumThreadsAttr 0x{{.*}} 4 5 6
// CHECK-NEXT: HLSLNodeLaunchAttr 0x{{.*}} "coalescing"
// CHECK-NEXT: HLSLShaderAttr 0x{{.*}} "node"
    [Shader("node")]
    [NodeLaunch("coalescing")]
    [NumThreads(4,5,6)]
    void myFancyNode(

        [MaxRecords(4)] GroupNodeInputRecords<MY_INPUT_RECORD> myInput,

        [MaxRecords(4)] NodeOutput<MY_RECORD> myFascinatingNode,

        [NodeID("myNiftyNode",3)] [MaxRecords(4)] NodeOutput<MY_RECORD> myRecords,

        [MaxRecordsSharedWith(myRecords)]
        [AllowSparseNodes]
        [NodeArraySize(63)] NodeOutputArray<MY_MATERIAL_RECORD> myMaterials,

        // an output that has empty record size
        [MaxRecords(20)] EmptyNodeOutput myProgressCounter
        )
    {
    }
