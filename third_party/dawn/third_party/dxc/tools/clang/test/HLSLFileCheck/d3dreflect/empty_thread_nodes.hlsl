// RUN: %dxc -T lib_6_8  %s | %D3DReflect %s | FileCheck %s


// CHECK:DxilRuntimeData (size = {{[0-9]+}} bytes):
// CHECK:  StringBuffer (size = {{[0-9]+}} bytes)
// CHECK:  IndexTable (size = {{[0-9]+}} bytes)
// CHECK:  RawBytes (size = {{[0-9]+}} bytes)
// CHECK:  RecordTable (stride = {{[0-9]+}} bytes) FunctionTable[1] = {
// CHECK:    <0:RuntimeDataFunctionInfo{{.*}}> = {
// CHECK:      Name: "Input2Output"
// CHECK:      UnmangledName: "Input2Output"
// CHECK:      Resources: <RecordArrayRef<RuntimeDataResourceInfo>[0]> = {}
// CHECK:      FunctionDependencies: <string[0]> = {}
// CHECK:      ShaderKind: Node
// CHECK:      PayloadSizeInBytes: 0
// CHECK:      AttributeSizeInBytes: 0
// CHECK:      FeatureInfo1: 0
// CHECK:      FeatureInfo2: 0
// CHECK:      ShaderStageFlag: (Node)
// CHECK:      MinShaderTarget: 0xf0068
// CHECK:      MinimumExpectedWaveLaneCount: 0
// CHECK:      MaximumExpectedWaveLaneCount: 0
// CHECK:      ShaderFlags: 0 (None)
// CHECK:      Node: <0:NodeShaderInfo> = {
// CHECK:        LaunchType: Thread
// CHECK:        GroupSharedBytesUsed: 0
// CHECK:        Attribs: <4:RecordArrayRef<NodeShaderFuncAttrib>[3]>  = {
// CHECK:          [0]: <0:NodeShaderFuncAttrib> = {
// CHECK:            AttribKind: ID
// CHECK:            ID: <0:NodeID> = {
// CHECK:              Name: "Input2Output"
// CHECK:              Index: 0
// CHECK:            }
// CHECK:          }
// CHECK:          [1]: <1:NodeShaderFuncAttrib> = {
// CHECK:            AttribKind: NumThreads
// CHECK:            NumThreads: <0:array[3]> = { 1, 1, 1 }
// CHECK:          }
// CHECK:          [2]: <2:NodeShaderFuncAttrib> = {
// CHECK:            AttribKind: LocalRootArgumentsTableIndex
// CHECK:            LocalRootArgumentsTableIndex: 2
// CHECK:          }
// CHECK:        }
// CHECK:        Outputs: <25:RecordArrayRef<IONode>[2]>  = {
// CHECK:          [0]: <1:IONode> = {
// CHECK:            IOFlagsAndKind: 262
// CHECK:            Attribs: <{{[0-9]+}}:RecordArrayRef<NodeShaderIOAttrib>[5]>  = {
// CHECK:              [0]: <2:NodeShaderIOAttrib> = {
// CHECK:                AttribKind: OutputID
// CHECK:                OutputID: <1:NodeID> = {
// CHECK:                  Name: "Output1"
// CHECK:                  Index: 0
// CHECK:                }
// CHECK:              }
// CHECK:              [1]: <3:NodeShaderIOAttrib> = {
// CHECK:                AttribKind: OutputArraySize
// CHECK:                OutputArraySize: 0
// CHECK:              }
// CHECK:              [2]: <4:NodeShaderIOAttrib> = {
// CHECK:                AttribKind: MaxRecordsSharedWith
// CHECK:                MaxRecordsSharedWith: 1
// CHECK:              }
// CHECK:              [3]: <0:NodeShaderIOAttrib> = {
// CHECK:                AttribKind: RecordSizeInBytes
// CHECK:                RecordSizeInBytes: 8
// CHECK:              }
// CHECK:              [4]: <1:NodeShaderIOAttrib> = {
// CHECK:                AttribKind: RecordAlignmentInBytes
// CHECK:                RecordAlignmentInBytes: 4
// CHECK:              }
// CHECK:            }
// CHECK:          }
// CHECK:          [1]: <2:IONode> = {
// CHECK:            IOFlagsAndKind: 6
// CHECK:            Attribs: <{{[0-9]+}}:RecordArrayRef<NodeShaderIOAttrib>[5]>  = {
// CHECK:              [0]: <5:NodeShaderIOAttrib> = {
// CHECK:                AttribKind: OutputID
// CHECK:                OutputID: <2:NodeID> = {
// CHECK:                  Name: "Output2ID"
// CHECK:                  Index: 1
// CHECK:                }
// CHECK:              }
// CHECK:              [1]: <3:NodeShaderIOAttrib>
// CHECK:              [2]: <6:NodeShaderIOAttrib> = {
// CHECK:                AttribKind: MaxRecords
// CHECK:                MaxRecords: 5
// CHECK:              }
// CHECK:              [3]: <0:NodeShaderIOAttrib>
// CHECK:              [4]: <1:NodeShaderIOAttrib>
// CHECK:            }
// CHECK:          }
// CHECK:        }
// CHECK:        Inputs: <{{[0-9]+}}:RecordArrayRef<IONode>[1]>  = {
// CHECK:          [0]: <0:IONode> = {
// CHECK:            IOFlagsAndKind: 37
// CHECK:            Attribs: <{{[0-9]+}}:RecordArrayRef<NodeShaderIOAttrib>[2]>  = {
// CHECK:              [0]: <0:NodeShaderIOAttrib>
// CHECK:              [1]: <1:NodeShaderIOAttrib>
// CHECK:            }
// CHECK:          }
// CHECK:        }
// CHECK:      }
// CHECK:    }
// CHECK:  }
// CHECK:  RecordTable (stride = {{[0-9]+}} bytes) NodeIDTable[3] = {
// CHECK:    <0:NodeID> = {
// CHECK:      Name: "Input2Output"
// CHECK:      Index: 0
// CHECK:    }
// CHECK:    <1:NodeID> = {
// CHECK:      Name: "Output1"
// CHECK:      Index: 0
// CHECK:    }
// CHECK:    <2:NodeID> = {
// CHECK:      Name: "Output2ID"
// CHECK:      Index: 1
// CHECK:    }
// CHECK:  }
// CHECK:  RecordTable (stride = {{[0-9]+}} bytes) NodeShaderFuncAttribTable[3] = {
// CHECK:    <0:NodeShaderFuncAttrib> = {
// CHECK:      AttribKind: ID
// CHECK:      ID: <0:NodeID> = {
// CHECK:        Name: "Input2Output"
// CHECK:        Index: 0
// CHECK:      }
// CHECK:    }
// CHECK:    <1:NodeShaderFuncAttrib> = {
// CHECK:      AttribKind: NumThreads
// CHECK:      NumThreads: <0:array[3]> = { 1, 1, 1 }
// CHECK:    }
// CHECK:    <2:NodeShaderFuncAttrib> = {
// CHECK:      AttribKind: LocalRootArgumentsTableIndex
// CHECK:      LocalRootArgumentsTableIndex: 2
// CHECK:    }
// CHECK:  }
// CHECK:  RecordTable (stride = {{[0-9]+}} bytes) NodeShaderIOAttribTable[7] = {
// CHECK:    <0:NodeShaderIOAttrib> = {
// CHECK:      AttribKind: RecordSizeInBytes
// CHECK:      RecordSizeInBytes: 8
// CHECK:    }
// CHECK:    <1:NodeShaderIOAttrib> = {
// CHECK:      AttribKind: RecordAlignmentInBytes
// CHECK:      RecordAlignmentInBytes: 4
// CHECK:    }
// CHECK:    <2:NodeShaderIOAttrib> = {
// CHECK:      AttribKind: OutputID
// CHECK:      OutputID: <1:NodeID> = {
// CHECK:        Name: "Output1"
// CHECK:        Index: 0
// CHECK:      }
// CHECK:    }
// CHECK:    <3:NodeShaderIOAttrib> = {
// CHECK:      AttribKind: OutputArraySize
// CHECK:      OutputArraySize: 0
// CHECK:    }
// CHECK:    <4:NodeShaderIOAttrib> = {
// CHECK:      AttribKind: MaxRecordsSharedWith
// CHECK:      MaxRecordsSharedWith: 1
// CHECK:    }
// CHECK:    <5:NodeShaderIOAttrib> = {
// CHECK:      AttribKind: OutputID
// CHECK:      OutputID: <2:NodeID> = {
// CHECK:        Name: "Output2ID"
// CHECK:        Index: 1
// CHECK:      }
// CHECK:    }
// CHECK:    <6:NodeShaderIOAttrib> = {
// CHECK:      AttribKind: MaxRecords
// CHECK:      MaxRecords: 5
// CHECK:    }
// CHECK:  }
// CHECK:  RecordTable (stride = {{[0-9]+}} bytes) IONodeTable[3] = {
// CHECK:    <0:IONode> = {
// CHECK:      IOFlagsAndKind: 37
// CHECK:      Attribs: <8:RecordArrayRef<NodeShaderIOAttrib>[2]>  = {
// CHECK:        [0]: <0:NodeShaderIOAttrib> = {
// CHECK:          AttribKind: RecordSizeInBytes
// CHECK:          RecordSizeInBytes: 8
// CHECK:        }
// CHECK:        [1]: <1:NodeShaderIOAttrib> = {
// CHECK:          AttribKind: RecordAlignmentInBytes
// CHECK:          RecordAlignmentInBytes: 4
// CHECK:        }
// CHECK:      }
// CHECK:    }
// CHECK:    <1:IONode> = {
// CHECK:      IOFlagsAndKind: 262
// CHECK:      Attribs: <13:RecordArrayRef<NodeShaderIOAttrib>[5]>  = {
// CHECK:        [0]: <2:NodeShaderIOAttrib> = {
// CHECK:          AttribKind: OutputID
// CHECK:          OutputID: <1:NodeID> = {
// CHECK:            Name: "Output1"
// CHECK:            Index: 0
// CHECK:          }
// CHECK:        }
// CHECK:        [1]: <3:NodeShaderIOAttrib> = {
// CHECK:          AttribKind: OutputArraySize
// CHECK:          OutputArraySize: 0
// CHECK:        }
// CHECK:        [2]: <4:NodeShaderIOAttrib> = {
// CHECK:          AttribKind: MaxRecordsSharedWith
// CHECK:          MaxRecordsSharedWith: 1
// CHECK:        }
// CHECK:        [3]: <0:NodeShaderIOAttrib> = {
// CHECK:          AttribKind: RecordSizeInBytes
// CHECK:          RecordSizeInBytes: 8
// CHECK:        }
// CHECK:        [4]: <1:NodeShaderIOAttrib> = {
// CHECK:          AttribKind: RecordAlignmentInBytes
// CHECK:          RecordAlignmentInBytes: 4
// CHECK:        }
// CHECK:      }
// CHECK:    }
// CHECK:    <2:IONode> = {
// CHECK:      IOFlagsAndKind: 6
// CHECK:      Attribs: <19:RecordArrayRef<NodeShaderIOAttrib>[5]>  = {
// CHECK:        [0]: <5:NodeShaderIOAttrib> = {
// CHECK:          AttribKind: OutputID
// CHECK:          OutputID: <2:NodeID> = {
// CHECK:            Name: "Output2ID"
// CHECK:            Index: 1
// CHECK:          }
// CHECK:        }
// CHECK:        [1]: <3:NodeShaderIOAttrib> = {
// CHECK:          AttribKind: OutputArraySize
// CHECK:          OutputArraySize: 0
// CHECK:        }
// CHECK:        [2]: <6:NodeShaderIOAttrib> = {
// CHECK:          AttribKind: MaxRecords
// CHECK:          MaxRecords: 5
// CHECK:        }
// CHECK:        [3]: <0:NodeShaderIOAttrib> = {
// CHECK:          AttribKind: RecordSizeInBytes
// CHECK:          RecordSizeInBytes: 8
// CHECK:        }
// CHECK:        [4]: <1:NodeShaderIOAttrib> = {
// CHECK:          AttribKind: RecordAlignmentInBytes
// CHECK:          RecordAlignmentInBytes: 4
// CHECK:        }
// CHECK:      }
// CHECK:    }
// CHECK:  }
// CHECK:  RecordTable (stride = {{[0-9]+}} bytes) NodeShaderInfoTable[1] = {
// CHECK:    <0:NodeShaderInfo> = {
// CHECK:      LaunchType: Thread
// CHECK:      GroupSharedBytesUsed: 0
// CHECK:      Attribs: <4:RecordArrayRef<NodeShaderFuncAttrib>[3]>  = {
// CHECK:        [0]: <0:NodeShaderFuncAttrib> = {
// CHECK:          AttribKind: ID
// CHECK:          ID: <0:NodeID> = {
// CHECK:            Name: "Input2Output"
// CHECK:            Index: 0
// CHECK:          }
// CHECK:        }
// CHECK:        [1]: <1:NodeShaderFuncAttrib> = {
// CHECK:          AttribKind: NumThreads
// CHECK:          NumThreads: <0:array[3]> = { 1, 1, 1 }
// CHECK:        }
// CHECK:        [2]: <2:NodeShaderFuncAttrib> = {
// CHECK:          AttribKind: LocalRootArgumentsTableIndex
// CHECK:          LocalRootArgumentsTableIndex: 2
// CHECK:        }
// CHECK:      }
// CHECK:      Outputs: <25:RecordArrayRef<IONode>[2]>  = {
// CHECK:        [0]: <1:IONode> = {
// CHECK:          IOFlagsAndKind: 262
// CHECK:          Attribs: <13:RecordArrayRef<NodeShaderIOAttrib>[5]>  = {
// CHECK:            [0]: <2:NodeShaderIOAttrib> = {
// CHECK:              AttribKind: OutputID
// CHECK:              OutputID: <1:NodeID> = {
// CHECK:                Name: "Output1"
// CHECK:                Index: 0
// CHECK:              }
// CHECK:            }
// CHECK:            [1]: <3:NodeShaderIOAttrib> = {
// CHECK:              AttribKind: OutputArraySize
// CHECK:              OutputArraySize: 0
// CHECK:            }
// CHECK:            [2]: <4:NodeShaderIOAttrib> = {
// CHECK:              AttribKind: MaxRecordsSharedWith
// CHECK:              MaxRecordsSharedWith: 1
// CHECK:            }
// CHECK:            [3]: <0:NodeShaderIOAttrib> = {
// CHECK:              AttribKind: RecordSizeInBytes
// CHECK:              RecordSizeInBytes: 8
// CHECK:            }
// CHECK:            [4]: <1:NodeShaderIOAttrib> = {
// CHECK:              AttribKind: RecordAlignmentInBytes
// CHECK:              RecordAlignmentInBytes: 4
// CHECK:            }
// CHECK:          }
// CHECK:        }
// CHECK:        [1]: <2:IONode> = {
// CHECK:          IOFlagsAndKind: 6
// CHECK:          Attribs: <19:RecordArrayRef<NodeShaderIOAttrib>[5]>  = {
// CHECK:            [0]: <5:NodeShaderIOAttrib> = {
// CHECK:              AttribKind: OutputID
// CHECK:              OutputID: <2:NodeID> = {
// CHECK:                Name: "Output2ID"
// CHECK:                Index: 1
// CHECK:              }
// CHECK:            }
// CHECK:            [1]: <3:NodeShaderIOAttrib>
// CHECK:            [2]: <6:NodeShaderIOAttrib> = {
// CHECK:              AttribKind: MaxRecords
// CHECK:              MaxRecords: 5
// CHECK:            }
// CHECK:            [3]: <0:NodeShaderIOAttrib>
// CHECK:            [4]: <1:NodeShaderIOAttrib>
// CHECK:          }
// CHECK:        }
// CHECK:      }
// CHECK:      Inputs: <11:RecordArrayRef<IONode>[1]>  = {
// CHECK:        [0]: <0:IONode> = {
// CHECK:          IOFlagsAndKind: 37
// CHECK:          Attribs: <8:RecordArrayRef<NodeShaderIOAttrib>[2]>  = {
// CHECK:            [0]: <0:NodeShaderIOAttrib>
// CHECK:            [1]: <1:NodeShaderIOAttrib>
// CHECK:          }
// CHECK:        }
// CHECK:      }
// CHECK:    }
// CHECK:  }
// CHECK:ID3D12LibraryReflection:
// CHECK:  D3D12_LIBRARY_DESC:
// FIXME:    Creator: <nullptr>
// CHECK:    Flags: 0
// CHECK:    FunctionCount: 1
// CHECK:  ID3D12FunctionReflection:
// CHECK:    D3D12_FUNCTION_DESC: Name: Input2Output
// FIXME:      Shader Version: <unknown> 6.8
// FIXME:      Creator: <nullptr>
// CHECK:      Flags: 0
// CHECK:      ConstantBuffers: 0
// CHECK:      BoundResources: 0
// CHECK:      FunctionParameterCount: 0
// CHECK:      HasReturn: FALSE

struct rec0
{
    int i0;
    float f0;
};

struct rec1
{
    float f1;
    int i1;
};

struct [NodeTrackRWInputSharing] rec2
{
    float f1;
    int i1;
};

[Shader("node")]
[NodeLaunch("thread")]
[NodeLocalRootArgumentsTableIndex(2)]
void Input2Output(
  RWThreadNodeInputRecord<rec0> Inputy,
  [MaxRecordsSharedWith(Output2)] NodeOutput<rec2> Output1,
  [NodeID("Output2ID",1)] [MaxRecords(5)] NodeOutput<rec1> Output2)
{
}
