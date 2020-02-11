/*Copyright (c) 2014-2016 The Khronos Group Inc.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and/or associated documentation files (the "Materials"),
to deal in the Materials without restriction, including without limitation
the rights to use, copy, modify, merge, publish, distribute, sublicense,
and/or sell copies of the Materials, and to permit persons to whom the
Materials are furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Materials.

MODIFICATIONS TO THIS FILE MAY MEAN IT NO LONGER ACCURATELY REFLECTS KHRONOS
STANDARDS. THE UNMODIFIED, NORMATIVE VERSIONS OF KHRONOS SPECIFICATIONS AND
HEADER INFORMATION ARE LOCATED AT https://www.khronos.org/registry/ 

THE MATERIALS ARE PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
FROM,OUT OF OR IN CONNECTION WITH THE MATERIALS OR THE USE OR OTHER DEALINGS
IN THE MATERIALS.*/

// THIS FILE IS GENERATED WITH tools/process_grammar.rb

export default {
  "magic": "0x07230203",
  "version": [
    1,
    5
  ],
  "instructions": {
    "OpNop": {
      "opcode": 0,
      "operands": [

      ]
    },
    "OpUndef": {
      "opcode": 1,
      "operands": [
        {
          "kind": "IdResultType"
        },
        {
          "kind": "IdResult"
        }
      ]
    },
    "OpSourceContinued": {
      "opcode": 2,
      "operands": [
        {
          "kind": "LiteralString"
        }
      ]
    },
    "OpSource": {
      "opcode": 3,
      "operands": [
        {
          "kind": "SourceLanguage"
        },
        {
          "kind": "LiteralInteger"
        },
        {
          "kind": "IdRef",
          "quantifier": "?"
        },
        {
          "kind": "LiteralString",
          "quantifier": "?"
        }
      ]
    },
    "OpSourceExtension": {
      "opcode": 4,
      "operands": [
        {
          "kind": "LiteralString"
        }
      ]
    },
    "OpName": {
      "opcode": 5,
      "operands": [
        {
          "kind": "IdRef"
        },
        {
          "kind": "LiteralString"
        }
      ]
    },
    "OpMemberName": {
      "opcode": 6,
      "operands": [
        {
          "kind": "IdRef"
        },
        {
          "kind": "LiteralInteger"
        },
        {
          "kind": "LiteralString"
        }
      ]
    },
    "OpString": {
      "opcode": 7,
      "operands": [
        {
          "kind": "IdResult"
        },
        {
          "kind": "LiteralString"
        }
      ]
    },
    "OpLine": {
      "opcode": 8,
      "operands": [
        {
          "kind": "IdRef"
        },
        {
          "kind": "LiteralInteger"
        },
        {
          "kind": "LiteralInteger"
        }
      ]
    },
    "OpExtension": {
      "opcode": 10,
      "operands": [
        {
          "kind": "LiteralString"
        }
      ]
    },
    "OpExtInstImport": {
      "opcode": 11,
      "operands": [
        {
          "kind": "IdResult"
        },
        {
          "kind": "LiteralString"
        }
      ]
    },
    "OpExtInst": {
      "opcode": 12,
      "operands": [
        {
          "kind": "IdResultType"
        },
        {
          "kind": "IdResult"
        },
        {
          "kind": "IdRef"
        },
        {
          "kind": "LiteralExtInstInteger"
        },
        {
          "kind": "IdRef",
          "quantifier": "*"
        }
      ]
    },
    "OpMemoryModel": {
      "opcode": 14,
      "operands": [
        {
          "kind": "AddressingModel"
        },
        {
          "kind": "MemoryModel"
        }
      ]
    },
    "OpEntryPoint": {
      "opcode": 15,
      "operands": [
        {
          "kind": "ExecutionModel"
        },
        {
          "kind": "IdRef"
        },
        {
          "kind": "LiteralString"
        },
        {
          "kind": "IdRef",
          "quantifier": "*"
        }
      ]
    },
    "OpExecutionMode": {
      "opcode": 16,
      "operands": [
        {
          "kind": "IdRef"
        },
        {
          "kind": "ExecutionMode"
        }
      ]
    },
    "OpCapability": {
      "opcode": 17,
      "operands": [
        {
          "kind": "Capability"
        }
      ]
    },
    "OpTypeVoid": {
      "opcode": 19,
      "operands": [
        {
          "kind": "IdResult"
        }
      ]
    },
    "OpTypeBool": {
      "opcode": 20,
      "operands": [
        {
          "kind": "IdResult"
        }
      ]
    },
    "OpTypeInt": {
      "opcode": 21,
      "operands": [
        {
          "kind": "IdResult"
        },
        {
          "kind": "LiteralInteger"
        },
        {
          "kind": "LiteralInteger"
        }
      ]
    },
    "OpTypeFloat": {
      "opcode": 22,
      "operands": [
        {
          "kind": "IdResult"
        },
        {
          "kind": "LiteralInteger"
        }
      ]
    },
    "OpTypeVector": {
      "opcode": 23,
      "operands": [
        {
          "kind": "IdResult"
        },
        {
          "kind": "IdRef"
        },
        {
          "kind": "LiteralInteger"
        }
      ]
    },
    "OpTypeMatrix": {
      "opcode": 24,
      "operands": [
        {
          "kind": "IdResult"
        },
        {
          "kind": "IdRef"
        },
        {
          "kind": "LiteralInteger"
        }
      ]
    },
    "OpTypeImage": {
      "opcode": 25,
      "operands": [
        {
          "kind": "IdResult"
        },
        {
          "kind": "IdRef"
        },
        {
          "kind": "Dim"
        },
        {
          "kind": "LiteralInteger"
        },
        {
          "kind": "LiteralInteger"
        },
        {
          "kind": "LiteralInteger"
        },
        {
          "kind": "LiteralInteger"
        },
        {
          "kind": "ImageFormat"
        },
        {
          "kind": "AccessQualifier",
          "quantifier": "?"
        }
      ]
    },
    "OpTypeSampler": {
      "opcode": 26,
      "operands": [
        {
          "kind": "IdResult"
        }
      ]
    },
    "OpTypeSampledImage": {
      "opcode": 27,
      "operands": [
        {
          "kind": "IdResult"
        },
        {
          "kind": "IdRef"
        }
      ]
    },
    "OpTypeArray": {
      "opcode": 28,
      "operands": [
        {
          "kind": "IdResult"
        },
        {
          "kind": "IdRef"
        },
        {
          "kind": "IdRef"
        }
      ]
    },
    "OpTypeRuntimeArray": {
      "opcode": 29,
      "operands": [
        {
          "kind": "IdResult"
        },
        {
          "kind": "IdRef"
        }
      ]
    },
    "OpTypeStruct": {
      "opcode": 30,
      "operands": [
        {
          "kind": "IdResult"
        },
        {
          "kind": "IdRef",
          "quantifier": "*"
        }
      ]
    },
    "OpTypePointer": {
      "opcode": 32,
      "operands": [
        {
          "kind": "IdResult"
        },
        {
          "kind": "StorageClass"
        },
        {
          "kind": "IdRef"
        }
      ]
    },
    "OpTypeFunction": {
      "opcode": 33,
      "operands": [
        {
          "kind": "IdResult"
        },
        {
          "kind": "IdRef"
        },
        {
          "kind": "IdRef",
          "quantifier": "*"
        }
      ]
    },
    "OpConstantTrue": {
      "opcode": 41,
      "operands": [
        {
          "kind": "IdResultType"
        },
        {
          "kind": "IdResult"
        }
      ]
    },
    "OpConstantFalse": {
      "opcode": 42,
      "operands": [
        {
          "kind": "IdResultType"
        },
        {
          "kind": "IdResult"
        }
      ]
    },
    "OpConstant": {
      "opcode": 43,
      "operands": [
        {
          "kind": "IdResultType"
        },
        {
          "kind": "IdResult"
        },
        {
          "kind": "LiteralContextDependentNumber"
        }
      ]
    },
    "OpConstantComposite": {
      "opcode": 44,
      "operands": [
        {
          "kind": "IdResultType"
        },
        {
          "kind": "IdResult"
        },
        {
          "kind": "IdRef",
          "quantifier": "*"
        }
      ]
    },
    "OpConstantNull": {
      "opcode": 46,
      "operands": [
        {
          "kind": "IdResultType"
        },
        {
          "kind": "IdResult"
        }
      ]
    },
    "OpSpecConstantTrue": {
      "opcode": 48,
      "operands": [
        {
          "kind": "IdResultType"
        },
        {
          "kind": "IdResult"
        }
      ]
    },
    "OpSpecConstantFalse": {
      "opcode": 49,
      "operands": [
        {
          "kind": "IdResultType"
        },
        {
          "kind": "IdResult"
        }
      ]
    },
    "OpSpecConstant": {
      "opcode": 50,
      "operands": [
        {
          "kind": "IdResultType"
        },
        {
          "kind": "IdResult"
        },
        {
          "kind": "LiteralContextDependentNumber"
        }
      ]
    },
    "OpSpecConstantComposite": {
      "opcode": 51,
      "operands": [
        {
          "kind": "IdResultType"
        },
        {
          "kind": "IdResult"
        },
        {
          "kind": "IdRef",
          "quantifier": "*"
        }
      ]
    },
    "OpSpecConstantOp": {
      "opcode": 52,
      "operands": [
        {
          "kind": "IdResultType"
        },
        {
          "kind": "IdResult"
        },
        {
          "kind": "LiteralSpecConstantOpInteger"
        }
      ]
    },
    "OpFunction": {
      "opcode": 54,
      "operands": [
        {
          "kind": "IdResultType"
        },
        {
          "kind": "IdResult"
        },
        {
          "kind": "FunctionControl"
        },
        {
          "kind": "IdRef"
        }
      ]
    },
    "OpFunctionParameter": {
      "opcode": 55,
      "operands": [
        {
          "kind": "IdResultType"
        },
        {
          "kind": "IdResult"
        }
      ]
    },
    "OpFunctionEnd": {
      "opcode": 56,
      "operands": [

      ]
    },
    "OpFunctionCall": {
      "opcode": 57,
      "operands": [
        {
          "kind": "IdResultType"
        },
        {
          "kind": "IdResult"
        },
        {
          "kind": "IdRef"
        },
        {
          "kind": "IdRef",
          "quantifier": "*"
        }
      ]
    },
    "OpVariable": {
      "opcode": 59,
      "operands": [
        {
          "kind": "IdResultType"
        },
        {
          "kind": "IdResult"
        },
        {
          "kind": "StorageClass"
        },
        {
          "kind": "IdRef",
          "quantifier": "?"
        }
      ]
    },
    "OpImageTexelPointer": {
      "opcode": 60,
      "operands": [
        {
          "kind": "IdResultType"
        },
        {
          "kind": "IdResult"
        },
        {
          "kind": "IdRef"
        },
        {
          "kind": "IdRef"
        },
        {
          "kind": "IdRef"
        }
      ]
    },
    "OpLoad": {
      "opcode": 61,
      "operands": [
        {
          "kind": "IdResultType"
        },
        {
          "kind": "IdResult"
        },
        {
          "kind": "IdRef"
        },
        {
          "kind": "MemoryAccess",
          "quantifier": "?"
        }
      ]
    },
    "OpStore": {
      "opcode": 62,
      "operands": [
        {
          "kind": "IdRef"
        },
        {
          "kind": "IdRef"
        },
        {
          "kind": "MemoryAccess",
          "quantifier": "?"
        }
      ]
    },
    "OpCopyMemory": {
      "opcode": 63,
      "operands": [
        {
          "kind": "IdRef"
        },
        {
          "kind": "IdRef"
        },
        {
          "kind": "MemoryAccess",
          "quantifier": "?"
        },
        {
          "kind": "MemoryAccess",
          "quantifier": "?"
        }
      ]
    },
    "OpAccessChain": {
      "opcode": 65,
      "operands": [
        {
          "kind": "IdResultType"
        },
        {
          "kind": "IdResult"
        },
        {
          "kind": "IdRef"
        },
        {
          "kind": "IdRef",
          "quantifier": "*"
        }
      ]
    },
    "OpInBoundsAccessChain": {
      "opcode": 66,
      "operands": [
        {
          "kind": "IdResultType"
        },
        {
          "kind": "IdResult"
        },
        {
          "kind": "IdRef"
        },
        {
          "kind": "IdRef",
          "quantifier": "*"
        }
      ]
    },
    "OpArrayLength": {
      "opcode": 68,
      "operands": [
        {
          "kind": "IdResultType"
        },
        {
          "kind": "IdResult"
        },
        {
          "kind": "IdRef"
        },
        {
          "kind": "LiteralInteger"
        }
      ]
    },
    "OpDecorate": {
      "opcode": 71,
      "operands": [
        {
          "kind": "IdRef"
        },
        {
          "kind": "Decoration"
        }
      ]
    },
    "OpMemberDecorate": {
      "opcode": 72,
      "operands": [
        {
          "kind": "IdRef"
        },
        {
          "kind": "LiteralInteger"
        },
        {
          "kind": "Decoration"
        }
      ]
    },
    "OpDecorationGroup": {
      "opcode": 73,
      "operands": [
        {
          "kind": "IdResult"
        }
      ]
    },
    "OpGroupDecorate": {
      "opcode": 74,
      "operands": [
        {
          "kind": "IdRef"
        },
        {
          "kind": "IdRef",
          "quantifier": "*"
        }
      ]
    },
    "OpGroupMemberDecorate": {
      "opcode": 75,
      "operands": [
        {
          "kind": "IdRef"
        },
        {
          "kind": "PairIdRefLiteralInteger",
          "quantifier": "*"
        }
      ]
    },
    "OpVectorExtractDynamic": {
      "opcode": 77,
      "operands": [
        {
          "kind": "IdResultType"
        },
        {
          "kind": "IdResult"
        },
        {
          "kind": "IdRef"
        },
        {
          "kind": "IdRef"
        }
      ]
    },
    "OpVectorInsertDynamic": {
      "opcode": 78,
      "operands": [
        {
          "kind": "IdResultType"
        },
        {
          "kind": "IdResult"
        },
        {
          "kind": "IdRef"
        },
        {
          "kind": "IdRef"
        },
        {
          "kind": "IdRef"
        }
      ]
    },
    "OpVectorShuffle": {
      "opcode": 79,
      "operands": [
        {
          "kind": "IdResultType"
        },
        {
          "kind": "IdResult"
        },
        {
          "kind": "IdRef"
        },
        {
          "kind": "IdRef"
        },
        {
          "kind": "LiteralInteger",
          "quantifier": "*"
        }
      ]
    },
    "OpCompositeConstruct": {
      "opcode": 80,
      "operands": [
        {
          "kind": "IdResultType"
        },
        {
          "kind": "IdResult"
        },
        {
          "kind": "IdRef",
          "quantifier": "*"
        }
      ]
    },
    "OpCompositeExtract": {
      "opcode": 81,
      "operands": [
        {
          "kind": "IdResultType"
        },
        {
          "kind": "IdResult"
        },
        {
          "kind": "IdRef"
        },
        {
          "kind": "LiteralInteger",
          "quantifier": "*"
        }
      ]
    },
    "OpCompositeInsert": {
      "opcode": 82,
      "operands": [
        {
          "kind": "IdResultType"
        },
        {
          "kind": "IdResult"
        },
        {
          "kind": "IdRef"
        },
        {
          "kind": "IdRef"
        },
        {
          "kind": "LiteralInteger",
          "quantifier": "*"
        }
      ]
    },
    "OpCopyObject": {
      "opcode": 83,
      "operands": [
        {
          "kind": "IdResultType"
        },
        {
          "kind": "IdResult"
        },
        {
          "kind": "IdRef"
        }
      ]
    },
    "OpTranspose": {
      "opcode": 84,
      "operands": [
        {
          "kind": "IdResultType"
        },
        {
          "kind": "IdResult"
        },
        {
          "kind": "IdRef"
        }
      ]
    },
    "OpSampledImage": {
      "opcode": 86,
      "operands": [
        {
          "kind": "IdResultType"
        },
        {
          "kind": "IdResult"
        },
        {
          "kind": "IdRef"
        },
        {
          "kind": "IdRef"
        }
      ]
    },
    "OpImageSampleImplicitLod": {
      "opcode": 87,
      "operands": [
        {
          "kind": "IdResultType"
        },
        {
          "kind": "IdResult"
        },
        {
          "kind": "IdRef"
        },
        {
          "kind": "IdRef"
        },
        {
          "kind": "ImageOperands",
          "quantifier": "?"
        }
      ]
    },
    "OpImageSampleExplicitLod": {
      "opcode": 88,
      "operands": [
        {
          "kind": "IdResultType"
        },
        {
          "kind": "IdResult"
        },
        {
          "kind": "IdRef"
        },
        {
          "kind": "IdRef"
        },
        {
          "kind": "ImageOperands"
        }
      ]
    },
    "OpImageSampleDrefImplicitLod": {
      "opcode": 89,
      "operands": [
        {
          "kind": "IdResultType"
        },
        {
          "kind": "IdResult"
        },
        {
          "kind": "IdRef"
        },
        {
          "kind": "IdRef"
        },
        {
          "kind": "IdRef"
        },
        {
          "kind": "ImageOperands",
          "quantifier": "?"
        }
      ]
    },
    "OpImageSampleDrefExplicitLod": {
      "opcode": 90,
      "operands": [
        {
          "kind": "IdResultType"
        },
        {
          "kind": "IdResult"
        },
        {
          "kind": "IdRef"
        },
        {
          "kind": "IdRef"
        },
        {
          "kind": "IdRef"
        },
        {
          "kind": "ImageOperands"
        }
      ]
    },
    "OpImageSampleProjImplicitLod": {
      "opcode": 91,
      "operands": [
        {
          "kind": "IdResultType"
        },
        {
          "kind": "IdResult"
        },
        {
          "kind": "IdRef"
        },
        {
          "kind": "IdRef"
        },
        {
          "kind": "ImageOperands",
          "quantifier": "?"
        }
      ]
    },
    "OpImageSampleProjExplicitLod": {
      "opcode": 92,
      "operands": [
        {
          "kind": "IdResultType"
        },
        {
          "kind": "IdResult"
        },
        {
          "kind": "IdRef"
        },
        {
          "kind": "IdRef"
        },
        {
          "kind": "ImageOperands"
        }
      ]
    },
    "OpImageSampleProjDrefImplicitLod": {
      "opcode": 93,
      "operands": [
        {
          "kind": "IdResultType"
        },
        {
          "kind": "IdResult"
        },
        {
          "kind": "IdRef"
        },
        {
          "kind": "IdRef"
        },
        {
          "kind": "IdRef"
        },
        {
          "kind": "ImageOperands",
          "quantifier": "?"
        }
      ]
    },
    "OpImageSampleProjDrefExplicitLod": {
      "opcode": 94,
      "operands": [
        {
          "kind": "IdResultType"
        },
        {
          "kind": "IdResult"
        },
        {
          "kind": "IdRef"
        },
        {
          "kind": "IdRef"
        },
        {
          "kind": "IdRef"
        },
        {
          "kind": "ImageOperands"
        }
      ]
    },
    "OpImageFetch": {
      "opcode": 95,
      "operands": [
        {
          "kind": "IdResultType"
        },
        {
          "kind": "IdResult"
        },
        {
          "kind": "IdRef"
        },
        {
          "kind": "IdRef"
        },
        {
          "kind": "ImageOperands",
          "quantifier": "?"
        }
      ]
    },
    "OpImageGather": {
      "opcode": 96,
      "operands": [
        {
          "kind": "IdResultType"
        },
        {
          "kind": "IdResult"
        },
        {
          "kind": "IdRef"
        },
        {
          "kind": "IdRef"
        },
        {
          "kind": "IdRef"
        },
        {
          "kind": "ImageOperands",
          "quantifier": "?"
        }
      ]
    },
    "OpImageDrefGather": {
      "opcode": 97,
      "operands": [
        {
          "kind": "IdResultType"
        },
        {
          "kind": "IdResult"
        },
        {
          "kind": "IdRef"
        },
        {
          "kind": "IdRef"
        },
        {
          "kind": "IdRef"
        },
        {
          "kind": "ImageOperands",
          "quantifier": "?"
        }
      ]
    },
    "OpImageRead": {
      "opcode": 98,
      "operands": [
        {
          "kind": "IdResultType"
        },
        {
          "kind": "IdResult"
        },
        {
          "kind": "IdRef"
        },
        {
          "kind": "IdRef"
        },
        {
          "kind": "ImageOperands",
          "quantifier": "?"
        }
      ]
    },
    "OpImageWrite": {
      "opcode": 99,
      "operands": [
        {
          "kind": "IdRef"
        },
        {
          "kind": "IdRef"
        },
        {
          "kind": "IdRef"
        },
        {
          "kind": "ImageOperands",
          "quantifier": "?"
        }
      ]
    },
    "OpImage": {
      "opcode": 100,
      "operands": [
        {
          "kind": "IdResultType"
        },
        {
          "kind": "IdResult"
        },
        {
          "kind": "IdRef"
        }
      ]
    },
    "OpImageQuerySizeLod": {
      "opcode": 103,
      "operands": [
        {
          "kind": "IdResultType"
        },
        {
          "kind": "IdResult"
        },
        {
          "kind": "IdRef"
        },
        {
          "kind": "IdRef"
        }
      ]
    },
    "OpImageQuerySize": {
      "opcode": 104,
      "operands": [
        {
          "kind": "IdResultType"
        },
        {
          "kind": "IdResult"
        },
        {
          "kind": "IdRef"
        }
      ]
    },
    "OpImageQueryLod": {
      "opcode": 105,
      "operands": [
        {
          "kind": "IdResultType"
        },
        {
          "kind": "IdResult"
        },
        {
          "kind": "IdRef"
        },
        {
          "kind": "IdRef"
        }
      ]
    },
    "OpImageQueryLevels": {
      "opcode": 106,
      "operands": [
        {
          "kind": "IdResultType"
        },
        {
          "kind": "IdResult"
        },
        {
          "kind": "IdRef"
        }
      ]
    },
    "OpImageQuerySamples": {
      "opcode": 107,
      "operands": [
        {
          "kind": "IdResultType"
        },
        {
          "kind": "IdResult"
        },
        {
          "kind": "IdRef"
        }
      ]
    },
    "OpConvertFToU": {
      "opcode": 109,
      "operands": [
        {
          "kind": "IdResultType"
        },
        {
          "kind": "IdResult"
        },
        {
          "kind": "IdRef"
        }
      ]
    },
    "OpConvertFToS": {
      "opcode": 110,
      "operands": [
        {
          "kind": "IdResultType"
        },
        {
          "kind": "IdResult"
        },
        {
          "kind": "IdRef"
        }
      ]
    },
    "OpConvertSToF": {
      "opcode": 111,
      "operands": [
        {
          "kind": "IdResultType"
        },
        {
          "kind": "IdResult"
        },
        {
          "kind": "IdRef"
        }
      ]
    },
    "OpConvertUToF": {
      "opcode": 112,
      "operands": [
        {
          "kind": "IdResultType"
        },
        {
          "kind": "IdResult"
        },
        {
          "kind": "IdRef"
        }
      ]
    },
    "OpUConvert": {
      "opcode": 113,
      "operands": [
        {
          "kind": "IdResultType"
        },
        {
          "kind": "IdResult"
        },
        {
          "kind": "IdRef"
        }
      ]
    },
    "OpSConvert": {
      "opcode": 114,
      "operands": [
        {
          "kind": "IdResultType"
        },
        {
          "kind": "IdResult"
        },
        {
          "kind": "IdRef"
        }
      ]
    },
    "OpFConvert": {
      "opcode": 115,
      "operands": [
        {
          "kind": "IdResultType"
        },
        {
          "kind": "IdResult"
        },
        {
          "kind": "IdRef"
        }
      ]
    },
    "OpQuantizeToF16": {
      "opcode": 116,
      "operands": [
        {
          "kind": "IdResultType"
        },
        {
          "kind": "IdResult"
        },
        {
          "kind": "IdRef"
        }
      ]
    },
    "OpBitcast": {
      "opcode": 124,
      "operands": [
        {
          "kind": "IdResultType"
        },
        {
          "kind": "IdResult"
        },
        {
          "kind": "IdRef"
        }
      ]
    },
    "OpSNegate": {
      "opcode": 126,
      "operands": [
        {
          "kind": "IdResultType"
        },
        {
          "kind": "IdResult"
        },
        {
          "kind": "IdRef"
        }
      ]
    },
    "OpFNegate": {
      "opcode": 127,
      "operands": [
        {
          "kind": "IdResultType"
        },
        {
          "kind": "IdResult"
        },
        {
          "kind": "IdRef"
        }
      ]
    },
    "OpIAdd": {
      "opcode": 128,
      "operands": [
        {
          "kind": "IdResultType"
        },
        {
          "kind": "IdResult"
        },
        {
          "kind": "IdRef"
        },
        {
          "kind": "IdRef"
        }
      ]
    },
    "OpFAdd": {
      "opcode": 129,
      "operands": [
        {
          "kind": "IdResultType"
        },
        {
          "kind": "IdResult"
        },
        {
          "kind": "IdRef"
        },
        {
          "kind": "IdRef"
        }
      ]
    },
    "OpISub": {
      "opcode": 130,
      "operands": [
        {
          "kind": "IdResultType"
        },
        {
          "kind": "IdResult"
        },
        {
          "kind": "IdRef"
        },
        {
          "kind": "IdRef"
        }
      ]
    },
    "OpFSub": {
      "opcode": 131,
      "operands": [
        {
          "kind": "IdResultType"
        },
        {
          "kind": "IdResult"
        },
        {
          "kind": "IdRef"
        },
        {
          "kind": "IdRef"
        }
      ]
    },
    "OpIMul": {
      "opcode": 132,
      "operands": [
        {
          "kind": "IdResultType"
        },
        {
          "kind": "IdResult"
        },
        {
          "kind": "IdRef"
        },
        {
          "kind": "IdRef"
        }
      ]
    },
    "OpFMul": {
      "opcode": 133,
      "operands": [
        {
          "kind": "IdResultType"
        },
        {
          "kind": "IdResult"
        },
        {
          "kind": "IdRef"
        },
        {
          "kind": "IdRef"
        }
      ]
    },
    "OpUDiv": {
      "opcode": 134,
      "operands": [
        {
          "kind": "IdResultType"
        },
        {
          "kind": "IdResult"
        },
        {
          "kind": "IdRef"
        },
        {
          "kind": "IdRef"
        }
      ]
    },
    "OpSDiv": {
      "opcode": 135,
      "operands": [
        {
          "kind": "IdResultType"
        },
        {
          "kind": "IdResult"
        },
        {
          "kind": "IdRef"
        },
        {
          "kind": "IdRef"
        }
      ]
    },
    "OpFDiv": {
      "opcode": 136,
      "operands": [
        {
          "kind": "IdResultType"
        },
        {
          "kind": "IdResult"
        },
        {
          "kind": "IdRef"
        },
        {
          "kind": "IdRef"
        }
      ]
    },
    "OpUMod": {
      "opcode": 137,
      "operands": [
        {
          "kind": "IdResultType"
        },
        {
          "kind": "IdResult"
        },
        {
          "kind": "IdRef"
        },
        {
          "kind": "IdRef"
        }
      ]
    },
    "OpSRem": {
      "opcode": 138,
      "operands": [
        {
          "kind": "IdResultType"
        },
        {
          "kind": "IdResult"
        },
        {
          "kind": "IdRef"
        },
        {
          "kind": "IdRef"
        }
      ]
    },
    "OpSMod": {
      "opcode": 139,
      "operands": [
        {
          "kind": "IdResultType"
        },
        {
          "kind": "IdResult"
        },
        {
          "kind": "IdRef"
        },
        {
          "kind": "IdRef"
        }
      ]
    },
    "OpFRem": {
      "opcode": 140,
      "operands": [
        {
          "kind": "IdResultType"
        },
        {
          "kind": "IdResult"
        },
        {
          "kind": "IdRef"
        },
        {
          "kind": "IdRef"
        }
      ]
    },
    "OpFMod": {
      "opcode": 141,
      "operands": [
        {
          "kind": "IdResultType"
        },
        {
          "kind": "IdResult"
        },
        {
          "kind": "IdRef"
        },
        {
          "kind": "IdRef"
        }
      ]
    },
    "OpVectorTimesScalar": {
      "opcode": 142,
      "operands": [
        {
          "kind": "IdResultType"
        },
        {
          "kind": "IdResult"
        },
        {
          "kind": "IdRef"
        },
        {
          "kind": "IdRef"
        }
      ]
    },
    "OpMatrixTimesScalar": {
      "opcode": 143,
      "operands": [
        {
          "kind": "IdResultType"
        },
        {
          "kind": "IdResult"
        },
        {
          "kind": "IdRef"
        },
        {
          "kind": "IdRef"
        }
      ]
    },
    "OpVectorTimesMatrix": {
      "opcode": 144,
      "operands": [
        {
          "kind": "IdResultType"
        },
        {
          "kind": "IdResult"
        },
        {
          "kind": "IdRef"
        },
        {
          "kind": "IdRef"
        }
      ]
    },
    "OpMatrixTimesVector": {
      "opcode": 145,
      "operands": [
        {
          "kind": "IdResultType"
        },
        {
          "kind": "IdResult"
        },
        {
          "kind": "IdRef"
        },
        {
          "kind": "IdRef"
        }
      ]
    },
    "OpMatrixTimesMatrix": {
      "opcode": 146,
      "operands": [
        {
          "kind": "IdResultType"
        },
        {
          "kind": "IdResult"
        },
        {
          "kind": "IdRef"
        },
        {
          "kind": "IdRef"
        }
      ]
    },
    "OpOuterProduct": {
      "opcode": 147,
      "operands": [
        {
          "kind": "IdResultType"
        },
        {
          "kind": "IdResult"
        },
        {
          "kind": "IdRef"
        },
        {
          "kind": "IdRef"
        }
      ]
    },
    "OpDot": {
      "opcode": 148,
      "operands": [
        {
          "kind": "IdResultType"
        },
        {
          "kind": "IdResult"
        },
        {
          "kind": "IdRef"
        },
        {
          "kind": "IdRef"
        }
      ]
    },
    "OpIAddCarry": {
      "opcode": 149,
      "operands": [
        {
          "kind": "IdResultType"
        },
        {
          "kind": "IdResult"
        },
        {
          "kind": "IdRef"
        },
        {
          "kind": "IdRef"
        }
      ]
    },
    "OpISubBorrow": {
      "opcode": 150,
      "operands": [
        {
          "kind": "IdResultType"
        },
        {
          "kind": "IdResult"
        },
        {
          "kind": "IdRef"
        },
        {
          "kind": "IdRef"
        }
      ]
    },
    "OpUMulExtended": {
      "opcode": 151,
      "operands": [
        {
          "kind": "IdResultType"
        },
        {
          "kind": "IdResult"
        },
        {
          "kind": "IdRef"
        },
        {
          "kind": "IdRef"
        }
      ]
    },
    "OpSMulExtended": {
      "opcode": 152,
      "operands": [
        {
          "kind": "IdResultType"
        },
        {
          "kind": "IdResult"
        },
        {
          "kind": "IdRef"
        },
        {
          "kind": "IdRef"
        }
      ]
    },
    "OpAny": {
      "opcode": 154,
      "operands": [
        {
          "kind": "IdResultType"
        },
        {
          "kind": "IdResult"
        },
        {
          "kind": "IdRef"
        }
      ]
    },
    "OpAll": {
      "opcode": 155,
      "operands": [
        {
          "kind": "IdResultType"
        },
        {
          "kind": "IdResult"
        },
        {
          "kind": "IdRef"
        }
      ]
    },
    "OpIsNan": {
      "opcode": 156,
      "operands": [
        {
          "kind": "IdResultType"
        },
        {
          "kind": "IdResult"
        },
        {
          "kind": "IdRef"
        }
      ]
    },
    "OpIsInf": {
      "opcode": 157,
      "operands": [
        {
          "kind": "IdResultType"
        },
        {
          "kind": "IdResult"
        },
        {
          "kind": "IdRef"
        }
      ]
    },
    "OpLogicalEqual": {
      "opcode": 164,
      "operands": [
        {
          "kind": "IdResultType"
        },
        {
          "kind": "IdResult"
        },
        {
          "kind": "IdRef"
        },
        {
          "kind": "IdRef"
        }
      ]
    },
    "OpLogicalNotEqual": {
      "opcode": 165,
      "operands": [
        {
          "kind": "IdResultType"
        },
        {
          "kind": "IdResult"
        },
        {
          "kind": "IdRef"
        },
        {
          "kind": "IdRef"
        }
      ]
    },
    "OpLogicalOr": {
      "opcode": 166,
      "operands": [
        {
          "kind": "IdResultType"
        },
        {
          "kind": "IdResult"
        },
        {
          "kind": "IdRef"
        },
        {
          "kind": "IdRef"
        }
      ]
    },
    "OpLogicalAnd": {
      "opcode": 167,
      "operands": [
        {
          "kind": "IdResultType"
        },
        {
          "kind": "IdResult"
        },
        {
          "kind": "IdRef"
        },
        {
          "kind": "IdRef"
        }
      ]
    },
    "OpLogicalNot": {
      "opcode": 168,
      "operands": [
        {
          "kind": "IdResultType"
        },
        {
          "kind": "IdResult"
        },
        {
          "kind": "IdRef"
        }
      ]
    },
    "OpSelect": {
      "opcode": 169,
      "operands": [
        {
          "kind": "IdResultType"
        },
        {
          "kind": "IdResult"
        },
        {
          "kind": "IdRef"
        },
        {
          "kind": "IdRef"
        },
        {
          "kind": "IdRef"
        }
      ]
    },
    "OpIEqual": {
      "opcode": 170,
      "operands": [
        {
          "kind": "IdResultType"
        },
        {
          "kind": "IdResult"
        },
        {
          "kind": "IdRef"
        },
        {
          "kind": "IdRef"
        }
      ]
    },
    "OpINotEqual": {
      "opcode": 171,
      "operands": [
        {
          "kind": "IdResultType"
        },
        {
          "kind": "IdResult"
        },
        {
          "kind": "IdRef"
        },
        {
          "kind": "IdRef"
        }
      ]
    },
    "OpUGreaterThan": {
      "opcode": 172,
      "operands": [
        {
          "kind": "IdResultType"
        },
        {
          "kind": "IdResult"
        },
        {
          "kind": "IdRef"
        },
        {
          "kind": "IdRef"
        }
      ]
    },
    "OpSGreaterThan": {
      "opcode": 173,
      "operands": [
        {
          "kind": "IdResultType"
        },
        {
          "kind": "IdResult"
        },
        {
          "kind": "IdRef"
        },
        {
          "kind": "IdRef"
        }
      ]
    },
    "OpUGreaterThanEqual": {
      "opcode": 174,
      "operands": [
        {
          "kind": "IdResultType"
        },
        {
          "kind": "IdResult"
        },
        {
          "kind": "IdRef"
        },
        {
          "kind": "IdRef"
        }
      ]
    },
    "OpSGreaterThanEqual": {
      "opcode": 175,
      "operands": [
        {
          "kind": "IdResultType"
        },
        {
          "kind": "IdResult"
        },
        {
          "kind": "IdRef"
        },
        {
          "kind": "IdRef"
        }
      ]
    },
    "OpULessThan": {
      "opcode": 176,
      "operands": [
        {
          "kind": "IdResultType"
        },
        {
          "kind": "IdResult"
        },
        {
          "kind": "IdRef"
        },
        {
          "kind": "IdRef"
        }
      ]
    },
    "OpSLessThan": {
      "opcode": 177,
      "operands": [
        {
          "kind": "IdResultType"
        },
        {
          "kind": "IdResult"
        },
        {
          "kind": "IdRef"
        },
        {
          "kind": "IdRef"
        }
      ]
    },
    "OpULessThanEqual": {
      "opcode": 178,
      "operands": [
        {
          "kind": "IdResultType"
        },
        {
          "kind": "IdResult"
        },
        {
          "kind": "IdRef"
        },
        {
          "kind": "IdRef"
        }
      ]
    },
    "OpSLessThanEqual": {
      "opcode": 179,
      "operands": [
        {
          "kind": "IdResultType"
        },
        {
          "kind": "IdResult"
        },
        {
          "kind": "IdRef"
        },
        {
          "kind": "IdRef"
        }
      ]
    },
    "OpFOrdEqual": {
      "opcode": 180,
      "operands": [
        {
          "kind": "IdResultType"
        },
        {
          "kind": "IdResult"
        },
        {
          "kind": "IdRef"
        },
        {
          "kind": "IdRef"
        }
      ]
    },
    "OpFUnordEqual": {
      "opcode": 181,
      "operands": [
        {
          "kind": "IdResultType"
        },
        {
          "kind": "IdResult"
        },
        {
          "kind": "IdRef"
        },
        {
          "kind": "IdRef"
        }
      ]
    },
    "OpFOrdNotEqual": {
      "opcode": 182,
      "operands": [
        {
          "kind": "IdResultType"
        },
        {
          "kind": "IdResult"
        },
        {
          "kind": "IdRef"
        },
        {
          "kind": "IdRef"
        }
      ]
    },
    "OpFUnordNotEqual": {
      "opcode": 183,
      "operands": [
        {
          "kind": "IdResultType"
        },
        {
          "kind": "IdResult"
        },
        {
          "kind": "IdRef"
        },
        {
          "kind": "IdRef"
        }
      ]
    },
    "OpFOrdLessThan": {
      "opcode": 184,
      "operands": [
        {
          "kind": "IdResultType"
        },
        {
          "kind": "IdResult"
        },
        {
          "kind": "IdRef"
        },
        {
          "kind": "IdRef"
        }
      ]
    },
    "OpFUnordLessThan": {
      "opcode": 185,
      "operands": [
        {
          "kind": "IdResultType"
        },
        {
          "kind": "IdResult"
        },
        {
          "kind": "IdRef"
        },
        {
          "kind": "IdRef"
        }
      ]
    },
    "OpFOrdGreaterThan": {
      "opcode": 186,
      "operands": [
        {
          "kind": "IdResultType"
        },
        {
          "kind": "IdResult"
        },
        {
          "kind": "IdRef"
        },
        {
          "kind": "IdRef"
        }
      ]
    },
    "OpFUnordGreaterThan": {
      "opcode": 187,
      "operands": [
        {
          "kind": "IdResultType"
        },
        {
          "kind": "IdResult"
        },
        {
          "kind": "IdRef"
        },
        {
          "kind": "IdRef"
        }
      ]
    },
    "OpFOrdLessThanEqual": {
      "opcode": 188,
      "operands": [
        {
          "kind": "IdResultType"
        },
        {
          "kind": "IdResult"
        },
        {
          "kind": "IdRef"
        },
        {
          "kind": "IdRef"
        }
      ]
    },
    "OpFUnordLessThanEqual": {
      "opcode": 189,
      "operands": [
        {
          "kind": "IdResultType"
        },
        {
          "kind": "IdResult"
        },
        {
          "kind": "IdRef"
        },
        {
          "kind": "IdRef"
        }
      ]
    },
    "OpFOrdGreaterThanEqual": {
      "opcode": 190,
      "operands": [
        {
          "kind": "IdResultType"
        },
        {
          "kind": "IdResult"
        },
        {
          "kind": "IdRef"
        },
        {
          "kind": "IdRef"
        }
      ]
    },
    "OpFUnordGreaterThanEqual": {
      "opcode": 191,
      "operands": [
        {
          "kind": "IdResultType"
        },
        {
          "kind": "IdResult"
        },
        {
          "kind": "IdRef"
        },
        {
          "kind": "IdRef"
        }
      ]
    },
    "OpShiftRightLogical": {
      "opcode": 194,
      "operands": [
        {
          "kind": "IdResultType"
        },
        {
          "kind": "IdResult"
        },
        {
          "kind": "IdRef"
        },
        {
          "kind": "IdRef"
        }
      ]
    },
    "OpShiftRightArithmetic": {
      "opcode": 195,
      "operands": [
        {
          "kind": "IdResultType"
        },
        {
          "kind": "IdResult"
        },
        {
          "kind": "IdRef"
        },
        {
          "kind": "IdRef"
        }
      ]
    },
    "OpShiftLeftLogical": {
      "opcode": 196,
      "operands": [
        {
          "kind": "IdResultType"
        },
        {
          "kind": "IdResult"
        },
        {
          "kind": "IdRef"
        },
        {
          "kind": "IdRef"
        }
      ]
    },
    "OpBitwiseOr": {
      "opcode": 197,
      "operands": [
        {
          "kind": "IdResultType"
        },
        {
          "kind": "IdResult"
        },
        {
          "kind": "IdRef"
        },
        {
          "kind": "IdRef"
        }
      ]
    },
    "OpBitwiseXor": {
      "opcode": 198,
      "operands": [
        {
          "kind": "IdResultType"
        },
        {
          "kind": "IdResult"
        },
        {
          "kind": "IdRef"
        },
        {
          "kind": "IdRef"
        }
      ]
    },
    "OpBitwiseAnd": {
      "opcode": 199,
      "operands": [
        {
          "kind": "IdResultType"
        },
        {
          "kind": "IdResult"
        },
        {
          "kind": "IdRef"
        },
        {
          "kind": "IdRef"
        }
      ]
    },
    "OpNot": {
      "opcode": 200,
      "operands": [
        {
          "kind": "IdResultType"
        },
        {
          "kind": "IdResult"
        },
        {
          "kind": "IdRef"
        }
      ]
    },
    "OpBitFieldInsert": {
      "opcode": 201,
      "operands": [
        {
          "kind": "IdResultType"
        },
        {
          "kind": "IdResult"
        },
        {
          "kind": "IdRef"
        },
        {
          "kind": "IdRef"
        },
        {
          "kind": "IdRef"
        },
        {
          "kind": "IdRef"
        }
      ]
    },
    "OpBitFieldSExtract": {
      "opcode": 202,
      "operands": [
        {
          "kind": "IdResultType"
        },
        {
          "kind": "IdResult"
        },
        {
          "kind": "IdRef"
        },
        {
          "kind": "IdRef"
        },
        {
          "kind": "IdRef"
        }
      ]
    },
    "OpBitFieldUExtract": {
      "opcode": 203,
      "operands": [
        {
          "kind": "IdResultType"
        },
        {
          "kind": "IdResult"
        },
        {
          "kind": "IdRef"
        },
        {
          "kind": "IdRef"
        },
        {
          "kind": "IdRef"
        }
      ]
    },
    "OpBitReverse": {
      "opcode": 204,
      "operands": [
        {
          "kind": "IdResultType"
        },
        {
          "kind": "IdResult"
        },
        {
          "kind": "IdRef"
        }
      ]
    },
    "OpBitCount": {
      "opcode": 205,
      "operands": [
        {
          "kind": "IdResultType"
        },
        {
          "kind": "IdResult"
        },
        {
          "kind": "IdRef"
        }
      ]
    },
    "OpDPdx": {
      "opcode": 207,
      "operands": [
        {
          "kind": "IdResultType"
        },
        {
          "kind": "IdResult"
        },
        {
          "kind": "IdRef"
        }
      ]
    },
    "OpDPdy": {
      "opcode": 208,
      "operands": [
        {
          "kind": "IdResultType"
        },
        {
          "kind": "IdResult"
        },
        {
          "kind": "IdRef"
        }
      ]
    },
    "OpFwidth": {
      "opcode": 209,
      "operands": [
        {
          "kind": "IdResultType"
        },
        {
          "kind": "IdResult"
        },
        {
          "kind": "IdRef"
        }
      ]
    },
    "OpDPdxFine": {
      "opcode": 210,
      "operands": [
        {
          "kind": "IdResultType"
        },
        {
          "kind": "IdResult"
        },
        {
          "kind": "IdRef"
        }
      ]
    },
    "OpDPdyFine": {
      "opcode": 211,
      "operands": [
        {
          "kind": "IdResultType"
        },
        {
          "kind": "IdResult"
        },
        {
          "kind": "IdRef"
        }
      ]
    },
    "OpFwidthFine": {
      "opcode": 212,
      "operands": [
        {
          "kind": "IdResultType"
        },
        {
          "kind": "IdResult"
        },
        {
          "kind": "IdRef"
        }
      ]
    },
    "OpDPdxCoarse": {
      "opcode": 213,
      "operands": [
        {
          "kind": "IdResultType"
        },
        {
          "kind": "IdResult"
        },
        {
          "kind": "IdRef"
        }
      ]
    },
    "OpDPdyCoarse": {
      "opcode": 214,
      "operands": [
        {
          "kind": "IdResultType"
        },
        {
          "kind": "IdResult"
        },
        {
          "kind": "IdRef"
        }
      ]
    },
    "OpFwidthCoarse": {
      "opcode": 215,
      "operands": [
        {
          "kind": "IdResultType"
        },
        {
          "kind": "IdResult"
        },
        {
          "kind": "IdRef"
        }
      ]
    },
    "OpControlBarrier": {
      "opcode": 224,
      "operands": [
        {
          "kind": "IdScope"
        },
        {
          "kind": "IdScope"
        },
        {
          "kind": "IdMemorySemantics"
        }
      ]
    },
    "OpMemoryBarrier": {
      "opcode": 225,
      "operands": [
        {
          "kind": "IdScope"
        },
        {
          "kind": "IdMemorySemantics"
        }
      ]
    },
    "OpAtomicLoad": {
      "opcode": 227,
      "operands": [
        {
          "kind": "IdResultType"
        },
        {
          "kind": "IdResult"
        },
        {
          "kind": "IdRef"
        },
        {
          "kind": "IdScope"
        },
        {
          "kind": "IdMemorySemantics"
        }
      ]
    },
    "OpAtomicStore": {
      "opcode": 228,
      "operands": [
        {
          "kind": "IdRef"
        },
        {
          "kind": "IdScope"
        },
        {
          "kind": "IdMemorySemantics"
        },
        {
          "kind": "IdRef"
        }
      ]
    },
    "OpAtomicExchange": {
      "opcode": 229,
      "operands": [
        {
          "kind": "IdResultType"
        },
        {
          "kind": "IdResult"
        },
        {
          "kind": "IdRef"
        },
        {
          "kind": "IdScope"
        },
        {
          "kind": "IdMemorySemantics"
        },
        {
          "kind": "IdRef"
        }
      ]
    },
    "OpAtomicCompareExchange": {
      "opcode": 230,
      "operands": [
        {
          "kind": "IdResultType"
        },
        {
          "kind": "IdResult"
        },
        {
          "kind": "IdRef"
        },
        {
          "kind": "IdScope"
        },
        {
          "kind": "IdMemorySemantics"
        },
        {
          "kind": "IdMemorySemantics"
        },
        {
          "kind": "IdRef"
        },
        {
          "kind": "IdRef"
        }
      ]
    },
    "OpAtomicIIncrement": {
      "opcode": 232,
      "operands": [
        {
          "kind": "IdResultType"
        },
        {
          "kind": "IdResult"
        },
        {
          "kind": "IdRef"
        },
        {
          "kind": "IdScope"
        },
        {
          "kind": "IdMemorySemantics"
        }
      ]
    },
    "OpAtomicIDecrement": {
      "opcode": 233,
      "operands": [
        {
          "kind": "IdResultType"
        },
        {
          "kind": "IdResult"
        },
        {
          "kind": "IdRef"
        },
        {
          "kind": "IdScope"
        },
        {
          "kind": "IdMemorySemantics"
        }
      ]
    },
    "OpAtomicIAdd": {
      "opcode": 234,
      "operands": [
        {
          "kind": "IdResultType"
        },
        {
          "kind": "IdResult"
        },
        {
          "kind": "IdRef"
        },
        {
          "kind": "IdScope"
        },
        {
          "kind": "IdMemorySemantics"
        },
        {
          "kind": "IdRef"
        }
      ]
    },
    "OpAtomicISub": {
      "opcode": 235,
      "operands": [
        {
          "kind": "IdResultType"
        },
        {
          "kind": "IdResult"
        },
        {
          "kind": "IdRef"
        },
        {
          "kind": "IdScope"
        },
        {
          "kind": "IdMemorySemantics"
        },
        {
          "kind": "IdRef"
        }
      ]
    },
    "OpAtomicSMin": {
      "opcode": 236,
      "operands": [
        {
          "kind": "IdResultType"
        },
        {
          "kind": "IdResult"
        },
        {
          "kind": "IdRef"
        },
        {
          "kind": "IdScope"
        },
        {
          "kind": "IdMemorySemantics"
        },
        {
          "kind": "IdRef"
        }
      ]
    },
    "OpAtomicUMin": {
      "opcode": 237,
      "operands": [
        {
          "kind": "IdResultType"
        },
        {
          "kind": "IdResult"
        },
        {
          "kind": "IdRef"
        },
        {
          "kind": "IdScope"
        },
        {
          "kind": "IdMemorySemantics"
        },
        {
          "kind": "IdRef"
        }
      ]
    },
    "OpAtomicSMax": {
      "opcode": 238,
      "operands": [
        {
          "kind": "IdResultType"
        },
        {
          "kind": "IdResult"
        },
        {
          "kind": "IdRef"
        },
        {
          "kind": "IdScope"
        },
        {
          "kind": "IdMemorySemantics"
        },
        {
          "kind": "IdRef"
        }
      ]
    },
    "OpAtomicUMax": {
      "opcode": 239,
      "operands": [
        {
          "kind": "IdResultType"
        },
        {
          "kind": "IdResult"
        },
        {
          "kind": "IdRef"
        },
        {
          "kind": "IdScope"
        },
        {
          "kind": "IdMemorySemantics"
        },
        {
          "kind": "IdRef"
        }
      ]
    },
    "OpAtomicAnd": {
      "opcode": 240,
      "operands": [
        {
          "kind": "IdResultType"
        },
        {
          "kind": "IdResult"
        },
        {
          "kind": "IdRef"
        },
        {
          "kind": "IdScope"
        },
        {
          "kind": "IdMemorySemantics"
        },
        {
          "kind": "IdRef"
        }
      ]
    },
    "OpAtomicOr": {
      "opcode": 241,
      "operands": [
        {
          "kind": "IdResultType"
        },
        {
          "kind": "IdResult"
        },
        {
          "kind": "IdRef"
        },
        {
          "kind": "IdScope"
        },
        {
          "kind": "IdMemorySemantics"
        },
        {
          "kind": "IdRef"
        }
      ]
    },
    "OpAtomicXor": {
      "opcode": 242,
      "operands": [
        {
          "kind": "IdResultType"
        },
        {
          "kind": "IdResult"
        },
        {
          "kind": "IdRef"
        },
        {
          "kind": "IdScope"
        },
        {
          "kind": "IdMemorySemantics"
        },
        {
          "kind": "IdRef"
        }
      ]
    },
    "OpPhi": {
      "opcode": 245,
      "operands": [
        {
          "kind": "IdResultType"
        },
        {
          "kind": "IdResult"
        },
        {
          "kind": "PairIdRefIdRef",
          "quantifier": "*"
        }
      ]
    },
    "OpLoopMerge": {
      "opcode": 246,
      "operands": [
        {
          "kind": "IdRef"
        },
        {
          "kind": "IdRef"
        },
        {
          "kind": "LoopControl"
        }
      ]
    },
    "OpSelectionMerge": {
      "opcode": 247,
      "operands": [
        {
          "kind": "IdRef"
        },
        {
          "kind": "SelectionControl"
        }
      ]
    },
    "OpLabel": {
      "opcode": 248,
      "operands": [
        {
          "kind": "IdResult"
        }
      ]
    },
    "OpBranch": {
      "opcode": 249,
      "operands": [
        {
          "kind": "IdRef"
        }
      ]
    },
    "OpBranchConditional": {
      "opcode": 250,
      "operands": [
        {
          "kind": "IdRef"
        },
        {
          "kind": "IdRef"
        },
        {
          "kind": "IdRef"
        },
        {
          "kind": "LiteralInteger",
          "quantifier": "*"
        }
      ]
    },
    "OpSwitch": {
      "opcode": 251,
      "operands": [
        {
          "kind": "IdRef"
        },
        {
          "kind": "IdRef"
        },
        {
          "kind": "PairLiteralIntegerIdRef",
          "quantifier": "*"
        }
      ]
    },
    "OpKill": {
      "opcode": 252,
      "operands": [

      ]
    },
    "OpReturn": {
      "opcode": 253,
      "operands": [

      ]
    },
    "OpReturnValue": {
      "opcode": 254,
      "operands": [
        {
          "kind": "IdRef"
        }
      ]
    },
    "OpUnreachable": {
      "opcode": 255,
      "operands": [

      ]
    },
    "OpNoLine": {
      "opcode": 317,
      "operands": [

      ]
    },
    "OpModuleProcessed": {
      "opcode": 330,
      "operands": [
        {
          "kind": "LiteralString"
        }
      ]
    },
    "OpExecutionModeId": {
      "opcode": 331,
      "operands": [
        {
          "kind": "IdRef"
        },
        {
          "kind": "ExecutionMode"
        }
      ]
    },
    "OpDecorateId": {
      "opcode": 332,
      "operands": [
        {
          "kind": "IdRef"
        },
        {
          "kind": "Decoration"
        }
      ]
    },
    "OpCopyLogical": {
      "opcode": 400,
      "operands": [
        {
          "kind": "IdResultType"
        },
        {
          "kind": "IdResult"
        },
        {
          "kind": "IdRef"
        }
      ]
    },
    "OpPtrEqual": {
      "opcode": 401,
      "operands": [
        {
          "kind": "IdResultType"
        },
        {
          "kind": "IdResult"
        },
        {
          "kind": "IdRef"
        },
        {
          "kind": "IdRef"
        }
      ]
    },
    "OpPtrNotEqual": {
      "opcode": 402,
      "operands": [
        {
          "kind": "IdResultType"
        },
        {
          "kind": "IdResult"
        },
        {
          "kind": "IdRef"
        },
        {
          "kind": "IdRef"
        }
      ]
    },
    "OpDecorateString": {
      "opcode": 5632,
      "operands": [
        {
          "kind": "IdRef"
        },
        {
          "kind": "Decoration"
        }
      ]
    },
    "OpDecorateStringGOOGLE": {
      "opcode": 5632,
      "operands": [
        {
          "kind": "IdRef"
        },
        {
          "kind": "Decoration"
        }
      ]
    },
    "OpMemberDecorateString": {
      "opcode": 5633,
      "operands": [
        {
          "kind": "IdRef"
        },
        {
          "kind": "LiteralInteger"
        },
        {
          "kind": "Decoration"
        }
      ]
    },
    "OpMemberDecorateStringGOOGLE": {
      "opcode": 5633,
      "operands": [
        {
          "kind": "IdRef"
        },
        {
          "kind": "LiteralInteger"
        },
        {
          "kind": "Decoration"
        }
      ]
    }
  },
  "operand_kinds": {
    "ImageOperands": {
      "type": "BitEnum",
      "values": {
        "None": {
          "value": 0
        },
        "Bias": {
          "value": 1,
          "params": [
            "IdRef"
          ]
        },
        "Lod": {
          "value": 2,
          "params": [
            "IdRef"
          ]
        },
        "Grad": {
          "value": 4,
          "params": [
            "IdRef",
            "IdRef"
          ]
        },
        "ConstOffset": {
          "value": 8,
          "params": [
            "IdRef"
          ]
        },
        "Sample": {
          "value": 64,
          "params": [
            "IdRef"
          ]
        },
        "MakeTexelAvailable": {
          "value": 256,
          "params": [
            "IdScope"
          ]
        },
        "MakeTexelAvailableKHR": {
          "value": 256,
          "params": [
            "IdScope"
          ]
        },
        "MakeTexelVisible": {
          "value": 512,
          "params": [
            "IdScope"
          ]
        },
        "MakeTexelVisibleKHR": {
          "value": 512,
          "params": [
            "IdScope"
          ]
        },
        "NonPrivateTexel": {
          "value": 1024
        },
        "NonPrivateTexelKHR": {
          "value": 1024
        },
        "VolatileTexel": {
          "value": 2048
        },
        "VolatileTexelKHR": {
          "value": 2048
        },
        "SignExtend": {
          "value": 4096
        },
        "ZeroExtend": {
          "value": 8192
        }
      }
    },
    "FPFastMathMode": {
      "type": "BitEnum",
      "values": {
        "None": {
          "value": 0
        }
      }
    },
    "SelectionControl": {
      "type": "BitEnum",
      "values": {
        "None": {
          "value": 0
        },
        "Flatten": {
          "value": 1
        },
        "DontFlatten": {
          "value": 2
        }
      }
    },
    "LoopControl": {
      "type": "BitEnum",
      "values": {
        "None": {
          "value": 0
        },
        "Unroll": {
          "value": 1
        },
        "DontUnroll": {
          "value": 2
        },
        "DependencyInfinite": {
          "value": 4
        },
        "DependencyLength": {
          "value": 8,
          "params": [
            "LiteralInteger"
          ]
        },
        "MinIterations": {
          "value": 16,
          "params": [
            "LiteralInteger"
          ]
        },
        "MaxIterations": {
          "value": 32,
          "params": [
            "LiteralInteger"
          ]
        },
        "IterationMultiple": {
          "value": 64,
          "params": [
            "LiteralInteger"
          ]
        },
        "PeelCount": {
          "value": 128,
          "params": [
            "LiteralInteger"
          ]
        },
        "PartialCount": {
          "value": 256,
          "params": [
            "LiteralInteger"
          ]
        }
      }
    },
    "FunctionControl": {
      "type": "BitEnum",
      "values": {
        "None": {
          "value": 0
        },
        "Inline": {
          "value": 1
        },
        "DontInline": {
          "value": 2
        },
        "Pure": {
          "value": 4
        },
        "Const": {
          "value": 8
        }
      }
    },
    "MemorySemantics": {
      "type": "BitEnum",
      "values": {
        "Relaxed": {
          "value": 0
        },
        "None": {
          "value": 0
        },
        "Acquire": {
          "value": 2
        },
        "Release": {
          "value": 4
        },
        "AcquireRelease": {
          "value": 8
        },
        "SequentiallyConsistent": {
          "value": 16
        },
        "UniformMemory": {
          "value": 64
        },
        "SubgroupMemory": {
          "value": 128
        },
        "WorkgroupMemory": {
          "value": 256
        },
        "CrossWorkgroupMemory": {
          "value": 512
        },
        "ImageMemory": {
          "value": 2048
        },
        "OutputMemory": {
          "value": 4096
        },
        "OutputMemoryKHR": {
          "value": 4096
        },
        "MakeAvailable": {
          "value": 8192
        },
        "MakeAvailableKHR": {
          "value": 8192
        },
        "MakeVisible": {
          "value": 16384
        },
        "MakeVisibleKHR": {
          "value": 16384
        },
        "Volatile": {
          "value": 32768
        }
      }
    },
    "MemoryAccess": {
      "type": "BitEnum",
      "values": {
        "None": {
          "value": 0
        },
        "Volatile": {
          "value": 1
        },
        "Aligned": {
          "value": 2,
          "params": [
            "LiteralInteger"
          ]
        },
        "Nontemporal": {
          "value": 4
        },
        "MakePointerAvailable": {
          "value": 8,
          "params": [
            "IdScope"
          ]
        },
        "MakePointerAvailableKHR": {
          "value": 8,
          "params": [
            "IdScope"
          ]
        },
        "MakePointerVisible": {
          "value": 16,
          "params": [
            "IdScope"
          ]
        },
        "MakePointerVisibleKHR": {
          "value": 16,
          "params": [
            "IdScope"
          ]
        },
        "NonPrivatePointer": {
          "value": 32
        },
        "NonPrivatePointerKHR": {
          "value": 32
        }
      }
    },
    "KernelProfilingInfo": {
      "type": "BitEnum",
      "values": {
        "None": {
          "value": 0
        }
      }
    },
    "SourceLanguage": {
      "type": "ValueEnum",
      "values": {
        "Unknown": {
          "value": 0
        },
        "ESSL": {
          "value": 1
        },
        "GLSL": {
          "value": 2
        },
        "OpenCL_C": {
          "value": 3
        },
        "OpenCL_CPP": {
          "value": 4
        },
        "HLSL": {
          "value": 5
        }
      }
    },
    "ExecutionModel": {
      "type": "ValueEnum",
      "values": {
        "Vertex": {
          "value": 0
        },
        "Fragment": {
          "value": 4
        },
        "GLCompute": {
          "value": 5
        }
      }
    },
    "AddressingModel": {
      "type": "ValueEnum",
      "values": {
        "Logical": {
          "value": 0
        }
      }
    },
    "MemoryModel": {
      "type": "ValueEnum",
      "values": {
        "Simple": {
          "value": 0
        },
        "GLSL450": {
          "value": 1
        },
        "Vulkan": {
          "value": 3
        },
        "VulkanKHR": {
          "value": 3
        }
      }
    },
    "ExecutionMode": {
      "type": "ValueEnum",
      "values": {
        "PixelCenterInteger": {
          "value": 6
        },
        "OriginUpperLeft": {
          "value": 7
        },
        "OriginLowerLeft": {
          "value": 8
        },
        "EarlyFragmentTests": {
          "value": 9
        },
        "DepthReplacing": {
          "value": 12
        },
        "DepthGreater": {
          "value": 14
        },
        "DepthLess": {
          "value": 15
        },
        "DepthUnchanged": {
          "value": 16
        },
        "LocalSize": {
          "value": 17,
          "params": [
            "LiteralInteger",
            "LiteralInteger",
            "LiteralInteger"
          ]
        },
        "LocalSizeId": {
          "value": 38,
          "params": [
            "IdRef",
            "IdRef",
            "IdRef"
          ]
        }
      }
    },
    "StorageClass": {
      "type": "ValueEnum",
      "values": {
        "UniformConstant": {
          "value": 0
        },
        "Input": {
          "value": 1
        },
        "Uniform": {
          "value": 2
        },
        "Output": {
          "value": 3
        },
        "Workgroup": {
          "value": 4
        },
        "CrossWorkgroup": {
          "value": 5
        },
        "Private": {
          "value": 6
        },
        "Function": {
          "value": 7
        },
        "PushConstant": {
          "value": 9
        },
        "Image": {
          "value": 11
        },
        "StorageBuffer": {
          "value": 12
        }
      }
    },
    "Dim": {
      "type": "ValueEnum",
      "values": {
        "1D": {
          "value": 0
        },
        "2D": {
          "value": 1
        },
        "3D": {
          "value": 2
        },
        "Cube": {
          "value": 3
        }
      }
    },
    "ImageFormat": {
      "type": "ValueEnum",
      "values": {
        "Unknown": {
          "value": 0
        },
        "Rgba32f": {
          "value": 1
        },
        "Rgba16f": {
          "value": 2
        },
        "R32f": {
          "value": 3
        },
        "Rgba8": {
          "value": 4
        },
        "Rgba8Snorm": {
          "value": 5
        },
        "Rgba32i": {
          "value": 21
        },
        "Rgba16i": {
          "value": 22
        },
        "Rgba8i": {
          "value": 23
        },
        "R32i": {
          "value": 24
        },
        "Rgba32ui": {
          "value": 30
        },
        "Rgba16ui": {
          "value": 31
        },
        "Rgba8ui": {
          "value": 32
        },
        "R32ui": {
          "value": 33
        }
      }
    },
    "FPRoundingMode": {
      "type": "ValueEnum",
      "values": {
        "RTE": {
          "value": 0
        },
        "RTZ": {
          "value": 1
        },
        "RTP": {
          "value": 2
        },
        "RTN": {
          "value": 3
        }
      }
    },
    "Decoration": {
      "type": "ValueEnum",
      "values": {
        "RelaxedPrecision": {
          "value": 0
        },
        "SpecId": {
          "value": 1,
          "params": [
            "LiteralInteger"
          ]
        },
        "Block": {
          "value": 2
        },
        "BufferBlock": {
          "value": 3
        },
        "RowMajor": {
          "value": 4
        },
        "ColMajor": {
          "value": 5
        },
        "ArrayStride": {
          "value": 6,
          "params": [
            "LiteralInteger"
          ]
        },
        "MatrixStride": {
          "value": 7,
          "params": [
            "LiteralInteger"
          ]
        },
        "GLSLShared": {
          "value": 8
        },
        "GLSLPacked": {
          "value": 9
        },
        "BuiltIn": {
          "value": 11,
          "params": [
            "BuiltIn"
          ]
        },
        "NoPerspective": {
          "value": 13
        },
        "Flat": {
          "value": 14
        },
        "Centroid": {
          "value": 16
        },
        "Invariant": {
          "value": 18
        },
        "Restrict": {
          "value": 19
        },
        "Aliased": {
          "value": 20
        },
        "Volatile": {
          "value": 21
        },
        "Coherent": {
          "value": 23
        },
        "NonWritable": {
          "value": 24
        },
        "NonReadable": {
          "value": 25
        },
        "Uniform": {
          "value": 26
        },
        "UniformId": {
          "value": 27,
          "params": [
            "IdScope"
          ]
        },
        "Location": {
          "value": 30,
          "params": [
            "LiteralInteger"
          ]
        },
        "Component": {
          "value": 31,
          "params": [
            "LiteralInteger"
          ]
        },
        "Index": {
          "value": 32,
          "params": [
            "LiteralInteger"
          ]
        },
        "Binding": {
          "value": 33,
          "params": [
            "LiteralInteger"
          ]
        },
        "DescriptorSet": {
          "value": 34,
          "params": [
            "LiteralInteger"
          ]
        },
        "Offset": {
          "value": 35,
          "params": [
            "LiteralInteger"
          ]
        },
        "FPRoundingMode": {
          "value": 39,
          "params": [
            "FPRoundingMode"
          ]
        },
        "NoContraction": {
          "value": 42
        },
        "NoSignedWrap": {
          "value": 4469
        },
        "NoUnsignedWrap": {
          "value": 4470
        },
        "ExplicitInterpAMD": {
          "value": 4999
        },
        "CounterBuffer": {
          "value": 5634,
          "params": [
            "IdRef"
          ]
        },
        "HlslCounterBufferGOOGLE": {
          "value": 5634,
          "params": [
            "IdRef"
          ]
        },
        "UserSemantic": {
          "value": 5635,
          "params": [
            "LiteralString"
          ]
        },
        "HlslSemanticGOOGLE": {
          "value": 5635,
          "params": [
            "LiteralString"
          ]
        },
        "UserTypeGOOGLE": {
          "value": 5636,
          "params": [
            "LiteralString"
          ]
        }
      }
    },
    "BuiltIn": {
      "type": "ValueEnum",
      "values": {
        "Position": {
          "value": 0
        },
        "PointSize": {
          "value": 1
        },
        "VertexId": {
          "value": 5
        },
        "InstanceId": {
          "value": 6
        },
        "FragCoord": {
          "value": 15
        },
        "PointCoord": {
          "value": 16
        },
        "FrontFacing": {
          "value": 17
        },
        "SampleMask": {
          "value": 20
        },
        "FragDepth": {
          "value": 22
        },
        "HelperInvocation": {
          "value": 23
        },
        "NumWorkgroups": {
          "value": 24
        },
        "WorkgroupSize": {
          "value": 25
        },
        "WorkgroupId": {
          "value": 26
        },
        "LocalInvocationId": {
          "value": 27
        },
        "GlobalInvocationId": {
          "value": 28
        },
        "LocalInvocationIndex": {
          "value": 29
        },
        "VertexIndex": {
          "value": 42
        },
        "InstanceIndex": {
          "value": 43
        },
        "BaryCoordNoPerspAMD": {
          "value": 4992
        },
        "BaryCoordNoPerspCentroidAMD": {
          "value": 4993
        },
        "BaryCoordNoPerspSampleAMD": {
          "value": 4994
        },
        "BaryCoordSmoothAMD": {
          "value": 4995
        },
        "BaryCoordSmoothCentroidAMD": {
          "value": 4996
        },
        "BaryCoordSmoothSampleAMD": {
          "value": 4997
        },
        "BaryCoordPullModelAMD": {
          "value": 4998
        }
      }
    },
    "Scope": {
      "type": "ValueEnum",
      "values": {
        "CrossDevice": {
          "value": 0
        },
        "Device": {
          "value": 1
        },
        "Workgroup": {
          "value": 2
        },
        "Subgroup": {
          "value": 3
        },
        "Invocation": {
          "value": 4
        },
        "QueueFamily": {
          "value": 5
        },
        "QueueFamilyKHR": {
          "value": 5
        }
      }
    },
    "Capability": {
      "type": "ValueEnum",
      "values": {
        "Matrix": {
          "value": 0
        },
        "Shader": {
          "value": 1
        },
        "Geometry": {
          "value": 2
        },
        "Tessellation": {
          "value": 3
        },
        "Addresses": {
          "value": 4
        },
        "Linkage": {
          "value": 5
        },
        "Kernel": {
          "value": 6
        },
        "Float16": {
          "value": 9
        },
        "Float64": {
          "value": 10
        },
        "Int64": {
          "value": 11
        },
        "Groups": {
          "value": 18
        },
        "AtomicStorage": {
          "value": 21
        },
        "Int16": {
          "value": 22
        },
        "ImageGatherExtended": {
          "value": 25
        },
        "StorageImageMultisample": {
          "value": 27
        },
        "UniformBufferArrayDynamicIndexing": {
          "value": 28
        },
        "SampledImageArrayDynamicIndexing": {
          "value": 29
        },
        "StorageBufferArrayDynamicIndexing": {
          "value": 30
        },
        "StorageImageArrayDynamicIndexing": {
          "value": 31
        },
        "ClipDistance": {
          "value": 32
        },
        "CullDistance": {
          "value": 33
        },
        "SampleRateShading": {
          "value": 35
        },
        "SampledRect": {
          "value": 37
        },
        "Int8": {
          "value": 39
        },
        "InputAttachment": {
          "value": 40
        },
        "SparseResidency": {
          "value": 41
        },
        "MinLod": {
          "value": 42
        },
        "Sampled1D": {
          "value": 43
        },
        "Image1D": {
          "value": 44
        },
        "SampledCubeArray": {
          "value": 45
        },
        "SampledBuffer": {
          "value": 46
        },
        "ImageMSArray": {
          "value": 48
        },
        "StorageImageExtendedFormats": {
          "value": 49
        },
        "ImageQuery": {
          "value": 50
        },
        "DerivativeControl": {
          "value": 51
        },
        "InterpolationFunction": {
          "value": 52
        },
        "TransformFeedback": {
          "value": 53
        },
        "StorageImageReadWithoutFormat": {
          "value": 55
        },
        "StorageImageWriteWithoutFormat": {
          "value": 56
        },
        "GroupNonUniform": {
          "value": 61
        },
        "ShaderLayer": {
          "value": 69
        },
        "ShaderViewportIndex": {
          "value": 70
        },
        "SubgroupBallotKHR": {
          "value": 4423
        },
        "DrawParameters": {
          "value": 4427
        },
        "SubgroupVoteKHR": {
          "value": 4431
        },
        "StorageBuffer16BitAccess": {
          "value": 4433
        },
        "StorageUniformBufferBlock16": {
          "value": 4433
        },
        "StoragePushConstant16": {
          "value": 4435
        },
        "StorageInputOutput16": {
          "value": 4436
        },
        "DeviceGroup": {
          "value": 4437
        },
        "MultiView": {
          "value": 4439
        },
        "VariablePointersStorageBuffer": {
          "value": 4441
        },
        "AtomicStorageOps": {
          "value": 4445
        },
        "SampleMaskPostDepthCoverage": {
          "value": 4447
        },
        "StorageBuffer8BitAccess": {
          "value": 4448
        },
        "StoragePushConstant8": {
          "value": 4450
        },
        "DenormPreserve": {
          "value": 4464
        },
        "DenormFlushToZero": {
          "value": 4465
        },
        "SignedZeroInfNanPreserve": {
          "value": 4466
        },
        "RoundingModeRTE": {
          "value": 4467
        },
        "RoundingModeRTZ": {
          "value": 4468
        },
        "Float16ImageAMD": {
          "value": 5008
        },
        "ImageGatherBiasLodAMD": {
          "value": 5009
        },
        "FragmentMaskAMD": {
          "value": 5010
        },
        "StencilExportEXT": {
          "value": 5013
        },
        "ImageReadWriteLodAMD": {
          "value": 5015
        },
        "ShaderClockKHR": {
          "value": 5055
        },
        "FragmentFullyCoveredEXT": {
          "value": 5265
        },
        "MeshShadingNV": {
          "value": 5266
        },
        "ImageFootprintNV": {
          "value": 5282
        },
        "FragmentBarycentricNV": {
          "value": 5284
        },
        "ComputeDerivativeGroupQuadsNV": {
          "value": 5288
        },
        "FragmentDensityEXT": {
          "value": 5291
        },
        "ShadingRateNV": {
          "value": 5291
        },
        "GroupNonUniformPartitionedNV": {
          "value": 5297
        },
        "ShaderNonUniform": {
          "value": 5301
        },
        "ShaderNonUniformEXT": {
          "value": 5301
        },
        "RuntimeDescriptorArray": {
          "value": 5302
        },
        "RuntimeDescriptorArrayEXT": {
          "value": 5302
        },
        "RayTracingNV": {
          "value": 5340
        },
        "VulkanMemoryModel": {
          "value": 5345
        },
        "VulkanMemoryModelKHR": {
          "value": 5345
        },
        "VulkanMemoryModelDeviceScope": {
          "value": 5346
        },
        "VulkanMemoryModelDeviceScopeKHR": {
          "value": 5346
        },
        "PhysicalStorageBufferAddresses": {
          "value": 5347
        },
        "PhysicalStorageBufferAddressesEXT": {
          "value": 5347
        },
        "ComputeDerivativeGroupLinearNV": {
          "value": 5350
        },
        "CooperativeMatrixNV": {
          "value": 5357
        },
        "FragmentShaderSampleInterlockEXT": {
          "value": 5363
        },
        "FragmentShaderShadingRateInterlockEXT": {
          "value": 5372
        },
        "ShaderSMBuiltinsNV": {
          "value": 5373
        },
        "FragmentShaderPixelInterlockEXT": {
          "value": 5378
        },
        "DemoteToHelperInvocationEXT": {
          "value": 5379
        },
        "SubgroupShuffleINTEL": {
          "value": 5568
        },
        "SubgroupBufferBlockIOINTEL": {
          "value": 5569
        },
        "SubgroupImageBlockIOINTEL": {
          "value": 5570
        },
        "SubgroupImageMediaBlockIOINTEL": {
          "value": 5579
        },
        "IntegerFunctions2INTEL": {
          "value": 5584
        },
        "SubgroupAvcMotionEstimationINTEL": {
          "value": 5696
        },
        "SubgroupAvcMotionEstimationIntraINTEL": {
          "value": 5697
        },
        "SubgroupAvcMotionEstimationChromaINTEL": {
          "value": 5698
        }
      }
    }
  },
  "ext": {
    "Round": 1,
    "RoundEven": 2,
    "Trunc": 3,
    "FAbs": 4,
    "SAbs": 5,
    "FSign": 6,
    "SSign": 7,
    "Floor": 8,
    "Ceil": 9,
    "Fract": 10,
    "Radians": 11,
    "Degrees": 12,
    "Sin": 13,
    "Cos": 14,
    "Tan": 15,
    "Asin": 16,
    "Acos": 17,
    "Atan": 18,
    "Sinh": 19,
    "Cosh": 20,
    "Tanh": 21,
    "Asinh": 22,
    "Acosh": 23,
    "Atanh": 24,
    "Atan2": 25,
    "Pow": 26,
    "Exp": 27,
    "Log": 28,
    "Exp2": 29,
    "Log2": 30,
    "Sqrt": 31,
    "InverseSqrt": 32,
    "Determinant": 33,
    "MatrixInverse": 34,
    "Modf": 35,
    "ModfStruct": 36,
    "FMin": 37,
    "UMin": 38,
    "SMin": 39,
    "FMax": 40,
    "UMax": 41,
    "SMax": 42,
    "FClamp": 43,
    "UClamp": 44,
    "SClamp": 45,
    "FMix": 46,
    "IMix": 47,
    "Step": 48,
    "SmoothStep": 49,
    "Fma": 50,
    "Frexp": 51,
    "FrexpStruct": 52,
    "Ldexp": 53,
    "PackSnorm4x8": 54,
    "PackUnorm4x8": 55,
    "PackSnorm2x16": 56,
    "PackUnorm2x16": 57,
    "PackHalf2x16": 58,
    "PackDouble2x32": 59,
    "UnpackSnorm2x16": 60,
    "UnpackUnorm2x16": 61,
    "UnpackHalf2x16": 62,
    "UnpackSnorm4x8": 63,
    "UnpackUnorm4x8": 64,
    "UnpackDouble2x32": 65,
    "Length": 66,
    "Distance": 67,
    "Cross": 68,
    "Normalize": 69,
    "FaceForward": 70,
    "Reflect": 71,
    "Refract": 72,
    "FindILsb": 73,
    "FindSMsb": 74,
    "FindUMsb": 75,
    "InterpolateAtCentroid": 76,
    "InterpolateAtSample": 77,
    "InterpolateAtOffset": 78,
    "NMin": 79,
    "NMax": 80,
    "NClamp": 81
  }
}
