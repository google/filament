///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// DxilContainerAssembler.h                                                  //
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
// This file is distributed under the University of Illinois Open Source     //
// License. See LICENSE.TXT for details.                                     //
//                                                                           //
// Helpers for writing to dxil container.                                    //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#pragma once

#include "dxc/DxilContainer/DxilContainer.h"
#include "llvm/ADT/StringRef.h"
#include <functional>

struct IDxcVersionInfo;
struct IStream;
class DxilPipelineStateValidation;

namespace llvm {
class Module;
}

namespace hlsl {

class AbstractMemoryStream;
class DxilModule;
class RootSignatureHandle;
class ShaderModel;
namespace DXIL {
enum class SignatureKind;
}

class DxilPartWriter {
public:
  virtual ~DxilPartWriter() {}
  virtual uint32_t size() const = 0;
  virtual void write(AbstractMemoryStream *pStream) = 0;
};

class DxilContainerWriter : public DxilPartWriter {
public:
  typedef std::function<void(AbstractMemoryStream *)> WriteFn;
  virtual ~DxilContainerWriter() {}
  virtual void AddPart(uint32_t FourCC, uint32_t Size, WriteFn Write) = 0;
};

DxilPartWriter *NewProgramSignatureWriter(const DxilModule &M,
                                          DXIL::SignatureKind Kind);
DxilPartWriter *NewRootSignatureWriter(const RootSignatureHandle &S);
DxilPartWriter *NewFeatureInfoWriter(const DxilModule &M);
DxilPartWriter *NewPSVWriter(const DxilModule &M,
                             uint32_t PSVVersion = UINT_MAX);
// DxilModule is non-const because it caches per-function flag computations
// used by both CollectShaderFlagsForModule and RDATWriter.
DxilPartWriter *NewRDATWriter(DxilModule &M);
DxilPartWriter *NewVersionWriter(IDxcVersionInfo *pVersionInfo);

// Store serialized ViewID data from DxilModule to PipelineStateValidation.
void StoreViewIDStateToPSV(const uint32_t *pInputData,
                           unsigned InputSizeInUInts,
                           DxilPipelineStateValidation &PSV);
// Load ViewID state from PSV back to DxilModule view state vector.
// Pass nullptr for pOutputData to compute and return needed OutputSizeInUInts.
unsigned LoadViewIDStateFromPSV(unsigned *pOutputData,
                                unsigned OutputSizeInUInts,
                                const DxilPipelineStateValidation &PSV);

// Unaligned is for matching container for validator version < 1.7.
DxilContainerWriter *NewDxilContainerWriter(bool bUnaligned = false);

// Set validator version to 0,0 (not validated) then re-emit as much reflection
// metadata as possible.
void ReEmitLatestReflectionData(llvm::Module *pReflectionM);

// Strip functions and serialize module.
void StripAndCreateReflectionStream(
    llvm::Module *pReflectionM, uint32_t *pReflectionPartSizeInBytes,
    AbstractMemoryStream **ppReflectionStreamOut);

void WriteProgramPart(const hlsl::ShaderModel *pModel,
                      AbstractMemoryStream *pModuleBitcode, IStream *pStream);

void SerializeDxilContainerForModule(
    hlsl::DxilModule *pModule, AbstractMemoryStream *pModuleBitcode,
    IDxcVersionInfo *DXCVersionInfo, AbstractMemoryStream *pStream,
    llvm::StringRef DebugName, SerializeDxilFlags Flags,
    DxilShaderHash *pShaderHashOut = nullptr,
    AbstractMemoryStream *pReflectionStreamOut = nullptr,
    AbstractMemoryStream *pRootSigStreamOut = nullptr,
    void *pPrivateData = nullptr, size_t PrivateDataSize = 0);
void SerializeDxilContainerForRootSignature(
    hlsl::RootSignatureHandle *pRootSigHandle, AbstractMemoryStream *pStream);

} // namespace hlsl