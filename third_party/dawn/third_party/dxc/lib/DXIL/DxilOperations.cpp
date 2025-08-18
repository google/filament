///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// DxilOperations.cpp                                                        //
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
// This file is distributed under the University of Illinois Open Source     //
// License. See LICENSE.TXT for details.                                     //
//                                                                           //
// Implementation of DXIL operation tables.                                  //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#include "dxc/DXIL/DxilOperations.h"
#include "dxc/DXIL/DxilConstants.h"
#include "dxc/DXIL/DxilInstructions.h"
#include "dxc/DXIL/DxilModule.h"
#include "dxc/Support/Global.h"

#include "llvm/ADT/ArrayRef.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Type.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;

namespace hlsl {

using OC = OP::OpCode;
using OCC = OP::OpCodeClass;

//------------------------------------------------------------------------------
//
//  OP class const-static data and related static methods.
//
/* <py>
import hctdb_instrhelp
</py> */
/* <py::lines('OPCODE-OLOADS')>hctdb_instrhelp.get_oloads_props()</py>*/
// OPCODE-OLOADS:BEGIN
const OP::OpCodeProperty OP::m_OpCodeProps[(unsigned)OP::OpCode::NumOpCodes] = {
    // Temporary, indexable, input, output registers
    {OC::TempRegLoad,
     "TempRegLoad",
     OCC::TempRegLoad,
     "tempRegLoad",
     Attribute::ReadOnly,
     1,
     {{0x63}},
     {{0x0}}}, // Overloads: hfwi
    {OC::TempRegStore,
     "TempRegStore",
     OCC::TempRegStore,
     "tempRegStore",
     Attribute::None,
     1,
     {{0x63}},
     {{0x0}}}, // Overloads: hfwi
    {OC::MinPrecXRegLoad,
     "MinPrecXRegLoad",
     OCC::MinPrecXRegLoad,
     "minPrecXRegLoad",
     Attribute::ReadOnly,
     1,
     {{0x21}},
     {{0x0}}}, // Overloads: hw
    {OC::MinPrecXRegStore,
     "MinPrecXRegStore",
     OCC::MinPrecXRegStore,
     "minPrecXRegStore",
     Attribute::None,
     1,
     {{0x21}},
     {{0x0}}}, // Overloads: hw
    {OC::LoadInput,
     "LoadInput",
     OCC::LoadInput,
     "loadInput",
     Attribute::ReadNone,
     1,
     {{0x63}},
     {{0x0}}}, // Overloads: hfwi
    {OC::StoreOutput,
     "StoreOutput",
     OCC::StoreOutput,
     "storeOutput",
     Attribute::None,
     1,
     {{0x63}},
     {{0x0}}}, // Overloads: hfwi

    // Unary float
    {OC::FAbs,
     "FAbs",
     OCC::Unary,
     "unary",
     Attribute::ReadNone,
     1,
     {{0x407}},
     {{0x7}}}, // Overloads: hfd<hfd
    {OC::Saturate,
     "Saturate",
     OCC::Unary,
     "unary",
     Attribute::ReadNone,
     1,
     {{0x407}},
     {{0x7}}}, // Overloads: hfd<hfd
    {OC::IsNaN,
     "IsNaN",
     OCC::IsSpecialFloat,
     "isSpecialFloat",
     Attribute::ReadNone,
     1,
     {{0x403}},
     {{0x3}}}, // Overloads: hf<hf
    {OC::IsInf,
     "IsInf",
     OCC::IsSpecialFloat,
     "isSpecialFloat",
     Attribute::ReadNone,
     1,
     {{0x403}},
     {{0x3}}}, // Overloads: hf<hf
    {OC::IsFinite,
     "IsFinite",
     OCC::IsSpecialFloat,
     "isSpecialFloat",
     Attribute::ReadNone,
     1,
     {{0x403}},
     {{0x3}}}, // Overloads: hf<hf
    {OC::IsNormal,
     "IsNormal",
     OCC::IsSpecialFloat,
     "isSpecialFloat",
     Attribute::ReadNone,
     1,
     {{0x403}},
     {{0x3}}}, // Overloads: hf<hf
    {OC::Cos,
     "Cos",
     OCC::Unary,
     "unary",
     Attribute::ReadNone,
     1,
     {{0x403}},
     {{0x3}}}, // Overloads: hf<hf
    {OC::Sin,
     "Sin",
     OCC::Unary,
     "unary",
     Attribute::ReadNone,
     1,
     {{0x403}},
     {{0x3}}}, // Overloads: hf<hf
    {OC::Tan,
     "Tan",
     OCC::Unary,
     "unary",
     Attribute::ReadNone,
     1,
     {{0x403}},
     {{0x3}}}, // Overloads: hf<hf
    {OC::Acos,
     "Acos",
     OCC::Unary,
     "unary",
     Attribute::ReadNone,
     1,
     {{0x403}},
     {{0x3}}}, // Overloads: hf<hf
    {OC::Asin,
     "Asin",
     OCC::Unary,
     "unary",
     Attribute::ReadNone,
     1,
     {{0x403}},
     {{0x3}}}, // Overloads: hf<hf
    {OC::Atan,
     "Atan",
     OCC::Unary,
     "unary",
     Attribute::ReadNone,
     1,
     {{0x403}},
     {{0x3}}}, // Overloads: hf<hf
    {OC::Hcos,
     "Hcos",
     OCC::Unary,
     "unary",
     Attribute::ReadNone,
     1,
     {{0x403}},
     {{0x3}}}, // Overloads: hf<hf
    {OC::Hsin,
     "Hsin",
     OCC::Unary,
     "unary",
     Attribute::ReadNone,
     1,
     {{0x403}},
     {{0x3}}}, // Overloads: hf<hf
    {OC::Htan,
     "Htan",
     OCC::Unary,
     "unary",
     Attribute::ReadNone,
     1,
     {{0x403}},
     {{0x3}}}, // Overloads: hf<hf
    {OC::Exp,
     "Exp",
     OCC::Unary,
     "unary",
     Attribute::ReadNone,
     1,
     {{0x403}},
     {{0x3}}}, // Overloads: hf<hf
    {OC::Frc,
     "Frc",
     OCC::Unary,
     "unary",
     Attribute::ReadNone,
     1,
     {{0x403}},
     {{0x3}}}, // Overloads: hf<hf
    {OC::Log,
     "Log",
     OCC::Unary,
     "unary",
     Attribute::ReadNone,
     1,
     {{0x403}},
     {{0x3}}}, // Overloads: hf<hf
    {OC::Sqrt,
     "Sqrt",
     OCC::Unary,
     "unary",
     Attribute::ReadNone,
     1,
     {{0x403}},
     {{0x3}}}, // Overloads: hf<hf
    {OC::Rsqrt,
     "Rsqrt",
     OCC::Unary,
     "unary",
     Attribute::ReadNone,
     1,
     {{0x403}},
     {{0x3}}}, // Overloads: hf<hf

    // Unary float - rounding
    {OC::Round_ne,
     "Round_ne",
     OCC::Unary,
     "unary",
     Attribute::ReadNone,
     1,
     {{0x403}},
     {{0x3}}}, // Overloads: hf<hf
    {OC::Round_ni,
     "Round_ni",
     OCC::Unary,
     "unary",
     Attribute::ReadNone,
     1,
     {{0x403}},
     {{0x3}}}, // Overloads: hf<hf
    {OC::Round_pi,
     "Round_pi",
     OCC::Unary,
     "unary",
     Attribute::ReadNone,
     1,
     {{0x403}},
     {{0x3}}}, // Overloads: hf<hf
    {OC::Round_z,
     "Round_z",
     OCC::Unary,
     "unary",
     Attribute::ReadNone,
     1,
     {{0x403}},
     {{0x3}}}, // Overloads: hf<hf

    // Unary int
    {OC::Bfrev,
     "Bfrev",
     OCC::Unary,
     "unary",
     Attribute::ReadNone,
     1,
     {{0x4e0}},
     {{0xe0}}}, // Overloads: wil<wil
    {OC::Countbits,
     "Countbits",
     OCC::UnaryBits,
     "unaryBits",
     Attribute::ReadNone,
     1,
     {{0x4e0}},
     {{0xe0}}}, // Overloads: wil<wil
    {OC::FirstbitLo,
     "FirstbitLo",
     OCC::UnaryBits,
     "unaryBits",
     Attribute::ReadNone,
     1,
     {{0x4e0}},
     {{0xe0}}}, // Overloads: wil<wil

    // Unary uint
    {OC::FirstbitHi,
     "FirstbitHi",
     OCC::UnaryBits,
     "unaryBits",
     Attribute::ReadNone,
     1,
     {{0x4e0}},
     {{0xe0}}}, // Overloads: wil<wil

    // Unary int
    {OC::FirstbitSHi,
     "FirstbitSHi",
     OCC::UnaryBits,
     "unaryBits",
     Attribute::ReadNone,
     1,
     {{0x4e0}},
     {{0xe0}}}, // Overloads: wil<wil

    // Binary float
    {OC::FMax,
     "FMax",
     OCC::Binary,
     "binary",
     Attribute::ReadNone,
     1,
     {{0x407}},
     {{0x7}}}, // Overloads: hfd<hfd
    {OC::FMin,
     "FMin",
     OCC::Binary,
     "binary",
     Attribute::ReadNone,
     1,
     {{0x407}},
     {{0x7}}}, // Overloads: hfd<hfd

    // Binary int
    {OC::IMax,
     "IMax",
     OCC::Binary,
     "binary",
     Attribute::ReadNone,
     1,
     {{0x4e0}},
     {{0xe0}}}, // Overloads: wil<wil
    {OC::IMin,
     "IMin",
     OCC::Binary,
     "binary",
     Attribute::ReadNone,
     1,
     {{0x4e0}},
     {{0xe0}}}, // Overloads: wil<wil

    // Binary uint
    {OC::UMax,
     "UMax",
     OCC::Binary,
     "binary",
     Attribute::ReadNone,
     1,
     {{0x4e0}},
     {{0xe0}}}, // Overloads: wil<wil
    {OC::UMin,
     "UMin",
     OCC::Binary,
     "binary",
     Attribute::ReadNone,
     1,
     {{0x4e0}},
     {{0xe0}}}, // Overloads: wil<wil

    // Binary int with two outputs
    {OC::IMul,
     "IMul",
     OCC::BinaryWithTwoOuts,
     "binaryWithTwoOuts",
     Attribute::ReadNone,
     1,
     {{0x40}},
     {{0x0}}}, // Overloads: i

    // Binary uint with two outputs
    {OC::UMul,
     "UMul",
     OCC::BinaryWithTwoOuts,
     "binaryWithTwoOuts",
     Attribute::ReadNone,
     1,
     {{0x40}},
     {{0x0}}}, // Overloads: i
    {OC::UDiv,
     "UDiv",
     OCC::BinaryWithTwoOuts,
     "binaryWithTwoOuts",
     Attribute::ReadNone,
     1,
     {{0x40}},
     {{0x0}}}, // Overloads: i

    // Binary uint with carry or borrow
    {OC::UAddc,
     "UAddc",
     OCC::BinaryWithCarryOrBorrow,
     "binaryWithCarryOrBorrow",
     Attribute::ReadNone,
     1,
     {{0x40}},
     {{0x0}}}, // Overloads: i
    {OC::USubb,
     "USubb",
     OCC::BinaryWithCarryOrBorrow,
     "binaryWithCarryOrBorrow",
     Attribute::ReadNone,
     1,
     {{0x40}},
     {{0x0}}}, // Overloads: i

    // Tertiary float
    {OC::FMad,
     "FMad",
     OCC::Tertiary,
     "tertiary",
     Attribute::ReadNone,
     1,
     {{0x407}},
     {{0x7}}}, // Overloads: hfd<hfd
    {OC::Fma,
     "Fma",
     OCC::Tertiary,
     "tertiary",
     Attribute::ReadNone,
     1,
     {{0x404}},
     {{0x4}}}, // Overloads: d<d

    // Tertiary int
    {OC::IMad,
     "IMad",
     OCC::Tertiary,
     "tertiary",
     Attribute::ReadNone,
     1,
     {{0x4e0}},
     {{0xe0}}}, // Overloads: wil<wil

    // Tertiary uint
    {OC::UMad,
     "UMad",
     OCC::Tertiary,
     "tertiary",
     Attribute::ReadNone,
     1,
     {{0x4e0}},
     {{0xe0}}}, // Overloads: wil<wil

    // Tertiary int
    {OC::Msad,
     "Msad",
     OCC::Tertiary,
     "tertiary",
     Attribute::ReadNone,
     1,
     {{0xc0}},
     {{0x0}}}, // Overloads: il
    {OC::Ibfe,
     "Ibfe",
     OCC::Tertiary,
     "tertiary",
     Attribute::ReadNone,
     1,
     {{0xc0}},
     {{0x0}}}, // Overloads: il

    // Tertiary uint
    {OC::Ubfe,
     "Ubfe",
     OCC::Tertiary,
     "tertiary",
     Attribute::ReadNone,
     1,
     {{0xc0}},
     {{0x0}}}, // Overloads: il

    // Quaternary
    {OC::Bfi,
     "Bfi",
     OCC::Quaternary,
     "quaternary",
     Attribute::ReadNone,
     1,
     {{0x40}},
     {{0x0}}}, // Overloads: i

    // Dot
    {OC::Dot2,
     "Dot2",
     OCC::Dot2,
     "dot2",
     Attribute::ReadNone,
     1,
     {{0x3}},
     {{0x0}}}, // Overloads: hf
    {OC::Dot3,
     "Dot3",
     OCC::Dot3,
     "dot3",
     Attribute::ReadNone,
     1,
     {{0x3}},
     {{0x0}}}, // Overloads: hf
    {OC::Dot4,
     "Dot4",
     OCC::Dot4,
     "dot4",
     Attribute::ReadNone,
     1,
     {{0x3}},
     {{0x0}}}, // Overloads: hf

    // Resources
    {OC::CreateHandle,
     "CreateHandle",
     OCC::CreateHandle,
     "createHandle",
     Attribute::ReadOnly,
     0,
     {},
     {}}, // Overloads: v
    {OC::CBufferLoad,
     "CBufferLoad",
     OCC::CBufferLoad,
     "cbufferLoad",
     Attribute::ReadOnly,
     1,
     {{0xf7}},
     {{0x0}}}, // Overloads: hfd8wil
    {OC::CBufferLoadLegacy,
     "CBufferLoadLegacy",
     OCC::CBufferLoadLegacy,
     "cbufferLoadLegacy",
     Attribute::ReadOnly,
     1,
     {{0xe7}},
     {{0x0}}}, // Overloads: hfdwil

    // Resources - sample
    {OC::Sample,
     "Sample",
     OCC::Sample,
     "sample",
     Attribute::ReadOnly,
     1,
     {{0x63}},
     {{0x0}}}, // Overloads: hfwi
    {OC::SampleBias,
     "SampleBias",
     OCC::SampleBias,
     "sampleBias",
     Attribute::ReadOnly,
     1,
     {{0x63}},
     {{0x0}}}, // Overloads: hfwi
    {OC::SampleLevel,
     "SampleLevel",
     OCC::SampleLevel,
     "sampleLevel",
     Attribute::ReadOnly,
     1,
     {{0x63}},
     {{0x0}}}, // Overloads: hfwi
    {OC::SampleGrad,
     "SampleGrad",
     OCC::SampleGrad,
     "sampleGrad",
     Attribute::ReadOnly,
     1,
     {{0x63}},
     {{0x0}}}, // Overloads: hfwi
    {OC::SampleCmp,
     "SampleCmp",
     OCC::SampleCmp,
     "sampleCmp",
     Attribute::ReadOnly,
     1,
     {{0x3}},
     {{0x0}}}, // Overloads: hf
    {OC::SampleCmpLevelZero,
     "SampleCmpLevelZero",
     OCC::SampleCmpLevelZero,
     "sampleCmpLevelZero",
     Attribute::ReadOnly,
     1,
     {{0x3}},
     {{0x0}}}, // Overloads: hf

    // Resources
    {OC::TextureLoad,
     "TextureLoad",
     OCC::TextureLoad,
     "textureLoad",
     Attribute::ReadOnly,
     1,
     {{0x63}},
     {{0x0}}}, // Overloads: hfwi
    {OC::TextureStore,
     "TextureStore",
     OCC::TextureStore,
     "textureStore",
     Attribute::None,
     1,
     {{0x63}},
     {{0x0}}}, // Overloads: hfwi
    {OC::BufferLoad,
     "BufferLoad",
     OCC::BufferLoad,
     "bufferLoad",
     Attribute::ReadOnly,
     1,
     {{0x63}},
     {{0x0}}}, // Overloads: hfwi
    {OC::BufferStore,
     "BufferStore",
     OCC::BufferStore,
     "bufferStore",
     Attribute::None,
     1,
     {{0x63}},
     {{0x0}}}, // Overloads: hfwi
    {OC::BufferUpdateCounter,
     "BufferUpdateCounter",
     OCC::BufferUpdateCounter,
     "bufferUpdateCounter",
     Attribute::None,
     0,
     {},
     {}}, // Overloads: v
    {OC::CheckAccessFullyMapped,
     "CheckAccessFullyMapped",
     OCC::CheckAccessFullyMapped,
     "checkAccessFullyMapped",
     Attribute::ReadOnly,
     1,
     {{0x40}},
     {{0x0}}}, // Overloads: i
    {OC::GetDimensions,
     "GetDimensions",
     OCC::GetDimensions,
     "getDimensions",
     Attribute::ReadOnly,
     0,
     {},
     {}}, // Overloads: v

    // Resources - gather
    {OC::TextureGather,
     "TextureGather",
     OCC::TextureGather,
     "textureGather",
     Attribute::ReadOnly,
     1,
     {{0x63}},
     {{0x0}}}, // Overloads: hfwi
    {OC::TextureGatherCmp,
     "TextureGatherCmp",
     OCC::TextureGatherCmp,
     "textureGatherCmp",
     Attribute::ReadOnly,
     1,
     {{0x63}},
     {{0x0}}}, // Overloads: hfwi

    // Resources - sample
    {OC::Texture2DMSGetSamplePosition,
     "Texture2DMSGetSamplePosition",
     OCC::Texture2DMSGetSamplePosition,
     "texture2DMSGetSamplePosition",
     Attribute::ReadOnly,
     0,
     {},
     {}}, // Overloads: v
    {OC::RenderTargetGetSamplePosition,
     "RenderTargetGetSamplePosition",
     OCC::RenderTargetGetSamplePosition,
     "renderTargetGetSamplePosition",
     Attribute::ReadOnly,
     0,
     {},
     {}}, // Overloads: v
    {OC::RenderTargetGetSampleCount,
     "RenderTargetGetSampleCount",
     OCC::RenderTargetGetSampleCount,
     "renderTargetGetSampleCount",
     Attribute::ReadOnly,
     0,
     {},
     {}}, // Overloads: v

    // Synchronization
    {OC::AtomicBinOp,
     "AtomicBinOp",
     OCC::AtomicBinOp,
     "atomicBinOp",
     Attribute::None,
     1,
     {{0xc0}},
     {{0x0}}}, // Overloads: li
    {OC::AtomicCompareExchange,
     "AtomicCompareExchange",
     OCC::AtomicCompareExchange,
     "atomicCompareExchange",
     Attribute::None,
     1,
     {{0xc0}},
     {{0x0}}}, // Overloads: li
    {OC::Barrier,
     "Barrier",
     OCC::Barrier,
     "barrier",
     Attribute::NoDuplicate,
     0,
     {},
     {}}, // Overloads: v

    // Derivatives
    {OC::CalculateLOD,
     "CalculateLOD",
     OCC::CalculateLOD,
     "calculateLOD",
     Attribute::ReadOnly,
     1,
     {{0x2}},
     {{0x0}}}, // Overloads: f

    // Pixel shader
    {OC::Discard,
     "Discard",
     OCC::Discard,
     "discard",
     Attribute::None,
     0,
     {},
     {}}, // Overloads: v

    // Derivatives
    {OC::DerivCoarseX,
     "DerivCoarseX",
     OCC::Unary,
     "unary",
     Attribute::ReadNone,
     1,
     {{0x403}},
     {{0x3}}}, // Overloads: hf<hf
    {OC::DerivCoarseY,
     "DerivCoarseY",
     OCC::Unary,
     "unary",
     Attribute::ReadNone,
     1,
     {{0x403}},
     {{0x3}}}, // Overloads: hf<hf
    {OC::DerivFineX,
     "DerivFineX",
     OCC::Unary,
     "unary",
     Attribute::ReadNone,
     1,
     {{0x403}},
     {{0x3}}}, // Overloads: hf<hf
    {OC::DerivFineY,
     "DerivFineY",
     OCC::Unary,
     "unary",
     Attribute::ReadNone,
     1,
     {{0x403}},
     {{0x3}}}, // Overloads: hf<hf

    // Pixel shader
    {OC::EvalSnapped,
     "EvalSnapped",
     OCC::EvalSnapped,
     "evalSnapped",
     Attribute::ReadNone,
     1,
     {{0x3}},
     {{0x0}}}, // Overloads: hf
    {OC::EvalSampleIndex,
     "EvalSampleIndex",
     OCC::EvalSampleIndex,
     "evalSampleIndex",
     Attribute::ReadNone,
     1,
     {{0x3}},
     {{0x0}}}, // Overloads: hf
    {OC::EvalCentroid,
     "EvalCentroid",
     OCC::EvalCentroid,
     "evalCentroid",
     Attribute::ReadNone,
     1,
     {{0x3}},
     {{0x0}}}, // Overloads: hf
    {OC::SampleIndex,
     "SampleIndex",
     OCC::SampleIndex,
     "sampleIndex",
     Attribute::ReadNone,
     1,
     {{0x40}},
     {{0x0}}}, // Overloads: i
    {OC::Coverage,
     "Coverage",
     OCC::Coverage,
     "coverage",
     Attribute::ReadNone,
     1,
     {{0x40}},
     {{0x0}}}, // Overloads: i
    {OC::InnerCoverage,
     "InnerCoverage",
     OCC::InnerCoverage,
     "innerCoverage",
     Attribute::ReadNone,
     1,
     {{0x40}},
     {{0x0}}}, // Overloads: i

    // Compute/Mesh/Amplification/Node shader
    {OC::ThreadId,
     "ThreadId",
     OCC::ThreadId,
     "threadId",
     Attribute::ReadNone,
     1,
     {{0x40}},
     {{0x0}}}, // Overloads: i
    {OC::GroupId,
     "GroupId",
     OCC::GroupId,
     "groupId",
     Attribute::ReadNone,
     1,
     {{0x40}},
     {{0x0}}}, // Overloads: i
    {OC::ThreadIdInGroup,
     "ThreadIdInGroup",
     OCC::ThreadIdInGroup,
     "threadIdInGroup",
     Attribute::ReadNone,
     1,
     {{0x40}},
     {{0x0}}}, // Overloads: i
    {OC::FlattenedThreadIdInGroup,
     "FlattenedThreadIdInGroup",
     OCC::FlattenedThreadIdInGroup,
     "flattenedThreadIdInGroup",
     Attribute::ReadNone,
     1,
     {{0x40}},
     {{0x0}}}, // Overloads: i

    // Geometry shader
    {OC::EmitStream,
     "EmitStream",
     OCC::EmitStream,
     "emitStream",
     Attribute::None,
     0,
     {},
     {}}, // Overloads: v
    {OC::CutStream,
     "CutStream",
     OCC::CutStream,
     "cutStream",
     Attribute::None,
     0,
     {},
     {}}, // Overloads: v
    {OC::EmitThenCutStream,
     "EmitThenCutStream",
     OCC::EmitThenCutStream,
     "emitThenCutStream",
     Attribute::None,
     0,
     {},
     {}}, // Overloads: v
    {OC::GSInstanceID,
     "GSInstanceID",
     OCC::GSInstanceID,
     "gsInstanceID",
     Attribute::ReadNone,
     1,
     {{0x40}},
     {{0x0}}}, // Overloads: i

    // Double precision
    {OC::MakeDouble,
     "MakeDouble",
     OCC::MakeDouble,
     "makeDouble",
     Attribute::ReadNone,
     1,
     {{0x4}},
     {{0x0}}}, // Overloads: d
    {OC::SplitDouble,
     "SplitDouble",
     OCC::SplitDouble,
     "splitDouble",
     Attribute::ReadNone,
     1,
     {{0x4}},
     {{0x0}}}, // Overloads: d

    // Domain and hull shader
    {OC::LoadOutputControlPoint,
     "LoadOutputControlPoint",
     OCC::LoadOutputControlPoint,
     "loadOutputControlPoint",
     Attribute::ReadNone,
     1,
     {{0x63}},
     {{0x0}}}, // Overloads: hfwi
    {OC::LoadPatchConstant,
     "LoadPatchConstant",
     OCC::LoadPatchConstant,
     "loadPatchConstant",
     Attribute::ReadNone,
     1,
     {{0x63}},
     {{0x0}}}, // Overloads: hfwi

    // Domain shader
    {OC::DomainLocation,
     "DomainLocation",
     OCC::DomainLocation,
     "domainLocation",
     Attribute::ReadNone,
     1,
     {{0x2}},
     {{0x0}}}, // Overloads: f

    // Hull shader
    {OC::StorePatchConstant,
     "StorePatchConstant",
     OCC::StorePatchConstant,
     "storePatchConstant",
     Attribute::None,
     1,
     {{0x63}},
     {{0x0}}}, // Overloads: hfwi
    {OC::OutputControlPointID,
     "OutputControlPointID",
     OCC::OutputControlPointID,
     "outputControlPointID",
     Attribute::ReadNone,
     1,
     {{0x40}},
     {{0x0}}}, // Overloads: i

    // Hull, Domain and Geometry shaders
    {OC::PrimitiveID,
     "PrimitiveID",
     OCC::PrimitiveID,
     "primitiveID",
     Attribute::ReadNone,
     1,
     {{0x40}},
     {{0x0}}}, // Overloads: i

    // Other
    {OC::CycleCounterLegacy,
     "CycleCounterLegacy",
     OCC::CycleCounterLegacy,
     "cycleCounterLegacy",
     Attribute::None,
     0,
     {},
     {}}, // Overloads: v

    // Wave
    {OC::WaveIsFirstLane,
     "WaveIsFirstLane",
     OCC::WaveIsFirstLane,
     "waveIsFirstLane",
     Attribute::None,
     0,
     {},
     {}}, // Overloads: v
    {OC::WaveGetLaneIndex,
     "WaveGetLaneIndex",
     OCC::WaveGetLaneIndex,
     "waveGetLaneIndex",
     Attribute::ReadOnly,
     0,
     {},
     {}}, // Overloads: v
    {OC::WaveGetLaneCount,
     "WaveGetLaneCount",
     OCC::WaveGetLaneCount,
     "waveGetLaneCount",
     Attribute::ReadNone,
     0,
     {},
     {}}, // Overloads: v
    {OC::WaveAnyTrue,
     "WaveAnyTrue",
     OCC::WaveAnyTrue,
     "waveAnyTrue",
     Attribute::None,
     0,
     {},
     {}}, // Overloads: v
    {OC::WaveAllTrue,
     "WaveAllTrue",
     OCC::WaveAllTrue,
     "waveAllTrue",
     Attribute::None,
     0,
     {},
     {}}, // Overloads: v
    {OC::WaveActiveAllEqual,
     "WaveActiveAllEqual",
     OCC::WaveActiveAllEqual,
     "waveActiveAllEqual",
     Attribute::None,
     1,
     {{0xff}},
     {{0x0}}}, // Overloads: hfd18wil
    {OC::WaveActiveBallot,
     "WaveActiveBallot",
     OCC::WaveActiveBallot,
     "waveActiveBallot",
     Attribute::None,
     0,
     {},
     {}}, // Overloads: v
    {OC::WaveReadLaneAt,
     "WaveReadLaneAt",
     OCC::WaveReadLaneAt,
     "waveReadLaneAt",
     Attribute::None,
     1,
     {{0xff}},
     {{0x0}}}, // Overloads: hfd18wil
    {OC::WaveReadLaneFirst,
     "WaveReadLaneFirst",
     OCC::WaveReadLaneFirst,
     "waveReadLaneFirst",
     Attribute::None,
     1,
     {{0xff}},
     {{0x0}}}, // Overloads: hfd18wil
    {OC::WaveActiveOp,
     "WaveActiveOp",
     OCC::WaveActiveOp,
     "waveActiveOp",
     Attribute::None,
     1,
     {{0xff}},
     {{0x0}}}, // Overloads: hfd18wil
    {OC::WaveActiveBit,
     "WaveActiveBit",
     OCC::WaveActiveBit,
     "waveActiveBit",
     Attribute::None,
     1,
     {{0xf0}},
     {{0x0}}}, // Overloads: 8wil
    {OC::WavePrefixOp,
     "WavePrefixOp",
     OCC::WavePrefixOp,
     "wavePrefixOp",
     Attribute::None,
     1,
     {{0xf7}},
     {{0x0}}}, // Overloads: hfd8wil

    // Quad Wave Ops
    {OC::QuadReadLaneAt,
     "QuadReadLaneAt",
     OCC::QuadReadLaneAt,
     "quadReadLaneAt",
     Attribute::None,
     1,
     {{0xff}},
     {{0x0}}}, // Overloads: hfd18wil
    {OC::QuadOp,
     "QuadOp",
     OCC::QuadOp,
     "quadOp",
     Attribute::None,
     1,
     {{0xf7}},
     {{0x0}}}, // Overloads: hfd8wil

    // Bitcasts with different sizes
    {OC::BitcastI16toF16,
     "BitcastI16toF16",
     OCC::BitcastI16toF16,
     "bitcastI16toF16",
     Attribute::ReadNone,
     0,
     {},
     {}}, // Overloads: v
    {OC::BitcastF16toI16,
     "BitcastF16toI16",
     OCC::BitcastF16toI16,
     "bitcastF16toI16",
     Attribute::ReadNone,
     0,
     {},
     {}}, // Overloads: v
    {OC::BitcastI32toF32,
     "BitcastI32toF32",
     OCC::BitcastI32toF32,
     "bitcastI32toF32",
     Attribute::ReadNone,
     0,
     {},
     {}}, // Overloads: v
    {OC::BitcastF32toI32,
     "BitcastF32toI32",
     OCC::BitcastF32toI32,
     "bitcastF32toI32",
     Attribute::ReadNone,
     0,
     {},
     {}}, // Overloads: v
    {OC::BitcastI64toF64,
     "BitcastI64toF64",
     OCC::BitcastI64toF64,
     "bitcastI64toF64",
     Attribute::ReadNone,
     0,
     {},
     {}}, // Overloads: v
    {OC::BitcastF64toI64,
     "BitcastF64toI64",
     OCC::BitcastF64toI64,
     "bitcastF64toI64",
     Attribute::ReadNone,
     0,
     {},
     {}}, // Overloads: v

    // Legacy floating-point
    {OC::LegacyF32ToF16,
     "LegacyF32ToF16",
     OCC::LegacyF32ToF16,
     "legacyF32ToF16",
     Attribute::ReadNone,
     0,
     {},
     {}}, // Overloads: v
    {OC::LegacyF16ToF32,
     "LegacyF16ToF32",
     OCC::LegacyF16ToF32,
     "legacyF16ToF32",
     Attribute::ReadNone,
     0,
     {},
     {}}, // Overloads: v

    // Double precision
    {OC::LegacyDoubleToFloat,
     "LegacyDoubleToFloat",
     OCC::LegacyDoubleToFloat,
     "legacyDoubleToFloat",
     Attribute::ReadNone,
     0,
     {},
     {}}, // Overloads: v
    {OC::LegacyDoubleToSInt32,
     "LegacyDoubleToSInt32",
     OCC::LegacyDoubleToSInt32,
     "legacyDoubleToSInt32",
     Attribute::ReadNone,
     0,
     {},
     {}}, // Overloads: v
    {OC::LegacyDoubleToUInt32,
     "LegacyDoubleToUInt32",
     OCC::LegacyDoubleToUInt32,
     "legacyDoubleToUInt32",
     Attribute::ReadNone,
     0,
     {},
     {}}, // Overloads: v

    // Wave
    {OC::WaveAllBitCount,
     "WaveAllBitCount",
     OCC::WaveAllOp,
     "waveAllOp",
     Attribute::None,
     0,
     {},
     {}}, // Overloads: v
    {OC::WavePrefixBitCount,
     "WavePrefixBitCount",
     OCC::WavePrefixOp,
     "wavePrefixOp",
     Attribute::None,
     0,
     {},
     {}}, // Overloads: v

    // Pixel shader
    {OC::AttributeAtVertex,
     "AttributeAtVertex",
     OCC::AttributeAtVertex,
     "attributeAtVertex",
     Attribute::ReadNone,
     1,
     {{0x63}},
     {{0x0}}}, // Overloads: hfiw

    // Graphics shader
    {OC::ViewID,
     "ViewID",
     OCC::ViewID,
     "viewID",
     Attribute::ReadNone,
     1,
     {{0x40}},
     {{0x0}}}, // Overloads: i

    // Resources
    {OC::RawBufferLoad,
     "RawBufferLoad",
     OCC::RawBufferLoad,
     "rawBufferLoad",
     Attribute::ReadOnly,
     1,
     {{0xe7}},
     {{0x0}}}, // Overloads: hfwidl
    {OC::RawBufferStore,
     "RawBufferStore",
     OCC::RawBufferStore,
     "rawBufferStore",
     Attribute::None,
     1,
     {{0xe7}},
     {{0x0}}}, // Overloads: hfwidl

    // Raytracing object space uint System Values
    {OC::InstanceID,
     "InstanceID",
     OCC::InstanceID,
     "instanceID",
     Attribute::ReadNone,
     1,
     {{0x40}},
     {{0x0}}}, // Overloads: i
    {OC::InstanceIndex,
     "InstanceIndex",
     OCC::InstanceIndex,
     "instanceIndex",
     Attribute::ReadNone,
     1,
     {{0x40}},
     {{0x0}}}, // Overloads: i

    // Raytracing hit uint System Values
    {OC::HitKind,
     "HitKind",
     OCC::HitKind,
     "hitKind",
     Attribute::ReadNone,
     1,
     {{0x40}},
     {{0x0}}}, // Overloads: i

    // Raytracing uint System Values
    {OC::RayFlags,
     "RayFlags",
     OCC::RayFlags,
     "rayFlags",
     Attribute::ReadNone,
     1,
     {{0x40}},
     {{0x0}}}, // Overloads: i

    // Ray Dispatch Arguments
    {OC::DispatchRaysIndex,
     "DispatchRaysIndex",
     OCC::DispatchRaysIndex,
     "dispatchRaysIndex",
     Attribute::ReadNone,
     1,
     {{0x40}},
     {{0x0}}}, // Overloads: i
    {OC::DispatchRaysDimensions,
     "DispatchRaysDimensions",
     OCC::DispatchRaysDimensions,
     "dispatchRaysDimensions",
     Attribute::ReadNone,
     1,
     {{0x40}},
     {{0x0}}}, // Overloads: i

    // Ray Vectors
    {OC::WorldRayOrigin,
     "WorldRayOrigin",
     OCC::WorldRayOrigin,
     "worldRayOrigin",
     Attribute::ReadNone,
     1,
     {{0x2}},
     {{0x0}}}, // Overloads: f
    {OC::WorldRayDirection,
     "WorldRayDirection",
     OCC::WorldRayDirection,
     "worldRayDirection",
     Attribute::ReadNone,
     1,
     {{0x2}},
     {{0x0}}}, // Overloads: f

    // Ray object space Vectors
    {OC::ObjectRayOrigin,
     "ObjectRayOrigin",
     OCC::ObjectRayOrigin,
     "objectRayOrigin",
     Attribute::ReadNone,
     1,
     {{0x2}},
     {{0x0}}}, // Overloads: f
    {OC::ObjectRayDirection,
     "ObjectRayDirection",
     OCC::ObjectRayDirection,
     "objectRayDirection",
     Attribute::ReadNone,
     1,
     {{0x2}},
     {{0x0}}}, // Overloads: f

    // Ray Transforms
    {OC::ObjectToWorld,
     "ObjectToWorld",
     OCC::ObjectToWorld,
     "objectToWorld",
     Attribute::ReadNone,
     1,
     {{0x2}},
     {{0x0}}}, // Overloads: f
    {OC::WorldToObject,
     "WorldToObject",
     OCC::WorldToObject,
     "worldToObject",
     Attribute::ReadNone,
     1,
     {{0x2}},
     {{0x0}}}, // Overloads: f

    // RayT
    {OC::RayTMin,
     "RayTMin",
     OCC::RayTMin,
     "rayTMin",
     Attribute::ReadNone,
     1,
     {{0x2}},
     {{0x0}}}, // Overloads: f
    {OC::RayTCurrent,
     "RayTCurrent",
     OCC::RayTCurrent,
     "rayTCurrent",
     Attribute::ReadOnly,
     1,
     {{0x2}},
     {{0x0}}}, // Overloads: f

    // AnyHit Terminals
    {OC::IgnoreHit,
     "IgnoreHit",
     OCC::IgnoreHit,
     "ignoreHit",
     Attribute::NoReturn,
     0,
     {},
     {}}, // Overloads: v
    {OC::AcceptHitAndEndSearch,
     "AcceptHitAndEndSearch",
     OCC::AcceptHitAndEndSearch,
     "acceptHitAndEndSearch",
     Attribute::NoReturn,
     0,
     {},
     {}}, // Overloads: v

    // Indirect Shader Invocation
    {OC::TraceRay,
     "TraceRay",
     OCC::TraceRay,
     "traceRay",
     Attribute::None,
     1,
     {{0x100}},
     {{0x0}}}, // Overloads: u
    {OC::ReportHit,
     "ReportHit",
     OCC::ReportHit,
     "reportHit",
     Attribute::None,
     1,
     {{0x100}},
     {{0x0}}}, // Overloads: u
    {OC::CallShader,
     "CallShader",
     OCC::CallShader,
     "callShader",
     Attribute::None,
     1,
     {{0x100}},
     {{0x0}}}, // Overloads: u

    // Library create handle from resource struct (like HL intrinsic)
    {OC::CreateHandleForLib,
     "CreateHandleForLib",
     OCC::CreateHandleForLib,
     "createHandleForLib",
     Attribute::ReadOnly,
     1,
     {{0x200}},
     {{0x0}}}, // Overloads: o

    // Raytracing object space uint System Values
    {OC::PrimitiveIndex,
     "PrimitiveIndex",
     OCC::PrimitiveIndex,
     "primitiveIndex",
     Attribute::ReadNone,
     1,
     {{0x40}},
     {{0x0}}}, // Overloads: i

    // Dot product with accumulate
    {OC::Dot2AddHalf,
     "Dot2AddHalf",
     OCC::Dot2AddHalf,
     "dot2AddHalf",
     Attribute::ReadNone,
     1,
     {{0x2}},
     {{0x0}}}, // Overloads: f
    {OC::Dot4AddI8Packed,
     "Dot4AddI8Packed",
     OCC::Dot4AddPacked,
     "dot4AddPacked",
     Attribute::ReadNone,
     1,
     {{0x40}},
     {{0x0}}}, // Overloads: i
    {OC::Dot4AddU8Packed,
     "Dot4AddU8Packed",
     OCC::Dot4AddPacked,
     "dot4AddPacked",
     Attribute::ReadNone,
     1,
     {{0x40}},
     {{0x0}}}, // Overloads: i

    // Wave
    {OC::WaveMatch,
     "WaveMatch",
     OCC::WaveMatch,
     "waveMatch",
     Attribute::None,
     1,
     {{0xf7}},
     {{0x0}}}, // Overloads: hfd8wil
    {OC::WaveMultiPrefixOp,
     "WaveMultiPrefixOp",
     OCC::WaveMultiPrefixOp,
     "waveMultiPrefixOp",
     Attribute::None,
     1,
     {{0xf7}},
     {{0x0}}}, // Overloads: hfd8wil
    {OC::WaveMultiPrefixBitCount,
     "WaveMultiPrefixBitCount",
     OCC::WaveMultiPrefixBitCount,
     "waveMultiPrefixBitCount",
     Attribute::None,
     0,
     {},
     {}}, // Overloads: v

    // Mesh shader instructions
    {OC::SetMeshOutputCounts,
     "SetMeshOutputCounts",
     OCC::SetMeshOutputCounts,
     "setMeshOutputCounts",
     Attribute::None,
     0,
     {},
     {}}, // Overloads: v
    {OC::EmitIndices,
     "EmitIndices",
     OCC::EmitIndices,
     "emitIndices",
     Attribute::None,
     0,
     {},
     {}}, // Overloads: v
    {OC::GetMeshPayload,
     "GetMeshPayload",
     OCC::GetMeshPayload,
     "getMeshPayload",
     Attribute::ReadOnly,
     1,
     {{0x100}},
     {{0x0}}}, // Overloads: u
    {OC::StoreVertexOutput,
     "StoreVertexOutput",
     OCC::StoreVertexOutput,
     "storeVertexOutput",
     Attribute::None,
     1,
     {{0x63}},
     {{0x0}}}, // Overloads: hfwi
    {OC::StorePrimitiveOutput,
     "StorePrimitiveOutput",
     OCC::StorePrimitiveOutput,
     "storePrimitiveOutput",
     Attribute::None,
     1,
     {{0x63}},
     {{0x0}}}, // Overloads: hfwi

    // Amplification shader instructions
    {OC::DispatchMesh,
     "DispatchMesh",
     OCC::DispatchMesh,
     "dispatchMesh",
     Attribute::None,
     1,
     {{0x100}},
     {{0x0}}}, // Overloads: u

    // Sampler Feedback
    {OC::WriteSamplerFeedback,
     "WriteSamplerFeedback",
     OCC::WriteSamplerFeedback,
     "writeSamplerFeedback",
     Attribute::None,
     0,
     {},
     {}}, // Overloads: v
    {OC::WriteSamplerFeedbackBias,
     "WriteSamplerFeedbackBias",
     OCC::WriteSamplerFeedbackBias,
     "writeSamplerFeedbackBias",
     Attribute::None,
     0,
     {},
     {}}, // Overloads: v
    {OC::WriteSamplerFeedbackLevel,
     "WriteSamplerFeedbackLevel",
     OCC::WriteSamplerFeedbackLevel,
     "writeSamplerFeedbackLevel",
     Attribute::None,
     0,
     {},
     {}}, // Overloads: v
    {OC::WriteSamplerFeedbackGrad,
     "WriteSamplerFeedbackGrad",
     OCC::WriteSamplerFeedbackGrad,
     "writeSamplerFeedbackGrad",
     Attribute::None,
     0,
     {},
     {}}, // Overloads: v

    // Inline Ray Query
    {OC::AllocateRayQuery,
     "AllocateRayQuery",
     OCC::AllocateRayQuery,
     "allocateRayQuery",
     Attribute::None,
     0,
     {},
     {}}, // Overloads: v
    {OC::RayQuery_TraceRayInline,
     "RayQuery_TraceRayInline",
     OCC::RayQuery_TraceRayInline,
     "rayQuery_TraceRayInline",
     Attribute::None,
     0,
     {},
     {}}, // Overloads: v
    {OC::RayQuery_Proceed,
     "RayQuery_Proceed",
     OCC::RayQuery_Proceed,
     "rayQuery_Proceed",
     Attribute::None,
     1,
     {{0x8}},
     {{0x0}}}, // Overloads: 1
    {OC::RayQuery_Abort,
     "RayQuery_Abort",
     OCC::RayQuery_Abort,
     "rayQuery_Abort",
     Attribute::None,
     0,
     {},
     {}}, // Overloads: v
    {OC::RayQuery_CommitNonOpaqueTriangleHit,
     "RayQuery_CommitNonOpaqueTriangleHit",
     OCC::RayQuery_CommitNonOpaqueTriangleHit,
     "rayQuery_CommitNonOpaqueTriangleHit",
     Attribute::None,
     0,
     {},
     {}}, // Overloads: v
    {OC::RayQuery_CommitProceduralPrimitiveHit,
     "RayQuery_CommitProceduralPrimitiveHit",
     OCC::RayQuery_CommitProceduralPrimitiveHit,
     "rayQuery_CommitProceduralPrimitiveHit",
     Attribute::None,
     0,
     {},
     {}}, // Overloads: v
    {OC::RayQuery_CommittedStatus,
     "RayQuery_CommittedStatus",
     OCC::RayQuery_StateScalar,
     "rayQuery_StateScalar",
     Attribute::ReadOnly,
     1,
     {{0x40}},
     {{0x0}}}, // Overloads: i
    {OC::RayQuery_CandidateType,
     "RayQuery_CandidateType",
     OCC::RayQuery_StateScalar,
     "rayQuery_StateScalar",
     Attribute::ReadOnly,
     1,
     {{0x40}},
     {{0x0}}}, // Overloads: i
    {OC::RayQuery_CandidateObjectToWorld3x4,
     "RayQuery_CandidateObjectToWorld3x4",
     OCC::RayQuery_StateMatrix,
     "rayQuery_StateMatrix",
     Attribute::ReadOnly,
     1,
     {{0x2}},
     {{0x0}}}, // Overloads: f
    {OC::RayQuery_CandidateWorldToObject3x4,
     "RayQuery_CandidateWorldToObject3x4",
     OCC::RayQuery_StateMatrix,
     "rayQuery_StateMatrix",
     Attribute::ReadOnly,
     1,
     {{0x2}},
     {{0x0}}}, // Overloads: f
    {OC::RayQuery_CommittedObjectToWorld3x4,
     "RayQuery_CommittedObjectToWorld3x4",
     OCC::RayQuery_StateMatrix,
     "rayQuery_StateMatrix",
     Attribute::ReadOnly,
     1,
     {{0x2}},
     {{0x0}}}, // Overloads: f
    {OC::RayQuery_CommittedWorldToObject3x4,
     "RayQuery_CommittedWorldToObject3x4",
     OCC::RayQuery_StateMatrix,
     "rayQuery_StateMatrix",
     Attribute::ReadOnly,
     1,
     {{0x2}},
     {{0x0}}}, // Overloads: f
    {OC::RayQuery_CandidateProceduralPrimitiveNonOpaque,
     "RayQuery_CandidateProceduralPrimitiveNonOpaque",
     OCC::RayQuery_StateScalar,
     "rayQuery_StateScalar",
     Attribute::ReadOnly,
     1,
     {{0x8}},
     {{0x0}}}, // Overloads: 1
    {OC::RayQuery_CandidateTriangleFrontFace,
     "RayQuery_CandidateTriangleFrontFace",
     OCC::RayQuery_StateScalar,
     "rayQuery_StateScalar",
     Attribute::ReadOnly,
     1,
     {{0x8}},
     {{0x0}}}, // Overloads: 1
    {OC::RayQuery_CommittedTriangleFrontFace,
     "RayQuery_CommittedTriangleFrontFace",
     OCC::RayQuery_StateScalar,
     "rayQuery_StateScalar",
     Attribute::ReadOnly,
     1,
     {{0x8}},
     {{0x0}}}, // Overloads: 1
    {OC::RayQuery_CandidateTriangleBarycentrics,
     "RayQuery_CandidateTriangleBarycentrics",
     OCC::RayQuery_StateVector,
     "rayQuery_StateVector",
     Attribute::ReadOnly,
     1,
     {{0x2}},
     {{0x0}}}, // Overloads: f
    {OC::RayQuery_CommittedTriangleBarycentrics,
     "RayQuery_CommittedTriangleBarycentrics",
     OCC::RayQuery_StateVector,
     "rayQuery_StateVector",
     Attribute::ReadOnly,
     1,
     {{0x2}},
     {{0x0}}}, // Overloads: f
    {OC::RayQuery_RayFlags,
     "RayQuery_RayFlags",
     OCC::RayQuery_StateScalar,
     "rayQuery_StateScalar",
     Attribute::ReadOnly,
     1,
     {{0x40}},
     {{0x0}}}, // Overloads: i
    {OC::RayQuery_WorldRayOrigin,
     "RayQuery_WorldRayOrigin",
     OCC::RayQuery_StateVector,
     "rayQuery_StateVector",
     Attribute::ReadOnly,
     1,
     {{0x2}},
     {{0x0}}}, // Overloads: f
    {OC::RayQuery_WorldRayDirection,
     "RayQuery_WorldRayDirection",
     OCC::RayQuery_StateVector,
     "rayQuery_StateVector",
     Attribute::ReadOnly,
     1,
     {{0x2}},
     {{0x0}}}, // Overloads: f
    {OC::RayQuery_RayTMin,
     "RayQuery_RayTMin",
     OCC::RayQuery_StateScalar,
     "rayQuery_StateScalar",
     Attribute::ReadOnly,
     1,
     {{0x2}},
     {{0x0}}}, // Overloads: f
    {OC::RayQuery_CandidateTriangleRayT,
     "RayQuery_CandidateTriangleRayT",
     OCC::RayQuery_StateScalar,
     "rayQuery_StateScalar",
     Attribute::ReadOnly,
     1,
     {{0x2}},
     {{0x0}}}, // Overloads: f
    {OC::RayQuery_CommittedRayT,
     "RayQuery_CommittedRayT",
     OCC::RayQuery_StateScalar,
     "rayQuery_StateScalar",
     Attribute::ReadOnly,
     1,
     {{0x2}},
     {{0x0}}}, // Overloads: f
    {OC::RayQuery_CandidateInstanceIndex,
     "RayQuery_CandidateInstanceIndex",
     OCC::RayQuery_StateScalar,
     "rayQuery_StateScalar",
     Attribute::ReadOnly,
     1,
     {{0x40}},
     {{0x0}}}, // Overloads: i
    {OC::RayQuery_CandidateInstanceID,
     "RayQuery_CandidateInstanceID",
     OCC::RayQuery_StateScalar,
     "rayQuery_StateScalar",
     Attribute::ReadOnly,
     1,
     {{0x40}},
     {{0x0}}}, // Overloads: i
    {OC::RayQuery_CandidateGeometryIndex,
     "RayQuery_CandidateGeometryIndex",
     OCC::RayQuery_StateScalar,
     "rayQuery_StateScalar",
     Attribute::ReadOnly,
     1,
     {{0x40}},
     {{0x0}}}, // Overloads: i
    {OC::RayQuery_CandidatePrimitiveIndex,
     "RayQuery_CandidatePrimitiveIndex",
     OCC::RayQuery_StateScalar,
     "rayQuery_StateScalar",
     Attribute::ReadOnly,
     1,
     {{0x40}},
     {{0x0}}}, // Overloads: i
    {OC::RayQuery_CandidateObjectRayOrigin,
     "RayQuery_CandidateObjectRayOrigin",
     OCC::RayQuery_StateVector,
     "rayQuery_StateVector",
     Attribute::ReadOnly,
     1,
     {{0x2}},
     {{0x0}}}, // Overloads: f
    {OC::RayQuery_CandidateObjectRayDirection,
     "RayQuery_CandidateObjectRayDirection",
     OCC::RayQuery_StateVector,
     "rayQuery_StateVector",
     Attribute::ReadOnly,
     1,
     {{0x2}},
     {{0x0}}}, // Overloads: f
    {OC::RayQuery_CommittedInstanceIndex,
     "RayQuery_CommittedInstanceIndex",
     OCC::RayQuery_StateScalar,
     "rayQuery_StateScalar",
     Attribute::ReadOnly,
     1,
     {{0x40}},
     {{0x0}}}, // Overloads: i
    {OC::RayQuery_CommittedInstanceID,
     "RayQuery_CommittedInstanceID",
     OCC::RayQuery_StateScalar,
     "rayQuery_StateScalar",
     Attribute::ReadOnly,
     1,
     {{0x40}},
     {{0x0}}}, // Overloads: i
    {OC::RayQuery_CommittedGeometryIndex,
     "RayQuery_CommittedGeometryIndex",
     OCC::RayQuery_StateScalar,
     "rayQuery_StateScalar",
     Attribute::ReadOnly,
     1,
     {{0x40}},
     {{0x0}}}, // Overloads: i
    {OC::RayQuery_CommittedPrimitiveIndex,
     "RayQuery_CommittedPrimitiveIndex",
     OCC::RayQuery_StateScalar,
     "rayQuery_StateScalar",
     Attribute::ReadOnly,
     1,
     {{0x40}},
     {{0x0}}}, // Overloads: i
    {OC::RayQuery_CommittedObjectRayOrigin,
     "RayQuery_CommittedObjectRayOrigin",
     OCC::RayQuery_StateVector,
     "rayQuery_StateVector",
     Attribute::ReadOnly,
     1,
     {{0x2}},
     {{0x0}}}, // Overloads: f
    {OC::RayQuery_CommittedObjectRayDirection,
     "RayQuery_CommittedObjectRayDirection",
     OCC::RayQuery_StateVector,
     "rayQuery_StateVector",
     Attribute::ReadOnly,
     1,
     {{0x2}},
     {{0x0}}}, // Overloads: f

    // Raytracing object space uint System Values, raytracing tier 1.1
    {OC::GeometryIndex,
     "GeometryIndex",
     OCC::GeometryIndex,
     "geometryIndex",
     Attribute::ReadNone,
     1,
     {{0x40}},
     {{0x0}}}, // Overloads: i

    // Inline Ray Query
    {OC::RayQuery_CandidateInstanceContributionToHitGroupIndex,
     "RayQuery_CandidateInstanceContributionToHitGroupIndex",
     OCC::RayQuery_StateScalar,
     "rayQuery_StateScalar",
     Attribute::ReadOnly,
     1,
     {{0x40}},
     {{0x0}}}, // Overloads: i
    {OC::RayQuery_CommittedInstanceContributionToHitGroupIndex,
     "RayQuery_CommittedInstanceContributionToHitGroupIndex",
     OCC::RayQuery_StateScalar,
     "rayQuery_StateScalar",
     Attribute::ReadOnly,
     1,
     {{0x40}},
     {{0x0}}}, // Overloads: i

    // Get handle from heap
    {OC::AnnotateHandle,
     "AnnotateHandle",
     OCC::AnnotateHandle,
     "annotateHandle",
     Attribute::ReadNone,
     0,
     {},
     {}}, // Overloads: v
    {OC::CreateHandleFromBinding,
     "CreateHandleFromBinding",
     OCC::CreateHandleFromBinding,
     "createHandleFromBinding",
     Attribute::ReadNone,
     0,
     {},
     {}}, // Overloads: v
    {OC::CreateHandleFromHeap,
     "CreateHandleFromHeap",
     OCC::CreateHandleFromHeap,
     "createHandleFromHeap",
     Attribute::ReadNone,
     0,
     {},
     {}}, // Overloads: v

    // Unpacking intrinsics
    {OC::Unpack4x8,
     "Unpack4x8",
     OCC::Unpack4x8,
     "unpack4x8",
     Attribute::ReadNone,
     1,
     {{0x60}},
     {{0x0}}}, // Overloads: iw

    // Packing intrinsics
    {OC::Pack4x8,
     "Pack4x8",
     OCC::Pack4x8,
     "pack4x8",
     Attribute::ReadNone,
     1,
     {{0x60}},
     {{0x0}}}, // Overloads: iw

    // Helper Lanes
    {OC::IsHelperLane,
     "IsHelperLane",
     OCC::IsHelperLane,
     "isHelperLane",
     Attribute::ReadOnly,
     1,
     {{0x8}},
     {{0x0}}}, // Overloads: 1

    // Quad Wave Ops
    {OC::QuadVote,
     "QuadVote",
     OCC::QuadVote,
     "quadVote",
     Attribute::None,
     1,
     {{0x8}},
     {{0x0}}}, // Overloads: 1

    // Resources - gather
    {OC::TextureGatherRaw,
     "TextureGatherRaw",
     OCC::TextureGatherRaw,
     "textureGatherRaw",
     Attribute::ReadOnly,
     1,
     {{0xe0}},
     {{0x0}}}, // Overloads: wil

    // Resources - sample
    {OC::SampleCmpLevel,
     "SampleCmpLevel",
     OCC::SampleCmpLevel,
     "sampleCmpLevel",
     Attribute::ReadOnly,
     1,
     {{0x3}},
     {{0x0}}}, // Overloads: hf

    // Resources
    {OC::TextureStoreSample,
     "TextureStoreSample",
     OCC::TextureStoreSample,
     "textureStoreSample",
     Attribute::None,
     1,
     {{0x63}},
     {{0x0}}}, // Overloads: hfwi

    {OC::Reserved0,
     "Reserved0",
     OCC::Reserved,
     "reserved",
     Attribute::None,
     0,
     {},
     {}}, // Overloads: v
    {OC::Reserved1,
     "Reserved1",
     OCC::Reserved,
     "reserved",
     Attribute::None,
     0,
     {},
     {}}, // Overloads: v
    {OC::Reserved2,
     "Reserved2",
     OCC::Reserved,
     "reserved",
     Attribute::None,
     0,
     {},
     {}}, // Overloads: v
    {OC::Reserved3,
     "Reserved3",
     OCC::Reserved,
     "reserved",
     Attribute::None,
     0,
     {},
     {}}, // Overloads: v
    {OC::Reserved4,
     "Reserved4",
     OCC::Reserved,
     "reserved",
     Attribute::None,
     0,
     {},
     {}}, // Overloads: v
    {OC::Reserved5,
     "Reserved5",
     OCC::Reserved,
     "reserved",
     Attribute::None,
     0,
     {},
     {}}, // Overloads: v
    {OC::Reserved6,
     "Reserved6",
     OCC::Reserved,
     "reserved",
     Attribute::None,
     0,
     {},
     {}}, // Overloads: v
    {OC::Reserved7,
     "Reserved7",
     OCC::Reserved,
     "reserved",
     Attribute::None,
     0,
     {},
     {}}, // Overloads: v
    {OC::Reserved8,
     "Reserved8",
     OCC::Reserved,
     "reserved",
     Attribute::None,
     0,
     {},
     {}}, // Overloads: v
    {OC::Reserved9,
     "Reserved9",
     OCC::Reserved,
     "reserved",
     Attribute::None,
     0,
     {},
     {}}, // Overloads: v
    {OC::Reserved10,
     "Reserved10",
     OCC::Reserved,
     "reserved",
     Attribute::None,
     0,
     {},
     {}}, // Overloads: v
    {OC::Reserved11,
     "Reserved11",
     OCC::Reserved,
     "reserved",
     Attribute::None,
     0,
     {},
     {}}, // Overloads: v

    // Create/Annotate Node Handles
    {OC::AllocateNodeOutputRecords,
     "AllocateNodeOutputRecords",
     OCC::AllocateNodeOutputRecords,
     "allocateNodeOutputRecords",
     Attribute::None,
     0,
     {},
     {}}, // Overloads: v

    // Get Pointer to Node Record in Address Space 6
    {OC::GetNodeRecordPtr,
     "GetNodeRecordPtr",
     OCC::GetNodeRecordPtr,
     "getNodeRecordPtr",
     Attribute::ReadNone,
     1,
     {{0x100}},
     {{0x0}}}, // Overloads: u

    // Work Graph intrinsics
    {OC::IncrementOutputCount,
     "IncrementOutputCount",
     OCC::IncrementOutputCount,
     "incrementOutputCount",
     Attribute::None,
     0,
     {},
     {}}, // Overloads: v
    {OC::OutputComplete,
     "OutputComplete",
     OCC::OutputComplete,
     "outputComplete",
     Attribute::None,
     0,
     {},
     {}}, // Overloads: v
    {OC::GetInputRecordCount,
     "GetInputRecordCount",
     OCC::GetInputRecordCount,
     "getInputRecordCount",
     Attribute::ReadOnly,
     0,
     {},
     {}}, // Overloads: v
    {OC::FinishedCrossGroupSharing,
     "FinishedCrossGroupSharing",
     OCC::FinishedCrossGroupSharing,
     "finishedCrossGroupSharing",
     Attribute::None,
     0,
     {},
     {}}, // Overloads: v

    // Synchronization
    {OC::BarrierByMemoryType,
     "BarrierByMemoryType",
     OCC::BarrierByMemoryType,
     "barrierByMemoryType",
     Attribute::NoDuplicate,
     0,
     {},
     {}}, // Overloads: v
    {OC::BarrierByMemoryHandle,
     "BarrierByMemoryHandle",
     OCC::BarrierByMemoryHandle,
     "barrierByMemoryHandle",
     Attribute::NoDuplicate,
     0,
     {},
     {}}, // Overloads: v
    {OC::BarrierByNodeRecordHandle,
     "BarrierByNodeRecordHandle",
     OCC::BarrierByNodeRecordHandle,
     "barrierByNodeRecordHandle",
     Attribute::NoDuplicate,
     0,
     {},
     {}}, // Overloads: v

    // Create/Annotate Node Handles
    {OC::CreateNodeOutputHandle,
     "CreateNodeOutputHandle",
     OCC::createNodeOutputHandle,
     "createNodeOutputHandle",
     Attribute::ReadNone,
     0,
     {},
     {}}, // Overloads: v
    {OC::IndexNodeHandle,
     "IndexNodeHandle",
     OCC::IndexNodeHandle,
     "indexNodeHandle",
     Attribute::ReadNone,
     0,
     {},
     {}}, // Overloads: v
    {OC::AnnotateNodeHandle,
     "AnnotateNodeHandle",
     OCC::AnnotateNodeHandle,
     "annotateNodeHandle",
     Attribute::ReadNone,
     0,
     {},
     {}}, // Overloads: v
    {OC::CreateNodeInputRecordHandle,
     "CreateNodeInputRecordHandle",
     OCC::CreateNodeInputRecordHandle,
     "createNodeInputRecordHandle",
     Attribute::ReadNone,
     0,
     {},
     {}}, // Overloads: v
    {OC::AnnotateNodeRecordHandle,
     "AnnotateNodeRecordHandle",
     OCC::AnnotateNodeRecordHandle,
     "annotateNodeRecordHandle",
     Attribute::ReadNone,
     0,
     {},
     {}}, // Overloads: v

    // Work Graph intrinsics
    {OC::NodeOutputIsValid,
     "NodeOutputIsValid",
     OCC::NodeOutputIsValid,
     "nodeOutputIsValid",
     Attribute::ReadOnly,
     0,
     {},
     {}}, // Overloads: v
    {OC::GetRemainingRecursionLevels,
     "GetRemainingRecursionLevels",
     OCC::GetRemainingRecursionLevels,
     "getRemainingRecursionLevels",
     Attribute::ReadOnly,
     0,
     {},
     {}}, // Overloads: v

    // Comparison Samples
    {OC::SampleCmpGrad,
     "SampleCmpGrad",
     OCC::SampleCmpGrad,
     "sampleCmpGrad",
     Attribute::ReadOnly,
     1,
     {{0x3}},
     {{0x0}}}, // Overloads: hf
    {OC::SampleCmpBias,
     "SampleCmpBias",
     OCC::SampleCmpBias,
     "sampleCmpBias",
     Attribute::ReadOnly,
     1,
     {{0x3}},
     {{0x0}}}, // Overloads: hf

    // Extended Command Information
    {OC::StartVertexLocation,
     "StartVertexLocation",
     OCC::StartVertexLocation,
     "startVertexLocation",
     Attribute::ReadNone,
     1,
     {{0x40}},
     {{0x0}}}, // Overloads: i
    {OC::StartInstanceLocation,
     "StartInstanceLocation",
     OCC::StartInstanceLocation,
     "startInstanceLocation",
     Attribute::ReadNone,
     1,
     {{0x40}},
     {{0x0}}}, // Overloads: i

    // Inline Ray Query
    {OC::AllocateRayQuery2,
     "AllocateRayQuery2",
     OCC::AllocateRayQuery2,
     "allocateRayQuery2",
     Attribute::None,
     0,
     {},
     {}}, // Overloads: v

    {OC::ReservedA0,
     "ReservedA0",
     OCC::Reserved,
     "reserved",
     Attribute::None,
     0,
     {},
     {}}, // Overloads: v
    {OC::ReservedA1,
     "ReservedA1",
     OCC::Reserved,
     "reserved",
     Attribute::None,
     0,
     {},
     {}}, // Overloads: v
    {OC::ReservedA2,
     "ReservedA2",
     OCC::Reserved,
     "reserved",
     Attribute::None,
     0,
     {},
     {}}, // Overloads: v

    // Shader Execution Reordering
    {OC::HitObject_TraceRay,
     "HitObject_TraceRay",
     OCC::HitObject_TraceRay,
     "hitObject_TraceRay",
     Attribute::None,
     1,
     {{0x100}},
     {{0x0}}}, // Overloads: u
    {OC::HitObject_FromRayQuery,
     "HitObject_FromRayQuery",
     OCC::HitObject_FromRayQuery,
     "hitObject_FromRayQuery",
     Attribute::ReadOnly,
     0,
     {},
     {}}, // Overloads: v
    {OC::HitObject_FromRayQueryWithAttrs,
     "HitObject_FromRayQueryWithAttrs",
     OCC::HitObject_FromRayQueryWithAttrs,
     "hitObject_FromRayQueryWithAttrs",
     Attribute::ReadOnly,
     1,
     {{0x100}},
     {{0x0}}}, // Overloads: u
    {OC::HitObject_MakeMiss,
     "HitObject_MakeMiss",
     OCC::HitObject_MakeMiss,
     "hitObject_MakeMiss",
     Attribute::ReadNone,
     0,
     {},
     {}}, // Overloads: v
    {OC::HitObject_MakeNop,
     "HitObject_MakeNop",
     OCC::HitObject_MakeNop,
     "hitObject_MakeNop",
     Attribute::ReadNone,
     0,
     {},
     {}}, // Overloads: v
    {OC::HitObject_Invoke,
     "HitObject_Invoke",
     OCC::HitObject_Invoke,
     "hitObject_Invoke",
     Attribute::None,
     1,
     {{0x100}},
     {{0x0}}}, // Overloads: u
    {OC::MaybeReorderThread,
     "MaybeReorderThread",
     OCC::MaybeReorderThread,
     "maybeReorderThread",
     Attribute::None,
     0,
     {},
     {}}, // Overloads: v
    {OC::HitObject_IsMiss,
     "HitObject_IsMiss",
     OCC::HitObject_StateScalar,
     "hitObject_StateScalar",
     Attribute::ReadNone,
     1,
     {{0x8}},
     {{0x0}}}, // Overloads: 1
    {OC::HitObject_IsHit,
     "HitObject_IsHit",
     OCC::HitObject_StateScalar,
     "hitObject_StateScalar",
     Attribute::ReadNone,
     1,
     {{0x8}},
     {{0x0}}}, // Overloads: 1
    {OC::HitObject_IsNop,
     "HitObject_IsNop",
     OCC::HitObject_StateScalar,
     "hitObject_StateScalar",
     Attribute::ReadNone,
     1,
     {{0x8}},
     {{0x0}}}, // Overloads: 1
    {OC::HitObject_RayFlags,
     "HitObject_RayFlags",
     OCC::HitObject_StateScalar,
     "hitObject_StateScalar",
     Attribute::ReadNone,
     1,
     {{0x40}},
     {{0x0}}}, // Overloads: i
    {OC::HitObject_RayTMin,
     "HitObject_RayTMin",
     OCC::HitObject_StateScalar,
     "hitObject_StateScalar",
     Attribute::ReadNone,
     1,
     {{0x2}},
     {{0x0}}}, // Overloads: f
    {OC::HitObject_RayTCurrent,
     "HitObject_RayTCurrent",
     OCC::HitObject_StateScalar,
     "hitObject_StateScalar",
     Attribute::ReadNone,
     1,
     {{0x2}},
     {{0x0}}}, // Overloads: f
    {OC::HitObject_WorldRayOrigin,
     "HitObject_WorldRayOrigin",
     OCC::HitObject_StateVector,
     "hitObject_StateVector",
     Attribute::ReadNone,
     1,
     {{0x2}},
     {{0x0}}}, // Overloads: f
    {OC::HitObject_WorldRayDirection,
     "HitObject_WorldRayDirection",
     OCC::HitObject_StateVector,
     "hitObject_StateVector",
     Attribute::ReadNone,
     1,
     {{0x2}},
     {{0x0}}}, // Overloads: f
    {OC::HitObject_ObjectRayOrigin,
     "HitObject_ObjectRayOrigin",
     OCC::HitObject_StateVector,
     "hitObject_StateVector",
     Attribute::ReadNone,
     1,
     {{0x2}},
     {{0x0}}}, // Overloads: f
    {OC::HitObject_ObjectRayDirection,
     "HitObject_ObjectRayDirection",
     OCC::HitObject_StateVector,
     "hitObject_StateVector",
     Attribute::ReadNone,
     1,
     {{0x2}},
     {{0x0}}}, // Overloads: f
    {OC::HitObject_ObjectToWorld3x4,
     "HitObject_ObjectToWorld3x4",
     OCC::HitObject_StateMatrix,
     "hitObject_StateMatrix",
     Attribute::ReadNone,
     1,
     {{0x2}},
     {{0x0}}}, // Overloads: f
    {OC::HitObject_WorldToObject3x4,
     "HitObject_WorldToObject3x4",
     OCC::HitObject_StateMatrix,
     "hitObject_StateMatrix",
     Attribute::ReadNone,
     1,
     {{0x2}},
     {{0x0}}}, // Overloads: f
    {OC::HitObject_GeometryIndex,
     "HitObject_GeometryIndex",
     OCC::HitObject_StateScalar,
     "hitObject_StateScalar",
     Attribute::ReadNone,
     1,
     {{0x40}},
     {{0x0}}}, // Overloads: i
    {OC::HitObject_InstanceIndex,
     "HitObject_InstanceIndex",
     OCC::HitObject_StateScalar,
     "hitObject_StateScalar",
     Attribute::ReadNone,
     1,
     {{0x40}},
     {{0x0}}}, // Overloads: i
    {OC::HitObject_InstanceID,
     "HitObject_InstanceID",
     OCC::HitObject_StateScalar,
     "hitObject_StateScalar",
     Attribute::ReadNone,
     1,
     {{0x40}},
     {{0x0}}}, // Overloads: i
    {OC::HitObject_PrimitiveIndex,
     "HitObject_PrimitiveIndex",
     OCC::HitObject_StateScalar,
     "hitObject_StateScalar",
     Attribute::ReadNone,
     1,
     {{0x40}},
     {{0x0}}}, // Overloads: i
    {OC::HitObject_HitKind,
     "HitObject_HitKind",
     OCC::HitObject_StateScalar,
     "hitObject_StateScalar",
     Attribute::ReadNone,
     1,
     {{0x40}},
     {{0x0}}}, // Overloads: i
    {OC::HitObject_ShaderTableIndex,
     "HitObject_ShaderTableIndex",
     OCC::HitObject_StateScalar,
     "hitObject_StateScalar",
     Attribute::ReadNone,
     1,
     {{0x40}},
     {{0x0}}}, // Overloads: i
    {OC::HitObject_SetShaderTableIndex,
     "HitObject_SetShaderTableIndex",
     OCC::HitObject_SetShaderTableIndex,
     "hitObject_SetShaderTableIndex",
     Attribute::ReadNone,
     0,
     {},
     {}}, // Overloads: v
    {OC::HitObject_LoadLocalRootTableConstant,
     "HitObject_LoadLocalRootTableConstant",
     OCC::HitObject_LoadLocalRootTableConstant,
     "hitObject_LoadLocalRootTableConstant",
     Attribute::ReadOnly,
     0,
     {},
     {}}, // Overloads: v
    {OC::HitObject_Attributes,
     "HitObject_Attributes",
     OCC::HitObject_Attributes,
     "hitObject_Attributes",
     Attribute::ArgMemOnly,
     1,
     {{0x100}},
     {{0x0}}}, // Overloads: u

    {OC::ReservedB28,
     "ReservedB28",
     OCC::Reserved,
     "reserved",
     Attribute::None,
     0,
     {},
     {}}, // Overloads: v
    {OC::ReservedB29,
     "ReservedB29",
     OCC::Reserved,
     "reserved",
     Attribute::None,
     0,
     {},
     {}}, // Overloads: v
    {OC::ReservedB30,
     "ReservedB30",
     OCC::Reserved,
     "reserved",
     Attribute::None,
     0,
     {},
     {}}, // Overloads: v
    {OC::ReservedC0,
     "ReservedC0",
     OCC::Reserved,
     "reserved",
     Attribute::None,
     0,
     {},
     {}}, // Overloads: v
    {OC::ReservedC1,
     "ReservedC1",
     OCC::Reserved,
     "reserved",
     Attribute::None,
     0,
     {},
     {}}, // Overloads: v
    {OC::ReservedC2,
     "ReservedC2",
     OCC::Reserved,
     "reserved",
     Attribute::None,
     0,
     {},
     {}}, // Overloads: v
    {OC::ReservedC3,
     "ReservedC3",
     OCC::Reserved,
     "reserved",
     Attribute::None,
     0,
     {},
     {}}, // Overloads: v
    {OC::ReservedC4,
     "ReservedC4",
     OCC::Reserved,
     "reserved",
     Attribute::None,
     0,
     {},
     {}}, // Overloads: v
    {OC::ReservedC5,
     "ReservedC5",
     OCC::Reserved,
     "reserved",
     Attribute::None,
     0,
     {},
     {}}, // Overloads: v
    {OC::ReservedC6,
     "ReservedC6",
     OCC::Reserved,
     "reserved",
     Attribute::None,
     0,
     {},
     {}}, // Overloads: v
    {OC::ReservedC7,
     "ReservedC7",
     OCC::Reserved,
     "reserved",
     Attribute::None,
     0,
     {},
     {}}, // Overloads: v
    {OC::ReservedC8,
     "ReservedC8",
     OCC::Reserved,
     "reserved",
     Attribute::None,
     0,
     {},
     {}}, // Overloads: v
    {OC::ReservedC9,
     "ReservedC9",
     OCC::Reserved,
     "reserved",
     Attribute::None,
     0,
     {},
     {}}, // Overloads: v

    // Resources
    {OC::RawBufferVectorLoad,
     "RawBufferVectorLoad",
     OCC::RawBufferVectorLoad,
     "rawBufferVectorLoad",
     Attribute::ReadOnly,
     1,
     {{0x4e7}},
     {{0xe7}}}, // Overloads: hfwidl<hfwidl
    {OC::RawBufferVectorStore,
     "RawBufferVectorStore",
     OCC::RawBufferVectorStore,
     "rawBufferVectorStore",
     Attribute::None,
     1,
     {{0x4e7}},
     {{0xe7}}}, // Overloads: hfwidl<hfwidl

    // Linear Algebra Operations
    {OC::MatVecMul,
     "MatVecMul",
     OCC::MatVecMul,
     "matVecMul",
     Attribute::ReadOnly,
     2,
     {{0x400}, {0x400}},
     {{0x63}, {0x63}}}, // Overloads: <hfwi,<hfwi
    {OC::MatVecMulAdd,
     "MatVecMulAdd",
     OCC::MatVecMulAdd,
     "matVecMulAdd",
     Attribute::ReadOnly,
     2,
     {{0x400}, {0x400}},
     {{0x63}, {0x63}}}, // Overloads: <hfwi,<hfwi
    {OC::OuterProductAccumulate,
     "OuterProductAccumulate",
     OCC::OuterProductAccumulate,
     "outerProductAccumulate",
     Attribute::None,
     2,
     {{0x400}, {0x400}},
     {{0x63}, {0x63}}}, // Overloads: <hfwi,<hfwi
    {OC::VectorAccumulate,
     "VectorAccumulate",
     OCC::VectorAccumulate,
     "vectorAccumulate",
     Attribute::None,
     1,
     {{0x400}},
     {{0x63}}}, // Overloads: <hfwi
};
// OPCODE-OLOADS:END

const char *OP::m_OverloadTypeName[TS_BasicCount] = {
    "f16", "f32", "f64", "i1", "i8", "i16", "i32", "i64"};

const char *OP::m_NamePrefix = "dx.op.";
const char *OP::m_TypePrefix = "dx.types.";
const char *OP::m_MatrixTypePrefix = "class.matrix."; // Allowed in library

// Keep sync with DXIL::AtomicBinOpCode
static const char *AtomicBinOpCodeName[] = {
    "AtomicAdd",    "AtomicAnd",  "AtomicOr",   "AtomicXor",      "AtomicIMin",
    "AtomicIMax",   "AtomicUMin", "AtomicUMax", "AtomicExchange",
    "AtomicInvalid" // Must be last.
};

unsigned OP::GetTypeSlot(Type *pType) {
  Type::TypeID T = pType->getTypeID();
  switch (T) {
  case Type::VoidTyID:
    return TS_Invalid;
  case Type::HalfTyID:
    return TS_F16;
  case Type::FloatTyID:
    return TS_F32;
  case Type::DoubleTyID:
    return TS_F64;
  case Type::IntegerTyID: {
    IntegerType *pIT = dyn_cast<IntegerType>(pType);
    unsigned Bits = pIT->getBitWidth();
    switch (Bits) {
    case 1:
      return TS_I1;
    case 8:
      return TS_I8;
    case 16:
      return TS_I16;
    case 32:
      return TS_I32;
    case 64:
      return TS_I64;
    }
    llvm_unreachable("Invalid Bits size");
    return TS_Invalid;
  }
  case Type::PointerTyID: {
    pType = cast<PointerType>(pType)->getElementType();
    if (pType->isStructTy())
      return TS_UDT;
    DXASSERT(!pType->isPointerTy(), "pointer-to-pointer type unsupported");
    return GetTypeSlot(pType);
  }
  case Type::StructTyID:
    // Named struct value (not pointer) indicates a built-in object type.
    // Anonymous struct value is used to wrap multi-overload dimensions.
    if (cast<StructType>(pType)->hasName())
      return TS_Object;
    else
      return TS_Extended;
  case Type::VectorTyID:
    return TS_Vector;
  default:
    break;
  }
  return TS_Invalid;
}

const char *OP::GetOverloadTypeName(unsigned TypeSlot) {
  DXASSERT(TypeSlot < TS_BasicCount, "otherwise caller passed OOB index");
  return m_OverloadTypeName[TypeSlot];
}

StringRef OP::GetTypeName(Type *Ty, SmallVectorImpl<char> &Storage) {
  DXASSERT(!Ty->isVoidTy(), "must not pass void type here");
  unsigned TypeSlot = OP::GetTypeSlot(Ty);
  if (TypeSlot < TS_BasicCount) {
    return GetOverloadTypeName(TypeSlot);
  } else if (TypeSlot == TS_UDT) {
    if (Ty->isPointerTy())
      Ty = Ty->getPointerElementType();
    StructType *ST = cast<StructType>(Ty);
    return ST->getStructName();
  } else if (TypeSlot == TS_Object) {
    StructType *ST = cast<StructType>(Ty);
    return ST->getStructName();
  } else if (TypeSlot == TS_Vector) {
    VectorType *VecTy = cast<VectorType>(Ty);
    return (Twine("v") + Twine(VecTy->getNumElements()) +
            Twine(
                GetOverloadTypeName(OP::GetTypeSlot(VecTy->getElementType()))))
        .toStringRef(Storage);
  } else if (TypeSlot == TS_Extended) {
    DXASSERT(isa<StructType>(Ty),
             "otherwise, extended overload type not wrapped in struct type.");
    StructType *ST = cast<StructType>(Ty);
    DXASSERT(ST->getNumElements() <= DXIL::kDxilMaxOloadDims,
             "otherwise, extended overload has too many dimensions.");
    // Iterate extended slots, recurse, separate with '.'
    raw_svector_ostream OS(Storage);
    for (unsigned I = 0; I < ST->getNumElements(); ++I) {
      if (I > 0)
        OS << ".";
      SmallVector<char, 32> TempStr;
      OS << GetTypeName(ST->getElementType(I), TempStr);
    }
    return OS.str();
  } else {
    raw_svector_ostream OS(Storage);
    Ty->print(OS);
    return OS.str();
  }
}

StringRef OP::ConstructOverloadName(Type *Ty, DXIL::OpCode opCode,
                                    SmallVectorImpl<char> &Storage) {
  if (Ty == Type::getVoidTy(Ty->getContext())) {
    return (Twine(OP::m_NamePrefix) + Twine(GetOpCodeClassName(opCode)))
        .toStringRef(Storage);
  } else {
    llvm::SmallVector<char, 64> TempStr;
    return (Twine(OP::m_NamePrefix) + Twine(GetOpCodeClassName(opCode)) + "." +
            GetTypeName(Ty, TempStr))
        .toStringRef(Storage);
  }
}

const char *OP::GetOpCodeName(OpCode opCode) {
  return m_OpCodeProps[(unsigned)opCode].pOpCodeName;
}

const char *OP::GetAtomicOpName(DXIL::AtomicBinOpCode OpCode) {
  unsigned opcode = static_cast<unsigned>(OpCode);
  DXASSERT_LOCALVAR(
      opcode, opcode < static_cast<unsigned>(DXIL::AtomicBinOpCode::Invalid),
      "otherwise caller passed OOB index");
  return AtomicBinOpCodeName[static_cast<unsigned>(OpCode)];
}

OP::OpCodeClass OP::GetOpCodeClass(OpCode opCode) {
  return m_OpCodeProps[(unsigned)opCode].opCodeClass;
}

const char *OP::GetOpCodeClassName(OpCode opCode) {
  return m_OpCodeProps[(unsigned)opCode].pOpCodeClassName;
}

llvm::Attribute::AttrKind OP::GetMemAccessAttr(OpCode opCode) {
  return m_OpCodeProps[(unsigned)opCode].FuncAttr;
}

bool OP::IsOverloadLegal(OpCode opCode, Type *pType) {
  if (static_cast<unsigned>(opCode) >=
      static_cast<unsigned>(OpCode::NumOpCodes))
    return false;
  if (!pType)
    return false;
  auto &OpProps = m_OpCodeProps[static_cast<unsigned>(opCode)];

  if (OpProps.NumOverloadDims == 0)
    return pType->isVoidTy();

  // Normalize 1+ overload dimensions into array.
  Type *Types[DXIL::kDxilMaxOloadDims] = {pType};
  if (OpProps.NumOverloadDims > 1) {
    StructType *ST = dyn_cast<StructType>(pType);
    // Make sure multi-overload is well-formed.
    if (!ST || ST->hasName() || ST->getNumElements() != OpProps.NumOverloadDims)
      return false;
    for (unsigned I = 0; I < ST->getNumElements(); ++I)
      Types[I] = ST->getElementType(I);
  }

  for (unsigned I = 0; I < OpProps.NumOverloadDims; ++I) {
    Type *Ty = Types[I];
    unsigned TypeSlot = GetTypeSlot(Ty);
    if (!OpProps.AllowedOverloads[I][TypeSlot])
      return false;
    if (TypeSlot == TS_Vector) {
      unsigned EltTypeSlot =
          GetTypeSlot(cast<VectorType>(Ty)->getElementType());
      if (!OpProps.AllowedVectorElements[I][EltTypeSlot])
        return false;
    }
  }

  return true;
}

bool OP::CheckOpCodeTable() {
  for (unsigned i = 0; i < (unsigned)OpCode::NumOpCodes; i++) {
    if ((unsigned)m_OpCodeProps[i].opCode != i)
      return false;
  }

  return true;
}

bool OP::IsDxilOpFuncName(StringRef name) {
  return name.startswith(OP::m_NamePrefix);
}

bool OP::IsDxilOpFunc(const llvm::Function *F) {
  // Test for null to allow IsDxilOpFunc(Call.getCalledFunc()) to be resilient
  // to indirect calls
  if (F == nullptr || !F->hasName())
    return false;
  return IsDxilOpFuncName(F->getName());
}

bool OP::IsDxilOpFuncCallInst(const llvm::Instruction *I) {
  const CallInst *CI = dyn_cast<CallInst>(I);
  if (CI == nullptr)
    return false;
  return IsDxilOpFunc(CI->getCalledFunction());
}

bool OP::IsDxilOpFuncCallInst(const llvm::Instruction *I, OpCode opcode) {
  if (!IsDxilOpFuncCallInst(I))
    return false;
  return (unsigned)getOpCode(I) == (unsigned)opcode;
}

OP::OpCode OP::getOpCode(const llvm::Instruction *I) {
  auto *OpConst = llvm::dyn_cast<llvm::ConstantInt>(I->getOperand(0));
  if (!OpConst)
    return OpCode::NumOpCodes;
  uint64_t OpCodeVal = OpConst->getZExtValue();
  if (OpCodeVal >= static_cast<uint64_t>(OP::OpCode::NumOpCodes))
    return OP::OpCode::NumOpCodes;
  return static_cast<OP::OpCode>(OpCodeVal);
}

OP::OpCode OP::GetDxilOpFuncCallInst(const llvm::Instruction *I) {
  DXASSERT(IsDxilOpFuncCallInst(I),
           "else caller didn't call IsDxilOpFuncCallInst to check");
  return getOpCode(I);
}

bool OP::IsDxilOpWave(OpCode C) {
  unsigned op = (unsigned)C;
  // clang-format off
  // Python lines need to be not formatted.
  /* <py::lines('OPCODE-WAVE')>hctdb_instrhelp.get_instrs_pred("op", "is_wave")</py>*/
  // clang-format on
  // OPCODE-WAVE:BEGIN
  // Instructions: WaveIsFirstLane=110, WaveGetLaneIndex=111,
  // WaveGetLaneCount=112, WaveAnyTrue=113, WaveAllTrue=114,
  // WaveActiveAllEqual=115, WaveActiveBallot=116, WaveReadLaneAt=117,
  // WaveReadLaneFirst=118, WaveActiveOp=119, WaveActiveBit=120,
  // WavePrefixOp=121, QuadReadLaneAt=122, QuadOp=123, WaveAllBitCount=135,
  // WavePrefixBitCount=136, WaveMatch=165, WaveMultiPrefixOp=166,
  // WaveMultiPrefixBitCount=167, QuadVote=222
  return (110 <= op && op <= 123) || (135 <= op && op <= 136) ||
         (165 <= op && op <= 167) || op == 222;
  // OPCODE-WAVE:END
}

bool OP::IsDxilOpGradient(OpCode C) {
  unsigned op = (unsigned)C;
  // clang-format off
  // Python lines need to be not formatted.
  /* <py::lines('OPCODE-GRADIENT')>hctdb_instrhelp.get_instrs_pred("op", "is_gradient")</py>*/
  // clang-format on
  // OPCODE-GRADIENT:BEGIN
  // Instructions: Sample=60, SampleBias=61, SampleCmp=64, CalculateLOD=81,
  // DerivCoarseX=83, DerivCoarseY=84, DerivFineX=85, DerivFineY=86,
  // WriteSamplerFeedback=174, WriteSamplerFeedbackBias=175, SampleCmpBias=255
  return (60 <= op && op <= 61) || op == 64 || op == 81 ||
         (83 <= op && op <= 86) || (174 <= op && op <= 175) || op == 255;
  // OPCODE-GRADIENT:END
}

bool OP::IsDxilOpFeedback(OpCode C) {
  unsigned op = (unsigned)C;
  // clang-format off
  // Python lines need to be not formatted.
  /* <py::lines('OPCODE-FEEDBACK')>hctdb_instrhelp.get_instrs_pred("op", "is_feedback")</py>*/
  // clang-format on
  // OPCODE-FEEDBACK:BEGIN
  // Instructions: WriteSamplerFeedback=174, WriteSamplerFeedbackBias=175,
  // WriteSamplerFeedbackLevel=176, WriteSamplerFeedbackGrad=177
  return (174 <= op && op <= 177);
  // OPCODE-FEEDBACK:END
}

bool OP::IsDxilOpBarrier(OpCode C) {
  unsigned op = (unsigned)C;
  // clang-format off
  // Python lines need to be not formatted.
  /* <py::lines('OPCODE-BARRIER')>hctdb_instrhelp.get_instrs_pred("op", "is_barrier")</py>*/
  // clang-format on
  // OPCODE-BARRIER:BEGIN
  // Instructions: Barrier=80, BarrierByMemoryType=244,
  // BarrierByMemoryHandle=245, BarrierByNodeRecordHandle=246
  return op == 80 || (244 <= op && op <= 246);
  // OPCODE-BARRIER:END
}

bool OP::IsDxilOpExtendedOverload(OpCode C) {
  if (C >= OpCode::NumOpCodes)
    return false;
  return m_OpCodeProps[static_cast<unsigned>(C)].NumOverloadDims > 1;
}

static unsigned MaskMemoryTypeFlagsIfAllowed(unsigned memoryTypeFlags,
                                             unsigned allowedMask) {
  // If the memory type is AllMemory, masking inapplicable flags is allowed.
  if (memoryTypeFlags != (unsigned)DXIL::MemoryTypeFlag::AllMemory)
    return memoryTypeFlags;
  return memoryTypeFlags & allowedMask;
}

bool OP::BarrierRequiresGroup(const llvm::CallInst *CI) {
  OpCode opcode = OP::GetDxilOpFuncCallInst(CI);
  switch (opcode) {
  case OpCode::Barrier: {
    DxilInst_Barrier barrier(const_cast<CallInst *>(CI));
    if (isa<ConstantInt>(barrier.get_barrierMode())) {
      unsigned mode = barrier.get_barrierMode_val();
      return (mode != (unsigned)DXIL::BarrierMode::UAVFenceGlobal);
    }
    return false;
  }
  case OpCode::BarrierByMemoryType: {
    DxilInst_BarrierByMemoryType barrier(const_cast<CallInst *>(CI));
    if (isa<ConstantInt>(barrier.get_MemoryTypeFlags())) {
      unsigned memoryTypeFlags = barrier.get_MemoryTypeFlags_val();
      memoryTypeFlags = MaskMemoryTypeFlagsIfAllowed(
          memoryTypeFlags, ~(unsigned)DXIL::MemoryTypeFlag::GroupFlags);
      if (memoryTypeFlags & (unsigned)DXIL::MemoryTypeFlag::GroupFlags)
        return true;
    }
  }
    LLVM_FALLTHROUGH;
  case OpCode::BarrierByMemoryHandle:
  case OpCode::BarrierByNodeRecordHandle: {
    // BarrierByMemoryType, BarrierByMemoryHandle, and BarrierByNodeRecordHandle
    // all have semanticFlags as the second operand.
    DxilInst_BarrierByMemoryType barrier(const_cast<CallInst *>(CI));
    if (isa<ConstantInt>(barrier.get_SemanticFlags())) {
      unsigned semanticFlags = barrier.get_SemanticFlags_val();
      if (semanticFlags & (unsigned)DXIL::BarrierSemanticFlag::GroupFlags)
        return true;
    }
    return false;
  }
  default:
    return false;
  }
}

bool OP::BarrierRequiresNode(const llvm::CallInst *CI) {
  OpCode opcode = OP::GetDxilOpFuncCallInst(CI);
  switch (opcode) {
  case OpCode::BarrierByNodeRecordHandle:
    return true;
  case OpCode::BarrierByMemoryType: {
    DxilInst_BarrierByMemoryType barrier(const_cast<CallInst *>(CI));
    if (isa<ConstantInt>(barrier.get_MemoryTypeFlags())) {
      unsigned memoryTypeFlags = barrier.get_MemoryTypeFlags_val();
      // Mask off node flags, if allowed.
      memoryTypeFlags = MaskMemoryTypeFlagsIfAllowed(
          memoryTypeFlags, ~(unsigned)DXIL::MemoryTypeFlag::NodeFlags);
      return (memoryTypeFlags & (unsigned)DXIL::MemoryTypeFlag::NodeFlags) != 0;
    }
    return false;
  }
  default:
    return false;
  }
}

bool OP::BarrierRequiresReorder(const llvm::CallInst *CI) {
  OpCode Opcode = OP::GetDxilOpFuncCallInst(CI);
  switch (Opcode) {
  case OpCode::BarrierByMemoryType: {
    DxilInst_BarrierByMemoryType Barrier(const_cast<CallInst *>(CI));
    if (!isa<ConstantInt>(Barrier.get_SemanticFlags()))
      return false;
    unsigned SemanticFlags = Barrier.get_SemanticFlags_val();
    return (SemanticFlags & static_cast<unsigned>(
                                DXIL::BarrierSemanticFlag::ReorderScope)) != 0U;
  }
  case OpCode::BarrierByMemoryHandle: {
    DxilInst_BarrierByMemoryHandle Barrier(const_cast<CallInst *>(CI));
    if (!isa<ConstantInt>(Barrier.get_SemanticFlags()))
      return false;
    unsigned SemanticFlags = Barrier.get_SemanticFlags_val();
    return (SemanticFlags & static_cast<unsigned>(
                                DXIL::BarrierSemanticFlag::ReorderScope)) != 0U;
  }
  default:
    return false;
  }
}

DXIL::BarrierMode OP::TranslateToBarrierMode(const llvm::CallInst *CI) {
  OpCode opcode = OP::GetDxilOpFuncCallInst(CI);
  switch (opcode) {
  case OpCode::Barrier: {
    DxilInst_Barrier barrier(const_cast<CallInst *>(CI));
    if (isa<ConstantInt>(barrier.get_barrierMode())) {
      unsigned mode = barrier.get_barrierMode_val();
      return static_cast<DXIL::BarrierMode>(mode);
    }
    return DXIL::BarrierMode::Invalid;
  }
  case OpCode::BarrierByMemoryType: {
    unsigned memoryTypeFlags = 0;
    unsigned semanticFlags = 0;
    DxilInst_BarrierByMemoryType barrier(const_cast<CallInst *>(CI));
    if (isa<ConstantInt>(barrier.get_MemoryTypeFlags())) {
      memoryTypeFlags = barrier.get_MemoryTypeFlags_val();
    }
    if (isa<ConstantInt>(barrier.get_SemanticFlags())) {
      semanticFlags = barrier.get_SemanticFlags_val();
    }

    // Disallow SM6.9+ semantic flags.
    if (semanticFlags &
        ~static_cast<unsigned>(DXIL::BarrierSemanticFlag::LegacyFlags)) {
      return DXIL::BarrierMode::Invalid;
    }

    // Mask to legacy flags, if allowed.
    memoryTypeFlags = MaskMemoryTypeFlagsIfAllowed(
        memoryTypeFlags, (unsigned)DXIL::MemoryTypeFlag::LegacyFlags);
    if (memoryTypeFlags & ~(unsigned)DXIL::MemoryTypeFlag::LegacyFlags)
      return DXIL::BarrierMode::Invalid;

    unsigned mode = 0;
    if (memoryTypeFlags & (unsigned)DXIL::MemoryTypeFlag::GroupSharedMemory)
      mode |= (unsigned)DXIL::BarrierMode::TGSMFence;
    if (memoryTypeFlags & (unsigned)DXIL::MemoryTypeFlag::UavMemory) {
      if (semanticFlags & (unsigned)DXIL::BarrierSemanticFlag::DeviceScope) {
        mode |= (unsigned)DXIL::BarrierMode::UAVFenceGlobal;
      } else if (semanticFlags &
                 (unsigned)DXIL::BarrierSemanticFlag::GroupScope) {
        mode |= (unsigned)DXIL::BarrierMode::UAVFenceThreadGroup;
      }
    }
    if (semanticFlags & (unsigned)DXIL::BarrierSemanticFlag::GroupSync)
      mode |= (unsigned)DXIL::BarrierMode::SyncThreadGroup;
    return static_cast<DXIL::BarrierMode>(mode);
  }
  default:
    return DXIL::BarrierMode::Invalid;
  }
}

#define SFLAG(stage) ((unsigned)1 << (unsigned)DXIL::ShaderKind::stage)
void OP::GetMinShaderModelAndMask(OpCode C, bool bWithTranslation,
                                  unsigned &major, unsigned &minor,
                                  unsigned &mask) {
  unsigned op = (unsigned)C;
  // Default is 6.0, all stages
  major = 6;
  minor = 0;
  mask = ((unsigned)1 << (unsigned)DXIL::ShaderKind::Invalid) - 1;
  // clang-format off
  // Python lines need to be not formatted.
  /* <py::lines('OPCODE-SMMASK')>hctdb_instrhelp.get_min_sm_and_mask_text()</py>*/
  // clang-format on
  // OPCODE-SMMASK:BEGIN
  // Instructions: ThreadId=93, GroupId=94, ThreadIdInGroup=95,
  // FlattenedThreadIdInGroup=96
  if ((93 <= op && op <= 96)) {
    mask = SFLAG(Compute) | SFLAG(Mesh) | SFLAG(Amplification) | SFLAG(Node);
    return;
  }
  // Instructions: DomainLocation=105
  if (op == 105) {
    mask = SFLAG(Domain);
    return;
  }
  // Instructions: LoadOutputControlPoint=103, LoadPatchConstant=104
  if ((103 <= op && op <= 104)) {
    mask = SFLAG(Domain) | SFLAG(Hull);
    return;
  }
  // Instructions: EmitStream=97, CutStream=98, EmitThenCutStream=99,
  // GSInstanceID=100
  if ((97 <= op && op <= 100)) {
    mask = SFLAG(Geometry);
    return;
  }
  // Instructions: PrimitiveID=108
  if (op == 108) {
    mask = SFLAG(Geometry) | SFLAG(Domain) | SFLAG(Hull);
    return;
  }
  // Instructions: StorePatchConstant=106, OutputControlPointID=107
  if ((106 <= op && op <= 107)) {
    mask = SFLAG(Hull);
    return;
  }
  // Instructions: QuadReadLaneAt=122, QuadOp=123
  if ((122 <= op && op <= 123)) {
    mask = SFLAG(Library) | SFLAG(Compute) | SFLAG(Amplification) |
           SFLAG(Mesh) | SFLAG(Pixel) | SFLAG(Node);
    return;
  }
  // Instructions: WaveIsFirstLane=110, WaveGetLaneIndex=111,
  // WaveGetLaneCount=112, WaveAnyTrue=113, WaveAllTrue=114,
  // WaveActiveAllEqual=115, WaveActiveBallot=116, WaveReadLaneAt=117,
  // WaveReadLaneFirst=118, WaveActiveOp=119, WaveActiveBit=120,
  // WavePrefixOp=121, WaveAllBitCount=135, WavePrefixBitCount=136
  if ((110 <= op && op <= 121) || (135 <= op && op <= 136)) {
    mask = SFLAG(Library) | SFLAG(Compute) | SFLAG(Amplification) |
           SFLAG(Mesh) | SFLAG(Pixel) | SFLAG(Vertex) | SFLAG(Hull) |
           SFLAG(Domain) | SFLAG(Geometry) | SFLAG(RayGeneration) |
           SFLAG(Intersection) | SFLAG(AnyHit) | SFLAG(ClosestHit) |
           SFLAG(Miss) | SFLAG(Callable) | SFLAG(Node);
    return;
  }
  // Instructions: Sample=60, SampleBias=61, SampleCmp=64, CalculateLOD=81,
  // DerivCoarseX=83, DerivCoarseY=84, DerivFineX=85, DerivFineY=86
  if ((60 <= op && op <= 61) || op == 64 || op == 81 ||
      (83 <= op && op <= 86)) {
    mask = SFLAG(Library) | SFLAG(Pixel) | SFLAG(Compute) |
           SFLAG(Amplification) | SFLAG(Mesh) | SFLAG(Node);
    return;
  }
  // Instructions: RenderTargetGetSamplePosition=76,
  // RenderTargetGetSampleCount=77, Discard=82, EvalSnapped=87,
  // EvalSampleIndex=88, EvalCentroid=89, SampleIndex=90, Coverage=91,
  // InnerCoverage=92
  if ((76 <= op && op <= 77) || op == 82 || (87 <= op && op <= 92)) {
    mask = SFLAG(Pixel);
    return;
  }
  // Instructions: AttributeAtVertex=137
  if (op == 137) {
    major = 6;
    minor = 1;
    mask = SFLAG(Pixel);
    return;
  }
  // Instructions: ViewID=138
  if (op == 138) {
    major = 6;
    minor = 1;
    mask = SFLAG(Vertex) | SFLAG(Hull) | SFLAG(Domain) | SFLAG(Geometry) |
           SFLAG(Pixel) | SFLAG(Mesh);
    return;
  }
  // Instructions: RawBufferLoad=139, RawBufferStore=140
  if ((139 <= op && op <= 140)) {
    if (bWithTranslation) {
      major = 6;
      minor = 0;
    } else {
      major = 6;
      minor = 2;
    }
    return;
  }
  // Instructions: IgnoreHit=155, AcceptHitAndEndSearch=156
  if ((155 <= op && op <= 156)) {
    major = 6;
    minor = 3;
    mask = SFLAG(AnyHit);
    return;
  }
  // Instructions: CallShader=159
  if (op == 159) {
    major = 6;
    minor = 3;
    mask = SFLAG(Library) | SFLAG(ClosestHit) | SFLAG(RayGeneration) |
           SFLAG(Miss) | SFLAG(Callable);
    return;
  }
  // Instructions: ReportHit=158
  if (op == 158) {
    major = 6;
    minor = 3;
    mask = SFLAG(Library) | SFLAG(Intersection);
    return;
  }
  // Instructions: InstanceID=141, InstanceIndex=142, HitKind=143,
  // ObjectRayOrigin=149, ObjectRayDirection=150, ObjectToWorld=151,
  // WorldToObject=152, PrimitiveIndex=161
  if ((141 <= op && op <= 143) || (149 <= op && op <= 152) || op == 161) {
    major = 6;
    minor = 3;
    mask = SFLAG(Library) | SFLAG(Intersection) | SFLAG(AnyHit) |
           SFLAG(ClosestHit);
    return;
  }
  // Instructions: RayFlags=144, WorldRayOrigin=147, WorldRayDirection=148,
  // RayTMin=153, RayTCurrent=154
  if (op == 144 || (147 <= op && op <= 148) || (153 <= op && op <= 154)) {
    major = 6;
    minor = 3;
    mask = SFLAG(Library) | SFLAG(Intersection) | SFLAG(AnyHit) |
           SFLAG(ClosestHit) | SFLAG(Miss);
    return;
  }
  // Instructions: TraceRay=157
  if (op == 157) {
    major = 6;
    minor = 3;
    mask =
        SFLAG(Library) | SFLAG(RayGeneration) | SFLAG(ClosestHit) | SFLAG(Miss);
    return;
  }
  // Instructions: DispatchRaysIndex=145, DispatchRaysDimensions=146
  if ((145 <= op && op <= 146)) {
    major = 6;
    minor = 3;
    mask = SFLAG(Library) | SFLAG(RayGeneration) | SFLAG(Intersection) |
           SFLAG(AnyHit) | SFLAG(ClosestHit) | SFLAG(Miss) | SFLAG(Callable);
    return;
  }
  // Instructions: CreateHandleForLib=160
  if (op == 160) {
    if (bWithTranslation) {
      major = 6;
      minor = 0;
    } else {
      major = 6;
      minor = 3;
    }
    return;
  }
  // Instructions: Dot2AddHalf=162, Dot4AddI8Packed=163, Dot4AddU8Packed=164
  if ((162 <= op && op <= 164)) {
    major = 6;
    minor = 4;
    return;
  }
  // Instructions: WriteSamplerFeedbackLevel=176, WriteSamplerFeedbackGrad=177,
  // AllocateRayQuery=178, RayQuery_TraceRayInline=179, RayQuery_Proceed=180,
  // RayQuery_Abort=181, RayQuery_CommitNonOpaqueTriangleHit=182,
  // RayQuery_CommitProceduralPrimitiveHit=183, RayQuery_CommittedStatus=184,
  // RayQuery_CandidateType=185, RayQuery_CandidateObjectToWorld3x4=186,
  // RayQuery_CandidateWorldToObject3x4=187,
  // RayQuery_CommittedObjectToWorld3x4=188,
  // RayQuery_CommittedWorldToObject3x4=189,
  // RayQuery_CandidateProceduralPrimitiveNonOpaque=190,
  // RayQuery_CandidateTriangleFrontFace=191,
  // RayQuery_CommittedTriangleFrontFace=192,
  // RayQuery_CandidateTriangleBarycentrics=193,
  // RayQuery_CommittedTriangleBarycentrics=194, RayQuery_RayFlags=195,
  // RayQuery_WorldRayOrigin=196, RayQuery_WorldRayDirection=197,
  // RayQuery_RayTMin=198, RayQuery_CandidateTriangleRayT=199,
  // RayQuery_CommittedRayT=200, RayQuery_CandidateInstanceIndex=201,
  // RayQuery_CandidateInstanceID=202, RayQuery_CandidateGeometryIndex=203,
  // RayQuery_CandidatePrimitiveIndex=204,
  // RayQuery_CandidateObjectRayOrigin=205,
  // RayQuery_CandidateObjectRayDirection=206,
  // RayQuery_CommittedInstanceIndex=207, RayQuery_CommittedInstanceID=208,
  // RayQuery_CommittedGeometryIndex=209, RayQuery_CommittedPrimitiveIndex=210,
  // RayQuery_CommittedObjectRayOrigin=211,
  // RayQuery_CommittedObjectRayDirection=212,
  // RayQuery_CandidateInstanceContributionToHitGroupIndex=214,
  // RayQuery_CommittedInstanceContributionToHitGroupIndex=215
  if ((176 <= op && op <= 212) || (214 <= op && op <= 215)) {
    major = 6;
    minor = 5;
    return;
  }
  // Instructions: DispatchMesh=173
  if (op == 173) {
    major = 6;
    minor = 5;
    mask = SFLAG(Amplification);
    return;
  }
  // Instructions: WaveMatch=165, WaveMultiPrefixOp=166,
  // WaveMultiPrefixBitCount=167
  if ((165 <= op && op <= 167)) {
    major = 6;
    minor = 5;
    mask = SFLAG(Library) | SFLAG(Compute) | SFLAG(Amplification) |
           SFLAG(Mesh) | SFLAG(Pixel) | SFLAG(Vertex) | SFLAG(Hull) |
           SFLAG(Domain) | SFLAG(Geometry) | SFLAG(RayGeneration) |
           SFLAG(Intersection) | SFLAG(AnyHit) | SFLAG(ClosestHit) |
           SFLAG(Miss) | SFLAG(Callable) | SFLAG(Node);
    return;
  }
  // Instructions: GeometryIndex=213
  if (op == 213) {
    major = 6;
    minor = 5;
    mask = SFLAG(Library) | SFLAG(Intersection) | SFLAG(AnyHit) |
           SFLAG(ClosestHit);
    return;
  }
  // Instructions: WriteSamplerFeedback=174, WriteSamplerFeedbackBias=175
  if ((174 <= op && op <= 175)) {
    major = 6;
    minor = 5;
    mask = SFLAG(Library) | SFLAG(Pixel);
    return;
  }
  // Instructions: SetMeshOutputCounts=168, EmitIndices=169, GetMeshPayload=170,
  // StoreVertexOutput=171, StorePrimitiveOutput=172
  if ((168 <= op && op <= 172)) {
    major = 6;
    minor = 5;
    mask = SFLAG(Mesh);
    return;
  }
  // Instructions: CreateHandleFromHeap=218, Unpack4x8=219, Pack4x8=220,
  // IsHelperLane=221
  if ((218 <= op && op <= 221)) {
    major = 6;
    minor = 6;
    return;
  }
  // Instructions: AnnotateHandle=216, CreateHandleFromBinding=217
  if ((216 <= op && op <= 217)) {
    if (bWithTranslation) {
      major = 6;
      minor = 0;
    } else {
      major = 6;
      minor = 6;
    }
    return;
  }
  // Instructions: TextureGatherRaw=223, SampleCmpLevel=224,
  // TextureStoreSample=225
  if ((223 <= op && op <= 225)) {
    major = 6;
    minor = 7;
    return;
  }
  // Instructions: QuadVote=222
  if (op == 222) {
    if (bWithTranslation) {
      major = 6;
      minor = 0;
    } else {
      major = 6;
      minor = 7;
    }
    mask = SFLAG(Library) | SFLAG(Compute) | SFLAG(Amplification) |
           SFLAG(Mesh) | SFLAG(Pixel) | SFLAG(Node);
    return;
  }
  // Instructions: BarrierByMemoryHandle=245, SampleCmpGrad=254
  if (op == 245 || op == 254) {
    major = 6;
    minor = 8;
    return;
  }
  // Instructions: SampleCmpBias=255
  if (op == 255) {
    major = 6;
    minor = 8;
    mask = SFLAG(Library) | SFLAG(Pixel) | SFLAG(Compute) |
           SFLAG(Amplification) | SFLAG(Mesh) | SFLAG(Node);
    return;
  }
  // Instructions: AllocateNodeOutputRecords=238, GetNodeRecordPtr=239,
  // IncrementOutputCount=240, OutputComplete=241, GetInputRecordCount=242,
  // FinishedCrossGroupSharing=243, BarrierByNodeRecordHandle=246,
  // CreateNodeOutputHandle=247, IndexNodeHandle=248, AnnotateNodeHandle=249,
  // CreateNodeInputRecordHandle=250, AnnotateNodeRecordHandle=251,
  // NodeOutputIsValid=252, GetRemainingRecursionLevels=253
  if ((238 <= op && op <= 243) || (246 <= op && op <= 253)) {
    major = 6;
    minor = 8;
    mask = SFLAG(Node);
    return;
  }
  // Instructions: StartVertexLocation=256, StartInstanceLocation=257
  if ((256 <= op && op <= 257)) {
    major = 6;
    minor = 8;
    mask = SFLAG(Vertex);
    return;
  }
  // Instructions: BarrierByMemoryType=244
  if (op == 244) {
    if (bWithTranslation) {
      major = 6;
      minor = 0;
    } else {
      major = 6;
      minor = 8;
    }
    return;
  }
  // Instructions: AllocateRayQuery2=258, RawBufferVectorLoad=303,
  // RawBufferVectorStore=304, MatVecMul=305, MatVecMulAdd=306,
  // OuterProductAccumulate=307, VectorAccumulate=308
  if (op == 258 || (303 <= op && op <= 308)) {
    major = 6;
    minor = 9;
    return;
  }
  // Instructions: MaybeReorderThread=268
  if (op == 268) {
    major = 6;
    minor = 9;
    mask = SFLAG(Library) | SFLAG(RayGeneration);
    return;
  }
  // Instructions: HitObject_TraceRay=262, HitObject_FromRayQuery=263,
  // HitObject_FromRayQueryWithAttrs=264, HitObject_MakeMiss=265,
  // HitObject_MakeNop=266, HitObject_Invoke=267, HitObject_IsMiss=269,
  // HitObject_IsHit=270, HitObject_IsNop=271, HitObject_RayFlags=272,
  // HitObject_RayTMin=273, HitObject_RayTCurrent=274,
  // HitObject_WorldRayOrigin=275, HitObject_WorldRayDirection=276,
  // HitObject_ObjectRayOrigin=277, HitObject_ObjectRayDirection=278,
  // HitObject_ObjectToWorld3x4=279, HitObject_WorldToObject3x4=280,
  // HitObject_GeometryIndex=281, HitObject_InstanceIndex=282,
  // HitObject_InstanceID=283, HitObject_PrimitiveIndex=284,
  // HitObject_HitKind=285, HitObject_ShaderTableIndex=286,
  // HitObject_SetShaderTableIndex=287,
  // HitObject_LoadLocalRootTableConstant=288, HitObject_Attributes=289
  if ((262 <= op && op <= 267) || (269 <= op && op <= 289)) {
    major = 6;
    minor = 9;
    mask =
        SFLAG(Library) | SFLAG(RayGeneration) | SFLAG(ClosestHit) | SFLAG(Miss);
    return;
  }
  // OPCODE-SMMASK:END
}

void OP::GetMinShaderModelAndMask(const llvm::CallInst *CI,
                                  bool bWithTranslation, unsigned valMajor,
                                  unsigned valMinor, unsigned &major,
                                  unsigned &minor, unsigned &mask) {
  OpCode opcode = OP::GetDxilOpFuncCallInst(CI);
  GetMinShaderModelAndMask(opcode, bWithTranslation, major, minor, mask);

  unsigned op = (unsigned)opcode;
  if (DXIL::CompareVersions(valMajor, valMinor, 1, 8) < 0) {
    // In prior validator versions, these ops excluded CS/MS/AS from mask.
    // In 1.8, we now have a mechanism to indicate derivative use with an
    // independent feature bit.  This allows us to fix up the min shader model
    // once all bits have been marged from the call graph to the entry point.
    // Instructions: Sample=60, SampleBias=61, SampleCmp=64, CalculateLOD=81,
    // DerivCoarseX=83, DerivCoarseY=84, DerivFineX=85, DerivFineY=86
    if ((60 <= op && op <= 61) || op == 64 || op == 81 ||
        (83 <= op && op <= 86)) {
      mask &= ~(SFLAG(Compute) | SFLAG(Amplification) | SFLAG(Mesh));
      return;
    }
  }

  if (DXIL::CompareVersions(valMajor, valMinor, 1, 5) < 0) {
    // validator 1.4 didn't exclude wave ops in mask
    if (IsDxilOpWave(opcode))
      mask = ((unsigned)1 << (unsigned)DXIL::ShaderKind::Mesh) - 1;
    // validator 1.4 didn't have any additional rules applied:
    return;
  }

  // Additional rules are applied manually here.

  // Barrier requiring node or group limit shader kinds.
  if (IsDxilOpBarrier(opcode)) {
    // If BarrierByMemoryType, check if translatable, or set min to 6.8.
    if (bWithTranslation && opcode == DXIL::OpCode::BarrierByMemoryType) {
      if (TranslateToBarrierMode(CI) == DXIL::BarrierMode::Invalid) {
        major = 6;
        minor = 8;
      }
    }
    if (BarrierRequiresReorder(CI)) {
      major = 6;
      minor = 9;
      mask &= SFLAG(Library) | SFLAG(RayGeneration);
      return;
    }
    if (BarrierRequiresNode(CI)) {
      mask &= SFLAG(Library) | SFLAG(Node);
      return;
    }
    if (BarrierRequiresGroup(CI)) {
      mask &= SFLAG(Library) | SFLAG(Compute) | SFLAG(Amplification) |
              SFLAG(Mesh) | SFLAG(Node);
      return;
    }
  }

  // 64-bit integer atomic ops require 6.6
  else if (opcode == DXIL::OpCode::AtomicBinOp ||
           opcode == DXIL::OpCode::AtomicCompareExchange) {
    Type *pOverloadType = GetOverloadType(opcode, CI->getCalledFunction());
    if (pOverloadType->isIntegerTy(64)) {
      major = 6;
      minor = 6;
    }
  }

  // AnnotateHandle and CreateHandleFromBinding can be translated down to
  // SM 6.0, but this wasn't set properly in validator version 6.6, so make it
  // match when using that version.
  else if (bWithTranslation &&
           DXIL::CompareVersions(valMajor, valMinor, 1, 6) == 0 &&
           (opcode == DXIL::OpCode::AnnotateHandle ||
            opcode == DXIL::OpCode::CreateHandleFromBinding)) {
    major = 6;
    minor = 6;
  }
}
#undef SFLAG

static Type *GetOrCreateStructType(LLVMContext &Ctx, ArrayRef<Type *> types,
                                   StringRef Name, Module *pModule) {
  if (StructType *ST = pModule->getTypeByName(Name)) {
    // TODO: validate the exist type match types if needed.
    return ST;
  } else
    return StructType::create(Ctx, types, Name);
}

//------------------------------------------------------------------------------
//
//  OP methods.
//
OP::OP(LLVMContext &Ctx, Module *pModule)
    : m_Ctx(Ctx), m_pModule(pModule),
      m_LowPrecisionMode(DXIL::LowPrecisionMode::Undefined) {
  memset(m_pResRetType, 0, sizeof(m_pResRetType));
  memset(m_pCBufferRetType, 0, sizeof(m_pCBufferRetType));
  memset(m_OpCodeClassCache, 0, sizeof(m_OpCodeClassCache));
  static_assert(_countof(OP::m_OpCodeProps) == (size_t)OP::OpCode::NumOpCodes,
                "forgot to update OP::m_OpCodeProps");

  m_pHandleType = GetOrCreateStructType(m_Ctx, Type::getInt8PtrTy(m_Ctx),
                                        "dx.types.Handle", pModule);
  m_pHitObjectType = GetOrCreateStructType(m_Ctx, Type::getInt8PtrTy(m_Ctx),
                                           "dx.types.HitObject", pModule);
  m_pNodeHandleType = GetOrCreateStructType(m_Ctx, Type::getInt8PtrTy(m_Ctx),
                                            "dx.types.NodeHandle", pModule);
  m_pNodeRecordHandleType = GetOrCreateStructType(
      m_Ctx, Type::getInt8PtrTy(m_Ctx), "dx.types.NodeRecordHandle", pModule);
  m_pResourcePropertiesType = GetOrCreateStructType(
      m_Ctx, {Type::getInt32Ty(m_Ctx), Type::getInt32Ty(m_Ctx)},
      "dx.types.ResourceProperties", pModule);
  m_pNodePropertiesType = GetOrCreateStructType(
      m_Ctx, {Type::getInt32Ty(m_Ctx), Type::getInt32Ty(m_Ctx)},
      "dx.types.NodeInfo", pModule);
  m_pNodeRecordPropertiesType = GetOrCreateStructType(
      m_Ctx, {Type::getInt32Ty(m_Ctx), Type::getInt32Ty(m_Ctx)},
      "dx.types.NodeRecordInfo", pModule);

  m_pResourceBindingType =
      GetOrCreateStructType(m_Ctx,
                            {Type::getInt32Ty(m_Ctx), Type::getInt32Ty(m_Ctx),
                             Type::getInt32Ty(m_Ctx), Type::getInt8Ty(m_Ctx)},
                            "dx.types.ResBind", pModule);

  Type *DimsType[4] = {Type::getInt32Ty(m_Ctx), Type::getInt32Ty(m_Ctx),
                       Type::getInt32Ty(m_Ctx), Type::getInt32Ty(m_Ctx)};
  m_pDimensionsType =
      GetOrCreateStructType(m_Ctx, DimsType, "dx.types.Dimensions", pModule);

  Type *SamplePosType[2] = {Type::getFloatTy(m_Ctx), Type::getFloatTy(m_Ctx)};
  m_pSamplePosType = GetOrCreateStructType(m_Ctx, SamplePosType,
                                           "dx.types.SamplePos", pModule);

  Type *I32cTypes[2] = {Type::getInt32Ty(m_Ctx), Type::getInt1Ty(m_Ctx)};
  m_pBinaryWithCarryType =
      GetOrCreateStructType(m_Ctx, I32cTypes, "dx.types.i32c", pModule);

  Type *TwoI32Types[2] = {Type::getInt32Ty(m_Ctx), Type::getInt32Ty(m_Ctx)};
  m_pBinaryWithTwoOutputsType =
      GetOrCreateStructType(m_Ctx, TwoI32Types, "dx.types.twoi32", pModule);

  Type *SplitDoubleTypes[2] = {Type::getInt32Ty(m_Ctx),
                               Type::getInt32Ty(m_Ctx)}; // Lo, Hi.
  m_pSplitDoubleType = GetOrCreateStructType(m_Ctx, SplitDoubleTypes,
                                             "dx.types.splitdouble", pModule);

  Type *FourI32Types[4] = {Type::getInt32Ty(m_Ctx), Type::getInt32Ty(m_Ctx),
                           Type::getInt32Ty(m_Ctx),
                           Type::getInt32Ty(m_Ctx)}; // HiHi, HiLo, LoHi, LoLo
  m_pFourI32Type =
      GetOrCreateStructType(m_Ctx, FourI32Types, "dx.types.fouri32", pModule);

  Type *FourI16Types[4] = {Type::getInt16Ty(m_Ctx), Type::getInt16Ty(m_Ctx),
                           Type::getInt16Ty(m_Ctx),
                           Type::getInt16Ty(m_Ctx)}; // HiHi, HiLo, LoHi, LoLo
  m_pFourI16Type =
      GetOrCreateStructType(m_Ctx, FourI16Types, "dx.types.fouri16", pModule);
}

void OP::RefreshCache() {
  for (Function &F : m_pModule->functions()) {
    if (OP::IsDxilOpFunc(&F) && !F.user_empty()) {
      CallInst *CI = cast<CallInst>(*F.user_begin());
      OpCode OpCode = OP::GetDxilOpFuncCallInst(CI);
      Type *pOverloadType = OP::GetOverloadType(OpCode, &F);
      GetOpFunc(OpCode, pOverloadType);
    }
  }
}

void OP::FixOverloadNames() {
  // When merging code from multiple sources, such as with linking,
  // type names that collide, but don't have the same type will be
  // automically renamed with .0+ name disambiguation.  However,
  // DXIL intrinsic overloads will not be renamed to disambiguate them,
  // since they exist in separate modules at the time.
  // This leads to name collisions between different types when linking.
  // Do this after loading into a shared context, and before copying
  // code into a common module, to prevent this problem.
  for (Function &F : m_pModule->functions()) {
    if (F.isDeclaration() && OP::IsDxilOpFunc(&F) && !F.user_empty()) {
      CallInst *CI = cast<CallInst>(*F.user_begin());
      DXIL::OpCode opCode = OP::GetDxilOpFuncCallInst(CI);
      if (!MayHaveNonCanonicalOverload(opCode))
        continue;
      llvm::Type *Ty = OP::GetOverloadType(opCode, &F);
      if (!OP::IsOverloadLegal(opCode, Ty))
        continue;
      SmallVector<char, 256> funcName;
      if (OP::ConstructOverloadName(Ty, opCode, funcName)
              .compare(F.getName()) != 0)
        F.setName(funcName);
    }
  }
}

void OP::UpdateCache(OpCodeClass opClass, Type *Ty, llvm::Function *F) {
  m_OpCodeClassCache[(unsigned)opClass].pOverloads[Ty] = F;
  m_FunctionToOpClass[F] = opClass;
}

bool OP::MayHaveNonCanonicalOverload(OpCode OC) {
  if (OC >= OpCode::NumOpCodes)
    return false;
  const unsigned CheckMask = (1 << TS_UDT) | (1 << TS_Object);
  auto &OpProps = m_OpCodeProps[static_cast<unsigned>(OC)];
  for (unsigned I = 0; I < OpProps.NumOverloadDims; ++I)
    if ((CheckMask & OpProps.AllowedOverloads[I].SlotMask) != 0)
      return true;
  return false;
}

Function *OP::GetOpFunc(OpCode OC, ArrayRef<Type *> OverloadTypes) {
  if (OC >= OpCode::NumOpCodes)
    return nullptr;
  if (OverloadTypes.size() !=
      m_OpCodeProps[static_cast<unsigned>(OC)].NumOverloadDims) {
    llvm_unreachable("incorrect overload dimensions");
    return nullptr;
  }
  if (OverloadTypes.size() == 0) {
    return GetOpFunc(OC, Type::getVoidTy(m_Ctx));
  } else if (OverloadTypes.size() == 1) {
    return GetOpFunc(OC, OverloadTypes[0]);
  }
  return GetOpFunc(OC, GetExtendedOverloadType(OverloadTypes));
}

Function *OP::GetOpFunc(OpCode opCode, Type *pOverloadType) {
  if (opCode >= OpCode::NumOpCodes)
    return nullptr;
  if (!pOverloadType)
    return nullptr;

  auto &OpProps = m_OpCodeProps[static_cast<unsigned>(opCode)];
  if (IsDxilOpExtendedOverload(opCode)) {
    // Make sure pOverloadType is well formed for an extended overload.
    StructType *ST = dyn_cast<StructType>(pOverloadType);
    DXASSERT(ST != nullptr,
             "otherwise, extended overload type is not a struct");
    if (ST == nullptr)
      return nullptr;
    bool EltCountValid = ST->getNumElements() == OpProps.NumOverloadDims;
    DXASSERT(EltCountValid,
             "otherwise, incorrect type count for extended overload.");
    if (!EltCountValid)
      return nullptr;
  }

  // Illegal overloads are generated and eliminated by DXIL op constant
  // evaluation for a number of cases where a double overload of an HL intrinsic
  // that otherwise does not support double is used for literal values, when
  // there is no constant evaluation for the intrinsic in CodeGen.
  // Illegal overloads of DXIL intrinsics may survive through to final DXIL,
  // but these will be caught by the validator, and this is not a regression.

  OpCodeClass opClass = OpProps.opCodeClass;
  Function *&F =
      m_OpCodeClassCache[(unsigned)opClass].pOverloads[pOverloadType];
  if (F != nullptr) {
    UpdateCache(opClass, pOverloadType, F);
    return F;
  }

  SmallVector<Type *, 32> ArgTypes; // RetType is ArgTypes[0]
  Type *pETy = pOverloadType;
  Type *pRes = GetHandleType();
  Type *pNodeHandle = GetNodeHandleType();
  Type *pNodeRecordHandle = GetNodeRecordHandleType();
  Type *pDim = GetDimensionsType();
  Type *pPos = GetSamplePosType();
  Type *pV = Type::getVoidTy(m_Ctx);
  Type *pI1 = Type::getInt1Ty(m_Ctx);
  Type *pOlTplI1 = Type::getInt1Ty(m_Ctx);
  Type *pI8 = Type::getInt8Ty(m_Ctx);
  Type *pI16 = Type::getInt16Ty(m_Ctx);
  Type *pI32 = Type::getInt32Ty(m_Ctx);
  Type *pOlTplI32 = Type::getInt32Ty(m_Ctx);
  if (pOverloadType->isVectorTy()) {
    pOlTplI32 =
        VectorType::get(pOlTplI32, pOverloadType->getVectorNumElements());
    pOlTplI1 = VectorType::get(pOlTplI1, pOverloadType->getVectorNumElements());
  }

  Type *pPI32 = Type::getInt32PtrTy(m_Ctx);
  (void)(pPI32); // Currently unused.
  Type *pI64 = Type::getInt64Ty(m_Ctx);
  (void)(pI64); // Currently unused.
  Type *pF16 = Type::getHalfTy(m_Ctx);
  Type *pF32 = Type::getFloatTy(m_Ctx);
  Type *pPF32 = Type::getFloatPtrTy(m_Ctx);
  Type *pI32C = GetBinaryWithCarryType();
  Type *p2I32 = GetBinaryWithTwoOutputsType();
  Type *pF64 = Type::getDoubleTy(m_Ctx);
  Type *pSDT = GetSplitDoubleType(); // Split double type.
  Type *p4I32 = GetFourI32Type();    // 4 i32s in a struct.
  Type *pHit = GetHitObjectType();

  Type *udt = pOverloadType;
  Type *obj = pOverloadType;
  Type *resProperty = GetResourcePropertiesType();
  Type *resBind = GetResourceBindingType();
  Type *nodeProperty = GetNodePropertiesType();
  Type *nodeRecordProperty = GetNodeRecordPropertiesType();

#define A(_x) ArgTypes.emplace_back(_x)
#define RRT(_y) A(GetResRetType(_y))
#define CBRT(_y) A(GetCBufferRetType(_y))
#define VEC4(_y) A(GetStructVectorType(4, _y))

// Extended Overload types are wrapped in an anonymous struct
#define EXT(_y) A(cast<StructType>(pOverloadType)->getElementType(_y))

  /* <py::lines('OPCODE-OLOAD-FUNCS')>hctdb_instrhelp.get_oloads_funcs()</py>*/
  switch (opCode) { // return     opCode
                    // OPCODE-OLOAD-FUNCS:BEGIN
                    // Temporary, indexable, input, output registers
  case OpCode::TempRegLoad:
    A(pETy);
    A(pI32);
    A(pI32);
    break;
  case OpCode::TempRegStore:
    A(pV);
    A(pI32);
    A(pI32);
    A(pETy);
    break;
  case OpCode::MinPrecXRegLoad:
    A(pETy);
    A(pI32);
    A(pPF32);
    A(pI32);
    A(pI8);
    break;
  case OpCode::MinPrecXRegStore:
    A(pV);
    A(pI32);
    A(pPF32);
    A(pI32);
    A(pI8);
    A(pETy);
    break;
  case OpCode::LoadInput:
    A(pETy);
    A(pI32);
    A(pI32);
    A(pI32);
    A(pI8);
    A(pI32);
    break;
  case OpCode::StoreOutput:
    A(pV);
    A(pI32);
    A(pI32);
    A(pI32);
    A(pI8);
    A(pETy);
    break;

    // Unary float
  case OpCode::FAbs:
    A(pETy);
    A(pI32);
    A(pETy);
    break;
  case OpCode::Saturate:
    A(pETy);
    A(pI32);
    A(pETy);
    break;
  case OpCode::IsNaN:
    A(pOlTplI1);
    A(pI32);
    A(pETy);
    break;
  case OpCode::IsInf:
    A(pOlTplI1);
    A(pI32);
    A(pETy);
    break;
  case OpCode::IsFinite:
    A(pOlTplI1);
    A(pI32);
    A(pETy);
    break;
  case OpCode::IsNormal:
    A(pOlTplI1);
    A(pI32);
    A(pETy);
    break;
  case OpCode::Cos:
    A(pETy);
    A(pI32);
    A(pETy);
    break;
  case OpCode::Sin:
    A(pETy);
    A(pI32);
    A(pETy);
    break;
  case OpCode::Tan:
    A(pETy);
    A(pI32);
    A(pETy);
    break;
  case OpCode::Acos:
    A(pETy);
    A(pI32);
    A(pETy);
    break;
  case OpCode::Asin:
    A(pETy);
    A(pI32);
    A(pETy);
    break;
  case OpCode::Atan:
    A(pETy);
    A(pI32);
    A(pETy);
    break;
  case OpCode::Hcos:
    A(pETy);
    A(pI32);
    A(pETy);
    break;
  case OpCode::Hsin:
    A(pETy);
    A(pI32);
    A(pETy);
    break;
  case OpCode::Htan:
    A(pETy);
    A(pI32);
    A(pETy);
    break;
  case OpCode::Exp:
    A(pETy);
    A(pI32);
    A(pETy);
    break;
  case OpCode::Frc:
    A(pETy);
    A(pI32);
    A(pETy);
    break;
  case OpCode::Log:
    A(pETy);
    A(pI32);
    A(pETy);
    break;
  case OpCode::Sqrt:
    A(pETy);
    A(pI32);
    A(pETy);
    break;
  case OpCode::Rsqrt:
    A(pETy);
    A(pI32);
    A(pETy);
    break;

    // Unary float - rounding
  case OpCode::Round_ne:
    A(pETy);
    A(pI32);
    A(pETy);
    break;
  case OpCode::Round_ni:
    A(pETy);
    A(pI32);
    A(pETy);
    break;
  case OpCode::Round_pi:
    A(pETy);
    A(pI32);
    A(pETy);
    break;
  case OpCode::Round_z:
    A(pETy);
    A(pI32);
    A(pETy);
    break;

    // Unary int
  case OpCode::Bfrev:
    A(pETy);
    A(pI32);
    A(pETy);
    break;
  case OpCode::Countbits:
    A(pOlTplI32);
    A(pI32);
    A(pETy);
    break;
  case OpCode::FirstbitLo:
    A(pOlTplI32);
    A(pI32);
    A(pETy);
    break;

    // Unary uint
  case OpCode::FirstbitHi:
    A(pOlTplI32);
    A(pI32);
    A(pETy);
    break;

    // Unary int
  case OpCode::FirstbitSHi:
    A(pOlTplI32);
    A(pI32);
    A(pETy);
    break;

    // Binary float
  case OpCode::FMax:
    A(pETy);
    A(pI32);
    A(pETy);
    A(pETy);
    break;
  case OpCode::FMin:
    A(pETy);
    A(pI32);
    A(pETy);
    A(pETy);
    break;

    // Binary int
  case OpCode::IMax:
    A(pETy);
    A(pI32);
    A(pETy);
    A(pETy);
    break;
  case OpCode::IMin:
    A(pETy);
    A(pI32);
    A(pETy);
    A(pETy);
    break;

    // Binary uint
  case OpCode::UMax:
    A(pETy);
    A(pI32);
    A(pETy);
    A(pETy);
    break;
  case OpCode::UMin:
    A(pETy);
    A(pI32);
    A(pETy);
    A(pETy);
    break;

    // Binary int with two outputs
  case OpCode::IMul:
    A(p2I32);
    A(pI32);
    A(pETy);
    A(pETy);
    break;

    // Binary uint with two outputs
  case OpCode::UMul:
    A(p2I32);
    A(pI32);
    A(pETy);
    A(pETy);
    break;
  case OpCode::UDiv:
    A(p2I32);
    A(pI32);
    A(pETy);
    A(pETy);
    break;

    // Binary uint with carry or borrow
  case OpCode::UAddc:
    A(pI32C);
    A(pI32);
    A(pETy);
    A(pETy);
    break;
  case OpCode::USubb:
    A(pI32C);
    A(pI32);
    A(pETy);
    A(pETy);
    break;

    // Tertiary float
  case OpCode::FMad:
    A(pETy);
    A(pI32);
    A(pETy);
    A(pETy);
    A(pETy);
    break;
  case OpCode::Fma:
    A(pETy);
    A(pI32);
    A(pETy);
    A(pETy);
    A(pETy);
    break;

    // Tertiary int
  case OpCode::IMad:
    A(pETy);
    A(pI32);
    A(pETy);
    A(pETy);
    A(pETy);
    break;

    // Tertiary uint
  case OpCode::UMad:
    A(pETy);
    A(pI32);
    A(pETy);
    A(pETy);
    A(pETy);
    break;

    // Tertiary int
  case OpCode::Msad:
    A(pETy);
    A(pI32);
    A(pETy);
    A(pETy);
    A(pETy);
    break;
  case OpCode::Ibfe:
    A(pETy);
    A(pI32);
    A(pETy);
    A(pETy);
    A(pETy);
    break;

    // Tertiary uint
  case OpCode::Ubfe:
    A(pETy);
    A(pI32);
    A(pETy);
    A(pETy);
    A(pETy);
    break;

    // Quaternary
  case OpCode::Bfi:
    A(pETy);
    A(pI32);
    A(pETy);
    A(pETy);
    A(pETy);
    A(pETy);
    break;

    // Dot
  case OpCode::Dot2:
    A(pETy);
    A(pI32);
    A(pETy);
    A(pETy);
    A(pETy);
    A(pETy);
    break;
  case OpCode::Dot3:
    A(pETy);
    A(pI32);
    A(pETy);
    A(pETy);
    A(pETy);
    A(pETy);
    A(pETy);
    A(pETy);
    break;
  case OpCode::Dot4:
    A(pETy);
    A(pI32);
    A(pETy);
    A(pETy);
    A(pETy);
    A(pETy);
    A(pETy);
    A(pETy);
    A(pETy);
    A(pETy);
    break;

    // Resources
  case OpCode::CreateHandle:
    A(pRes);
    A(pI32);
    A(pI8);
    A(pI32);
    A(pI32);
    A(pI1);
    break;
  case OpCode::CBufferLoad:
    A(pETy);
    A(pI32);
    A(pRes);
    A(pI32);
    A(pI32);
    break;
  case OpCode::CBufferLoadLegacy:
    CBRT(pETy);
    A(pI32);
    A(pRes);
    A(pI32);
    break;

    // Resources - sample
  case OpCode::Sample:
    RRT(pETy);
    A(pI32);
    A(pRes);
    A(pRes);
    A(pF32);
    A(pF32);
    A(pF32);
    A(pF32);
    A(pI32);
    A(pI32);
    A(pI32);
    A(pF32);
    break;
  case OpCode::SampleBias:
    RRT(pETy);
    A(pI32);
    A(pRes);
    A(pRes);
    A(pF32);
    A(pF32);
    A(pF32);
    A(pF32);
    A(pI32);
    A(pI32);
    A(pI32);
    A(pF32);
    A(pF32);
    break;
  case OpCode::SampleLevel:
    RRT(pETy);
    A(pI32);
    A(pRes);
    A(pRes);
    A(pF32);
    A(pF32);
    A(pF32);
    A(pF32);
    A(pI32);
    A(pI32);
    A(pI32);
    A(pF32);
    break;
  case OpCode::SampleGrad:
    RRT(pETy);
    A(pI32);
    A(pRes);
    A(pRes);
    A(pF32);
    A(pF32);
    A(pF32);
    A(pF32);
    A(pI32);
    A(pI32);
    A(pI32);
    A(pF32);
    A(pF32);
    A(pF32);
    A(pF32);
    A(pF32);
    A(pF32);
    A(pF32);
    break;
  case OpCode::SampleCmp:
    RRT(pETy);
    A(pI32);
    A(pRes);
    A(pRes);
    A(pF32);
    A(pF32);
    A(pF32);
    A(pF32);
    A(pI32);
    A(pI32);
    A(pI32);
    A(pF32);
    A(pF32);
    break;
  case OpCode::SampleCmpLevelZero:
    RRT(pETy);
    A(pI32);
    A(pRes);
    A(pRes);
    A(pF32);
    A(pF32);
    A(pF32);
    A(pF32);
    A(pI32);
    A(pI32);
    A(pI32);
    A(pF32);
    break;

    // Resources
  case OpCode::TextureLoad:
    RRT(pETy);
    A(pI32);
    A(pRes);
    A(pI32);
    A(pI32);
    A(pI32);
    A(pI32);
    A(pI32);
    A(pI32);
    A(pI32);
    break;
  case OpCode::TextureStore:
    A(pV);
    A(pI32);
    A(pRes);
    A(pI32);
    A(pI32);
    A(pI32);
    A(pETy);
    A(pETy);
    A(pETy);
    A(pETy);
    A(pI8);
    break;
  case OpCode::BufferLoad:
    RRT(pETy);
    A(pI32);
    A(pRes);
    A(pI32);
    A(pI32);
    break;
  case OpCode::BufferStore:
    A(pV);
    A(pI32);
    A(pRes);
    A(pI32);
    A(pI32);
    A(pETy);
    A(pETy);
    A(pETy);
    A(pETy);
    A(pI8);
    break;
  case OpCode::BufferUpdateCounter:
    A(pI32);
    A(pI32);
    A(pRes);
    A(pI8);
    break;
  case OpCode::CheckAccessFullyMapped:
    A(pI1);
    A(pI32);
    A(pI32);
    break;
  case OpCode::GetDimensions:
    A(pDim);
    A(pI32);
    A(pRes);
    A(pI32);
    break;

    // Resources - gather
  case OpCode::TextureGather:
    RRT(pETy);
    A(pI32);
    A(pRes);
    A(pRes);
    A(pF32);
    A(pF32);
    A(pF32);
    A(pF32);
    A(pI32);
    A(pI32);
    A(pI32);
    break;
  case OpCode::TextureGatherCmp:
    RRT(pETy);
    A(pI32);
    A(pRes);
    A(pRes);
    A(pF32);
    A(pF32);
    A(pF32);
    A(pF32);
    A(pI32);
    A(pI32);
    A(pI32);
    A(pF32);
    break;

    // Resources - sample
  case OpCode::Texture2DMSGetSamplePosition:
    A(pPos);
    A(pI32);
    A(pRes);
    A(pI32);
    break;
  case OpCode::RenderTargetGetSamplePosition:
    A(pPos);
    A(pI32);
    A(pI32);
    break;
  case OpCode::RenderTargetGetSampleCount:
    A(pI32);
    A(pI32);
    break;

    // Synchronization
  case OpCode::AtomicBinOp:
    A(pETy);
    A(pI32);
    A(pRes);
    A(pI32);
    A(pI32);
    A(pI32);
    A(pI32);
    A(pETy);
    break;
  case OpCode::AtomicCompareExchange:
    A(pETy);
    A(pI32);
    A(pRes);
    A(pI32);
    A(pI32);
    A(pI32);
    A(pETy);
    A(pETy);
    break;
  case OpCode::Barrier:
    A(pV);
    A(pI32);
    A(pI32);
    break;

    // Derivatives
  case OpCode::CalculateLOD:
    A(pF32);
    A(pI32);
    A(pRes);
    A(pRes);
    A(pF32);
    A(pF32);
    A(pF32);
    A(pI1);
    break;

    // Pixel shader
  case OpCode::Discard:
    A(pV);
    A(pI32);
    A(pI1);
    break;

    // Derivatives
  case OpCode::DerivCoarseX:
    A(pETy);
    A(pI32);
    A(pETy);
    break;
  case OpCode::DerivCoarseY:
    A(pETy);
    A(pI32);
    A(pETy);
    break;
  case OpCode::DerivFineX:
    A(pETy);
    A(pI32);
    A(pETy);
    break;
  case OpCode::DerivFineY:
    A(pETy);
    A(pI32);
    A(pETy);
    break;

    // Pixel shader
  case OpCode::EvalSnapped:
    A(pETy);
    A(pI32);
    A(pI32);
    A(pI32);
    A(pI8);
    A(pI32);
    A(pI32);
    break;
  case OpCode::EvalSampleIndex:
    A(pETy);
    A(pI32);
    A(pI32);
    A(pI32);
    A(pI8);
    A(pI32);
    break;
  case OpCode::EvalCentroid:
    A(pETy);
    A(pI32);
    A(pI32);
    A(pI32);
    A(pI8);
    break;
  case OpCode::SampleIndex:
    A(pI32);
    A(pI32);
    break;
  case OpCode::Coverage:
    A(pI32);
    A(pI32);
    break;
  case OpCode::InnerCoverage:
    A(pI32);
    A(pI32);
    break;

    // Compute/Mesh/Amplification/Node shader
  case OpCode::ThreadId:
    A(pI32);
    A(pI32);
    A(pI32);
    break;
  case OpCode::GroupId:
    A(pI32);
    A(pI32);
    A(pI32);
    break;
  case OpCode::ThreadIdInGroup:
    A(pI32);
    A(pI32);
    A(pI32);
    break;
  case OpCode::FlattenedThreadIdInGroup:
    A(pI32);
    A(pI32);
    break;

    // Geometry shader
  case OpCode::EmitStream:
    A(pV);
    A(pI32);
    A(pI8);
    break;
  case OpCode::CutStream:
    A(pV);
    A(pI32);
    A(pI8);
    break;
  case OpCode::EmitThenCutStream:
    A(pV);
    A(pI32);
    A(pI8);
    break;
  case OpCode::GSInstanceID:
    A(pI32);
    A(pI32);
    break;

    // Double precision
  case OpCode::MakeDouble:
    A(pF64);
    A(pI32);
    A(pI32);
    A(pI32);
    break;
  case OpCode::SplitDouble:
    A(pSDT);
    A(pI32);
    A(pF64);
    break;

    // Domain and hull shader
  case OpCode::LoadOutputControlPoint:
    A(pETy);
    A(pI32);
    A(pI32);
    A(pI32);
    A(pI8);
    A(pI32);
    break;
  case OpCode::LoadPatchConstant:
    A(pETy);
    A(pI32);
    A(pI32);
    A(pI32);
    A(pI8);
    break;

    // Domain shader
  case OpCode::DomainLocation:
    A(pF32);
    A(pI32);
    A(pI8);
    break;

    // Hull shader
  case OpCode::StorePatchConstant:
    A(pV);
    A(pI32);
    A(pI32);
    A(pI32);
    A(pI8);
    A(pETy);
    break;
  case OpCode::OutputControlPointID:
    A(pI32);
    A(pI32);
    break;

    // Hull, Domain and Geometry shaders
  case OpCode::PrimitiveID:
    A(pI32);
    A(pI32);
    break;

    // Other
  case OpCode::CycleCounterLegacy:
    A(p2I32);
    A(pI32);
    break;

    // Wave
  case OpCode::WaveIsFirstLane:
    A(pI1);
    A(pI32);
    break;
  case OpCode::WaveGetLaneIndex:
    A(pI32);
    A(pI32);
    break;
  case OpCode::WaveGetLaneCount:
    A(pI32);
    A(pI32);
    break;
  case OpCode::WaveAnyTrue:
    A(pI1);
    A(pI32);
    A(pI1);
    break;
  case OpCode::WaveAllTrue:
    A(pI1);
    A(pI32);
    A(pI1);
    break;
  case OpCode::WaveActiveAllEqual:
    A(pOlTplI1);
    A(pI32);
    A(pETy);
    break;
  case OpCode::WaveActiveBallot:
    A(p4I32);
    A(pI32);
    A(pI1);
    break;
  case OpCode::WaveReadLaneAt:
    A(pETy);
    A(pI32);
    A(pETy);
    A(pI32);
    break;
  case OpCode::WaveReadLaneFirst:
    A(pETy);
    A(pI32);
    A(pETy);
    break;
  case OpCode::WaveActiveOp:
    A(pETy);
    A(pI32);
    A(pETy);
    A(pI8);
    A(pI8);
    break;
  case OpCode::WaveActiveBit:
    A(pETy);
    A(pI32);
    A(pETy);
    A(pI8);
    break;
  case OpCode::WavePrefixOp:
    A(pETy);
    A(pI32);
    A(pETy);
    A(pI8);
    A(pI8);
    break;

    // Quad Wave Ops
  case OpCode::QuadReadLaneAt:
    A(pETy);
    A(pI32);
    A(pETy);
    A(pI32);
    break;
  case OpCode::QuadOp:
    A(pETy);
    A(pI32);
    A(pETy);
    A(pI8);
    break;

    // Bitcasts with different sizes
  case OpCode::BitcastI16toF16:
    A(pF16);
    A(pI32);
    A(pI16);
    break;
  case OpCode::BitcastF16toI16:
    A(pI16);
    A(pI32);
    A(pF16);
    break;
  case OpCode::BitcastI32toF32:
    A(pF32);
    A(pI32);
    A(pI32);
    break;
  case OpCode::BitcastF32toI32:
    A(pI32);
    A(pI32);
    A(pF32);
    break;
  case OpCode::BitcastI64toF64:
    A(pF64);
    A(pI32);
    A(pI64);
    break;
  case OpCode::BitcastF64toI64:
    A(pI64);
    A(pI32);
    A(pF64);
    break;

    // Legacy floating-point
  case OpCode::LegacyF32ToF16:
    A(pI32);
    A(pI32);
    A(pF32);
    break;
  case OpCode::LegacyF16ToF32:
    A(pF32);
    A(pI32);
    A(pI32);
    break;

    // Double precision
  case OpCode::LegacyDoubleToFloat:
    A(pF32);
    A(pI32);
    A(pF64);
    break;
  case OpCode::LegacyDoubleToSInt32:
    A(pI32);
    A(pI32);
    A(pF64);
    break;
  case OpCode::LegacyDoubleToUInt32:
    A(pI32);
    A(pI32);
    A(pF64);
    break;

    // Wave
  case OpCode::WaveAllBitCount:
    A(pI32);
    A(pI32);
    A(pI1);
    break;
  case OpCode::WavePrefixBitCount:
    A(pI32);
    A(pI32);
    A(pI1);
    break;

    // Pixel shader
  case OpCode::AttributeAtVertex:
    A(pETy);
    A(pI32);
    A(pI32);
    A(pI32);
    A(pI8);
    A(pI8);
    break;

    // Graphics shader
  case OpCode::ViewID:
    A(pI32);
    A(pI32);
    break;

    // Resources
  case OpCode::RawBufferLoad:
    RRT(pETy);
    A(pI32);
    A(pRes);
    A(pI32);
    A(pI32);
    A(pI8);
    A(pI32);
    break;
  case OpCode::RawBufferStore:
    A(pV);
    A(pI32);
    A(pRes);
    A(pI32);
    A(pI32);
    A(pETy);
    A(pETy);
    A(pETy);
    A(pETy);
    A(pI8);
    A(pI32);
    break;

    // Raytracing object space uint System Values
  case OpCode::InstanceID:
    A(pI32);
    A(pI32);
    break;
  case OpCode::InstanceIndex:
    A(pI32);
    A(pI32);
    break;

    // Raytracing hit uint System Values
  case OpCode::HitKind:
    A(pI32);
    A(pI32);
    break;

    // Raytracing uint System Values
  case OpCode::RayFlags:
    A(pI32);
    A(pI32);
    break;

    // Ray Dispatch Arguments
  case OpCode::DispatchRaysIndex:
    A(pI32);
    A(pI32);
    A(pI8);
    break;
  case OpCode::DispatchRaysDimensions:
    A(pI32);
    A(pI32);
    A(pI8);
    break;

    // Ray Vectors
  case OpCode::WorldRayOrigin:
    A(pF32);
    A(pI32);
    A(pI8);
    break;
  case OpCode::WorldRayDirection:
    A(pF32);
    A(pI32);
    A(pI8);
    break;

    // Ray object space Vectors
  case OpCode::ObjectRayOrigin:
    A(pF32);
    A(pI32);
    A(pI8);
    break;
  case OpCode::ObjectRayDirection:
    A(pF32);
    A(pI32);
    A(pI8);
    break;

    // Ray Transforms
  case OpCode::ObjectToWorld:
    A(pF32);
    A(pI32);
    A(pI32);
    A(pI8);
    break;
  case OpCode::WorldToObject:
    A(pF32);
    A(pI32);
    A(pI32);
    A(pI8);
    break;

    // RayT
  case OpCode::RayTMin:
    A(pF32);
    A(pI32);
    break;
  case OpCode::RayTCurrent:
    A(pF32);
    A(pI32);
    break;

    // AnyHit Terminals
  case OpCode::IgnoreHit:
    A(pV);
    A(pI32);
    break;
  case OpCode::AcceptHitAndEndSearch:
    A(pV);
    A(pI32);
    break;

    // Indirect Shader Invocation
  case OpCode::TraceRay:
    A(pV);
    A(pI32);
    A(pRes);
    A(pI32);
    A(pI32);
    A(pI32);
    A(pI32);
    A(pI32);
    A(pF32);
    A(pF32);
    A(pF32);
    A(pF32);
    A(pF32);
    A(pF32);
    A(pF32);
    A(pF32);
    A(udt);
    break;
  case OpCode::ReportHit:
    A(pI1);
    A(pI32);
    A(pF32);
    A(pI32);
    A(udt);
    break;
  case OpCode::CallShader:
    A(pV);
    A(pI32);
    A(pI32);
    A(udt);
    break;

    // Library create handle from resource struct (like HL intrinsic)
  case OpCode::CreateHandleForLib:
    A(pRes);
    A(pI32);
    A(obj);
    break;

    // Raytracing object space uint System Values
  case OpCode::PrimitiveIndex:
    A(pI32);
    A(pI32);
    break;

    // Dot product with accumulate
  case OpCode::Dot2AddHalf:
    A(pETy);
    A(pI32);
    A(pETy);
    A(pF16);
    A(pF16);
    A(pF16);
    A(pF16);
    break;
  case OpCode::Dot4AddI8Packed:
    A(pI32);
    A(pI32);
    A(pI32);
    A(pI32);
    A(pI32);
    break;
  case OpCode::Dot4AddU8Packed:
    A(pI32);
    A(pI32);
    A(pI32);
    A(pI32);
    A(pI32);
    break;

    // Wave
  case OpCode::WaveMatch:
    A(p4I32);
    A(pI32);
    A(pETy);
    break;
  case OpCode::WaveMultiPrefixOp:
    A(pETy);
    A(pI32);
    A(pETy);
    A(pI32);
    A(pI32);
    A(pI32);
    A(pI32);
    A(pI8);
    A(pI8);
    break;
  case OpCode::WaveMultiPrefixBitCount:
    A(pI32);
    A(pI32);
    A(pI1);
    A(pI32);
    A(pI32);
    A(pI32);
    A(pI32);
    break;

    // Mesh shader instructions
  case OpCode::SetMeshOutputCounts:
    A(pV);
    A(pI32);
    A(pI32);
    A(pI32);
    break;
  case OpCode::EmitIndices:
    A(pV);
    A(pI32);
    A(pI32);
    A(pI32);
    A(pI32);
    A(pI32);
    break;
  case OpCode::GetMeshPayload:
    A(pETy);
    A(pI32);
    break;
  case OpCode::StoreVertexOutput:
    A(pV);
    A(pI32);
    A(pI32);
    A(pI32);
    A(pI8);
    A(pETy);
    A(pI32);
    break;
  case OpCode::StorePrimitiveOutput:
    A(pV);
    A(pI32);
    A(pI32);
    A(pI32);
    A(pI8);
    A(pETy);
    A(pI32);
    break;

    // Amplification shader instructions
  case OpCode::DispatchMesh:
    A(pV);
    A(pI32);
    A(pI32);
    A(pI32);
    A(pI32);
    A(pETy);
    break;

    // Sampler Feedback
  case OpCode::WriteSamplerFeedback:
    A(pV);
    A(pI32);
    A(pRes);
    A(pRes);
    A(pRes);
    A(pF32);
    A(pF32);
    A(pF32);
    A(pF32);
    A(pF32);
    break;
  case OpCode::WriteSamplerFeedbackBias:
    A(pV);
    A(pI32);
    A(pRes);
    A(pRes);
    A(pRes);
    A(pF32);
    A(pF32);
    A(pF32);
    A(pF32);
    A(pF32);
    A(pF32);
    break;
  case OpCode::WriteSamplerFeedbackLevel:
    A(pV);
    A(pI32);
    A(pRes);
    A(pRes);
    A(pRes);
    A(pF32);
    A(pF32);
    A(pF32);
    A(pF32);
    A(pF32);
    break;
  case OpCode::WriteSamplerFeedbackGrad:
    A(pV);
    A(pI32);
    A(pRes);
    A(pRes);
    A(pRes);
    A(pF32);
    A(pF32);
    A(pF32);
    A(pF32);
    A(pF32);
    A(pF32);
    A(pF32);
    A(pF32);
    A(pF32);
    A(pF32);
    A(pF32);
    break;

    // Inline Ray Query
  case OpCode::AllocateRayQuery:
    A(pI32);
    A(pI32);
    A(pI32);
    break;
  case OpCode::RayQuery_TraceRayInline:
    A(pV);
    A(pI32);
    A(pI32);
    A(pRes);
    A(pI32);
    A(pI32);
    A(pF32);
    A(pF32);
    A(pF32);
    A(pF32);
    A(pF32);
    A(pF32);
    A(pF32);
    A(pF32);
    break;
  case OpCode::RayQuery_Proceed:
    A(pI1);
    A(pI32);
    A(pI32);
    break;
  case OpCode::RayQuery_Abort:
    A(pV);
    A(pI32);
    A(pI32);
    break;
  case OpCode::RayQuery_CommitNonOpaqueTriangleHit:
    A(pV);
    A(pI32);
    A(pI32);
    break;
  case OpCode::RayQuery_CommitProceduralPrimitiveHit:
    A(pV);
    A(pI32);
    A(pI32);
    A(pF32);
    break;
  case OpCode::RayQuery_CommittedStatus:
    A(pI32);
    A(pI32);
    A(pI32);
    break;
  case OpCode::RayQuery_CandidateType:
    A(pI32);
    A(pI32);
    A(pI32);
    break;
  case OpCode::RayQuery_CandidateObjectToWorld3x4:
    A(pF32);
    A(pI32);
    A(pI32);
    A(pI32);
    A(pI8);
    break;
  case OpCode::RayQuery_CandidateWorldToObject3x4:
    A(pF32);
    A(pI32);
    A(pI32);
    A(pI32);
    A(pI8);
    break;
  case OpCode::RayQuery_CommittedObjectToWorld3x4:
    A(pF32);
    A(pI32);
    A(pI32);
    A(pI32);
    A(pI8);
    break;
  case OpCode::RayQuery_CommittedWorldToObject3x4:
    A(pF32);
    A(pI32);
    A(pI32);
    A(pI32);
    A(pI8);
    break;
  case OpCode::RayQuery_CandidateProceduralPrimitiveNonOpaque:
    A(pI1);
    A(pI32);
    A(pI32);
    break;
  case OpCode::RayQuery_CandidateTriangleFrontFace:
    A(pI1);
    A(pI32);
    A(pI32);
    break;
  case OpCode::RayQuery_CommittedTriangleFrontFace:
    A(pI1);
    A(pI32);
    A(pI32);
    break;
  case OpCode::RayQuery_CandidateTriangleBarycentrics:
    A(pF32);
    A(pI32);
    A(pI32);
    A(pI8);
    break;
  case OpCode::RayQuery_CommittedTriangleBarycentrics:
    A(pF32);
    A(pI32);
    A(pI32);
    A(pI8);
    break;
  case OpCode::RayQuery_RayFlags:
    A(pI32);
    A(pI32);
    A(pI32);
    break;
  case OpCode::RayQuery_WorldRayOrigin:
    A(pF32);
    A(pI32);
    A(pI32);
    A(pI8);
    break;
  case OpCode::RayQuery_WorldRayDirection:
    A(pF32);
    A(pI32);
    A(pI32);
    A(pI8);
    break;
  case OpCode::RayQuery_RayTMin:
    A(pF32);
    A(pI32);
    A(pI32);
    break;
  case OpCode::RayQuery_CandidateTriangleRayT:
    A(pF32);
    A(pI32);
    A(pI32);
    break;
  case OpCode::RayQuery_CommittedRayT:
    A(pF32);
    A(pI32);
    A(pI32);
    break;
  case OpCode::RayQuery_CandidateInstanceIndex:
    A(pI32);
    A(pI32);
    A(pI32);
    break;
  case OpCode::RayQuery_CandidateInstanceID:
    A(pI32);
    A(pI32);
    A(pI32);
    break;
  case OpCode::RayQuery_CandidateGeometryIndex:
    A(pI32);
    A(pI32);
    A(pI32);
    break;
  case OpCode::RayQuery_CandidatePrimitiveIndex:
    A(pI32);
    A(pI32);
    A(pI32);
    break;
  case OpCode::RayQuery_CandidateObjectRayOrigin:
    A(pF32);
    A(pI32);
    A(pI32);
    A(pI8);
    break;
  case OpCode::RayQuery_CandidateObjectRayDirection:
    A(pF32);
    A(pI32);
    A(pI32);
    A(pI8);
    break;
  case OpCode::RayQuery_CommittedInstanceIndex:
    A(pI32);
    A(pI32);
    A(pI32);
    break;
  case OpCode::RayQuery_CommittedInstanceID:
    A(pI32);
    A(pI32);
    A(pI32);
    break;
  case OpCode::RayQuery_CommittedGeometryIndex:
    A(pI32);
    A(pI32);
    A(pI32);
    break;
  case OpCode::RayQuery_CommittedPrimitiveIndex:
    A(pI32);
    A(pI32);
    A(pI32);
    break;
  case OpCode::RayQuery_CommittedObjectRayOrigin:
    A(pF32);
    A(pI32);
    A(pI32);
    A(pI8);
    break;
  case OpCode::RayQuery_CommittedObjectRayDirection:
    A(pF32);
    A(pI32);
    A(pI32);
    A(pI8);
    break;

    // Raytracing object space uint System Values, raytracing tier 1.1
  case OpCode::GeometryIndex:
    A(pI32);
    A(pI32);
    break;

    // Inline Ray Query
  case OpCode::RayQuery_CandidateInstanceContributionToHitGroupIndex:
    A(pI32);
    A(pI32);
    A(pI32);
    break;
  case OpCode::RayQuery_CommittedInstanceContributionToHitGroupIndex:
    A(pI32);
    A(pI32);
    A(pI32);
    break;

    // Get handle from heap
  case OpCode::AnnotateHandle:
    A(pRes);
    A(pI32);
    A(pRes);
    A(resProperty);
    break;
  case OpCode::CreateHandleFromBinding:
    A(pRes);
    A(pI32);
    A(resBind);
    A(pI32);
    A(pI1);
    break;
  case OpCode::CreateHandleFromHeap:
    A(pRes);
    A(pI32);
    A(pI32);
    A(pI1);
    A(pI1);
    break;

    // Unpacking intrinsics
  case OpCode::Unpack4x8:
    VEC4(pETy);
    A(pI32);
    A(pI8);
    A(pI32);
    break;

    // Packing intrinsics
  case OpCode::Pack4x8:
    A(pI32);
    A(pI32);
    A(pI8);
    A(pETy);
    A(pETy);
    A(pETy);
    A(pETy);
    break;

    // Helper Lanes
  case OpCode::IsHelperLane:
    A(pI1);
    A(pI32);
    break;

    // Quad Wave Ops
  case OpCode::QuadVote:
    A(pOlTplI1);
    A(pI32);
    A(pI1);
    A(pI8);
    break;

    // Resources - gather
  case OpCode::TextureGatherRaw:
    RRT(pETy);
    A(pI32);
    A(pRes);
    A(pRes);
    A(pF32);
    A(pF32);
    A(pF32);
    A(pF32);
    A(pI32);
    A(pI32);
    break;

    // Resources - sample
  case OpCode::SampleCmpLevel:
    RRT(pETy);
    A(pI32);
    A(pRes);
    A(pRes);
    A(pF32);
    A(pF32);
    A(pF32);
    A(pF32);
    A(pI32);
    A(pI32);
    A(pI32);
    A(pF32);
    A(pF32);
    break;

    // Resources
  case OpCode::TextureStoreSample:
    A(pV);
    A(pI32);
    A(pRes);
    A(pI32);
    A(pI32);
    A(pI32);
    A(pETy);
    A(pETy);
    A(pETy);
    A(pETy);
    A(pI8);
    A(pI32);
    break;

    //
  case OpCode::Reserved0:
    A(pV);
    A(pI32);
    break;
  case OpCode::Reserved1:
    A(pV);
    A(pI32);
    break;
  case OpCode::Reserved2:
    A(pV);
    A(pI32);
    break;
  case OpCode::Reserved3:
    A(pV);
    A(pI32);
    break;
  case OpCode::Reserved4:
    A(pV);
    A(pI32);
    break;
  case OpCode::Reserved5:
    A(pV);
    A(pI32);
    break;
  case OpCode::Reserved6:
    A(pV);
    A(pI32);
    break;
  case OpCode::Reserved7:
    A(pV);
    A(pI32);
    break;
  case OpCode::Reserved8:
    A(pV);
    A(pI32);
    break;
  case OpCode::Reserved9:
    A(pV);
    A(pI32);
    break;
  case OpCode::Reserved10:
    A(pV);
    A(pI32);
    break;
  case OpCode::Reserved11:
    A(pV);
    A(pI32);
    break;

    // Create/Annotate Node Handles
  case OpCode::AllocateNodeOutputRecords:
    A(pNodeRecordHandle);
    A(pI32);
    A(pNodeHandle);
    A(pI32);
    A(pI1);
    break;

    // Get Pointer to Node Record in Address Space 6
  case OpCode::GetNodeRecordPtr:
    A(pETy);
    A(pI32);
    A(pNodeRecordHandle);
    A(pI32);
    break;

    // Work Graph intrinsics
  case OpCode::IncrementOutputCount:
    A(pV);
    A(pI32);
    A(pNodeHandle);
    A(pI32);
    A(pI1);
    break;
  case OpCode::OutputComplete:
    A(pV);
    A(pI32);
    A(pNodeRecordHandle);
    break;
  case OpCode::GetInputRecordCount:
    A(pI32);
    A(pI32);
    A(pNodeRecordHandle);
    break;
  case OpCode::FinishedCrossGroupSharing:
    A(pI1);
    A(pI32);
    A(pNodeRecordHandle);
    break;

    // Synchronization
  case OpCode::BarrierByMemoryType:
    A(pV);
    A(pI32);
    A(pI32);
    A(pI32);
    break;
  case OpCode::BarrierByMemoryHandle:
    A(pV);
    A(pI32);
    A(pRes);
    A(pI32);
    break;
  case OpCode::BarrierByNodeRecordHandle:
    A(pV);
    A(pI32);
    A(pNodeRecordHandle);
    A(pI32);
    break;

    // Create/Annotate Node Handles
  case OpCode::CreateNodeOutputHandle:
    A(pNodeHandle);
    A(pI32);
    A(pI32);
    break;
  case OpCode::IndexNodeHandle:
    A(pNodeHandle);
    A(pI32);
    A(pNodeHandle);
    A(pI32);
    break;
  case OpCode::AnnotateNodeHandle:
    A(pNodeHandle);
    A(pI32);
    A(pNodeHandle);
    A(nodeProperty);
    break;
  case OpCode::CreateNodeInputRecordHandle:
    A(pNodeRecordHandle);
    A(pI32);
    A(pI32);
    break;
  case OpCode::AnnotateNodeRecordHandle:
    A(pNodeRecordHandle);
    A(pI32);
    A(pNodeRecordHandle);
    A(nodeRecordProperty);
    break;

    // Work Graph intrinsics
  case OpCode::NodeOutputIsValid:
    A(pI1);
    A(pI32);
    A(pNodeHandle);
    break;
  case OpCode::GetRemainingRecursionLevels:
    A(pI32);
    A(pI32);
    break;

    // Comparison Samples
  case OpCode::SampleCmpGrad:
    RRT(pETy);
    A(pI32);
    A(pRes);
    A(pRes);
    A(pF32);
    A(pF32);
    A(pF32);
    A(pF32);
    A(pI32);
    A(pI32);
    A(pI32);
    A(pF32);
    A(pF32);
    A(pF32);
    A(pF32);
    A(pF32);
    A(pF32);
    A(pF32);
    A(pF32);
    break;
  case OpCode::SampleCmpBias:
    RRT(pETy);
    A(pI32);
    A(pRes);
    A(pRes);
    A(pF32);
    A(pF32);
    A(pF32);
    A(pF32);
    A(pI32);
    A(pI32);
    A(pI32);
    A(pF32);
    A(pF32);
    A(pF32);
    break;

    // Extended Command Information
  case OpCode::StartVertexLocation:
    A(pI32);
    A(pI32);
    break;
  case OpCode::StartInstanceLocation:
    A(pI32);
    A(pI32);
    break;

    // Inline Ray Query
  case OpCode::AllocateRayQuery2:
    A(pI32);
    A(pI32);
    A(pI32);
    A(pI32);
    break;

    //
  case OpCode::ReservedA0:
    A(pV);
    A(pI32);
    break;
  case OpCode::ReservedA1:
    A(pV);
    A(pI32);
    break;
  case OpCode::ReservedA2:
    A(pV);
    A(pI32);
    break;

    // Shader Execution Reordering
  case OpCode::HitObject_TraceRay:
    A(pHit);
    A(pI32);
    A(pRes);
    A(pI32);
    A(pI32);
    A(pI32);
    A(pI32);
    A(pI32);
    A(pF32);
    A(pF32);
    A(pF32);
    A(pF32);
    A(pF32);
    A(pF32);
    A(pF32);
    A(pF32);
    A(udt);
    break;
  case OpCode::HitObject_FromRayQuery:
    A(pHit);
    A(pI32);
    A(pI32);
    break;
  case OpCode::HitObject_FromRayQueryWithAttrs:
    A(pHit);
    A(pI32);
    A(pI32);
    A(pI32);
    A(udt);
    break;
  case OpCode::HitObject_MakeMiss:
    A(pHit);
    A(pI32);
    A(pI32);
    A(pI32);
    A(pF32);
    A(pF32);
    A(pF32);
    A(pF32);
    A(pF32);
    A(pF32);
    A(pF32);
    A(pF32);
    break;
  case OpCode::HitObject_MakeNop:
    A(pHit);
    A(pI32);
    break;
  case OpCode::HitObject_Invoke:
    A(pV);
    A(pI32);
    A(pHit);
    A(udt);
    break;
  case OpCode::MaybeReorderThread:
    A(pV);
    A(pI32);
    A(pHit);
    A(pI32);
    A(pI32);
    break;
  case OpCode::HitObject_IsMiss:
    A(pI1);
    A(pI32);
    A(pHit);
    break;
  case OpCode::HitObject_IsHit:
    A(pI1);
    A(pI32);
    A(pHit);
    break;
  case OpCode::HitObject_IsNop:
    A(pI1);
    A(pI32);
    A(pHit);
    break;
  case OpCode::HitObject_RayFlags:
    A(pI32);
    A(pI32);
    A(pHit);
    break;
  case OpCode::HitObject_RayTMin:
    A(pF32);
    A(pI32);
    A(pHit);
    break;
  case OpCode::HitObject_RayTCurrent:
    A(pF32);
    A(pI32);
    A(pHit);
    break;
  case OpCode::HitObject_WorldRayOrigin:
    A(pF32);
    A(pI32);
    A(pHit);
    A(pI32);
    break;
  case OpCode::HitObject_WorldRayDirection:
    A(pF32);
    A(pI32);
    A(pHit);
    A(pI32);
    break;
  case OpCode::HitObject_ObjectRayOrigin:
    A(pF32);
    A(pI32);
    A(pHit);
    A(pI32);
    break;
  case OpCode::HitObject_ObjectRayDirection:
    A(pF32);
    A(pI32);
    A(pHit);
    A(pI32);
    break;
  case OpCode::HitObject_ObjectToWorld3x4:
    A(pF32);
    A(pI32);
    A(pHit);
    A(pI32);
    A(pI32);
    break;
  case OpCode::HitObject_WorldToObject3x4:
    A(pF32);
    A(pI32);
    A(pHit);
    A(pI32);
    A(pI32);
    break;
  case OpCode::HitObject_GeometryIndex:
    A(pI32);
    A(pI32);
    A(pHit);
    break;
  case OpCode::HitObject_InstanceIndex:
    A(pI32);
    A(pI32);
    A(pHit);
    break;
  case OpCode::HitObject_InstanceID:
    A(pI32);
    A(pI32);
    A(pHit);
    break;
  case OpCode::HitObject_PrimitiveIndex:
    A(pI32);
    A(pI32);
    A(pHit);
    break;
  case OpCode::HitObject_HitKind:
    A(pI32);
    A(pI32);
    A(pHit);
    break;
  case OpCode::HitObject_ShaderTableIndex:
    A(pI32);
    A(pI32);
    A(pHit);
    break;
  case OpCode::HitObject_SetShaderTableIndex:
    A(pHit);
    A(pI32);
    A(pHit);
    A(pI32);
    break;
  case OpCode::HitObject_LoadLocalRootTableConstant:
    A(pI32);
    A(pI32);
    A(pHit);
    A(pI32);
    break;
  case OpCode::HitObject_Attributes:
    A(pV);
    A(pI32);
    A(pHit);
    A(udt);
    break;

    //
  case OpCode::ReservedB28:
    A(pV);
    A(pI32);
    break;
  case OpCode::ReservedB29:
    A(pV);
    A(pI32);
    break;
  case OpCode::ReservedB30:
    A(pV);
    A(pI32);
    break;
  case OpCode::ReservedC0:
    A(pV);
    A(pI32);
    break;
  case OpCode::ReservedC1:
    A(pV);
    A(pI32);
    break;
  case OpCode::ReservedC2:
    A(pV);
    A(pI32);
    break;
  case OpCode::ReservedC3:
    A(pV);
    A(pI32);
    break;
  case OpCode::ReservedC4:
    A(pV);
    A(pI32);
    break;
  case OpCode::ReservedC5:
    A(pV);
    A(pI32);
    break;
  case OpCode::ReservedC6:
    A(pV);
    A(pI32);
    break;
  case OpCode::ReservedC7:
    A(pV);
    A(pI32);
    break;
  case OpCode::ReservedC8:
    A(pV);
    A(pI32);
    break;
  case OpCode::ReservedC9:
    A(pV);
    A(pI32);
    break;

    // Resources
  case OpCode::RawBufferVectorLoad:
    RRT(pETy);
    A(pI32);
    A(pRes);
    A(pI32);
    A(pI32);
    A(pI32);
    break;
  case OpCode::RawBufferVectorStore:
    A(pV);
    A(pI32);
    A(pRes);
    A(pI32);
    A(pI32);
    A(pETy);
    A(pI32);
    break;

    // Linear Algebra Operations
  case OpCode::MatVecMul:
    EXT(0);
    A(pI32);
    EXT(1);
    A(pI1);
    A(pI32);
    A(pRes);
    A(pI32);
    A(pI32);
    A(pI32);
    A(pI32);
    A(pI32);
    A(pI1);
    A(pI32);
    A(pI1);
    break;
  case OpCode::MatVecMulAdd:
    EXT(0);
    A(pI32);
    EXT(1);
    A(pI1);
    A(pI32);
    A(pRes);
    A(pI32);
    A(pI32);
    A(pI32);
    A(pI32);
    A(pI32);
    A(pI1);
    A(pI32);
    A(pRes);
    A(pI32);
    A(pI32);
    A(pI1);
    break;
  case OpCode::OuterProductAccumulate:
    A(pV);
    A(pI32);
    EXT(0);
    EXT(1);
    A(pRes);
    A(pI32);
    A(pI32);
    A(pI32);
    A(pI32);
    break;
  case OpCode::VectorAccumulate:
    A(pV);
    A(pI32);
    A(pETy);
    A(pRes);
    A(pI32);
    break;
  // OPCODE-OLOAD-FUNCS:END
  default:
    DXASSERT(false, "otherwise unhandled case");
    break;
  }
#undef RRT
#undef A

  FunctionType *pFT;
  DXASSERT(ArgTypes.size() > 1, "otherwise forgot to initialize arguments");
  pFT = FunctionType::get(
      ArgTypes[0], ArrayRef<Type *>(&ArgTypes[1], ArgTypes.size() - 1), false);

  SmallVector<char, 256> FuncStorage;
  StringRef FuncName =
      ConstructOverloadName(pOverloadType, opCode, FuncStorage);

  // Try to find existing function with the same name in the module.
  // This needs to happen after the switch statement that constructs arguments
  // and return values to ensure that ResRetType is constructed in the
  // RefreshCache case.
  if (Function *existF = m_pModule->getFunction(FuncName)) {
    if (existF->getFunctionType() != pFT)
      return nullptr;
    F = existF;
    UpdateCache(opClass, pOverloadType, F);
    return F;
  }

  F = cast<Function>(m_pModule->getOrInsertFunction(FuncName, pFT));

  UpdateCache(opClass, pOverloadType, F);
  F->setCallingConv(CallingConv::C);
  F->addFnAttr(Attribute::NoUnwind);
  if (OpProps.FuncAttr != Attribute::None)
    F->addFnAttr(OpProps.FuncAttr);

  return F;
}

const SmallMapVector<llvm::Type *, llvm::Function *, 8> &
OP::GetOpFuncList(OpCode opCode) const {
  return m_OpCodeClassCache[(unsigned)m_OpCodeProps[(unsigned)opCode]
                                .opCodeClass]
      .pOverloads;
}

bool OP::IsDxilOpUsed(OpCode opcode) const {
  auto &FnList = GetOpFuncList(opcode);
  for (auto &it : FnList) {
    llvm::Function *F = it.second;
    if (!F) {
      continue;
    }
    if (!F->user_empty()) {
      return true;
    }
  }
  return false;
}

void OP::RemoveFunction(Function *F) {
  if (OP::IsDxilOpFunc(F)) {
    OpCodeClass opClass = m_FunctionToOpClass[F];
    for (auto it : m_OpCodeClassCache[(unsigned)opClass].pOverloads) {
      if (it.second == F) {
        m_OpCodeClassCache[(unsigned)opClass].pOverloads.erase(it.first);
        m_FunctionToOpClass.erase(F);
        break;
      }
    }
  }
}

bool OP::GetOpCodeClass(const Function *F, OP::OpCodeClass &opClass) {
  auto iter = m_FunctionToOpClass.find(F);
  if (iter == m_FunctionToOpClass.end()) {
    // When no user, cannot get opcode.
    DXASSERT(F->user_empty() || !IsDxilOpFunc(F),
             "dxil function without an opcode class mapping?");
    opClass = OP::OpCodeClass::NumOpClasses;
    return false;
  }
  opClass = iter->second;
  return true;
}

bool OP::UseMinPrecision() {
  return m_LowPrecisionMode == DXIL::LowPrecisionMode::UseMinPrecision;
}

void OP::InitWithMinPrecision(bool bMinPrecision) {
  DXIL::LowPrecisionMode mode =
      bMinPrecision ? DXIL::LowPrecisionMode::UseMinPrecision
                    : DXIL::LowPrecisionMode::UseNativeLowPrecision;
  DXASSERT((mode == m_LowPrecisionMode ||
            m_LowPrecisionMode == DXIL::LowPrecisionMode::Undefined),
           "LowPrecisionMode should only be set once.");

  if (mode != m_LowPrecisionMode) {
    m_LowPrecisionMode = mode;

    // The following FixOverloadNames() and RefreshCache() calls interact with
    // the type cache, which can only be correctly constructed once we know
    // the min precision mode.  That's why they are called here, rather than
    // in the constructor.

    // When loading a module into an existing context where types are merged,
    // type names may change.  When this happens, any intrinsics overloaded on
    // UDT types will no longer have matching overload names.
    // This causes RefreshCache() to assert.
    // This fixes the function names to they match the expected types,
    // preventing RefreshCache() from failing due to this issue.
    FixOverloadNames();

    // Try to find existing intrinsic function.
    RefreshCache();
  }
}

uint64_t OP::GetAllocSizeForType(llvm::Type *Ty) {
  return m_pModule->getDataLayout().getTypeAllocSize(Ty);
}

llvm::Type *OP::GetOverloadType(OpCode opCode, llvm::Function *F) {
  DXASSERT(F, "not work on nullptr");
  Type *Ty = F->getReturnType();
  FunctionType *FT = F->getFunctionType();
  LLVMContext &Ctx = F->getContext();
  // clang-format off
  // Python lines need to be not formatted.
  /* <py::lines('OPCODE-OLOAD-TYPES')>hctdb_instrhelp.get_funcs_oload_type()</py>*/
  // clang-format on
  switch (opCode) { // return     OpCode
  // OPCODE-OLOAD-TYPES:BEGIN
  case OpCode::TempRegStore:
  case OpCode::CallShader:
  case OpCode::Pack4x8:
  case OpCode::HitObject_Invoke:
  case OpCode::HitObject_Attributes:
    if (FT->getNumParams() <= 2)
      return nullptr;
    return FT->getParamType(2);
  case OpCode::MinPrecXRegStore:
  case OpCode::StoreOutput:
  case OpCode::BufferStore:
  case OpCode::StorePatchConstant:
  case OpCode::RawBufferStore:
  case OpCode::StoreVertexOutput:
  case OpCode::StorePrimitiveOutput:
  case OpCode::DispatchMesh:
  case OpCode::RawBufferVectorStore:
    if (FT->getNumParams() <= 4)
      return nullptr;
    return FT->getParamType(4);
  case OpCode::IsNaN:
  case OpCode::IsInf:
  case OpCode::IsFinite:
  case OpCode::IsNormal:
  case OpCode::Countbits:
  case OpCode::FirstbitLo:
  case OpCode::FirstbitHi:
  case OpCode::FirstbitSHi:
  case OpCode::IMul:
  case OpCode::UMul:
  case OpCode::UDiv:
  case OpCode::UAddc:
  case OpCode::USubb:
  case OpCode::WaveActiveAllEqual:
  case OpCode::CreateHandleForLib:
  case OpCode::WaveMatch:
  case OpCode::VectorAccumulate:
    if (FT->getNumParams() <= 1)
      return nullptr;
    return FT->getParamType(1);
  case OpCode::TextureStore:
  case OpCode::TextureStoreSample:
    if (FT->getNumParams() <= 5)
      return nullptr;
    return FT->getParamType(5);
  case OpCode::TraceRay:
  case OpCode::HitObject_TraceRay:
    if (FT->getNumParams() <= 15)
      return nullptr;
    return FT->getParamType(15);
  case OpCode::ReportHit:
  case OpCode::HitObject_FromRayQueryWithAttrs:
    if (FT->getNumParams() <= 3)
      return nullptr;
    return FT->getParamType(3);
  case OpCode::CreateHandle:
  case OpCode::BufferUpdateCounter:
  case OpCode::GetDimensions:
  case OpCode::Texture2DMSGetSamplePosition:
  case OpCode::RenderTargetGetSamplePosition:
  case OpCode::RenderTargetGetSampleCount:
  case OpCode::Barrier:
  case OpCode::Discard:
  case OpCode::EmitStream:
  case OpCode::CutStream:
  case OpCode::EmitThenCutStream:
  case OpCode::CycleCounterLegacy:
  case OpCode::WaveIsFirstLane:
  case OpCode::WaveGetLaneIndex:
  case OpCode::WaveGetLaneCount:
  case OpCode::WaveAnyTrue:
  case OpCode::WaveAllTrue:
  case OpCode::WaveActiveBallot:
  case OpCode::BitcastI16toF16:
  case OpCode::BitcastF16toI16:
  case OpCode::BitcastI32toF32:
  case OpCode::BitcastF32toI32:
  case OpCode::BitcastI64toF64:
  case OpCode::BitcastF64toI64:
  case OpCode::LegacyF32ToF16:
  case OpCode::LegacyF16ToF32:
  case OpCode::LegacyDoubleToFloat:
  case OpCode::LegacyDoubleToSInt32:
  case OpCode::LegacyDoubleToUInt32:
  case OpCode::WaveAllBitCount:
  case OpCode::WavePrefixBitCount:
  case OpCode::IgnoreHit:
  case OpCode::AcceptHitAndEndSearch:
  case OpCode::WaveMultiPrefixBitCount:
  case OpCode::SetMeshOutputCounts:
  case OpCode::EmitIndices:
  case OpCode::WriteSamplerFeedback:
  case OpCode::WriteSamplerFeedbackBias:
  case OpCode::WriteSamplerFeedbackLevel:
  case OpCode::WriteSamplerFeedbackGrad:
  case OpCode::AllocateRayQuery:
  case OpCode::RayQuery_TraceRayInline:
  case OpCode::RayQuery_Abort:
  case OpCode::RayQuery_CommitNonOpaqueTriangleHit:
  case OpCode::RayQuery_CommitProceduralPrimitiveHit:
  case OpCode::AnnotateHandle:
  case OpCode::CreateHandleFromBinding:
  case OpCode::CreateHandleFromHeap:
  case OpCode::Reserved0:
  case OpCode::Reserved1:
  case OpCode::Reserved2:
  case OpCode::Reserved3:
  case OpCode::Reserved4:
  case OpCode::Reserved5:
  case OpCode::Reserved6:
  case OpCode::Reserved7:
  case OpCode::Reserved8:
  case OpCode::Reserved9:
  case OpCode::Reserved10:
  case OpCode::Reserved11:
  case OpCode::AllocateNodeOutputRecords:
  case OpCode::IncrementOutputCount:
  case OpCode::OutputComplete:
  case OpCode::GetInputRecordCount:
  case OpCode::FinishedCrossGroupSharing:
  case OpCode::BarrierByMemoryType:
  case OpCode::BarrierByMemoryHandle:
  case OpCode::BarrierByNodeRecordHandle:
  case OpCode::CreateNodeOutputHandle:
  case OpCode::IndexNodeHandle:
  case OpCode::AnnotateNodeHandle:
  case OpCode::CreateNodeInputRecordHandle:
  case OpCode::AnnotateNodeRecordHandle:
  case OpCode::NodeOutputIsValid:
  case OpCode::GetRemainingRecursionLevels:
  case OpCode::AllocateRayQuery2:
  case OpCode::ReservedA0:
  case OpCode::ReservedA1:
  case OpCode::ReservedA2:
  case OpCode::HitObject_FromRayQuery:
  case OpCode::HitObject_MakeMiss:
  case OpCode::HitObject_MakeNop:
  case OpCode::MaybeReorderThread:
  case OpCode::HitObject_SetShaderTableIndex:
  case OpCode::HitObject_LoadLocalRootTableConstant:
  case OpCode::ReservedB28:
  case OpCode::ReservedB29:
  case OpCode::ReservedB30:
  case OpCode::ReservedC0:
  case OpCode::ReservedC1:
  case OpCode::ReservedC2:
  case OpCode::ReservedC3:
  case OpCode::ReservedC4:
  case OpCode::ReservedC5:
  case OpCode::ReservedC6:
  case OpCode::ReservedC7:
  case OpCode::ReservedC8:
  case OpCode::ReservedC9:
    return Type::getVoidTy(Ctx);
  case OpCode::CheckAccessFullyMapped:
  case OpCode::SampleIndex:
  case OpCode::Coverage:
  case OpCode::InnerCoverage:
  case OpCode::ThreadId:
  case OpCode::GroupId:
  case OpCode::ThreadIdInGroup:
  case OpCode::FlattenedThreadIdInGroup:
  case OpCode::GSInstanceID:
  case OpCode::OutputControlPointID:
  case OpCode::PrimitiveID:
  case OpCode::ViewID:
  case OpCode::InstanceID:
  case OpCode::InstanceIndex:
  case OpCode::HitKind:
  case OpCode::RayFlags:
  case OpCode::DispatchRaysIndex:
  case OpCode::DispatchRaysDimensions:
  case OpCode::PrimitiveIndex:
  case OpCode::Dot4AddI8Packed:
  case OpCode::Dot4AddU8Packed:
  case OpCode::RayQuery_CommittedStatus:
  case OpCode::RayQuery_CandidateType:
  case OpCode::RayQuery_RayFlags:
  case OpCode::RayQuery_CandidateInstanceIndex:
  case OpCode::RayQuery_CandidateInstanceID:
  case OpCode::RayQuery_CandidateGeometryIndex:
  case OpCode::RayQuery_CandidatePrimitiveIndex:
  case OpCode::RayQuery_CommittedInstanceIndex:
  case OpCode::RayQuery_CommittedInstanceID:
  case OpCode::RayQuery_CommittedGeometryIndex:
  case OpCode::RayQuery_CommittedPrimitiveIndex:
  case OpCode::GeometryIndex:
  case OpCode::RayQuery_CandidateInstanceContributionToHitGroupIndex:
  case OpCode::RayQuery_CommittedInstanceContributionToHitGroupIndex:
  case OpCode::StartVertexLocation:
  case OpCode::StartInstanceLocation:
  case OpCode::HitObject_RayFlags:
  case OpCode::HitObject_GeometryIndex:
  case OpCode::HitObject_InstanceIndex:
  case OpCode::HitObject_InstanceID:
  case OpCode::HitObject_PrimitiveIndex:
  case OpCode::HitObject_HitKind:
  case OpCode::HitObject_ShaderTableIndex:
    return IntegerType::get(Ctx, 32);
  case OpCode::CalculateLOD:
  case OpCode::DomainLocation:
  case OpCode::WorldRayOrigin:
  case OpCode::WorldRayDirection:
  case OpCode::ObjectRayOrigin:
  case OpCode::ObjectRayDirection:
  case OpCode::ObjectToWorld:
  case OpCode::WorldToObject:
  case OpCode::RayTMin:
  case OpCode::RayTCurrent:
  case OpCode::RayQuery_CandidateObjectToWorld3x4:
  case OpCode::RayQuery_CandidateWorldToObject3x4:
  case OpCode::RayQuery_CommittedObjectToWorld3x4:
  case OpCode::RayQuery_CommittedWorldToObject3x4:
  case OpCode::RayQuery_CandidateTriangleBarycentrics:
  case OpCode::RayQuery_CommittedTriangleBarycentrics:
  case OpCode::RayQuery_WorldRayOrigin:
  case OpCode::RayQuery_WorldRayDirection:
  case OpCode::RayQuery_RayTMin:
  case OpCode::RayQuery_CandidateTriangleRayT:
  case OpCode::RayQuery_CommittedRayT:
  case OpCode::RayQuery_CandidateObjectRayOrigin:
  case OpCode::RayQuery_CandidateObjectRayDirection:
  case OpCode::RayQuery_CommittedObjectRayOrigin:
  case OpCode::RayQuery_CommittedObjectRayDirection:
  case OpCode::HitObject_RayTMin:
  case OpCode::HitObject_RayTCurrent:
  case OpCode::HitObject_WorldRayOrigin:
  case OpCode::HitObject_WorldRayDirection:
  case OpCode::HitObject_ObjectRayOrigin:
  case OpCode::HitObject_ObjectRayDirection:
  case OpCode::HitObject_ObjectToWorld3x4:
  case OpCode::HitObject_WorldToObject3x4:
    return Type::getFloatTy(Ctx);
  case OpCode::MakeDouble:
  case OpCode::SplitDouble:
    return Type::getDoubleTy(Ctx);
  case OpCode::RayQuery_Proceed:
  case OpCode::RayQuery_CandidateProceduralPrimitiveNonOpaque:
  case OpCode::RayQuery_CandidateTriangleFrontFace:
  case OpCode::RayQuery_CommittedTriangleFrontFace:
  case OpCode::IsHelperLane:
  case OpCode::QuadVote:
  case OpCode::HitObject_IsMiss:
  case OpCode::HitObject_IsHit:
  case OpCode::HitObject_IsNop:
    return IntegerType::get(Ctx, 1);
  case OpCode::CBufferLoadLegacy:
  case OpCode::Sample:
  case OpCode::SampleBias:
  case OpCode::SampleLevel:
  case OpCode::SampleGrad:
  case OpCode::SampleCmp:
  case OpCode::SampleCmpLevelZero:
  case OpCode::TextureLoad:
  case OpCode::BufferLoad:
  case OpCode::TextureGather:
  case OpCode::TextureGatherCmp:
  case OpCode::RawBufferLoad:
  case OpCode::Unpack4x8:
  case OpCode::TextureGatherRaw:
  case OpCode::SampleCmpLevel:
  case OpCode::SampleCmpGrad:
  case OpCode::SampleCmpBias:
  case OpCode::RawBufferVectorLoad: {
    StructType *ST = cast<StructType>(Ty);
    return ST->getElementType(0);
  }
  case OpCode::MatVecMul:
  case OpCode::MatVecMulAdd:
    if (FT->getNumParams() < 2)
      return nullptr;
    return llvm::StructType::get(Ctx,
                                 {FT->getReturnType(), FT->getParamType(1)});

  case OpCode::OuterProductAccumulate:
    if (FT->getNumParams() < 3)
      return nullptr;
    return llvm::StructType::get(Ctx,
                                 {FT->getParamType(1), FT->getParamType(2)});

  // OPCODE-OLOAD-TYPES:END
  default:
    return Ty;
  }
}

Type *OP::GetHandleType() const { return m_pHandleType; }

Type *OP::GetNodeHandleType() const { return m_pNodeHandleType; }

Type *OP::GetHitObjectType() const { return m_pHitObjectType; }

Type *OP::GetNodeRecordHandleType() const { return m_pNodeRecordHandleType; }

Type *OP::GetResourcePropertiesType() const {
  return m_pResourcePropertiesType;
}

Type *OP::GetNodePropertiesType() const { return m_pNodePropertiesType; }

Type *OP::GetNodeRecordPropertiesType() const {
  return m_pNodeRecordPropertiesType;
}

Type *OP::GetResourceBindingType() const { return m_pResourceBindingType; }

Type *OP::GetDimensionsType() const { return m_pDimensionsType; }

Type *OP::GetSamplePosType() const { return m_pSamplePosType; }

Type *OP::GetBinaryWithCarryType() const { return m_pBinaryWithCarryType; }

Type *OP::GetBinaryWithTwoOutputsType() const {
  return m_pBinaryWithTwoOutputsType;
}

Type *OP::GetSplitDoubleType() const { return m_pSplitDoubleType; }

Type *OP::GetFourI32Type() const { return m_pFourI32Type; }

Type *OP::GetFourI16Type() const { return m_pFourI16Type; }

bool OP::IsResRetType(llvm::Type *Ty) {
  if (!Ty || !Ty->isStructTy())
    return false;
  for (Type *ResTy : m_pResRetType) {
    if (Ty == ResTy)
      return true;
  }
  // Check for vector overload which isn't cached in m_pResRetType.
  StructType *ST = cast<StructType>(Ty);
  if (!ST->hasName() || ST->getNumElements() < 2 ||
      !ST->getElementType(0)->isVectorTy())
    return false;
  return Ty == GetResRetType(ST->getElementType(0));
}

Type *OP::GetResRetType(Type *pOverloadType) {
  unsigned TypeSlot = GetTypeSlot(pOverloadType);

  if (TypeSlot < TS_BasicCount) {
    if (m_pResRetType[TypeSlot] == nullptr) {
      SmallVector<char, 32> Storage;
      StringRef TypeName =
          (Twine("dx.types.ResRet.") + Twine(GetOverloadTypeName(TypeSlot)))
              .toStringRef(Storage);
      Type *FieldTypes[5] = {pOverloadType, pOverloadType, pOverloadType,
                             pOverloadType, Type::getInt32Ty(m_Ctx)};
      m_pResRetType[TypeSlot] =
          GetOrCreateStructType(m_Ctx, FieldTypes, TypeName, m_pModule);
    }
    return m_pResRetType[TypeSlot];
  } else if (TypeSlot == TS_Vector) {
    SmallVector<char, 32> Storage;
    VectorType *VecTy = cast<VectorType>(pOverloadType);
    StringRef TypeName =
        (Twine("dx.types.ResRet.v") + Twine(VecTy->getNumElements()) +
         Twine(GetOverloadTypeName(OP::GetTypeSlot(VecTy->getElementType()))))
            .toStringRef(Storage);
    Type *FieldTypes[2] = {pOverloadType, Type::getInt32Ty(m_Ctx)};
    return GetOrCreateStructType(m_Ctx, FieldTypes, TypeName, m_pModule);
  }

  llvm_unreachable("Invalid overload for GetResRetType");
  return nullptr;
}

Type *OP::GetCBufferRetType(Type *pOverloadType) {
  unsigned TypeSlot = GetTypeSlot(pOverloadType);

  if (TypeSlot >= TS_BasicCount) {
    llvm_unreachable("Invalid overload for GetResRetType");
    return nullptr;
  }

  if (m_pCBufferRetType[TypeSlot] == nullptr) {
    DXASSERT(m_LowPrecisionMode != DXIL::LowPrecisionMode::Undefined,
             "m_LowPrecisionMode must be set before constructing type.");
    SmallVector<char, 32> Storage;
    raw_svector_ostream OS(Storage);
    OS << "dx.types.CBufRet.";
    OS << GetOverloadTypeName(TypeSlot);
    Type *i64Ty = Type::getInt64Ty(pOverloadType->getContext());
    Type *i16Ty = Type::getInt16Ty(pOverloadType->getContext());
    if (pOverloadType->isDoubleTy() || pOverloadType == i64Ty) {
      Type *FieldTypes[2] = {pOverloadType, pOverloadType};
      m_pCBufferRetType[TypeSlot] =
          GetOrCreateStructType(m_Ctx, FieldTypes, OS.str(), m_pModule);
    } else if (!UseMinPrecision() &&
               (pOverloadType->isHalfTy() || pOverloadType == i16Ty)) {
      OS << ".8"; // dx.types.CBufRet.f16.8 for buffer of 8 halves
      Type *FieldTypes[8] = {
          pOverloadType, pOverloadType, pOverloadType, pOverloadType,
          pOverloadType, pOverloadType, pOverloadType, pOverloadType,
      };
      m_pCBufferRetType[TypeSlot] =
          GetOrCreateStructType(m_Ctx, FieldTypes, OS.str(), m_pModule);
    } else {
      Type *FieldTypes[4] = {pOverloadType, pOverloadType, pOverloadType,
                             pOverloadType};
      m_pCBufferRetType[TypeSlot] =
          GetOrCreateStructType(m_Ctx, FieldTypes, OS.str(), m_pModule);
    }
  }
  return m_pCBufferRetType[TypeSlot];
}

Type *OP::GetStructVectorType(unsigned numElements, Type *pOverloadType) {
  if (numElements == 4) {
    if (pOverloadType == Type::getInt32Ty(pOverloadType->getContext())) {
      return m_pFourI32Type;
    } else if (pOverloadType == Type::getInt16Ty(pOverloadType->getContext())) {
      return m_pFourI16Type;
    }
  }
  DXASSERT(false, "unexpected overload type");
  return nullptr;
}

StructType *OP::GetExtendedOverloadType(ArrayRef<Type *> OverloadTypes) {
  return StructType::get(m_Ctx, OverloadTypes);
}

//------------------------------------------------------------------------------
//
//  LLVM utility methods.
//
Constant *OP::GetI1Const(bool v) {
  return Constant::getIntegerValue(IntegerType::get(m_Ctx, 1), APInt(1, v));
}

Constant *OP::GetI8Const(char v) {
  return Constant::getIntegerValue(IntegerType::get(m_Ctx, 8), APInt(8, v));
}

Constant *OP::GetU8Const(unsigned char v) { return GetI8Const((char)v); }

Constant *OP::GetI16Const(int v) {
  return Constant::getIntegerValue(IntegerType::get(m_Ctx, 16), APInt(16, v));
}

Constant *OP::GetU16Const(unsigned v) { return GetI16Const((int)v); }

Constant *OP::GetI32Const(int v) {
  return Constant::getIntegerValue(IntegerType::get(m_Ctx, 32), APInt(32, v));
}

Constant *OP::GetU32Const(unsigned v) { return GetI32Const((int)v); }

Constant *OP::GetU64Const(unsigned long long v) {
  return Constant::getIntegerValue(IntegerType::get(m_Ctx, 64), APInt(64, v));
}

Constant *OP::GetFloatConst(float v) {
  return ConstantFP::get(m_Ctx, APFloat(v));
}

Constant *OP::GetDoubleConst(double v) {
  return ConstantFP::get(m_Ctx, APFloat(v));
}

} // namespace hlsl
