///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// DxilShaderModel.h                                                         //
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
// This file is distributed under the University of Illinois Open Source     //
// License. See LICENSE.TXT for details.                                     //
//                                                                           //
// Representation of HLSL shader models.                                     //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#pragma once

#include "dxc/DXIL/DxilConstants.h"

#include "llvm/ADT/StringRef.h"

namespace hlsl {

class Semantic;

/// <summary>
/// Use this class to represent HLSL shader model.
/// </summary>
class ShaderModel {
public:
  using Kind = DXIL::ShaderKind;

  // Major/Minor version of highest recognized shader model
  // clang-format off
  // Python lines need to be not formatted.
  /* <py::lines('VALRULE-TEXT')>hctdb_instrhelp.get_highest_shader_model()</py>*/
  // clang-format on
  // VALRULE-TEXT:BEGIN
  static const unsigned kHighestMajor = 6;
  static const unsigned kHighestMinor = 9;
  // VALRULE-TEXT:END

  // Major/Minor version of highest released shader model
  /* <py::lines('VALRULE-TEXT')>hctdb_instrhelp.get_highest_released_shader_model()</py>*/
  // clang-format on
  // VALRULE-TEXT:BEGIN
  static const unsigned kHighestReleasedMajor = 6;
  static const unsigned kHighestReleasedMinor = 8;
  // VALRULE-TEXT:END

  static const unsigned kOfflineMinor = 0xF;

  bool IsPS() const { return m_Kind == Kind::Pixel; }
  bool IsVS() const { return m_Kind == Kind::Vertex; }
  bool IsGS() const { return m_Kind == Kind::Geometry; }
  bool IsHS() const { return m_Kind == Kind::Hull; }
  bool IsDS() const { return m_Kind == Kind::Domain; }
  bool IsCS() const { return m_Kind == Kind::Compute; }
  bool IsLib() const { return m_Kind == Kind::Library; }
  bool IsMS() const { return m_Kind == Kind::Mesh; }
  bool IsAS() const { return m_Kind == Kind::Amplification; }
  bool IsValid() const;
  bool IsValidForDxil() const;

  Kind GetKind() const { return m_Kind; }
  unsigned GetMajor() const { return m_Major; }
  unsigned GetMinor() const { return m_Minor; }
  void GetDxilVersion(unsigned &DxilMajor, unsigned &DxilMinor) const;
  void GetMinValidatorVersion(unsigned &ValMajor, unsigned &ValMinor) const;
  bool IsSMAtLeast(unsigned Major, unsigned Minor) const {
    return m_Major > Major || (m_Major == Major && m_Minor >= Minor);
  }
  bool IsSM50Plus() const { return IsSMAtLeast(5, 0); }
  bool IsSM51Plus() const { return IsSMAtLeast(5, 1); }
  bool AllowDerivatives(DXIL::ShaderKind sk) const;
  // clang-format off
  // Python lines need to be not formatted.
  /* <py::lines('VALRULE-TEXT')>hctdb_instrhelp.get_is_shader_model_plus()</py>*/
  // clang-format on
  // VALRULE-TEXT:BEGIN
  bool IsSM60Plus() const { return IsSMAtLeast(6, 0); }
  bool IsSM61Plus() const { return IsSMAtLeast(6, 1); }
  bool IsSM62Plus() const { return IsSMAtLeast(6, 2); }
  bool IsSM63Plus() const { return IsSMAtLeast(6, 3); }
  bool IsSM64Plus() const { return IsSMAtLeast(6, 4); }
  bool IsSM65Plus() const { return IsSMAtLeast(6, 5); }
  bool IsSM66Plus() const { return IsSMAtLeast(6, 6); }
  bool IsSM67Plus() const { return IsSMAtLeast(6, 7); }
  bool IsSM68Plus() const { return IsSMAtLeast(6, 8); }
  bool IsSM69Plus() const { return IsSMAtLeast(6, 9); }
  // VALRULE-TEXT:END
  const char *GetName() const { return m_pszName; }
  const char *GetKindName() const;

  DXIL::PackingStrategy GetDefaultPackingStrategy() const {
    return DXIL::PackingStrategy::PrefixStable;
  }

  static const ShaderModel *Get(Kind Kind, unsigned Major, unsigned Minor);
  static const ShaderModel *GetByName(llvm::StringRef Name);
  static const char *GetKindName(Kind kind);
  static bool IsPreReleaseShaderModel(int Major, int Minor);
  static Kind GetKindFromName(llvm::StringRef Name);
  static DXIL::ShaderKind KindFromFullName(llvm::StringRef Name);
  static const llvm::StringRef FullNameFromKind(DXIL::ShaderKind sk);
  static const char *GetNodeLaunchTypeName(DXIL::NodeLaunchType launchTy);
  static DXIL::NodeLaunchType NodeLaunchTypeFromName(llvm::StringRef name);

  static bool HasVisibleGroup(
      DXIL::ShaderKind SK,
      DXIL::NodeLaunchType launchType = DXIL::NodeLaunchType::Invalid) {
    // Note: Library case is permissive; enforced at entry point.
    return SK == DXIL::ShaderKind::Compute || SK == DXIL::ShaderKind::Mesh ||
           SK == DXIL::ShaderKind::Amplification ||
           SK == DXIL::ShaderKind::Library ||
           (SK == DXIL::ShaderKind::Node &&
            (launchType == DXIL::NodeLaunchType::Broadcasting ||
             launchType == DXIL::NodeLaunchType::Coalescing));
  }

  bool operator==(const ShaderModel &other) const;
  bool operator!=(const ShaderModel &other) const { return !(*this == other); }

private:
  Kind m_Kind;
  unsigned m_Major;
  unsigned m_Minor;
  const char *m_pszName;
  unsigned m_NumInputRegs;
  unsigned m_NumOutputRegs;
  bool m_bTypedUavs;
  unsigned m_NumUAVRegs;

  ShaderModel() = delete;
  ShaderModel(Kind Kind, unsigned Major, unsigned Minor, const char *pszName,
              unsigned m_NumInputRegs, unsigned m_NumOutputRegs, bool m_bUAVs,
              bool m_bTypedUavs, unsigned m_UAVRegsLim);
  /* <py::lines('VALRULE-TEXT')>hctdb_instrhelp.get_num_shader_models()</py>*/
  // VALRULE-TEXT:BEGIN
  static const unsigned kNumShaderModels = 101;
  // VALRULE-TEXT:END
  static const ShaderModel ms_ShaderModels[kNumShaderModels];

  static const ShaderModel *GetInvalid();
};

} // namespace hlsl
