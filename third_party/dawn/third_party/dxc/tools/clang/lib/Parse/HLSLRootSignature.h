//===--- HLSLRootSignature.h ---- HLSL root signature parsing -------------===//
///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// HLSLRootSignature.h                                                       //
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
// This file is distributed under the University of Illinois Open Source     //
// License. See LICENSE.TXT for details.                                     //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#pragma once

#include "dxc/DXIL/DXIL.h"
#include "dxc/DxilRootSignature/DxilRootSignature.h"
#include "dxc/Support/WinIncludes.h"

namespace llvm {
class raw_ostream;
}

namespace hlsl {
class RootSignatureTokenizer {
public:
  class Token {
  public:
    enum Type {
      Unknown,
      EOL,
      Comma,
      LParen,
      RParen,
      OR,
      EQ,
      NumberI32,
      NumberU32,
      NumberFloat,
      TReg,
      SReg,
      UReg,
      BReg,
      numDescriptors,
      unbounded,
      space,
      offset, // OffsetInDescriptorsFromTableStart
      DESCRIPTOR_RANGE_OFFSET_APPEND,
      RootConstants,
      num32BitConstants,
      Sampler,
      StaticSampler,
      CBV,
      SRV,
      UAV,
      DescriptorTable,

      flags,
      DESCRIPTORS_VOLATILE,
      DATA_VOLATILE,
      DATA_STATIC,
      DATA_STATIC_WHILE_SET_AT_EXECUTE,
      DESCRIPTORS_STATIC_KEEPING_BUFFER_BOUNDS_CHECKS,

      // Visibility
      visibility,
      SHADER_VISIBILITY_ALL,
      SHADER_VISIBILITY_VERTEX,
      SHADER_VISIBILITY_HULL,
      SHADER_VISIBILITY_DOMAIN,
      SHADER_VISIBILITY_GEOMETRY,
      SHADER_VISIBILITY_PIXEL,
      SHADER_VISIBILITY_AMPLIFICATION,
      SHADER_VISIBILITY_MESH,

      // Root signature flags
      RootFlags,
      ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT,
      DENY_VERTEX_SHADER_ROOT_ACCESS,
      DENY_HULL_SHADER_ROOT_ACCESS,
      DENY_DOMAIN_SHADER_ROOT_ACCESS,
      DENY_GEOMETRY_SHADER_ROOT_ACCESS,
      DENY_PIXEL_SHADER_ROOT_ACCESS,
      DENY_AMPLIFICATION_SHADER_ROOT_ACCESS,
      DENY_MESH_SHADER_ROOT_ACCESS,
      ALLOW_STREAM_OUTPUT,
      LOCAL_ROOT_SIGNATURE,
      CBV_SRV_UAV_HEAP_DIRECTLY_INDEXED,
      SAMPLER_HEAP_DIRECTLY_INDEXED,

      // Filter
      filter,
      FILTER_MIN_MAG_MIP_POINT,
      FILTER_MIN_MAG_POINT_MIP_LINEAR,
      FILTER_MIN_POINT_MAG_LINEAR_MIP_POINT,
      FILTER_MIN_POINT_MAG_MIP_LINEAR,
      FILTER_MIN_LINEAR_MAG_MIP_POINT,
      FILTER_MIN_LINEAR_MAG_POINT_MIP_LINEAR,
      FILTER_MIN_MAG_LINEAR_MIP_POINT,
      FILTER_MIN_MAG_MIP_LINEAR,
      FILTER_ANISOTROPIC,
      FILTER_COMPARISON_MIN_MAG_MIP_POINT,
      FILTER_COMPARISON_MIN_MAG_POINT_MIP_LINEAR,
      FILTER_COMPARISON_MIN_POINT_MAG_LINEAR_MIP_POINT,
      FILTER_COMPARISON_MIN_POINT_MAG_MIP_LINEAR,
      FILTER_COMPARISON_MIN_LINEAR_MAG_MIP_POINT,
      FILTER_COMPARISON_MIN_LINEAR_MAG_POINT_MIP_LINEAR,
      FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT,
      FILTER_COMPARISON_MIN_MAG_MIP_LINEAR,
      FILTER_COMPARISON_ANISOTROPIC,
      FILTER_MINIMUM_MIN_MAG_MIP_POINT,
      FILTER_MINIMUM_MIN_MAG_POINT_MIP_LINEAR,
      FILTER_MINIMUM_MIN_POINT_MAG_LINEAR_MIP_POINT,
      FILTER_MINIMUM_MIN_POINT_MAG_MIP_LINEAR,
      FILTER_MINIMUM_MIN_LINEAR_MAG_MIP_POINT,
      FILTER_MINIMUM_MIN_LINEAR_MAG_POINT_MIP_LINEAR,
      FILTER_MINIMUM_MIN_MAG_LINEAR_MIP_POINT,
      FILTER_MINIMUM_MIN_MAG_MIP_LINEAR,
      FILTER_MINIMUM_ANISOTROPIC,
      FILTER_MAXIMUM_MIN_MAG_MIP_POINT,
      FILTER_MAXIMUM_MIN_MAG_POINT_MIP_LINEAR,
      FILTER_MAXIMUM_MIN_POINT_MAG_LINEAR_MIP_POINT,
      FILTER_MAXIMUM_MIN_POINT_MAG_MIP_LINEAR,
      FILTER_MAXIMUM_MIN_LINEAR_MAG_MIP_POINT,
      FILTER_MAXIMUM_MIN_LINEAR_MAG_POINT_MIP_LINEAR,
      FILTER_MAXIMUM_MIN_MAG_LINEAR_MIP_POINT,
      FILTER_MAXIMUM_MIN_MAG_MIP_LINEAR,
      FILTER_MAXIMUM_ANISOTROPIC,

      // Texture address mode
      addressU,
      addressV,
      addressW,
      TEXTURE_ADDRESS_WRAP,
      TEXTURE_ADDRESS_MIRROR,
      TEXTURE_ADDRESS_CLAMP,
      TEXTURE_ADDRESS_BORDER,
      TEXTURE_ADDRESS_MIRROR_ONCE,

      mipLODBias,
      maxAnisotropy,

      // Comparison function
      comparisonFunc,
      COMPARISON_NEVER,
      COMPARISON_LESS,
      COMPARISON_EQUAL,
      COMPARISON_LESS_EQUAL,
      COMPARISON_GREATER,
      COMPARISON_NOT_EQUAL,
      COMPARISON_GREATER_EQUAL,
      COMPARISON_ALWAYS,

      // Static border color
      borderColor,
      STATIC_BORDER_COLOR_TRANSPARENT_BLACK,
      STATIC_BORDER_COLOR_OPAQUE_BLACK,
      STATIC_BORDER_COLOR_OPAQUE_WHITE,
      STATIC_BORDER_COLOR_OPAQUE_BLACK_UINT,
      STATIC_BORDER_COLOR_OPAQUE_WHITE_UINT,

      minLOD,
      maxLOD,
    };

  private:
    Type m_Type;
    const char *m_pStr;
    union {
      int m_I32Value;
      uint32_t m_U32Value;
      float m_FloatValue;
    };

  public:
    Token() : m_Type(Unknown), m_pStr(NULL), m_I32Value(0) {}
    Token(enum Type t, const char *pStr, uint32_t n = 0)
        : m_Type(t), m_pStr(pStr), m_U32Value(n) {}
    Token(enum Type t, const char *pStr, float f)
        : m_Type(t), m_pStr(pStr), m_FloatValue(f) {}
    Token(const Token &o)
        : m_Type(o.m_Type), m_pStr(o.m_pStr), m_U32Value(o.m_U32Value) {}
    Token &operator=(const Token &o) {
      if (this != &o) {
        m_Type = o.m_Type;
        m_pStr = o.m_pStr;
        m_U32Value = o.m_U32Value;
      }
      return *this;
    }

    enum Type GetType() const { return m_Type; }
    const char *GetStr() const { return m_pStr; }
    uint32_t GetU32Value() const { return m_U32Value; }
    int GetI32Value() const { return m_I32Value; }
    float GetFloatValue() const { return m_FloatValue; }
    bool IsEOL() const { return m_Type == Type::EOL; }
  };

public:
  RootSignatureTokenizer(const char *pStr);
  RootSignatureTokenizer(const char *pStr, size_t len);

  Token GetToken();
  Token PeekToken();

private:
  const char *m_pStrPos;
  const char *m_pEndPos;

  const static uint32_t kMaxTokenLength = 127;
  const static uint32_t kNumBuffers = 2;
  Token m_Tokens[kNumBuffers];
  char m_TokenStrings[kNumBuffers][kMaxTokenLength + 1];
  uint32_t m_TokenBufferIdx;

  void ReadNextToken(uint32_t BufferIdx);
  bool IsDone() const;

  void EatSpace();
  bool ToI32(const char *pBuf, int &n);
  bool ToU32(const char *pBuf, uint32_t &n);
  bool ToFloat(const char *pBuf, float &n);
  bool ToRegister(const char *pBuf, Token &T);
  bool ToKeyword(const char *pBuf, Token &T, const char *pKeyWord,
                 Token::Type Type);

  bool IsSeparator(char c) const;
  bool IsDigit(char c) const;
  bool IsAlpha(char c) const;
};

class RootSignatureParser {
public:
  RootSignatureParser(RootSignatureTokenizer *pTokenizer,
                      DxilRootSignatureVersion DefaultVersion,
                      DxilRootSignatureCompilationFlags Flags,
                      llvm::raw_ostream &OS);

  HRESULT Parse(DxilVersionedRootSignatureDesc **ppRootSignature);

private:
  typedef RootSignatureTokenizer::Token TokenType;

  RootSignatureTokenizer *m_pTokenizer;
  DxilRootSignatureVersion m_Version;
  DxilRootSignatureCompilationFlags m_CompilationFlags;
  llvm::raw_ostream &m_OS;

  HRESULT GetAndMatchToken(TokenType &Token, TokenType::Type Type);
  HRESULT Error(uint32_t uErrorNum, const char *pError, ...);

  HRESULT ParseRootSignature(DxilVersionedRootSignatureDesc **ppRootSignature);
  HRESULT ParseRootSignatureFlags(DxilRootSignatureFlags &Flags);
  HRESULT ParseRootConstants(DxilRootParameter1 &P);
  HRESULT ParseRootShaderResource(TokenType::Type TokType,
                                  TokenType::Type RegType,
                                  DxilRootParameterType ResType,
                                  DxilRootParameter1 &P);
  HRESULT ParseRootDescriptorTable(DxilRootParameter1 &P);
  HRESULT ParseStaticSampler(DxilStaticSamplerDesc &P);

  HRESULT ParseDescTableResource(TokenType::Type TokType,
                                 TokenType::Type RegType,
                                 DxilDescriptorRangeType RangeType,
                                 DxilDescriptorRange1 &R);

  HRESULT ParseRegister(TokenType::Type RegType, uint32_t &Reg);
  HRESULT ParseSpace(uint32_t &Space);
  HRESULT ParseNumDescriptors(uint32_t &NumDescriptors);
  HRESULT ParseRootDescFlags(DxilRootDescriptorFlags &Flags);
  HRESULT ParseDescRangeFlags(DxilDescriptorRangeType RangeType,
                              DxilDescriptorRangeFlags &Flags);
  HRESULT ParseOffset(uint32_t &Offset);
  HRESULT ParseVisibility(DxilShaderVisibility &Vis);
  HRESULT ParseNum32BitConstants(uint32_t &NumConst);

  HRESULT ParseFilter(DxilFilter &Filter);
  HRESULT ParseTextureAddressMode(DxilTextureAddressMode &AddressMode);
  HRESULT ParseMipLODBias(float &MipLODBias);
  HRESULT ParseMaxAnisotropy(uint32_t &MaxAnisotropy);
  HRESULT ParseComparisonFunction(DxilComparisonFunc &ComparisonFunc);
  HRESULT ParseBorderColor(DxilStaticBorderColor &BorderColor);
  HRESULT ParseMinLOD(float &MinLOD);
  HRESULT ParseMaxLOD(float &MaxLOD);

  HRESULT ParseFloat(float &v);

  HRESULT MarkParameter(bool &bSeen, const char *pName);
};

} // namespace hlsl
