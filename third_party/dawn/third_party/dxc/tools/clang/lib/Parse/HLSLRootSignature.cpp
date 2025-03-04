//===--- HLSLRootSignature.cpp -- HLSL root signature parsing -------------===//
///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// HLSLRootSignature.cpp                                                     //
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
// This file is distributed under the University of Illinois Open Source     //
// License. See LICENSE.TXT for details.                                     //
//                                                                           //
//  This file implements the parser for the root signature mini-language.    //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#include "HLSLRootSignature.h"
#include "dxc/Support/Global.h"
#include "dxc/Support/WinFunctions.h"
#include "dxc/Support/WinIncludes.h"
#include "clang/Basic/Diagnostic.h"
#include "clang/Parse/ParseDiagnostic.h"
#include "clang/Parse/ParseHLSL.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/Support/raw_ostream.h"

#include <float.h>

using namespace llvm;
using namespace hlsl;

// Error codes. These could be useful for localization at some point.
#define ERR_RS_UNEXPECTED_TOKEN 4612
#define ERR_RS_ROOTFLAGS_MORE_THAN_ONCE 4613
#define ERR_RS_CREATION_FAILED 4614
#define ERR_RS_BAD_SM 4615
#define ERR_RS_UNDEFINED_REGISTER 4616
#define ERR_RS_INCORRECT_REGISTER_TYPE 4617
#define ERR_RS_NOT_ALLOWED_FOR_HS_PATCH_CONST 4618
#define ERR_RS_WRONG_ROOT_DESC_FLAG 4619
#define ERR_RS_WRONG_DESC_RANGE_FLAG 4620
#define ERR_RS_LOCAL_FLAG_ON_GLOBAL 4621
#define ERR_RS_BAD_FLOAT 5000

// Non-throwing new
#define NEW new (std::nothrow)

DEFINE_ENUM_FLAG_OPERATORS(DxilRootSignatureFlags)
DEFINE_ENUM_FLAG_OPERATORS(DxilRootDescriptorFlags)
DEFINE_ENUM_FLAG_OPERATORS(DxilDescriptorRangeFlags)
DEFINE_ENUM_FLAG_OPERATORS(DxilRootSignatureCompilationFlags)

RootSignatureTokenizer::RootSignatureTokenizer(const char *pStr, size_t len)
    : m_pStrPos(pStr), m_pEndPos(pStr + len) {
  m_TokenBufferIdx = 0;
  ReadNextToken(m_TokenBufferIdx);
}

RootSignatureTokenizer::RootSignatureTokenizer(const char *pStr)
    : m_pStrPos(pStr), m_pEndPos(pStr + strlen(pStr)) {
  m_TokenBufferIdx = 0;
  ReadNextToken(m_TokenBufferIdx);
}

RootSignatureTokenizer::Token RootSignatureTokenizer::GetToken() {
  uint32_t CurBufferIdx = m_TokenBufferIdx;
  m_TokenBufferIdx = (m_TokenBufferIdx + 1) % kNumBuffers;
  ReadNextToken(m_TokenBufferIdx);
  return m_Tokens[CurBufferIdx];
}

RootSignatureTokenizer::Token RootSignatureTokenizer::PeekToken() {
  return m_Tokens[m_TokenBufferIdx];
}

bool RootSignatureTokenizer::IsDone() const { return m_pStrPos == m_pEndPos; }

void RootSignatureTokenizer::ReadNextToken(uint32_t BufferIdx) {
  char *pBuffer = m_TokenStrings[BufferIdx];
  Token &T = m_Tokens[BufferIdx];
  bool bFloat = false;
  bool bKW = false;
  char c = 0;

  EatSpace();

  // Identify token
  uint32_t TokLen = 0;
  char ch;
  // Any time m_pStrPos moves, update ch; no null termination assumptions made.
  // Test ch, not m_StrPos[0], which may be at end.
#define STRPOS_MOVED() ch = m_pStrPos == m_pEndPos ? '\0' : m_pStrPos[0];
  STRPOS_MOVED();
  if (IsSeparator(ch)) {
    pBuffer[TokLen++] = *m_pStrPos++;
  } else if (IsDigit(ch) || ch == '+' || ch == '-' || ch == '.') {
    if (ch == '+' || ch == '-') {
      pBuffer[TokLen++] = *m_pStrPos++;
      STRPOS_MOVED();
    }

    bool bSeenDigit = false;
    while (IsDigit(ch) && TokLen < kMaxTokenLength) {
      pBuffer[TokLen++] = *m_pStrPos++;
      STRPOS_MOVED();
      bSeenDigit = true;
    }

    if (ch == '.') {
      bFloat = true;
      pBuffer[TokLen++] = *m_pStrPos++;
      STRPOS_MOVED();
      if (!bSeenDigit && !IsDigit(ch)) {
        goto lUnknownToken;
      }
      while (IsDigit(ch) && TokLen < kMaxTokenLength) {
        pBuffer[TokLen++] = *m_pStrPos++;
        STRPOS_MOVED();
        bSeenDigit = true;
      }
    }

    if (!bSeenDigit) {
      goto lUnknownToken;
    }

    if (ch == 'e' || ch == 'E') {
      bFloat = true;
      pBuffer[TokLen++] = *m_pStrPos++;
      STRPOS_MOVED()
      if (ch == '+' || ch == '-') {
        pBuffer[TokLen++] = *m_pStrPos++;
        STRPOS_MOVED();
      }
      if (!IsDigit(ch)) {
        goto lUnknownToken;
      }
      while (IsDigit(ch) && TokLen < kMaxTokenLength) {
        pBuffer[TokLen++] = *m_pStrPos++;
        STRPOS_MOVED();
      }
    }

    if (ch == 'f' || ch == 'F') {
      bFloat = true;
      pBuffer[TokLen++] = *m_pStrPos++;
      STRPOS_MOVED();
    }
  } else if (IsAlpha(ch) || ch == '_') {
    while ((IsAlpha(ch) || ch == '_' || IsDigit(ch)) &&
           TokLen < kMaxTokenLength) {
      pBuffer[TokLen++] = *m_pStrPos++;
      STRPOS_MOVED();
    }
  } else {
    while (m_pStrPos < m_pEndPos && TokLen < kMaxTokenLength) {
      pBuffer[TokLen++] = *m_pStrPos++;
      STRPOS_MOVED(); // not really needed, but good for consistency
    }
  }
  pBuffer[TokLen] = '\0';
#undef STRPOS_MOVED

  //
  // Classify token
  //
  c = pBuffer[0];

  // Delimiters
  switch (c) {
  case '\0':
    T = Token(Token::Type::EOL, pBuffer);
    return;
  case ',':
    T = Token(Token::Type::Comma, pBuffer);
    return;
  case '(':
    T = Token(Token::Type::LParen, pBuffer);
    return;
  case ')':
    T = Token(Token::Type::RParen, pBuffer);
    return;
  case '=':
    T = Token(Token::Type::EQ, pBuffer);
    return;
  case '|':
    T = Token(Token::Type::OR, pBuffer);
    return;
  }

  // Number
  if (IsDigit(c) || c == '+' || c == '-' || c == '.') {
    if (bFloat) {
      float v = 0.f;
      if (ToFloat(pBuffer, v)) {
        T = Token(Token::Type::NumberFloat, pBuffer, v);
        return;
      }
    } else {
      if (c == '-') {
        int n = 0;
        if (ToI32(pBuffer, n)) {
          T = Token(Token::Type::NumberI32, pBuffer, (uint32_t)n);
          return;
        }
      } else {
        uint32_t n = 0;
        if (ToU32(pBuffer, n)) {
          T = Token(Token::Type::NumberU32, pBuffer, n);
          return;
        }
      }
    }

    goto lUnknownToken;
  }

  // Register
  if (IsDigit(pBuffer[1]) && (c == 't' || c == 's' || c == 'u' || c == 'b')) {
    if (ToRegister(pBuffer, T))
      return;
  }

  // Keyword
#define KW(__name) ToKeyword(pBuffer, T, #__name, Token::Type::__name)

  // Case-incensitive
  switch (toupper(c)) {
  case 'A':
    bKW = KW(ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT) || KW(ALLOW_STREAM_OUTPUT) ||
          KW(addressU) || KW(addressV) || KW(addressW);
    break;

  case 'B':
    bKW = KW(borderColor);
    break;

  case 'C':
    bKW = KW(CBV) || KW(comparisonFunc) || KW(COMPARISON_NEVER) ||
          KW(COMPARISON_LESS) || KW(COMPARISON_EQUAL) ||
          KW(COMPARISON_LESS_EQUAL) || KW(COMPARISON_GREATER) ||
          KW(COMPARISON_NOT_EQUAL) || KW(COMPARISON_GREATER_EQUAL) ||
          KW(COMPARISON_ALWAYS) || KW(CBV_SRV_UAV_HEAP_DIRECTLY_INDEXED);
    break;

  case 'D':
    bKW = KW(DescriptorTable) || KW(DESCRIPTOR_RANGE_OFFSET_APPEND) ||
          KW(DENY_VERTEX_SHADER_ROOT_ACCESS) ||
          KW(DENY_HULL_SHADER_ROOT_ACCESS) ||
          KW(DENY_DOMAIN_SHADER_ROOT_ACCESS) ||
          KW(DENY_GEOMETRY_SHADER_ROOT_ACCESS) ||
          KW(DENY_PIXEL_SHADER_ROOT_ACCESS) ||
          KW(DENY_AMPLIFICATION_SHADER_ROOT_ACCESS) ||
          KW(DENY_MESH_SHADER_ROOT_ACCESS) || KW(DESCRIPTORS_VOLATILE) ||
          KW(DATA_VOLATILE) || KW(DATA_STATIC) ||
          KW(DATA_STATIC_WHILE_SET_AT_EXECUTE) ||
          KW(DESCRIPTORS_STATIC_KEEPING_BUFFER_BOUNDS_CHECKS);
    break;

  case 'F':
    bKW =
        KW(flags) || KW(filter) || KW(FILTER_MIN_MAG_MIP_POINT) ||
        KW(FILTER_MIN_MAG_POINT_MIP_LINEAR) ||
        KW(FILTER_MIN_POINT_MAG_LINEAR_MIP_POINT) ||
        KW(FILTER_MIN_POINT_MAG_MIP_LINEAR) ||
        KW(FILTER_MIN_LINEAR_MAG_MIP_POINT) ||
        KW(FILTER_MIN_LINEAR_MAG_POINT_MIP_LINEAR) ||
        KW(FILTER_MIN_MAG_LINEAR_MIP_POINT) || KW(FILTER_MIN_MAG_MIP_LINEAR) ||
        KW(FILTER_ANISOTROPIC) || KW(FILTER_COMPARISON_MIN_MAG_MIP_POINT) ||
        KW(FILTER_COMPARISON_MIN_MAG_POINT_MIP_LINEAR) ||
        KW(FILTER_COMPARISON_MIN_POINT_MAG_LINEAR_MIP_POINT) ||
        KW(FILTER_COMPARISON_MIN_POINT_MAG_MIP_LINEAR) ||
        KW(FILTER_COMPARISON_MIN_LINEAR_MAG_MIP_POINT) ||
        KW(FILTER_COMPARISON_MIN_LINEAR_MAG_POINT_MIP_LINEAR) ||
        KW(FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT) ||
        KW(FILTER_COMPARISON_MIN_MAG_MIP_LINEAR) ||
        KW(FILTER_COMPARISON_ANISOTROPIC) ||
        KW(FILTER_MINIMUM_MIN_MAG_MIP_POINT) ||
        KW(FILTER_MINIMUM_MIN_MAG_POINT_MIP_LINEAR) ||
        KW(FILTER_MINIMUM_MIN_POINT_MAG_LINEAR_MIP_POINT) ||
        KW(FILTER_MINIMUM_MIN_POINT_MAG_MIP_LINEAR) ||
        KW(FILTER_MINIMUM_MIN_LINEAR_MAG_MIP_POINT) ||
        KW(FILTER_MINIMUM_MIN_LINEAR_MAG_POINT_MIP_LINEAR) ||
        KW(FILTER_MINIMUM_MIN_MAG_LINEAR_MIP_POINT) ||
        KW(FILTER_MINIMUM_MIN_MAG_MIP_LINEAR) ||
        KW(FILTER_MINIMUM_ANISOTROPIC) ||
        KW(FILTER_MAXIMUM_MIN_MAG_MIP_POINT) ||
        KW(FILTER_MAXIMUM_MIN_MAG_POINT_MIP_LINEAR) ||
        KW(FILTER_MAXIMUM_MIN_POINT_MAG_LINEAR_MIP_POINT) ||
        KW(FILTER_MAXIMUM_MIN_POINT_MAG_MIP_LINEAR) ||
        KW(FILTER_MAXIMUM_MIN_LINEAR_MAG_MIP_POINT) ||
        KW(FILTER_MAXIMUM_MIN_LINEAR_MAG_POINT_MIP_LINEAR) ||
        KW(FILTER_MAXIMUM_MIN_MAG_LINEAR_MIP_POINT) ||
        KW(FILTER_MAXIMUM_MIN_MAG_MIP_LINEAR) || KW(FILTER_MAXIMUM_ANISOTROPIC);
    break;

  case 'L':
    bKW = KW(LOCAL_ROOT_SIGNATURE);
    break;

  case 'M':
    bKW = KW(maxAnisotropy) || KW(mipLODBias) || KW(minLOD) || KW(maxLOD);
    break;

  case 'N':
    bKW = KW(numDescriptors) || KW(num32BitConstants);
    break;

  case 'O':
    bKW = KW(offset);
    break;

  case 'R':
    bKW = KW(RootFlags) || KW(RootConstants);
    break;

  case 'S':
    bKW = KW(space) || KW(Sampler) || KW(StaticSampler) || KW(SRV) ||
          KW(SAMPLER_HEAP_DIRECTLY_INDEXED) || KW(SHADER_VISIBILITY_ALL) ||
          KW(SHADER_VISIBILITY_VERTEX) || KW(SHADER_VISIBILITY_HULL) ||
          KW(SHADER_VISIBILITY_DOMAIN) || KW(SHADER_VISIBILITY_GEOMETRY) ||
          KW(SHADER_VISIBILITY_PIXEL) || KW(SHADER_VISIBILITY_AMPLIFICATION) ||
          KW(SHADER_VISIBILITY_MESH) ||
          KW(STATIC_BORDER_COLOR_TRANSPARENT_BLACK) ||
          KW(STATIC_BORDER_COLOR_OPAQUE_BLACK) ||
          KW(STATIC_BORDER_COLOR_OPAQUE_WHITE) ||
          KW(STATIC_BORDER_COLOR_OPAQUE_BLACK_UINT) ||
          KW(STATIC_BORDER_COLOR_OPAQUE_WHITE_UINT) ||
          KW(SAMPLER_HEAP_DIRECTLY_INDEXED);

    break;

  case 'T':
    bKW = KW(TEXTURE_ADDRESS_WRAP) || KW(TEXTURE_ADDRESS_MIRROR) ||
          KW(TEXTURE_ADDRESS_CLAMP) || KW(TEXTURE_ADDRESS_BORDER) ||
          KW(TEXTURE_ADDRESS_MIRROR_ONCE);
    break;

  case 'U':
    bKW = KW(unbounded) || KW(UAV);
    break;

  case 'V':
    bKW = KW(visibility);
    break;
  }
#undef KW

  if (!bKW)
    goto lUnknownToken;

  return;

lUnknownToken:
  T = Token(Token::Type::Unknown, pBuffer);
}

void RootSignatureTokenizer::EatSpace() {
  while (m_pStrPos < m_pEndPos && *m_pStrPos == ' ')
    m_pStrPos++;
}

bool RootSignatureTokenizer::ToI32(LPCSTR pBuf, int &n) {
  if (pBuf[0] == '\0')
    return false;

  long long N = _atoi64(pBuf);
  if (N > INT_MAX || N < INT_MIN)
    return false;

  n = static_cast<int>(N);
  return true;
}

bool RootSignatureTokenizer::ToU32(LPCSTR pBuf, uint32_t &n) {
  if (pBuf[0] == '\0')
    return false;

  long long N = _atoi64(pBuf);
  if (N > UINT32_MAX || N < 0)
    return false;

  n = static_cast<uint32_t>(N);
  return true;
}

bool RootSignatureTokenizer::ToFloat(LPCSTR pBuf, float &n) {
  if (pBuf[0] == '\0')
    return false;

  errno = 0;
  double N = strtod(pBuf, NULL);
  if (errno == ERANGE || (N > FLT_MAX || N < -FLT_MAX)) {
    return false;
  }

  n = static_cast<float>(N);
  return true;
}

bool RootSignatureTokenizer::ToRegister(LPCSTR pBuf, Token &T) {
  uint32_t n;
  if (ToU32(&pBuf[1], n)) {
    switch (pBuf[0]) {
    case 't':
      T = Token(Token::Type::TReg, pBuf, n);
      return true;
    case 's':
      T = Token(Token::Type::SReg, pBuf, n);
      return true;
    case 'u':
      T = Token(Token::Type::UReg, pBuf, n);
      return true;
    case 'b':
      T = Token(Token::Type::BReg, pBuf, n);
      return true;
    }
  }

  return false;
}

bool RootSignatureTokenizer::ToKeyword(LPCSTR pBuf, Token &T,
                                       const char *pKeyword, Token::Type Type) {
  // Tokens are case-insencitive to allow more flexibility for programmers
  if (_stricmp(pBuf, pKeyword) == 0) {
    T = Token(Type, pBuf);
    return true;
  } else {
    T = Token(Token::Type::Unknown, pBuf);
    return false;
  }
}

bool RootSignatureTokenizer::IsSeparator(char c) const {
  return (c == ',' || c == '=' || c == '|' || c == '(' || c == ')' ||
          c == ' ' || c == '\t' || c == '\n');
}

bool RootSignatureTokenizer::IsDigit(char c) const { return isdigit(c) > 0; }

bool RootSignatureTokenizer::IsAlpha(char c) const { return isalpha(c) > 0; }

RootSignatureParser::RootSignatureParser(
    RootSignatureTokenizer *pTokenizer, DxilRootSignatureVersion DefaultVersion,
    DxilRootSignatureCompilationFlags CompilationFlags, llvm::raw_ostream &OS)
    : m_pTokenizer(pTokenizer), m_Version(DefaultVersion),
      m_CompilationFlags(CompilationFlags), m_OS(OS) {}

HRESULT
RootSignatureParser::Parse(DxilVersionedRootSignatureDesc **ppRootSignature) {
  HRESULT hr = S_OK;
  DxilVersionedRootSignatureDesc *pRS = NULL;

  DXASSERT(!((bool)(m_CompilationFlags &
                    DxilRootSignatureCompilationFlags::GlobalRootSignature) &&
             (bool)(m_CompilationFlags &
                    DxilRootSignatureCompilationFlags::LocalRootSignature)),
           "global and local cannot be both set");

  if (ppRootSignature != NULL) {
    IFC(ParseRootSignature(&pRS));
    *ppRootSignature = pRS;
  }

Cleanup:
  return hr;
}

HRESULT RootSignatureParser::GetAndMatchToken(TokenType &Token,
                                              TokenType::Type Type) {
  HRESULT hr = S_OK;
  Token = m_pTokenizer->GetToken();
  if (Token.GetType() != Type)
    IFC(Error(ERR_RS_UNEXPECTED_TOKEN, "Unexpected token '%s'",
              Token.GetStr()));

Cleanup:
  return hr;
}

HRESULT RootSignatureParser::Error(uint32_t uErrorNum, LPCSTR pError, ...) {
  va_list Args;
  char msg[512];
  va_start(Args, pError);
  vsnprintf_s(msg, _countof(msg), pError, Args);
  va_end(Args);
  try {
    m_OS << msg;
  }
  CATCH_CPP_RETURN_HRESULT();
  return E_FAIL;
}

HRESULT RootSignatureParser::ParseRootSignature(
    DxilVersionedRootSignatureDesc **ppRootSignature) {
  HRESULT hr = S_OK;
  TokenType Token;
  bool bSeenFlags = false;
  SmallVector<DxilRootParameter1, 8> RSParameters;
  DxilRootParameter1 *pRSParams = NULL;
  SmallVector<DxilStaticSamplerDesc, 8> StaticSamplers;
  DxilStaticSamplerDesc *pStaticSamplers = NULL;
  DxilVersionedRootSignatureDesc *pRS = NULL;

  *ppRootSignature = NULL;
  IFCOOM(pRS = NEW DxilVersionedRootSignatureDesc);
  // Always parse root signature string to the latest version.
  pRS->Version = DxilRootSignatureVersion::Version_1_1;
  memset(&pRS->Desc_1_1, 0, sizeof(pRS->Desc_1_1));

  Token = m_pTokenizer->PeekToken();
  while (Token.GetType() != TokenType::EOL) {
    switch (Token.GetType()) {
    case TokenType::RootFlags:
      if (!bSeenFlags) {
        IFC(ParseRootSignatureFlags(pRS->Desc_1_1.Flags));
        bSeenFlags = true;
      } else {
        IFC(Error(ERR_RS_ROOTFLAGS_MORE_THAN_ONCE,
                  "RootFlags cannot be specified more than once"));
      }
      break;

    case TokenType::RootConstants: {
      DxilRootParameter1 P;
      IFC(ParseRootConstants(P));
      RSParameters.push_back(P);
      break;
    }

    case TokenType::CBV: {
      DxilRootParameter1 P;
      IFC(ParseRootShaderResource(Token.GetType(), TokenType::BReg,
                                  DxilRootParameterType::CBV, P));
      RSParameters.push_back(P);
      break;
    }

    case TokenType::SRV: {
      DxilRootParameter1 P;
      IFC(ParseRootShaderResource(Token.GetType(), TokenType::TReg,
                                  DxilRootParameterType::SRV, P));
      RSParameters.push_back(P);
      break;
    }

    case TokenType::UAV: {
      DxilRootParameter1 P;
      IFC(ParseRootShaderResource(Token.GetType(), TokenType::UReg,
                                  DxilRootParameterType::UAV, P));
      RSParameters.push_back(P);
      break;
    }

    case TokenType::DescriptorTable: {
      DxilRootParameter1 P;
      IFC(ParseRootDescriptorTable(P));
      RSParameters.push_back(P);
      break;
    }

    case TokenType::StaticSampler: {
      DxilStaticSamplerDesc P;
      IFC(ParseStaticSampler(P));
      StaticSamplers.push_back(P);
      break;
    }

    default:
      IFC(Error(ERR_RS_UNEXPECTED_TOKEN,
                "Unexpected token '%s' when parsing root signature",
                Token.GetStr()));
    }

    Token = m_pTokenizer->GetToken();
    if (Token.GetType() == TokenType::EOL)
      break;

    // Consume ','
    if (Token.GetType() != TokenType::Comma)
      IFC(Error(ERR_RS_UNEXPECTED_TOKEN, "Expected ',', found: '%s'",
                Token.GetStr()));

    Token = m_pTokenizer->PeekToken();
  }

  if (RSParameters.size() > 0) {
    IFCOOM(pRSParams = NEW DxilRootParameter1[RSParameters.size()]);
    pRS->Desc_1_1.NumParameters = RSParameters.size();
    memcpy(pRSParams, RSParameters.data(),
           pRS->Desc_1_1.NumParameters * sizeof(DxilRootParameter1));
    pRS->Desc_1_1.pParameters = pRSParams;
  }
  if (StaticSamplers.size() > 0) {
    IFCOOM(pStaticSamplers = NEW DxilStaticSamplerDesc[StaticSamplers.size()]);
    pRS->Desc_1_1.NumStaticSamplers = StaticSamplers.size();
    memcpy(pStaticSamplers, StaticSamplers.data(),
           pRS->Desc_1_1.NumStaticSamplers * sizeof(DxilStaticSamplerDesc));
    pRS->Desc_1_1.pStaticSamplers = pStaticSamplers;
  }

  // Set local signature flag if not already on
  if ((bool)(m_CompilationFlags &
             DxilRootSignatureCompilationFlags::LocalRootSignature))
    pRS->Desc_1_1.Flags |= DxilRootSignatureFlags::LocalRootSignature;

  // Down-convert root signature to the right version, if needed.
  if (pRS->Version != m_Version) {
    DxilVersionedRootSignatureDesc *pRS1 = NULL;
    try {
      hlsl::ConvertRootSignature(
          pRS, m_Version,
          const_cast<const DxilVersionedRootSignatureDesc **>(&pRS1));
    }
    CATCH_CPP_ASSIGN_HRESULT();
    IFC(hr);
    hlsl::DeleteRootSignature(pRS);
    pRS = pRS1;
  }

  *ppRootSignature = pRS;

Cleanup:
  if (FAILED(hr)) {
    hlsl::DeleteRootSignature(pRS);
  }
  return hr;
}

HRESULT
RootSignatureParser::ParseRootSignatureFlags(DxilRootSignatureFlags &Flags) {
  // RootFlags(ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
  // DENY_VERTEX_SHADER_ROOT_ACCESS)
  //   ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT
  //   DENY_VERTEX_SHADER_ROOT_ACCESS
  //   DENY_HULL_SHADER_ROOT_ACCESS
  //   DENY_DOMAIN_SHADER_ROOT_ACCESS
  //   DENY_GEOMETRY_SHADER_ROOT_ACCESS
  //   DENY_PIXEL_SHADER_ROOT_ACCESS
  //   DENY_AMPLIFICATION_SHADER_ROOT_ACCESS
  //   DENY_MESH_SHADER_ROOT_ACCESS
  //   ALLOW_STREAM_OUTPUT
  //   LOCAL_ROOT_SIGNATURE

  HRESULT hr = S_OK;
  TokenType Token;

  IFC(GetAndMatchToken(Token, TokenType::RootFlags));
  IFC(GetAndMatchToken(Token, TokenType::LParen));

  Flags = DxilRootSignatureFlags::None;

  Token = m_pTokenizer->PeekToken();
  if (Token.GetType() == TokenType::NumberU32) {
    IFC(GetAndMatchToken(Token, TokenType::NumberU32));
    uint32_t n = Token.GetU32Value();
    if (n != 0) {
      IFC(Error(ERR_RS_UNEXPECTED_TOKEN,
                "Root signature flag values can only be 0 or flag enum values, "
                "found: '%s'",
                Token.GetStr()));
    }
  } else {
    for (;;) {
      Token = m_pTokenizer->GetToken();
      switch (Token.GetType()) {
      case TokenType::ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT:
        Flags |= DxilRootSignatureFlags::AllowInputAssemblerInputLayout;
        break;
      case TokenType::DENY_VERTEX_SHADER_ROOT_ACCESS:
        Flags |= DxilRootSignatureFlags::DenyVertexShaderRootAccess;
        break;
      case TokenType::DENY_HULL_SHADER_ROOT_ACCESS:
        Flags |= DxilRootSignatureFlags::DenyHullShaderRootAccess;
        break;
      case TokenType::DENY_DOMAIN_SHADER_ROOT_ACCESS:
        Flags |= DxilRootSignatureFlags::DenyDomainShaderRootAccess;
        break;
      case TokenType::DENY_GEOMETRY_SHADER_ROOT_ACCESS:
        Flags |= DxilRootSignatureFlags::DenyGeometryShaderRootAccess;
        break;
      case TokenType::DENY_PIXEL_SHADER_ROOT_ACCESS:
        Flags |= DxilRootSignatureFlags::DenyPixelShaderRootAccess;
        break;
      case TokenType::DENY_AMPLIFICATION_SHADER_ROOT_ACCESS:
        Flags |= DxilRootSignatureFlags::DenyAmplificationShaderRootAccess;
        break;
      case TokenType::DENY_MESH_SHADER_ROOT_ACCESS:
        Flags |= DxilRootSignatureFlags::DenyMeshShaderRootAccess;
        break;
      case TokenType::ALLOW_STREAM_OUTPUT:
        Flags |= DxilRootSignatureFlags::AllowStreamOutput;
        break;
      case TokenType::LOCAL_ROOT_SIGNATURE:
        if ((bool)(m_CompilationFlags &
                   DxilRootSignatureCompilationFlags::GlobalRootSignature))
          IFC(Error(ERR_RS_LOCAL_FLAG_ON_GLOBAL,
                    "LOCAL_ROOT_SIGNATURE flag used in global root signature"));
        Flags |= DxilRootSignatureFlags::LocalRootSignature;
        break;
      case TokenType::CBV_SRV_UAV_HEAP_DIRECTLY_INDEXED:
        Flags |= DxilRootSignatureFlags::CBVSRVUAVHeapDirectlyIndexed;
        break;
      case TokenType::SAMPLER_HEAP_DIRECTLY_INDEXED:
        Flags |= DxilRootSignatureFlags::SamplerHeapDirectlyIndexed;
        break;
      default:
        IFC(Error(ERR_RS_UNEXPECTED_TOKEN,
                  "Expected a root signature flag value, found: '%s'",
                  Token.GetStr()));
      }

      Token = m_pTokenizer->PeekToken();
      if (Token.GetType() == TokenType::RParen)
        break;

      IFC(GetAndMatchToken(Token, TokenType::OR));
    }
  }

  IFC(GetAndMatchToken(Token, TokenType::RParen));

Cleanup:
  return hr;
}

HRESULT RootSignatureParser::ParseRootConstants(DxilRootParameter1 &P) {
  //"RootConstants(num32BitConstants=3, b2 [, space=1,
  //visibility=SHADER_VISIBILITY_ALL ] ), "
  HRESULT hr = S_OK;
  TokenType Token;
  memset(&P, 0, sizeof(P));
  DXASSERT(P.ShaderVisibility == DxilShaderVisibility::All,
           "else default isn't zero");
  P.ParameterType = DxilRootParameterType::Constants32Bit;
  bool bSeenNum32BitConstants = false;
  bool bSeenBReg = false;
  bool bSeenSpace = false;
  bool bSeenVisibility = false;

  IFC(GetAndMatchToken(Token, TokenType::RootConstants));
  IFC(GetAndMatchToken(Token, TokenType::LParen));

  for (;;) {
    Token = m_pTokenizer->PeekToken();

    switch (Token.GetType()) {
    case TokenType::num32BitConstants:
      IFC(MarkParameter(bSeenNum32BitConstants, "num32BitConstants"));
      IFC(ParseNum32BitConstants(P.Constants.Num32BitValues));
      break;
    case TokenType::BReg:
      IFC(MarkParameter(bSeenBReg, "cbuffer register b#"));
      IFC(ParseRegister(TokenType::BReg, P.Constants.ShaderRegister));
      break;
    case TokenType::space:
      IFC(MarkParameter(bSeenSpace, "space"));
      IFC(ParseSpace(P.Constants.RegisterSpace));
      break;
    case TokenType::visibility:
      IFC(MarkParameter(bSeenVisibility, "visibility"));
      IFC(ParseVisibility(P.ShaderVisibility));
      break;
    default:
      IFC(Error(ERR_RS_UNEXPECTED_TOKEN, "Unexpected token '%s'",
                Token.GetStr()));
      break;
    }

    Token = m_pTokenizer->GetToken();
    if (Token.GetType() == TokenType::RParen)
      break;
    else if (Token.GetType() != TokenType::Comma)
      IFC(Error(ERR_RS_UNEXPECTED_TOKEN, "Unexpected token '%s'",
                Token.GetStr()));
  }

  if (!bSeenNum32BitConstants) {
    IFC(Error(ERR_RS_UNDEFINED_REGISTER,
              "num32BitConstants must be defined for each RootConstants"));
  }
  if (!bSeenBReg) {
    IFC(Error(
        ERR_RS_UNDEFINED_REGISTER,
        "Constant buffer register b# must be defined for each RootConstants"));
  }

Cleanup:
  return hr;
}

HRESULT RootSignatureParser::ParseRootShaderResource(
    TokenType::Type TokType, TokenType::Type RegType,
    DxilRootParameterType ResType, DxilRootParameter1 &P) {
  // CBV(b0 [, space=3, flags=0, visibility=VISIBILITY_ALL] )
  HRESULT hr = S_OK;
  TokenType Token;
  P.ParameterType = ResType;
  P.ShaderVisibility = DxilShaderVisibility::All;
  P.Descriptor.ShaderRegister = 0;
  P.Descriptor.RegisterSpace = 0;
  P.Descriptor.Flags = DxilRootDescriptorFlags::None;
  bool bSeenReg = false;
  bool bSeenFlags = false;
  bool bSeenSpace = false;
  bool bSeenVisibility = false;

  IFC(GetAndMatchToken(Token, TokType));
  IFC(GetAndMatchToken(Token, TokenType::LParen));

  for (;;) {
    Token = m_pTokenizer->PeekToken();

    switch (Token.GetType()) {
    case TokenType::BReg:
    case TokenType::TReg:
    case TokenType::UReg:
      IFC(MarkParameter(bSeenReg, "shader register"));
      IFC(ParseRegister(RegType, P.Descriptor.ShaderRegister));
      break;
    case TokenType::flags:
      IFC(MarkParameter(bSeenFlags, "flags"));
      IFC(ParseRootDescFlags(P.Descriptor.Flags));
      break;
    case TokenType::space:
      IFC(MarkParameter(bSeenSpace, "space"));
      IFC(ParseSpace(P.Descriptor.RegisterSpace));
      break;
    case TokenType::visibility:
      IFC(MarkParameter(bSeenVisibility, "visibility"));
      IFC(ParseVisibility(P.ShaderVisibility));
      break;
    default:
      IFC(Error(ERR_RS_UNEXPECTED_TOKEN, "Unexpected token '%s'",
                Token.GetStr()));
      break;
    }

    Token = m_pTokenizer->GetToken();
    if (Token.GetType() == TokenType::RParen)
      break;
    else if (Token.GetType() != TokenType::Comma)
      IFC(Error(ERR_RS_UNEXPECTED_TOKEN, "Unexpected token '%s'",
                Token.GetStr()));
  }

  if (!bSeenReg) {
    IFC(Error(ERR_RS_UNDEFINED_REGISTER,
              "shader register must be defined for each CBV/SRV/UAV"));
  }

Cleanup:
  return hr;
}

HRESULT RootSignatureParser::ParseRootDescriptorTable(DxilRootParameter1 &P) {
  // DescriptorTable( SRV(t2, numDescriptors = 6), UAV(u0, numDescriptors = 4)
  // [, visibility = SHADER_VISIBILITY_ALL ] )
  HRESULT hr = S_OK;
  TokenType Token;
  memset(&P, 0, sizeof(P));
  DXASSERT(P.ShaderVisibility == DxilShaderVisibility::All,
           "else default isn't zero");
  P.ParameterType = DxilRootParameterType::DescriptorTable;
  bool bSeenVisibility = false;
  DxilDescriptorRange1 *pDescs = NULL;
  SmallVector<DxilDescriptorRange1, 4> Ranges;

  IFC(GetAndMatchToken(Token, TokenType::DescriptorTable));
  IFC(GetAndMatchToken(Token, TokenType::LParen));

  for (;;) {
    Token = m_pTokenizer->PeekToken();

    switch (Token.GetType()) {
    case TokenType::CBV: {
      DxilDescriptorRange1 R;
      IFC(ParseDescTableResource(Token.GetType(), TokenType::BReg,
                                 DxilDescriptorRangeType::CBV, R));
      Ranges.push_back(R);
      break;
    }
    case TokenType::SRV: {
      DxilDescriptorRange1 R;
      IFC(ParseDescTableResource(Token.GetType(), TokenType::TReg,
                                 DxilDescriptorRangeType::SRV, R));
      Ranges.push_back(R);
      break;
    }
    case TokenType::UAV: {
      DxilDescriptorRange1 R;
      IFC(ParseDescTableResource(Token.GetType(), TokenType::UReg,
                                 DxilDescriptorRangeType::UAV, R));
      Ranges.push_back(R);
      break;
    }
    case TokenType::Sampler: {
      DxilDescriptorRange1 R;
      IFC(ParseDescTableResource(Token.GetType(), TokenType::SReg,
                                 DxilDescriptorRangeType::Sampler, R));
      Ranges.push_back(R);
      break;
    }
    case TokenType::visibility:
      IFC(MarkParameter(bSeenVisibility, "visibility"));
      IFC(ParseVisibility(P.ShaderVisibility));
      break;
    default:
      IFC(Error(ERR_RS_UNEXPECTED_TOKEN, "Unexpected token '%s'",
                Token.GetStr()));
      break;
    }

    Token = m_pTokenizer->GetToken();
    if (Token.GetType() == TokenType::RParen)
      break;
    else if (Token.GetType() != TokenType::Comma)
      IFC(Error(ERR_RS_UNEXPECTED_TOKEN, "Unexpected token '%s'",
                Token.GetStr()));
  }

  if (Ranges.size() > 0) {
    IFCOOM(pDescs = NEW DxilDescriptorRange1[Ranges.size()]);
    for (uint32_t i = 0; i < Ranges.size(); i++) {
      pDescs[i] = Ranges[i];
    }
    P.DescriptorTable.pDescriptorRanges = pDescs;
    P.DescriptorTable.NumDescriptorRanges = Ranges.size();
  }

Cleanup:
  if (FAILED(hr)) {
    delete[] pDescs;
  }
  return hr;
}

HRESULT RootSignatureParser::ParseDescTableResource(
    TokenType::Type TokType, TokenType::Type RegType,
    DxilDescriptorRangeType RangeType, DxilDescriptorRange1 &R) {
  HRESULT hr = S_OK;
  TokenType Token;
  // CBV(b0 [, numDescriptors = 1, space=0, flags=0, offset =
  // DESCRIPTOR_RANGE_OFFSET_APPEND] )

  R.RangeType = RangeType;
  R.NumDescriptors = 1;
  R.BaseShaderRegister = 0;
  R.RegisterSpace = 0;
  R.Flags = DxilDescriptorRangeFlags::None;
  R.OffsetInDescriptorsFromTableStart = DxilDescriptorRangeOffsetAppend;
  bool bSeenReg = false;
  bool bSeenNumDescriptors = false;
  bool bSeenSpace = false;
  bool bSeenFlags = false;
  bool bSeenOffset = false;

  IFC(GetAndMatchToken(Token, TokType));
  IFC(GetAndMatchToken(Token, TokenType::LParen));

  for (;;) {
    Token = m_pTokenizer->PeekToken();

    switch (Token.GetType()) {
    case TokenType::BReg:
    case TokenType::TReg:
    case TokenType::UReg:
    case TokenType::SReg:
      IFC(MarkParameter(bSeenReg, "shader register"));
      IFC(ParseRegister(RegType, R.BaseShaderRegister));
      break;
    case TokenType::numDescriptors:
      IFC(MarkParameter(bSeenNumDescriptors, "numDescriptors"));
      IFC(ParseNumDescriptors(R.NumDescriptors));
      break;
    case TokenType::space:
      IFC(MarkParameter(bSeenSpace, "space"));
      IFC(ParseSpace(R.RegisterSpace));
      break;
    case TokenType::flags:
      IFC(MarkParameter(bSeenFlags, "flags"));
      IFC(ParseDescRangeFlags(RangeType, R.Flags));
      break;
    case TokenType::offset:
      IFC(MarkParameter(bSeenOffset, "offset"));
      IFC(ParseOffset(R.OffsetInDescriptorsFromTableStart));
      break;
    default:
      IFC(Error(ERR_RS_UNEXPECTED_TOKEN, "Unexpected token '%s'",
                Token.GetStr()));
      break;
    }

    Token = m_pTokenizer->GetToken();
    if (Token.GetType() == TokenType::RParen)
      break;
    else if (Token.GetType() != TokenType::Comma)
      IFC(Error(ERR_RS_UNEXPECTED_TOKEN, "Unexpected token '%s'",
                Token.GetStr()));
  }

  if (!bSeenReg) {
    IFC(Error(ERR_RS_UNDEFINED_REGISTER,
              "shader register must be defined for each CBV/SRV/UAV"));
  }

Cleanup:
  return hr;
}

HRESULT RootSignatureParser::ParseRegister(TokenType::Type RegType,
                                           uint32_t &Reg) {
  HRESULT hr = S_OK;
  TokenType Token = m_pTokenizer->PeekToken();

  switch (Token.GetType()) {
  case TokenType::BReg:
    IFC(GetAndMatchToken(Token, TokenType::BReg));
    break;
  case TokenType::TReg:
    IFC(GetAndMatchToken(Token, TokenType::TReg));
    break;
  case TokenType::UReg:
    IFC(GetAndMatchToken(Token, TokenType::UReg));
    break;
  case TokenType::SReg:
    IFC(GetAndMatchToken(Token, TokenType::SReg));
    break;
  default:
    IFC(Error(ERR_RS_UNEXPECTED_TOKEN,
              "Expected a register token (CBV, SRV, UAV, Sampler), found: '%s'",
              Token.GetStr()));
  }

  if (Token.GetType() != RegType) {
    switch (RegType) {
    case TokenType::BReg:
      IFC(Error(ERR_RS_INCORRECT_REGISTER_TYPE,
                "Incorrect register type '%s' in CBV (expected b#)",
                Token.GetStr()));
      break;
    case TokenType::TReg:
      IFC(Error(ERR_RS_INCORRECT_REGISTER_TYPE,
                "Incorrect register type '%s' in SRV (expected t#)",
                Token.GetStr()));
      break;
    case TokenType::UReg:
      IFC(Error(ERR_RS_INCORRECT_REGISTER_TYPE,
                "Incorrect register type '%s' in UAV (expected u#)",
                Token.GetStr()));
      break;
    case TokenType::SReg:
      IFC(Error(
          ERR_RS_INCORRECT_REGISTER_TYPE,
          "Incorrect register type '%s' in Sampler/StaticSampler (expected s#)",
          Token.GetStr()));
      break;
    default:
      // Only Register types are relevant.
      break;
    }
  }

  Reg = Token.GetU32Value();

Cleanup:
  return hr;
}

HRESULT RootSignatureParser::ParseSpace(uint32_t &Space) {
  HRESULT hr = S_OK;
  TokenType Token;

  IFC(GetAndMatchToken(Token, TokenType::space));
  IFC(GetAndMatchToken(Token, TokenType::EQ));
  IFC(GetAndMatchToken(Token, TokenType::NumberU32));
  Space = Token.GetU32Value();

Cleanup:
  return hr;
}

HRESULT RootSignatureParser::ParseNumDescriptors(uint32_t &NumDescriptors) {
  HRESULT hr = S_OK;
  TokenType Token;

  IFC(GetAndMatchToken(Token, TokenType::numDescriptors));
  IFC(GetAndMatchToken(Token, TokenType::EQ));
  Token = m_pTokenizer->PeekToken();
  if (Token.GetType() == TokenType::unbounded) {
    IFC(GetAndMatchToken(Token, TokenType::unbounded));
    NumDescriptors = UINT32_MAX;
  } else {
    IFC(GetAndMatchToken(Token, TokenType::NumberU32));
    NumDescriptors = Token.GetU32Value();
  }

Cleanup:
  return hr;
}

HRESULT
RootSignatureParser::ParseRootDescFlags(DxilRootDescriptorFlags &Flags) {
  // flags=DATA_VOLATILE | DATA_STATIC | DATA_STATIC_WHILE_SET_AT_EXECUTE

  HRESULT hr = S_OK;
  TokenType Token;

  if (m_Version == DxilRootSignatureVersion::Version_1_0) {
    IFC(Error(ERR_RS_WRONG_ROOT_DESC_FLAG,
              "Root descriptor flags cannot be specified for root_sig_1_0"));
  }

  IFC(GetAndMatchToken(Token, TokenType::flags));
  IFC(GetAndMatchToken(Token, TokenType::EQ));

  Flags = DxilRootDescriptorFlags::None;

  Token = m_pTokenizer->PeekToken();
  if (Token.GetType() == TokenType::NumberU32) {
    IFC(GetAndMatchToken(Token, TokenType::NumberU32));
    uint32_t n = Token.GetU32Value();
    if (n != 0) {
      IFC(Error(ERR_RS_UNEXPECTED_TOKEN,
                "Root descriptor flag values can only be 0 or flag enum "
                "values, found: '%s'",
                Token.GetStr()));
    }
  } else {
    for (;;) {
      Token = m_pTokenizer->GetToken();
      switch (Token.GetType()) {
      case TokenType::DATA_VOLATILE:
        Flags |= DxilRootDescriptorFlags::DataVolatile;
        break;
      case TokenType::DATA_STATIC:
        Flags |= DxilRootDescriptorFlags::DataStatic;
        break;
      case TokenType::DATA_STATIC_WHILE_SET_AT_EXECUTE:
        Flags |= DxilRootDescriptorFlags::DataStaticWhileSetAtExecute;
        break;
      default:
        IFC(Error(ERR_RS_UNEXPECTED_TOKEN,
                  "Expected a root descriptor flag value, found: '%s'",
                  Token.GetStr()));
      }

      Token = m_pTokenizer->PeekToken();
      if (Token.GetType() == TokenType::RParen ||
          Token.GetType() == TokenType::Comma)
        break;

      IFC(GetAndMatchToken(Token, TokenType::OR));
    }
  }

Cleanup:
  return hr;
}

HRESULT
RootSignatureParser::ParseDescRangeFlags(DxilDescriptorRangeType,
                                         DxilDescriptorRangeFlags &Flags) {
  // flags=DESCRIPTORS_VOLATILE | DATA_VOLATILE | DATA_STATIC |
  // DATA_STATIC_WHILE_SET_AT_EXECUTE |
  // DESCRIPTORS_STATIC_KEEPING_BUFFER_BOUNDS_CHECKS

  HRESULT hr = S_OK;
  TokenType Token;

  if (m_Version == DxilRootSignatureVersion::Version_1_0) {
    IFC(Error(ERR_RS_WRONG_DESC_RANGE_FLAG,
              "Descriptor range flags cannot be specified for root_sig_1_0"));
  }

  IFC(GetAndMatchToken(Token, TokenType::flags));
  IFC(GetAndMatchToken(Token, TokenType::EQ));

  Flags = DxilDescriptorRangeFlags::None;

  Token = m_pTokenizer->PeekToken();
  if (Token.GetType() == TokenType::NumberU32) {
    IFC(GetAndMatchToken(Token, TokenType::NumberU32));
    uint32_t n = Token.GetU32Value();
    if (n != 0) {
      IFC(Error(ERR_RS_UNEXPECTED_TOKEN,
                "Descriptor range flag values can only be 0 or flag enum "
                "values, found: '%s'",
                Token.GetStr()));
    }
  } else {
    for (;;) {
      Token = m_pTokenizer->GetToken();
      switch (Token.GetType()) {
      case TokenType::DESCRIPTORS_VOLATILE:
        Flags |= DxilDescriptorRangeFlags::DescriptorsVolatile;
        break;
      case TokenType::DATA_VOLATILE:
        Flags |= DxilDescriptorRangeFlags::DataVolatile;
        break;
      case TokenType::DATA_STATIC:
        Flags |= DxilDescriptorRangeFlags::DataStatic;
        break;
      case TokenType::DATA_STATIC_WHILE_SET_AT_EXECUTE:
        Flags |= DxilDescriptorRangeFlags::DataStaticWhileSetAtExecute;
        break;
      case TokenType::DESCRIPTORS_STATIC_KEEPING_BUFFER_BOUNDS_CHECKS:
        Flags |= DxilDescriptorRangeFlags::
            DescriptorsStaticKeepingBufferBoundsChecks;
        break;
      default:
        IFC(Error(ERR_RS_UNEXPECTED_TOKEN,
                  "Expected a descriptor range flag value, found: '%s'",
                  Token.GetStr()));
      }

      Token = m_pTokenizer->PeekToken();
      if (Token.GetType() == TokenType::RParen ||
          Token.GetType() == TokenType::Comma)
        break;

      IFC(GetAndMatchToken(Token, TokenType::OR));
    }
  }

Cleanup:
  return hr;
}

HRESULT RootSignatureParser::ParseOffset(uint32_t &Offset) {
  HRESULT hr = S_OK;
  TokenType Token;

  IFC(GetAndMatchToken(Token, TokenType::offset));
  IFC(GetAndMatchToken(Token, TokenType::EQ));
  Token = m_pTokenizer->PeekToken();
  if (Token.GetType() == TokenType::DESCRIPTOR_RANGE_OFFSET_APPEND) {
    IFC(GetAndMatchToken(Token, TokenType::DESCRIPTOR_RANGE_OFFSET_APPEND));
    Offset = DxilDescriptorRangeOffsetAppend;
  } else {
    IFC(GetAndMatchToken(Token, TokenType::NumberU32));
    Offset = Token.GetU32Value();
  }

Cleanup:
  return hr;
}

HRESULT RootSignatureParser::ParseVisibility(DxilShaderVisibility &Vis) {
  HRESULT hr = S_OK;
  TokenType Token;

  IFC(GetAndMatchToken(Token, TokenType::visibility));
  IFC(GetAndMatchToken(Token, TokenType::EQ));
  Token = m_pTokenizer->GetToken();

  switch (Token.GetType()) {
  case TokenType::SHADER_VISIBILITY_ALL:
    Vis = DxilShaderVisibility::All;
    break;
  case TokenType::SHADER_VISIBILITY_VERTEX:
    Vis = DxilShaderVisibility::Vertex;
    break;
  case TokenType::SHADER_VISIBILITY_HULL:
    Vis = DxilShaderVisibility::Hull;
    break;
  case TokenType::SHADER_VISIBILITY_DOMAIN:
    Vis = DxilShaderVisibility::Domain;
    break;
  case TokenType::SHADER_VISIBILITY_GEOMETRY:
    Vis = DxilShaderVisibility::Geometry;
    break;
  case TokenType::SHADER_VISIBILITY_PIXEL:
    Vis = DxilShaderVisibility::Pixel;
    break;
  case TokenType::SHADER_VISIBILITY_AMPLIFICATION:
    Vis = DxilShaderVisibility::Amplification;
    break;
  case TokenType::SHADER_VISIBILITY_MESH:
    Vis = DxilShaderVisibility::Mesh;
    break;
  default:
    IFC(Error(ERR_RS_UNEXPECTED_TOKEN, "Unexpected visibility value: '%s'.",
              Token.GetStr()));
  }

Cleanup:
  return hr;
}

HRESULT RootSignatureParser::ParseNum32BitConstants(uint32_t &NumConst) {
  HRESULT hr = S_OK;
  TokenType Token;

  IFC(GetAndMatchToken(Token, TokenType::num32BitConstants));
  IFC(GetAndMatchToken(Token, TokenType::EQ));
  IFC(GetAndMatchToken(Token, TokenType::NumberU32));
  NumConst = Token.GetU32Value();

Cleanup:
  return hr;
}

HRESULT RootSignatureParser::ParseStaticSampler(DxilStaticSamplerDesc &P) {
  // StaticSampler( s0,
  //                [ Filter = FILTER_ANISOTROPIC,
  //                  AddressU = TEXTURE_ADDRESS_WRAP,
  //                  AddressV = TEXTURE_ADDRESS_WRAP,
  //                  AddressW = TEXTURE_ADDRESS_WRAP,
  //                  MipLODBias = 0,
  //                  MaxAnisotropy = 16,
  //                  ComparisonFunc = COMPARISON_LESS_EQUAL,
  //                  BorderColor = STATIC_BORDER_COLOR_OPAQUE_WHITE,
  //                  MinLOD = 0.f,
  //                  MaxLOD = 3.402823466e+38f
  //                  space = 0,
  //                  visibility = SHADER_VISIBILITY_ALL ] )
  HRESULT hr = S_OK;
  TokenType Token;
  memset(&P, 0, sizeof(P));
  P.Filter = DxilFilter::ANISOTROPIC;
  P.AddressU = P.AddressV = P.AddressW = DxilTextureAddressMode::Wrap;
  P.MaxAnisotropy = 16;
  P.ComparisonFunc = DxilComparisonFunc::LessEqual;
  P.BorderColor = DxilStaticBorderColor::OpaqueWhite;
  P.MaxLOD = DxilFloat32Max;
  bool bSeenFilter = false;
  bool bSeenAddressU = false;
  bool bSeenAddressV = false;
  bool bSeenAddressW = false;
  bool bSeenMipLODBias = false;
  bool bSeenMaxAnisotropy = false;
  bool bSeenComparisonFunc = false;
  bool bSeenBorderColor = false;
  bool bSeenMinLOD = false;
  bool bSeenMaxLOD = false;
  bool bSeenSReg = false;
  bool bSeenSpace = false;
  bool bSeenVisibility = false;

  IFC(GetAndMatchToken(Token, TokenType::StaticSampler));
  IFC(GetAndMatchToken(Token, TokenType::LParen));

  for (;;) {
    Token = m_pTokenizer->PeekToken();

    switch (Token.GetType()) {
    case TokenType::filter:
      IFC(MarkParameter(bSeenFilter, "filter"));
      IFC(ParseFilter(P.Filter));
      break;
    case TokenType::addressU:
      IFC(MarkParameter(bSeenAddressU, "addressU"));
      IFC(ParseTextureAddressMode(P.AddressU));
      break;
    case TokenType::addressV:
      IFC(MarkParameter(bSeenAddressV, "addressV"));
      IFC(ParseTextureAddressMode(P.AddressV));
      break;
    case TokenType::addressW:
      IFC(MarkParameter(bSeenAddressW, "addressW"));
      IFC(ParseTextureAddressMode(P.AddressW));
      break;
    case TokenType::mipLODBias:
      IFC(MarkParameter(bSeenMipLODBias, "mipLODBias"));
      IFC(ParseMipLODBias(P.MipLODBias));
      break;
    case TokenType::maxAnisotropy:
      IFC(MarkParameter(bSeenMaxAnisotropy, "maxAnisotropy"));
      IFC(ParseMaxAnisotropy(P.MaxAnisotropy));
      break;
    case TokenType::comparisonFunc:
      IFC(MarkParameter(bSeenComparisonFunc, "comparisonFunc"));
      IFC(ParseComparisonFunction(P.ComparisonFunc));
      break;
    case TokenType::borderColor:
      IFC(MarkParameter(bSeenBorderColor, "borderColor"));
      IFC(ParseBorderColor(P.BorderColor));
      break;
    case TokenType::minLOD:
      IFC(MarkParameter(bSeenMinLOD, "minLOD"));
      IFC(ParseMinLOD(P.MinLOD));
      break;
    case TokenType::maxLOD:
      IFC(MarkParameter(bSeenMaxLOD, "maxLOD"));
      IFC(ParseMaxLOD(P.MaxLOD));
      break;
    case TokenType::SReg:
      IFC(MarkParameter(bSeenSReg, "sampler register s#"));
      IFC(ParseRegister(TokenType::SReg, P.ShaderRegister));
      break;
    case TokenType::space:
      IFC(MarkParameter(bSeenSpace, "space"));
      IFC(ParseSpace(P.RegisterSpace));
      break;
    case TokenType::visibility:
      IFC(MarkParameter(bSeenVisibility, "visibility"));
      IFC(ParseVisibility(P.ShaderVisibility));
      break;
    default:
      IFC(Error(ERR_RS_UNEXPECTED_TOKEN, "Unexpected token '%s'",
                Token.GetStr()));
      break;
    }

    Token = m_pTokenizer->GetToken();
    if (Token.GetType() == TokenType::RParen)
      break;
    else if (Token.GetType() != TokenType::Comma)
      IFC(Error(ERR_RS_UNEXPECTED_TOKEN, "Unexpected token '%s'",
                Token.GetStr()));
  }

  if (!bSeenSReg) {
    IFC(Error(ERR_RS_UNDEFINED_REGISTER,
              "Sampler register s# must be defined for each static sampler"));
  }

Cleanup:
  return hr;
}

HRESULT RootSignatureParser::ParseFilter(DxilFilter &Filter) {
  HRESULT hr = S_OK;
  TokenType Token;

  IFC(GetAndMatchToken(Token, TokenType::filter));
  IFC(GetAndMatchToken(Token, TokenType::EQ));
  Token = m_pTokenizer->GetToken();

  switch (Token.GetType()) {
  case TokenType::FILTER_MIN_MAG_MIP_POINT:
    Filter = DxilFilter::MIN_MAG_MIP_POINT;
    break;
  case TokenType::FILTER_MIN_MAG_POINT_MIP_LINEAR:
    Filter = DxilFilter::MIN_MAG_POINT_MIP_LINEAR;
    break;
  case TokenType::FILTER_MIN_POINT_MAG_LINEAR_MIP_POINT:
    Filter = DxilFilter::MIN_POINT_MAG_LINEAR_MIP_POINT;
    break;
  case TokenType::FILTER_MIN_POINT_MAG_MIP_LINEAR:
    Filter = DxilFilter::MIN_POINT_MAG_MIP_LINEAR;
    break;
  case TokenType::FILTER_MIN_LINEAR_MAG_MIP_POINT:
    Filter = DxilFilter::MIN_LINEAR_MAG_MIP_POINT;
    break;
  case TokenType::FILTER_MIN_LINEAR_MAG_POINT_MIP_LINEAR:
    Filter = DxilFilter::MIN_LINEAR_MAG_POINT_MIP_LINEAR;
    break;
  case TokenType::FILTER_MIN_MAG_LINEAR_MIP_POINT:
    Filter = DxilFilter::MIN_MAG_LINEAR_MIP_POINT;
    break;
  case TokenType::FILTER_MIN_MAG_MIP_LINEAR:
    Filter = DxilFilter::MIN_MAG_MIP_LINEAR;
    break;
  case TokenType::FILTER_ANISOTROPIC:
    Filter = DxilFilter::ANISOTROPIC;
    break;
  case TokenType::FILTER_COMPARISON_MIN_MAG_MIP_POINT:
    Filter = DxilFilter::COMPARISON_MIN_MAG_MIP_POINT;
    break;
  case TokenType::FILTER_COMPARISON_MIN_MAG_POINT_MIP_LINEAR:
    Filter = DxilFilter::COMPARISON_MIN_MAG_POINT_MIP_LINEAR;
    break;
  case TokenType::FILTER_COMPARISON_MIN_POINT_MAG_LINEAR_MIP_POINT:
    Filter = DxilFilter::COMPARISON_MIN_POINT_MAG_LINEAR_MIP_POINT;
    break;
  case TokenType::FILTER_COMPARISON_MIN_POINT_MAG_MIP_LINEAR:
    Filter = DxilFilter::COMPARISON_MIN_POINT_MAG_MIP_LINEAR;
    break;
  case TokenType::FILTER_COMPARISON_MIN_LINEAR_MAG_MIP_POINT:
    Filter = DxilFilter::COMPARISON_MIN_LINEAR_MAG_MIP_POINT;
    break;
  case TokenType::FILTER_COMPARISON_MIN_LINEAR_MAG_POINT_MIP_LINEAR:
    Filter = DxilFilter::COMPARISON_MIN_LINEAR_MAG_POINT_MIP_LINEAR;
    break;
  case TokenType::FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT:
    Filter = DxilFilter::COMPARISON_MIN_MAG_LINEAR_MIP_POINT;
    break;
  case TokenType::FILTER_COMPARISON_MIN_MAG_MIP_LINEAR:
    Filter = DxilFilter::COMPARISON_MIN_MAG_MIP_LINEAR;
    break;
  case TokenType::FILTER_COMPARISON_ANISOTROPIC:
    Filter = DxilFilter::COMPARISON_ANISOTROPIC;
    break;
  case TokenType::FILTER_MINIMUM_MIN_MAG_MIP_POINT:
    Filter = DxilFilter::MINIMUM_MIN_MAG_MIP_POINT;
    break;
  case TokenType::FILTER_MINIMUM_MIN_MAG_POINT_MIP_LINEAR:
    Filter = DxilFilter::MINIMUM_MIN_MAG_POINT_MIP_LINEAR;
    break;
  case TokenType::FILTER_MINIMUM_MIN_POINT_MAG_LINEAR_MIP_POINT:
    Filter = DxilFilter::MINIMUM_MIN_POINT_MAG_LINEAR_MIP_POINT;
    break;
  case TokenType::FILTER_MINIMUM_MIN_POINT_MAG_MIP_LINEAR:
    Filter = DxilFilter::MINIMUM_MIN_POINT_MAG_MIP_LINEAR;
    break;
  case TokenType::FILTER_MINIMUM_MIN_LINEAR_MAG_MIP_POINT:
    Filter = DxilFilter::MINIMUM_MIN_LINEAR_MAG_MIP_POINT;
    break;
  case TokenType::FILTER_MINIMUM_MIN_LINEAR_MAG_POINT_MIP_LINEAR:
    Filter = DxilFilter::MINIMUM_MIN_LINEAR_MAG_POINT_MIP_LINEAR;
    break;
  case TokenType::FILTER_MINIMUM_MIN_MAG_LINEAR_MIP_POINT:
    Filter = DxilFilter::MINIMUM_MIN_MAG_LINEAR_MIP_POINT;
    break;
  case TokenType::FILTER_MINIMUM_MIN_MAG_MIP_LINEAR:
    Filter = DxilFilter::MINIMUM_MIN_MAG_MIP_LINEAR;
    break;
  case TokenType::FILTER_MINIMUM_ANISOTROPIC:
    Filter = DxilFilter::MINIMUM_ANISOTROPIC;
    break;
  case TokenType::FILTER_MAXIMUM_MIN_MAG_MIP_POINT:
    Filter = DxilFilter::MAXIMUM_MIN_MAG_MIP_POINT;
    break;
  case TokenType::FILTER_MAXIMUM_MIN_MAG_POINT_MIP_LINEAR:
    Filter = DxilFilter::MAXIMUM_MIN_MAG_POINT_MIP_LINEAR;
    break;
  case TokenType::FILTER_MAXIMUM_MIN_POINT_MAG_LINEAR_MIP_POINT:
    Filter = DxilFilter::MAXIMUM_MIN_POINT_MAG_LINEAR_MIP_POINT;
    break;
  case TokenType::FILTER_MAXIMUM_MIN_POINT_MAG_MIP_LINEAR:
    Filter = DxilFilter::MAXIMUM_MIN_POINT_MAG_MIP_LINEAR;
    break;
  case TokenType::FILTER_MAXIMUM_MIN_LINEAR_MAG_MIP_POINT:
    Filter = DxilFilter::MAXIMUM_MIN_LINEAR_MAG_MIP_POINT;
    break;
  case TokenType::FILTER_MAXIMUM_MIN_LINEAR_MAG_POINT_MIP_LINEAR:
    Filter = DxilFilter::MAXIMUM_MIN_LINEAR_MAG_POINT_MIP_LINEAR;
    break;
  case TokenType::FILTER_MAXIMUM_MIN_MAG_LINEAR_MIP_POINT:
    Filter = DxilFilter::MAXIMUM_MIN_MAG_LINEAR_MIP_POINT;
    break;
  case TokenType::FILTER_MAXIMUM_MIN_MAG_MIP_LINEAR:
    Filter = DxilFilter::MAXIMUM_MIN_MAG_MIP_LINEAR;
    break;
  case TokenType::FILTER_MAXIMUM_ANISOTROPIC:
    Filter = DxilFilter::MAXIMUM_ANISOTROPIC;
    break;
  default:
    IFC(Error(ERR_RS_UNEXPECTED_TOKEN, "Unexpected filter value: '%s'.",
              Token.GetStr()));
  }

Cleanup:
  return hr;
}

HRESULT RootSignatureParser::ParseTextureAddressMode(
    DxilTextureAddressMode &AddressMode) {
  HRESULT hr = S_OK;
  TokenType Token = m_pTokenizer->GetToken();
  DXASSERT_NOMSG(Token.GetType() == TokenType::addressU ||
                 Token.GetType() == TokenType::addressV ||
                 Token.GetType() == TokenType::addressW);
  IFC(GetAndMatchToken(Token, TokenType::EQ));
  Token = m_pTokenizer->GetToken();

  switch (Token.GetType()) {
  case TokenType::TEXTURE_ADDRESS_WRAP:
    AddressMode = DxilTextureAddressMode::Wrap;
    break;
  case TokenType::TEXTURE_ADDRESS_MIRROR:
    AddressMode = DxilTextureAddressMode::Mirror;
    break;
  case TokenType::TEXTURE_ADDRESS_CLAMP:
    AddressMode = DxilTextureAddressMode::Clamp;
    break;
  case TokenType::TEXTURE_ADDRESS_BORDER:
    AddressMode = DxilTextureAddressMode::Border;
    break;
  case TokenType::TEXTURE_ADDRESS_MIRROR_ONCE:
    AddressMode = DxilTextureAddressMode::MirrorOnce;
    break;
  default:
    IFC(Error(ERR_RS_UNEXPECTED_TOKEN,
              "Unexpected texture address mode value: '%s'.", Token.GetStr()));
  }

Cleanup:
  return hr;
}

HRESULT RootSignatureParser::ParseMipLODBias(float &MipLODBias) {
  HRESULT hr = S_OK;
  TokenType Token;

  IFC(GetAndMatchToken(Token, TokenType::mipLODBias));
  IFC(GetAndMatchToken(Token, TokenType::EQ));
  IFC(ParseFloat(MipLODBias));

Cleanup:
  return hr;
}

HRESULT RootSignatureParser::ParseMaxAnisotropy(uint32_t &MaxAnisotropy) {
  HRESULT hr = S_OK;
  TokenType Token;

  IFC(GetAndMatchToken(Token, TokenType::maxAnisotropy));
  IFC(GetAndMatchToken(Token, TokenType::EQ));
  IFC(GetAndMatchToken(Token, TokenType::NumberU32));
  MaxAnisotropy = Token.GetU32Value();

Cleanup:
  return hr;
}

HRESULT RootSignatureParser::ParseComparisonFunction(
    DxilComparisonFunc &ComparisonFunc) {
  HRESULT hr = S_OK;
  TokenType Token;

  IFC(GetAndMatchToken(Token, TokenType::comparisonFunc));
  IFC(GetAndMatchToken(Token, TokenType::EQ));
  Token = m_pTokenizer->GetToken();

  switch (Token.GetType()) {
  case TokenType::COMPARISON_NEVER:
    ComparisonFunc = DxilComparisonFunc::Never;
    break;
  case TokenType::COMPARISON_LESS:
    ComparisonFunc = DxilComparisonFunc::Less;
    break;
  case TokenType::COMPARISON_EQUAL:
    ComparisonFunc = DxilComparisonFunc::Equal;
    break;
  case TokenType::COMPARISON_LESS_EQUAL:
    ComparisonFunc = DxilComparisonFunc::LessEqual;
    break;
  case TokenType::COMPARISON_GREATER:
    ComparisonFunc = DxilComparisonFunc::Greater;
    break;
  case TokenType::COMPARISON_NOT_EQUAL:
    ComparisonFunc = DxilComparisonFunc::NotEqual;
    break;
  case TokenType::COMPARISON_GREATER_EQUAL:
    ComparisonFunc = DxilComparisonFunc::GreaterEqual;
    break;
  case TokenType::COMPARISON_ALWAYS:
    ComparisonFunc = DxilComparisonFunc::Always;
    break;
  default:
    IFC(Error(ERR_RS_UNEXPECTED_TOKEN,
              "Unexpected texture address mode value: '%s'.", Token.GetStr()));
  }

Cleanup:
  return hr;
}

HRESULT
RootSignatureParser::ParseBorderColor(DxilStaticBorderColor &BorderColor) {
  HRESULT hr = S_OK;
  TokenType Token;

  IFC(GetAndMatchToken(Token, TokenType::borderColor));
  IFC(GetAndMatchToken(Token, TokenType::EQ));
  Token = m_pTokenizer->GetToken();

  switch (Token.GetType()) {
  case TokenType::STATIC_BORDER_COLOR_TRANSPARENT_BLACK:
    BorderColor = DxilStaticBorderColor::TransparentBlack;
    break;
  case TokenType::STATIC_BORDER_COLOR_OPAQUE_BLACK:
    BorderColor = DxilStaticBorderColor::OpaqueBlack;
    break;
  case TokenType::STATIC_BORDER_COLOR_OPAQUE_WHITE:
    BorderColor = DxilStaticBorderColor::OpaqueWhite;
    break;
  case TokenType::STATIC_BORDER_COLOR_OPAQUE_BLACK_UINT:
    BorderColor = DxilStaticBorderColor::OpaqueBlackUint;
    break;
  case TokenType::STATIC_BORDER_COLOR_OPAQUE_WHITE_UINT:
    BorderColor = DxilStaticBorderColor::OpaqueWhiteUint;
    break;
  default:
    IFC(Error(ERR_RS_UNEXPECTED_TOKEN,
              "Unexpected texture address mode value: '%s'.", Token.GetStr()));
  }

Cleanup:
  return hr;
}

HRESULT RootSignatureParser::ParseMinLOD(float &MinLOD) {
  HRESULT hr = S_OK;
  TokenType Token;

  IFC(GetAndMatchToken(Token, TokenType::minLOD));
  IFC(GetAndMatchToken(Token, TokenType::EQ));
  IFC(ParseFloat(MinLOD));

Cleanup:
  return hr;
}

HRESULT RootSignatureParser::ParseMaxLOD(float &MaxLOD) {
  HRESULT hr = S_OK;
  TokenType Token;

  IFC(GetAndMatchToken(Token, TokenType::maxLOD));
  IFC(GetAndMatchToken(Token, TokenType::EQ));
  IFC(ParseFloat(MaxLOD));

Cleanup:
  return hr;
}

HRESULT RootSignatureParser::ParseFloat(float &v) {
  HRESULT hr = S_OK;
  TokenType Token = m_pTokenizer->GetToken();
  if (Token.GetType() == TokenType::NumberU32) {
    v = (float)Token.GetU32Value();
  } else if (Token.GetType() == TokenType::NumberI32) {
    v = (float)Token.GetI32Value();
  } else if (Token.GetType() == TokenType::NumberFloat) {
    v = Token.GetFloatValue();
  } else {
    IFC(Error(ERR_RS_UNEXPECTED_TOKEN, "Expected float, found token '%s'",
              Token.GetStr()));
  }

Cleanup:
  return hr;
}

HRESULT RootSignatureParser::MarkParameter(bool &bSeen, LPCSTR pName) {
  HRESULT hr = S_OK;

  if (bSeen) {
    IFC(Error(ERR_RS_UNEXPECTED_TOKEN,
              "Parameter '%s' can be specified only once", pName));
  }

  bSeen = true;

Cleanup:
  return hr;
}

bool clang::ParseHLSLRootSignature(
    const char *pData, unsigned Len, hlsl::DxilRootSignatureVersion Ver,
    hlsl::DxilRootSignatureCompilationFlags Flags,
    hlsl::DxilVersionedRootSignatureDesc **ppDesc, SourceLocation Loc,
    clang::DiagnosticsEngine &Diags) {
  *ppDesc = nullptr;
  std::string OSStr;
  llvm::raw_string_ostream OS(OSStr);
  hlsl::RootSignatureTokenizer RST(pData, Len);
  hlsl::RootSignatureParser RSP(&RST, Ver, Flags, OS);
  hlsl::DxilVersionedRootSignatureDesc *D = nullptr;
  if (SUCCEEDED(RSP.Parse(&D))) {
    *ppDesc = D;
    return true;
  } else {
    // Create diagnostic error message.
    OS.flush();
    if (OSStr.empty()) {
      Diags.Report(Loc, clang::diag::err_hlsl_rootsig) << "unexpected";
    } else {
      Diags.Report(Loc, clang::diag::err_hlsl_rootsig) << OSStr.c_str();
    }
    return false;
  }
}

void clang::ReportHLSLRootSigError(clang::DiagnosticsEngine &Diags,
                                   clang::SourceLocation Loc, const char *pData,
                                   unsigned Len) {
  Diags.Report(Loc, clang::diag::err_hlsl_rootsig) << StringRef(pData, Len);
  return;
}
