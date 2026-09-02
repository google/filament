///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// D3DReflectionDumper.cpp                                                   //
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
// This file is distributed under the University of Illinois Open Source     //
// License. See LICENSE.TXT for details.                                     //
//                                                                           //
// Use this to dump D3D Reflection data for testing.                         //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#include "dxc/Test/D3DReflectionDumper.h"
#include "dxc/DXIL/DxilConstants.h"
#include "dxc/DxilContainer/DxilContainer.h"
#include "dxc/Support/Global.h"
#include "dxc/Test/D3DReflectionStrings.h"
#include "dxc/dxcapi.h"
#include <sstream>

namespace hlsl {
namespace dump {

void D3DReflectionDumper::DumpDefaultValue(LPCVOID pDefaultValue, UINT Size) {
  WriteLn("DefaultValue: ",
          pDefaultValue ? "<present>" : "<nullptr>"); // TODO: Dump DefaultValue
}
void D3DReflectionDumper::DumpShaderVersion(UINT Version) {
  const char *szType = "<unknown>";
  UINT Type = D3D12_SHVER_GET_TYPE(Version);
  switch (Type) {
  case (UINT)hlsl::DXIL::ShaderKind::Pixel:
    szType = "Pixel";
    break;
  case (UINT)hlsl::DXIL::ShaderKind::Vertex:
    szType = "Vertex";
    break;
  case (UINT)hlsl::DXIL::ShaderKind::Geometry:
    szType = "Geometry";
    break;
  case (UINT)hlsl::DXIL::ShaderKind::Hull:
    szType = "Hull";
    break;
  case (UINT)hlsl::DXIL::ShaderKind::Domain:
    szType = "Domain";
    break;
  case (UINT)hlsl::DXIL::ShaderKind::Compute:
    szType = "Compute";
    break;
  case (UINT)hlsl::DXIL::ShaderKind::Library:
    szType = "Library";
    break;
  case (UINT)hlsl::DXIL::ShaderKind::RayGeneration:
    szType = "RayGeneration";
    break;
  case (UINT)hlsl::DXIL::ShaderKind::Intersection:
    szType = "Intersection";
    break;
  case (UINT)hlsl::DXIL::ShaderKind::AnyHit:
    szType = "AnyHit";
    break;
  case (UINT)hlsl::DXIL::ShaderKind::ClosestHit:
    szType = "ClosestHit";
    break;
  case (UINT)hlsl::DXIL::ShaderKind::Miss:
    szType = "Miss";
    break;
  case (UINT)hlsl::DXIL::ShaderKind::Callable:
    szType = "Callable";
    break;
  case (UINT)hlsl::DXIL::ShaderKind::Mesh:
    szType = "Mesh";
    break;
  case (UINT)hlsl::DXIL::ShaderKind::Amplification:
    szType = "Amplification";
    break;
  case (UINT)hlsl::DXIL::ShaderKind::Invalid:
    szType = "Invalid";
    break;
  }
  UINT Major = D3D12_SHVER_GET_MAJOR(Version);
  UINT Minor = D3D12_SHVER_GET_MINOR(Version);
  WriteLn("Shader Version: ", szType, " ", Major, ".", Minor);
}

void D3DReflectionDumper::Dump(D3D12_SHADER_TYPE_DESC &tyDesc) {
  SetLastName(tyDesc.Name);
  WriteLn("D3D12_SHADER_TYPE_DESC: Name: ", m_LastName);
  Indent();
  DumpEnum("Class", tyDesc.Class);
  DumpEnum("Type", tyDesc.Type);
  WriteLn("Elements: ", tyDesc.Elements);
  WriteLn("Rows: ", tyDesc.Rows);
  WriteLn("Columns: ", tyDesc.Columns);
  WriteLn("Members: ", tyDesc.Members);
  WriteLn("Offset: ", tyDesc.Offset);
  Dedent();
}
void D3DReflectionDumper::Dump(D3D12_SHADER_VARIABLE_DESC &varDesc) {
  SetLastName(varDesc.Name);
  WriteLn("D3D12_SHADER_VARIABLE_DESC: Name: ", m_LastName);
  Indent();
  WriteLn("Size: ", varDesc.Size);
  WriteLn("StartOffset: ", varDesc.StartOffset);
  WriteLn("uFlags: ", FlagsValue<D3D_SHADER_VARIABLE_FLAGS>(varDesc.uFlags));
  DumpDefaultValue(varDesc.DefaultValue, varDesc.Size);
  Dedent();
}
void D3DReflectionDumper::Dump(D3D12_SHADER_BUFFER_DESC &Desc) {
  SetLastName(Desc.Name);
  WriteLn("D3D12_SHADER_BUFFER_DESC: Name: ", m_LastName);
  Indent();
  DumpEnum("Type", Desc.Type);
  WriteLn("Size: ", Desc.Size);
  WriteLn("uFlags: ", FlagsValue<D3D_SHADER_CBUFFER_FLAGS>(Desc.uFlags));
  WriteLn("Num Variables: ", Desc.Variables);
  Dedent();
}
void D3DReflectionDumper::Dump(D3D12_SHADER_INPUT_BIND_DESC &resDesc) {
  SetLastName(resDesc.Name);
  WriteLn("D3D12_SHADER_INPUT_BIND_DESC: Name: ", m_LastName);
  Indent();
  DumpEnum("Type", resDesc.Type);
  WriteLn("uID: ", resDesc.uID);
  WriteLn("BindCount: ", resDesc.BindCount);
  WriteLn("BindPoint: ", resDesc.BindPoint);
  WriteLn("Space: ", resDesc.Space);
  DumpEnum("ReturnType", resDesc.ReturnType);
  DumpEnum("Dimension", resDesc.Dimension);
  WriteLn("NumSamples (or stride): ", resDesc.NumSamples);
  WriteLn("uFlags: ", FlagsValue<D3D_SHADER_INPUT_FLAGS>(resDesc.uFlags));
  Dedent();
}
void D3DReflectionDumper::Dump(D3D12_SIGNATURE_PARAMETER_DESC &elDesc) {
  WriteLn("D3D12_SIGNATURE_PARAMETER_DESC: SemanticName: ", elDesc.SemanticName,
          " SemanticIndex: ", elDesc.SemanticIndex);
  Indent();
  WriteLn("Register: ", elDesc.Register);
  DumpEnum("SystemValueType", elDesc.SystemValueType);
  DumpEnum("ComponentType", elDesc.ComponentType);
  WriteLn("Mask: ", CompMaskToString(elDesc.Mask), " (", (UINT)elDesc.Mask,
          ")");
  WriteLn("ReadWriteMask: ", CompMaskToString(elDesc.ReadWriteMask), " (",
          (UINT)elDesc.ReadWriteMask, ") (AlwaysReads/NeverWrites)");
  WriteLn("Stream: ", elDesc.Stream);
  DumpEnum("MinPrecision", elDesc.MinPrecision);
  Dedent();
}
void D3DReflectionDumper::Dump(D3D12_SHADER_DESC &Desc) {
  WriteLn("D3D12_SHADER_DESC:");
  Indent();
  DumpShaderVersion(Desc.Version);
  WriteLn("Creator: ", Desc.Creator ? Desc.Creator : "<nullptr>");
  WriteLn("Flags: ", std::hex, std::showbase,
          Desc.Flags); // TODO: fxc compiler flags
  WriteLn("ConstantBuffers: ", Desc.ConstantBuffers);
  WriteLn("BoundResources: ", Desc.BoundResources);
  WriteLn("InputParameters: ", Desc.InputParameters);
  WriteLn("OutputParameters: ", Desc.OutputParameters);
  hlsl::DXIL::ShaderKind ShaderKind =
      (hlsl::DXIL::ShaderKind)D3D12_SHVER_GET_TYPE(Desc.Version);
  if (ShaderKind == hlsl::DXIL::ShaderKind::Geometry) {
    WriteLn("cGSInstanceCount: ", Desc.cGSInstanceCount);
    WriteLn("GSMaxOutputVertexCount: ", Desc.GSMaxOutputVertexCount);
    DumpEnum("GSOutputTopology", Desc.GSOutputTopology);
    DumpEnum("InputPrimitive", Desc.InputPrimitive);
  }
  if (ShaderKind == hlsl::DXIL::ShaderKind::Hull) {
    WriteLn("PatchConstantParameters: ", Desc.PatchConstantParameters);
    WriteLn("cControlPoints: ", Desc.cControlPoints);
    DumpEnum("InputPrimitive", Desc.InputPrimitive);
    DumpEnum("HSOutputPrimitive", Desc.HSOutputPrimitive);
    DumpEnum("HSPartitioning", Desc.HSPartitioning);
    DumpEnum("TessellatorDomain", Desc.TessellatorDomain);
  }
  if (ShaderKind == hlsl::DXIL::ShaderKind::Domain) {
    WriteLn("PatchConstantParameters: ", Desc.PatchConstantParameters);
    WriteLn("cControlPoints: ", Desc.cControlPoints);
    DumpEnum("TessellatorDomain", Desc.TessellatorDomain);
  }
  if (ShaderKind == hlsl::DXIL::ShaderKind::Mesh) {
    WriteLn("PatchConstantParameters: ", Desc.PatchConstantParameters,
            " (output primitive parameters for mesh shader)");
  }
  // Instruction Counts
  WriteLn("InstructionCount: ", Desc.InstructionCount);
  WriteLn("TempArrayCount: ", Desc.TempArrayCount);
  WriteLn("DynamicFlowControlCount: ", Desc.DynamicFlowControlCount);
  WriteLn("ArrayInstructionCount: ", Desc.ArrayInstructionCount);
  WriteLn("TextureNormalInstructions: ", Desc.TextureNormalInstructions);
  WriteLn("TextureLoadInstructions: ", Desc.TextureLoadInstructions);
  WriteLn("TextureCompInstructions: ", Desc.TextureCompInstructions);
  WriteLn("TextureBiasInstructions: ", Desc.TextureBiasInstructions);
  WriteLn("TextureGradientInstructions: ", Desc.TextureGradientInstructions);
  WriteLn("FloatInstructionCount: ", Desc.FloatInstructionCount);
  WriteLn("IntInstructionCount: ", Desc.IntInstructionCount);
  WriteLn("UintInstructionCount: ", Desc.UintInstructionCount);
  WriteLn("CutInstructionCount: ", Desc.CutInstructionCount);
  WriteLn("EmitInstructionCount: ", Desc.EmitInstructionCount);
  WriteLn("cBarrierInstructions: ", Desc.cBarrierInstructions);
  WriteLn("cInterlockedInstructions: ", Desc.cInterlockedInstructions);
  WriteLn("cTextureStoreInstructions: ", Desc.cTextureStoreInstructions);

  Dedent();
}
void D3DReflectionDumper::Dump(D3D12_FUNCTION_DESC &Desc) {
  SetLastName(Desc.Name);
  WriteLn("D3D12_FUNCTION_DESC: Name: ", EscapedString(m_LastName));
  Indent();
  DumpShaderVersion(Desc.Version);
  WriteLn("Creator: ", Desc.Creator ? Desc.Creator : "<nullptr>");
  WriteLn("Flags: ", std::hex, std::showbase, Desc.Flags);
  WriteLn("RequiredFeatureFlags: ", std::hex, std::showbase,
          Desc.RequiredFeatureFlags);
  WriteLn("ConstantBuffers: ", Desc.ConstantBuffers);
  WriteLn("BoundResources: ", Desc.BoundResources);
  WriteLn("FunctionParameterCount: ", Desc.FunctionParameterCount);
  WriteLn("HasReturn: ", Desc.HasReturn ? "TRUE" : "FALSE");
  Dedent();
}
void D3DReflectionDumper::Dump(D3D12_LIBRARY_DESC &Desc) {
  WriteLn("D3D12_LIBRARY_DESC:");
  Indent();
  WriteLn("Creator: ", Desc.Creator ? Desc.Creator : "<nullptr>");
  WriteLn("Flags: ", std::hex, std::showbase, Desc.Flags);
  WriteLn("FunctionCount: ", Desc.FunctionCount);
  Dedent();
}

void D3DReflectionDumper::Dump(ID3D12ShaderReflectionType *pType) {
  WriteLn("ID3D12ShaderReflectionType:");
  Indent();
  D3D12_SHADER_TYPE_DESC tyDesc;
  if (!pType || FAILED(pType->GetDesc(&tyDesc))) {
    Failure("GetDesc");
    Dedent();
    return;
  }
  Dump(tyDesc);
  if (tyDesc.Members) {
    WriteLn("{");
    Indent();
    for (UINT uMember = 0; uMember < tyDesc.Members; uMember++) {
      Dump(pType->GetMemberTypeByIndex(uMember));
    }
    Dedent();
    WriteLn("}");
  }
  Dedent();
}
void D3DReflectionDumper::Dump(ID3D12ShaderReflectionVariable *pVar) {
  WriteLn("ID3D12ShaderReflectionVariable:");
  Indent();
  D3D12_SHADER_VARIABLE_DESC varDesc;
  if (!pVar || FAILED(pVar->GetDesc(&varDesc))) {
    Failure("GetDesc");
    Dedent();
    return;
  }
  Dump(varDesc);
  Dump(pVar->GetType());
  ID3D12ShaderReflectionConstantBuffer *pCB = pVar->GetBuffer();
  D3D12_SHADER_BUFFER_DESC CBDesc;
  if (pCB && SUCCEEDED(pCB->GetDesc(&CBDesc))) {
    WriteLn("CBuffer: ", CBDesc.Name);
  }
  Dedent();
}

void D3DReflectionDumper::Dump(
    ID3D12ShaderReflectionConstantBuffer *pCBReflection) {
  WriteLn("ID3D12ShaderReflectionConstantBuffer:");
  Indent();
  D3D12_SHADER_BUFFER_DESC Desc;
  if (!pCBReflection || FAILED(pCBReflection->GetDesc(&Desc))) {
    Failure("GetDesc");
    Dedent();
    return;
  }
  Dump(Desc);
  if (Desc.Variables) {
    WriteLn("{");
    Indent();
    bool bCheckByNameFailed = false;
    for (UINT uVar = 0; uVar < Desc.Variables; uVar++) {
      if (m_bCheckByName)
        SetLastName();
      ID3D12ShaderReflectionVariable *pVar =
          pCBReflection->GetVariableByIndex(uVar);
      Dump(pVar);
      if (m_bCheckByName) {
        if (pCBReflection->GetVariableByName(m_LastName) != pVar) {
          bCheckByNameFailed = true;
          Failure("GetVariableByName ", m_LastName);
        }
      }
    }
    if (m_bCheckByName && !bCheckByNameFailed) {
      WriteLn("GetVariableByName checks succeeded.");
    }
    Dedent();
    WriteLn("}");
  }
  Dedent();
}

void D3DReflectionDumper::Dump(ID3D12ShaderReflection *pShaderReflection) {
  WriteLn("ID3D12ShaderReflection:");
  Indent();
  D3D12_SHADER_DESC Desc;
  if (!pShaderReflection || FAILED(pShaderReflection->GetDesc(&Desc))) {
    Failure("GetDesc");
    Dedent();
    return;
  }
  Dump(Desc);

  if (Desc.InputParameters) {
    WriteLn("InputParameter Elements: ", Desc.InputParameters);
    Indent();
    for (UINT i = 0; i < Desc.InputParameters; ++i) {
      D3D12_SIGNATURE_PARAMETER_DESC elDesc;
      if (FAILED(pShaderReflection->GetInputParameterDesc(i, &elDesc))) {
        Failure("GetInputParameterDesc ", i);
        continue;
      }
      Dump(elDesc);
    }
    Dedent();
  }
  if (Desc.OutputParameters) {
    WriteLn("OutputParameter Elements: ", Desc.OutputParameters);
    Indent();
    for (UINT i = 0; i < Desc.OutputParameters; ++i) {
      D3D12_SIGNATURE_PARAMETER_DESC elDesc;
      if (FAILED(pShaderReflection->GetOutputParameterDesc(i, &elDesc))) {
        Failure("GetOutputParameterDesc ", i);
        continue;
      }
      Dump(elDesc);
    }
    Dedent();
  }
  if (Desc.PatchConstantParameters) {
    WriteLn("PatchConstantParameter Elements: ", Desc.PatchConstantParameters);
    Indent();
    for (UINT i = 0; i < Desc.PatchConstantParameters; ++i) {
      D3D12_SIGNATURE_PARAMETER_DESC elDesc;
      if (FAILED(
              pShaderReflection->GetPatchConstantParameterDesc(i, &elDesc))) {
        Failure("GetPatchConstantParameterDesc ", i);
        continue;
      }
      Dump(elDesc);
    }
    Dedent();
  }

  if (Desc.ConstantBuffers) {
    WriteLn("Constant Buffers:");
    Indent();
    bool bCheckByNameFailed = false;
    for (UINT uCB = 0; uCB < Desc.ConstantBuffers; uCB++) {
      ID3D12ShaderReflectionConstantBuffer *pCB =
          pShaderReflection->GetConstantBufferByIndex(uCB);
      if (!pCB) {
        Failure("GetConstantBufferByIndex ", uCB);
        continue;
      }
      Dump(pCB);
      if (m_bCheckByName && m_LastName) {
        if (pShaderReflection->GetConstantBufferByName(m_LastName) != pCB) {
          bCheckByNameFailed = true;
          Failure("GetConstantBufferByName ", m_LastName);
        }
      }
    }
    if (m_bCheckByName && !bCheckByNameFailed) {
      WriteLn("GetConstantBufferByName checks succeeded.");
    }
    Dedent();
  }
  if (Desc.BoundResources) {
    WriteLn("Bound Resources:");
    Indent();
    bool bCheckByNameFailed = false;
    for (UINT uRes = 0; uRes < Desc.BoundResources; uRes++) {
      D3D12_SHADER_INPUT_BIND_DESC bindDesc;
      if (FAILED(pShaderReflection->GetResourceBindingDesc(uRes, &bindDesc))) {
        Failure("GetResourceBindingDesc ", uRes);
        continue;
      }
      Dump(bindDesc);
      if (m_bCheckByName && bindDesc.Name) {
        D3D12_SHADER_INPUT_BIND_DESC bindDesc2;
        if (FAILED(pShaderReflection->GetResourceBindingDescByName(
                bindDesc.Name, &bindDesc2)) ||
            bindDesc2.Name != bindDesc.Name) {
          bCheckByNameFailed = true;
          Failure("GetResourceBindingDescByName ", bindDesc.Name);
        }
      }
    }
    if (m_bCheckByName && !bCheckByNameFailed) {
      WriteLn("GetResourceBindingDescByName checks succeeded.");
    }
    Dedent();
  }
  // TODO
  Dedent();
}

void D3DReflectionDumper::Dump(ID3D12FunctionReflection *pFunctionReflection) {
  WriteLn("ID3D12FunctionReflection:");
  Indent();
  D3D12_FUNCTION_DESC Desc;
  if (!pFunctionReflection || FAILED(pFunctionReflection->GetDesc(&Desc))) {
    Failure("GetDesc");
    Dedent();
    return;
  }
  Dump(Desc);
  if (Desc.ConstantBuffers) {
    WriteLn("Constant Buffers:");
    Indent();
    bool bCheckByNameFailed = false;
    for (UINT uCB = 0; uCB < Desc.ConstantBuffers; uCB++) {
      ID3D12ShaderReflectionConstantBuffer *pCB =
          pFunctionReflection->GetConstantBufferByIndex(uCB);
      Dump(pCB);
      if (m_bCheckByName && m_LastName) {
        if (pFunctionReflection->GetConstantBufferByName(m_LastName) != pCB) {
          bCheckByNameFailed = true;
          Failure("GetConstantBufferByName ", m_LastName);
        }
      }
    }
    if (m_bCheckByName && !bCheckByNameFailed) {
      WriteLn("GetConstantBufferByName checks succeeded.");
    }
    Dedent();
  }
  if (Desc.BoundResources) {
    WriteLn("Bound Resources:");
    Indent();
    bool bCheckByNameFailed = false;
    for (UINT uRes = 0; uRes < Desc.BoundResources; uRes++) {
      D3D12_SHADER_INPUT_BIND_DESC bindDesc;
      if (FAILED(
              pFunctionReflection->GetResourceBindingDesc(uRes, &bindDesc))) {
      }
      Dump(bindDesc);
      if (m_bCheckByName && bindDesc.Name) {
        D3D12_SHADER_INPUT_BIND_DESC bindDesc2;
        if (FAILED(pFunctionReflection->GetResourceBindingDescByName(
                bindDesc.Name, &bindDesc2)) ||
            bindDesc2.Name != bindDesc.Name) {
          bCheckByNameFailed = true;
          Failure("GetResourceBindingDescByName ", bindDesc.Name);
        }
      }
    }
    if (m_bCheckByName && !bCheckByNameFailed) {
      WriteLn("GetResourceBindingDescByName checks succeeded.");
    }
    Dedent();
  }
  // TODO
  Dedent();
}

void D3DReflectionDumper::Dump(ID3D12LibraryReflection *pLibraryReflection) {
  WriteLn("ID3D12LibraryReflection:");
  Indent();
  D3D12_LIBRARY_DESC Desc;
  if (!pLibraryReflection || FAILED(pLibraryReflection->GetDesc(&Desc))) {
    Failure("GetDesc");
    Dedent();
    return;
  }
  Dump(Desc);
  if (Desc.FunctionCount) {
    for (UINT uFunc = 0; uFunc < Desc.FunctionCount; uFunc++)
      Dump(pLibraryReflection->GetFunctionByIndex((INT)uFunc));
  }
  Dedent();
}

} // namespace dump
} // namespace hlsl
