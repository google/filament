///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// DxilShaderModel.cpp                                                       //
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
// This file is distributed under the University of Illinois Open Source     //
// License. See LICENSE.TXT for details.                                     //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#include <limits.h>

#include "dxc/DXIL/DxilSemantic.h"
#include "dxc/DXIL/DxilShaderModel.h"
#include "dxc/Support/Global.h"

#include "llvm/ADT/StringSwitch.h"
#include "llvm/Support/ErrorHandling.h"

#include <algorithm>

namespace hlsl {

ShaderModel::ShaderModel(Kind Kind, unsigned Major, unsigned Minor,
                         const char *pszName, unsigned NumInputRegs,
                         unsigned NumOutputRegs, bool bUAVs, bool bTypedUavs,
                         unsigned NumUAVRegs)
    : m_Kind(Kind), m_Major(Major), m_Minor(Minor), m_pszName(pszName),
      m_NumInputRegs(NumInputRegs), m_NumOutputRegs(NumOutputRegs),
      m_bTypedUavs(bTypedUavs), m_NumUAVRegs(NumUAVRegs) {}

bool ShaderModel::operator==(const ShaderModel &other) const {
  return m_Kind == other.m_Kind && m_Major == other.m_Major &&
         m_Minor == other.m_Minor && strcmp(m_pszName, other.m_pszName) == 0 &&
         m_NumInputRegs == other.m_NumInputRegs &&
         m_NumOutputRegs == other.m_NumOutputRegs &&
         m_bTypedUavs == other.m_bTypedUavs &&
         m_NumUAVRegs == other.m_NumUAVRegs;
}

bool ShaderModel::IsValid() const {
  DXASSERT(IsPS() || IsVS() || IsGS() || IsHS() || IsDS() || IsCS() ||
               IsLib() || IsMS() || IsAS() || m_Kind == Kind::Invalid,
           "invalid shader model");
  return m_Kind != Kind::Invalid;
}

bool ShaderModel::IsValidForDxil() const {
  if (!IsValid())
    return false;
  switch (m_Major) {
  case 6: {
    switch (m_Minor) {
      // clang-format off
      // Python lines need to be not formatted.
      /* <py::lines('VALRULE-TEXT')>hctdb_instrhelp.get_is_valid_for_dxil()</py>*/
    // clang-format on
    // VALRULE-TEXT:BEGIN
    case 0:
    case 1:
    case 2:
    case 3:
    case 4:
    case 5:
    case 6:
    case 7:
    case 8:
    case 9:
      // VALRULE-TEXT:END
      return true;
    case kOfflineMinor:
      return m_Kind == Kind::Library;
    }
  } break;
  }
  return false;
}

const ShaderModel *ShaderModel::Get(Kind Kind, unsigned Major, unsigned Minor) {
  /* <py::lines('VALRULE-TEXT')>hctdb_instrhelp.get_shader_model_get()</py>*/
  // VALRULE-TEXT:BEGIN
  const static std::pair<unsigned, unsigned> hashToIdxMap[] = {
      {1024, 0},    // ps_4_0
      {1025, 1},    // ps_4_1
      {1280, 2},    // ps_5_0
      {1281, 3},    // ps_5_1
      {1536, 4},    // ps_6_0
      {1537, 5},    // ps_6_1
      {1538, 6},    // ps_6_2
      {1539, 7},    // ps_6_3
      {1540, 8},    // ps_6_4
      {1541, 9},    // ps_6_5
      {1542, 10},   // ps_6_6
      {1543, 11},   // ps_6_7
      {1544, 12},   // ps_6_8
      {1545, 13},   // ps_6_9
      {66560, 14},  // vs_4_0
      {66561, 15},  // vs_4_1
      {66816, 16},  // vs_5_0
      {66817, 17},  // vs_5_1
      {67072, 18},  // vs_6_0
      {67073, 19},  // vs_6_1
      {67074, 20},  // vs_6_2
      {67075, 21},  // vs_6_3
      {67076, 22},  // vs_6_4
      {67077, 23},  // vs_6_5
      {67078, 24},  // vs_6_6
      {67079, 25},  // vs_6_7
      {67080, 26},  // vs_6_8
      {67081, 27},  // vs_6_9
      {132096, 28}, // gs_4_0
      {132097, 29}, // gs_4_1
      {132352, 30}, // gs_5_0
      {132353, 31}, // gs_5_1
      {132608, 32}, // gs_6_0
      {132609, 33}, // gs_6_1
      {132610, 34}, // gs_6_2
      {132611, 35}, // gs_6_3
      {132612, 36}, // gs_6_4
      {132613, 37}, // gs_6_5
      {132614, 38}, // gs_6_6
      {132615, 39}, // gs_6_7
      {132616, 40}, // gs_6_8
      {132617, 41}, // gs_6_9
      {197888, 42}, // hs_5_0
      {197889, 43}, // hs_5_1
      {198144, 44}, // hs_6_0
      {198145, 45}, // hs_6_1
      {198146, 46}, // hs_6_2
      {198147, 47}, // hs_6_3
      {198148, 48}, // hs_6_4
      {198149, 49}, // hs_6_5
      {198150, 50}, // hs_6_6
      {198151, 51}, // hs_6_7
      {198152, 52}, // hs_6_8
      {198153, 53}, // hs_6_9
      {263424, 54}, // ds_5_0
      {263425, 55}, // ds_5_1
      {263680, 56}, // ds_6_0
      {263681, 57}, // ds_6_1
      {263682, 58}, // ds_6_2
      {263683, 59}, // ds_6_3
      {263684, 60}, // ds_6_4
      {263685, 61}, // ds_6_5
      {263686, 62}, // ds_6_6
      {263687, 63}, // ds_6_7
      {263688, 64}, // ds_6_8
      {263689, 65}, // ds_6_9
      {328704, 66}, // cs_4_0
      {328705, 67}, // cs_4_1
      {328960, 68}, // cs_5_0
      {328961, 69}, // cs_5_1
      {329216, 70}, // cs_6_0
      {329217, 71}, // cs_6_1
      {329218, 72}, // cs_6_2
      {329219, 73}, // cs_6_3
      {329220, 74}, // cs_6_4
      {329221, 75}, // cs_6_5
      {329222, 76}, // cs_6_6
      {329223, 77}, // cs_6_7
      {329224, 78}, // cs_6_8
      {329225, 79}, // cs_6_9
      {394753, 80}, // lib_6_1
      {394754, 81}, // lib_6_2
      {394755, 82}, // lib_6_3
      {394756, 83}, // lib_6_4
      {394757, 84}, // lib_6_5
      {394758, 85}, // lib_6_6
      {394759, 86}, // lib_6_7
      {394760, 87}, // lib_6_8
      {394761, 88}, // lib_6_9
      // lib_6_x is for offline linking only, and relaxes restrictions
      {394767, 89}, // lib_6_x
      {853509, 90}, // ms_6_5
      {853510, 91}, // ms_6_6
      {853511, 92}, // ms_6_7
      {853512, 93}, // ms_6_8
      {853513, 94}, // ms_6_9
      {919045, 95}, // as_6_5
      {919046, 96}, // as_6_6
      {919047, 97}, // as_6_7
      {919048, 98}, // as_6_8
      {919049, 99}, // as_6_9
  };
  unsigned hash = (unsigned)Kind << 16 | Major << 8 | Minor;
  auto pred = [](const std::pair<unsigned, unsigned> &elem, unsigned val) {
    return elem.first < val;
  };
  auto it = std::lower_bound(std::begin(hashToIdxMap), std::end(hashToIdxMap),
                             hash, pred);
  if (it == std::end(hashToIdxMap) || it->first != hash)
    return GetInvalid();
  return &ms_ShaderModels[it->second];
  // VALRULE-TEXT:END
}

bool ShaderModel::IsPreReleaseShaderModel(int major, int minor) {
  if (DXIL::CompareVersions(major, minor, kHighestReleasedMajor,
                            kHighestReleasedMinor) <= 0)
    return false;

  // now compare against highest recognized
  if (DXIL::CompareVersions(major, minor, kHighestMajor, kHighestMinor) <= 0)
    return true;
  return false;
}

ShaderModel::Kind ShaderModel::GetKindFromName(llvm::StringRef Name) {
  Kind kind;
  if (Name.empty()) {
    return Kind::Invalid;
  }

  switch (Name[0]) {
  case 'p':
    kind = Kind::Pixel;
    break;
  case 'v':
    kind = Kind::Vertex;
    break;
  case 'g':
    kind = Kind::Geometry;
    break;
  case 'h':
    kind = Kind::Hull;
    break;
  case 'd':
    kind = Kind::Domain;
    break;
  case 'c':
    kind = Kind::Compute;
    break;
  case 'l':
    kind = Kind::Library;
    break;
  case 'm':
    kind = Kind::Mesh;
    break;
  case 'a':
    kind = Kind::Amplification;
    break;
  default:
    return Kind::Invalid;
  }
  return kind;
}

const ShaderModel *ShaderModel::GetByName(llvm::StringRef Name) {
  // [ps|vs|gs|hs|ds|cs|ms|as]_[major]_[minor]
  Kind kind = GetKindFromName(Name);
  if (kind == Kind::Invalid)
    return GetInvalid();

  unsigned Idx = 3;
  if (kind != Kind::Library) {
    if (Name[1] != 's' || Name[2] != '_')
      return GetInvalid();
  } else {
    if (Name[1] != 'i' || Name[2] != 'b' || Name[3] != '_')
      return GetInvalid();
    Idx = 4;
  }

  unsigned Major;
  switch (Name[Idx++]) {
  case '4':
    Major = 4;
    break;
  case '5':
    Major = 5;
    break;
  case '6':
    Major = 6;
    break;
  default:
    return GetInvalid();
  }
  if (Name[Idx++] != '_')
    return GetInvalid();

  unsigned Minor;
  switch (Name[Idx++]) {
  case '0':
    Minor = 0;
    break;
  case '1':
    Minor = 1;
    break;
    // clang-format off
  // Python lines need to be not formatted.
  /* <py::lines('VALRULE-TEXT')>hctdb_instrhelp.get_shader_model_by_name()</py>*/
  // clang-format on
  // VALRULE-TEXT:BEGIN
  case '2':
    if (Major == 6) {
      Minor = 2;
      break;
    } else
      return GetInvalid();
  case '3':
    if (Major == 6) {
      Minor = 3;
      break;
    } else
      return GetInvalid();
  case '4':
    if (Major == 6) {
      Minor = 4;
      break;
    } else
      return GetInvalid();
  case '5':
    if (Major == 6) {
      Minor = 5;
      break;
    } else
      return GetInvalid();
  case '6':
    if (Major == 6) {
      Minor = 6;
      break;
    } else
      return GetInvalid();
  case '7':
    if (Major == 6) {
      Minor = 7;
      break;
    } else
      return GetInvalid();
  case '8':
    if (Major == 6) {
      Minor = 8;
      break;
    } else
      return GetInvalid();
  case '9':
    if (Major == 6) {
      Minor = 9;
      break;
    } else
      return GetInvalid();
    // VALRULE-TEXT:END
  case 'x':
    if (kind == Kind::Library && Major == 6) {
      Minor = kOfflineMinor;
      break;
    } else
      return GetInvalid();
  default:
    return GetInvalid();
  }
  // make sure there isn't anything after the minor
  if (Name.size() > Idx)
    return GetInvalid();

  return Get(kind, Major, Minor);
}

void ShaderModel::GetDxilVersion(unsigned &DxilMajor,
                                 unsigned &DxilMinor) const {
  DXASSERT(IsValidForDxil(), "invalid shader model");
  DxilMajor = 1;
  switch (m_Minor) {
  /* <py::lines('VALRULE-TEXT')>hctdb_instrhelp.get_dxil_version()</py>*/
  // VALRULE-TEXT:BEGIN
  case 0:
    DxilMinor = 0;
    break;
  case 1:
    DxilMinor = 1;
    break;
  case 2:
    DxilMinor = 2;
    break;
  case 3:
    DxilMinor = 3;
    break;
  case 4:
    DxilMinor = 4;
    break;
  case 5:
    DxilMinor = 5;
    break;
  case 6:
    DxilMinor = 6;
    break;
  case 7:
    DxilMinor = 7;
    break;
  case 8:
    DxilMinor = 8;
    break;
  case 9:
    DxilMinor = 9;
    break;
  case kOfflineMinor: // Always update this to highest dxil version
    DxilMinor = 9;
    break;
  // VALRULE-TEXT:END
  default:
    DXASSERT(0, "IsValidForDxil() should have caught this.");
    break;
  }
}

void ShaderModel::GetMinValidatorVersion(unsigned &ValMajor,
                                         unsigned &ValMinor) const {
  DXASSERT(IsValidForDxil(), "invalid shader model");
  ValMajor = 1;
  switch (m_Minor) {
    // clang-format off
  // Python lines need to be not formatted.
  /* <py::lines('VALRULE-TEXT')>hctdb_instrhelp.get_min_validator_version()</py>*/
  // clang-format on
  // VALRULE-TEXT:BEGIN
  case 0:
    ValMinor = 0;
    break;
  case 1:
    ValMinor = 1;
    break;
  case 2:
    ValMinor = 2;
    break;
  case 3:
    ValMinor = 3;
    break;
  case 4:
    ValMinor = 4;
    break;
  case 5:
    ValMinor = 5;
    break;
  case 6:
    ValMinor = 6;
    break;
  case 7:
    ValMinor = 7;
    break;
  case 8:
    ValMinor = 8;
    break;
  case 9:
    ValMinor = 9;
    break;
  // VALRULE-TEXT:END
  case kOfflineMinor:
    ValMajor = 0;
    ValMinor = 0;
    break;
  default:
    DXASSERT(0, "IsValidForDxil() should have caught this.");
    break;
  }
}

static const char *ShaderModelKindNames[] = {
    "ps",           "vs",     "gs",         "hs",
    "ds",           "cs",     "lib",        "raygeneration",
    "intersection", "anyhit", "closesthit", "miss",
    "callable",     "ms",     "as",         "node",
    "invalid",
};

const char *ShaderModel::GetKindName() const { return GetKindName(m_Kind); }

const char *ShaderModel::GetKindName(Kind kind) {
  static_assert(static_cast<unsigned>(Kind::Invalid) ==
                    _countof(ShaderModelKindNames) - 1,
                "Invalid kinds or names");
  return ShaderModelKindNames[static_cast<unsigned int>(kind)];
}

const ShaderModel *ShaderModel::GetInvalid() {
  return &ms_ShaderModels[kNumShaderModels - 1];
}

DXIL::ShaderKind ShaderModel::KindFromFullName(llvm::StringRef Name) {
  return llvm::StringSwitch<DXIL::ShaderKind>(Name)
      .Case("pixel", DXIL::ShaderKind::Pixel)
      .Case("vertex", DXIL::ShaderKind::Vertex)
      .Case("geometry", DXIL::ShaderKind::Geometry)
      .Case("hull", DXIL::ShaderKind::Hull)
      .Case("domain", DXIL::ShaderKind::Domain)
      .Case("compute", DXIL::ShaderKind::Compute)
      .Case("raygeneration", DXIL::ShaderKind::RayGeneration)
      .Case("intersection", DXIL::ShaderKind::Intersection)
      .Case("anyhit", DXIL::ShaderKind::AnyHit)
      .Case("closesthit", DXIL::ShaderKind::ClosestHit)
      .Case("miss", DXIL::ShaderKind::Miss)
      .Case("callable", DXIL::ShaderKind::Callable)
      .Case("mesh", DXIL::ShaderKind::Mesh)
      .Case("amplification", DXIL::ShaderKind::Amplification)
      .Case("node", DXIL::ShaderKind::Node)
      .Default(DXIL::ShaderKind::Invalid);
}

const llvm::StringRef ShaderModel::FullNameFromKind(DXIL::ShaderKind sk) {
  switch (sk) {
  case DXIL::ShaderKind::Pixel:
    return "pixel";
  case DXIL::ShaderKind::Vertex:
    return "vertex";
  case DXIL::ShaderKind::Geometry:
    return "geometry";
  case DXIL::ShaderKind::Hull:
    return "hull";
  case DXIL::ShaderKind::Domain:
    return "domain";
  case DXIL::ShaderKind::Compute:
    return "compute";
  // Library has no full name for use with shader attribute.
  case DXIL::ShaderKind::Library:
  case DXIL::ShaderKind::Invalid:
    return llvm::StringRef();
  case DXIL::ShaderKind::RayGeneration:
    return "raygeneration";
  case DXIL::ShaderKind::Intersection:
    return "intersection";
  case DXIL::ShaderKind::AnyHit:
    return "anyhit";
  case DXIL::ShaderKind::ClosestHit:
    return "closesthit";
  case DXIL::ShaderKind::Miss:
    return "miss";
  case DXIL::ShaderKind::Callable:
    return "callable";
  case DXIL::ShaderKind::Mesh:
    return "mesh";
  case DXIL::ShaderKind::Amplification:
    return "amplification";
  case DXIL::ShaderKind::Node:
    return "node";
  default:
    llvm_unreachable("unknown ShaderKind");
  }
}

bool ShaderModel::AllowDerivatives(DXIL::ShaderKind sk) const {
  switch (sk) {
  case DXIL::ShaderKind::Pixel:
  case DXIL::ShaderKind::Library:
  case DXIL::ShaderKind::Node:
    return true;
  case DXIL::ShaderKind::Compute:
  case DXIL::ShaderKind::Amplification:
  case DXIL::ShaderKind::Mesh:
    return IsSM66Plus();
  case DXIL::ShaderKind::Vertex:
  case DXIL::ShaderKind::Geometry:
  case DXIL::ShaderKind::Hull:
  case DXIL::ShaderKind::Domain:
  case DXIL::ShaderKind::RayGeneration:
  case DXIL::ShaderKind::Intersection:
  case DXIL::ShaderKind::AnyHit:
  case DXIL::ShaderKind::ClosestHit:
  case DXIL::ShaderKind::Miss:
  case DXIL::ShaderKind::Callable:
  case DXIL::ShaderKind::Invalid:
    return false;
  }
  static_assert(DXIL::ShaderKind::LastValid == DXIL::ShaderKind::Node,
                "otherwise, switch needs to be updated.");
  llvm_unreachable("unknown ShaderKind");
}

typedef ShaderModel SM;
typedef Semantic SE;
const ShaderModel ShaderModel::ms_ShaderModels[kNumShaderModels] = {
    //                                  IR  OR   UAV?   TyUAV? UAV base
    /* <py::lines('VALRULE-TEXT')>hctdb_instrhelp.get_shader_models()</py>*/
    // VALRULE-TEXT:BEGIN
    SM(Kind::Pixel, 4, 0, "ps_4_0", 32, 8, false, false, 0),
    SM(Kind::Pixel, 4, 1, "ps_4_1", 32, 8, false, false, 0),
    SM(Kind::Pixel, 5, 0, "ps_5_0", 32, 8, true, true, 64),
    SM(Kind::Pixel, 5, 1, "ps_5_1", 32, 8, true, true, 64),
    SM(Kind::Pixel, 6, 0, "ps_6_0", 32, 8, true, true, UINT_MAX),
    SM(Kind::Pixel, 6, 1, "ps_6_1", 32, 8, true, true, UINT_MAX),
    SM(Kind::Pixel, 6, 2, "ps_6_2", 32, 8, true, true, UINT_MAX),
    SM(Kind::Pixel, 6, 3, "ps_6_3", 32, 8, true, true, UINT_MAX),
    SM(Kind::Pixel, 6, 4, "ps_6_4", 32, 8, true, true, UINT_MAX),
    SM(Kind::Pixel, 6, 5, "ps_6_5", 32, 8, true, true, UINT_MAX),
    SM(Kind::Pixel, 6, 6, "ps_6_6", 32, 8, true, true, UINT_MAX),
    SM(Kind::Pixel, 6, 7, "ps_6_7", 32, 8, true, true, UINT_MAX),
    SM(Kind::Pixel, 6, 8, "ps_6_8", 32, 8, true, true, UINT_MAX),
    SM(Kind::Pixel, 6, 9, "ps_6_9", 32, 8, true, true, UINT_MAX),
    SM(Kind::Vertex, 4, 0, "vs_4_0", 16, 16, false, false, 0),
    SM(Kind::Vertex, 4, 1, "vs_4_1", 32, 32, false, false, 0),
    SM(Kind::Vertex, 5, 0, "vs_5_0", 32, 32, true, true, 64),
    SM(Kind::Vertex, 5, 1, "vs_5_1", 32, 32, true, true, 64),
    SM(Kind::Vertex, 6, 0, "vs_6_0", 32, 32, true, true, UINT_MAX),
    SM(Kind::Vertex, 6, 1, "vs_6_1", 32, 32, true, true, UINT_MAX),
    SM(Kind::Vertex, 6, 2, "vs_6_2", 32, 32, true, true, UINT_MAX),
    SM(Kind::Vertex, 6, 3, "vs_6_3", 32, 32, true, true, UINT_MAX),
    SM(Kind::Vertex, 6, 4, "vs_6_4", 32, 32, true, true, UINT_MAX),
    SM(Kind::Vertex, 6, 5, "vs_6_5", 32, 32, true, true, UINT_MAX),
    SM(Kind::Vertex, 6, 6, "vs_6_6", 32, 32, true, true, UINT_MAX),
    SM(Kind::Vertex, 6, 7, "vs_6_7", 32, 32, true, true, UINT_MAX),
    SM(Kind::Vertex, 6, 8, "vs_6_8", 32, 32, true, true, UINT_MAX),
    SM(Kind::Vertex, 6, 9, "vs_6_9", 32, 32, true, true, UINT_MAX),
    SM(Kind::Geometry, 4, 0, "gs_4_0", 16, 32, false, false, 0),
    SM(Kind::Geometry, 4, 1, "gs_4_1", 32, 32, false, false, 0),
    SM(Kind::Geometry, 5, 0, "gs_5_0", 32, 32, true, true, 64),
    SM(Kind::Geometry, 5, 1, "gs_5_1", 32, 32, true, true, 64),
    SM(Kind::Geometry, 6, 0, "gs_6_0", 32, 32, true, true, UINT_MAX),
    SM(Kind::Geometry, 6, 1, "gs_6_1", 32, 32, true, true, UINT_MAX),
    SM(Kind::Geometry, 6, 2, "gs_6_2", 32, 32, true, true, UINT_MAX),
    SM(Kind::Geometry, 6, 3, "gs_6_3", 32, 32, true, true, UINT_MAX),
    SM(Kind::Geometry, 6, 4, "gs_6_4", 32, 32, true, true, UINT_MAX),
    SM(Kind::Geometry, 6, 5, "gs_6_5", 32, 32, true, true, UINT_MAX),
    SM(Kind::Geometry, 6, 6, "gs_6_6", 32, 32, true, true, UINT_MAX),
    SM(Kind::Geometry, 6, 7, "gs_6_7", 32, 32, true, true, UINT_MAX),
    SM(Kind::Geometry, 6, 8, "gs_6_8", 32, 32, true, true, UINT_MAX),
    SM(Kind::Geometry, 6, 9, "gs_6_9", 32, 32, true, true, UINT_MAX),
    SM(Kind::Hull, 5, 0, "hs_5_0", 32, 32, true, true, 64),
    SM(Kind::Hull, 5, 1, "hs_5_1", 32, 32, true, true, 64),
    SM(Kind::Hull, 6, 0, "hs_6_0", 32, 32, true, true, UINT_MAX),
    SM(Kind::Hull, 6, 1, "hs_6_1", 32, 32, true, true, UINT_MAX),
    SM(Kind::Hull, 6, 2, "hs_6_2", 32, 32, true, true, UINT_MAX),
    SM(Kind::Hull, 6, 3, "hs_6_3", 32, 32, true, true, UINT_MAX),
    SM(Kind::Hull, 6, 4, "hs_6_4", 32, 32, true, true, UINT_MAX),
    SM(Kind::Hull, 6, 5, "hs_6_5", 32, 32, true, true, UINT_MAX),
    SM(Kind::Hull, 6, 6, "hs_6_6", 32, 32, true, true, UINT_MAX),
    SM(Kind::Hull, 6, 7, "hs_6_7", 32, 32, true, true, UINT_MAX),
    SM(Kind::Hull, 6, 8, "hs_6_8", 32, 32, true, true, UINT_MAX),
    SM(Kind::Hull, 6, 9, "hs_6_9", 32, 32, true, true, UINT_MAX),
    SM(Kind::Domain, 5, 0, "ds_5_0", 32, 32, true, true, 64),
    SM(Kind::Domain, 5, 1, "ds_5_1", 32, 32, true, true, 64),
    SM(Kind::Domain, 6, 0, "ds_6_0", 32, 32, true, true, UINT_MAX),
    SM(Kind::Domain, 6, 1, "ds_6_1", 32, 32, true, true, UINT_MAX),
    SM(Kind::Domain, 6, 2, "ds_6_2", 32, 32, true, true, UINT_MAX),
    SM(Kind::Domain, 6, 3, "ds_6_3", 32, 32, true, true, UINT_MAX),
    SM(Kind::Domain, 6, 4, "ds_6_4", 32, 32, true, true, UINT_MAX),
    SM(Kind::Domain, 6, 5, "ds_6_5", 32, 32, true, true, UINT_MAX),
    SM(Kind::Domain, 6, 6, "ds_6_6", 32, 32, true, true, UINT_MAX),
    SM(Kind::Domain, 6, 7, "ds_6_7", 32, 32, true, true, UINT_MAX),
    SM(Kind::Domain, 6, 8, "ds_6_8", 32, 32, true, true, UINT_MAX),
    SM(Kind::Domain, 6, 9, "ds_6_9", 32, 32, true, true, UINT_MAX),
    SM(Kind::Compute, 4, 0, "cs_4_0", 0, 0, false, false, 0),
    SM(Kind::Compute, 4, 1, "cs_4_1", 0, 0, false, false, 0),
    SM(Kind::Compute, 5, 0, "cs_5_0", 0, 0, true, true, 64),
    SM(Kind::Compute, 5, 1, "cs_5_1", 0, 0, true, true, 64),
    SM(Kind::Compute, 6, 0, "cs_6_0", 0, 0, true, true, UINT_MAX),
    SM(Kind::Compute, 6, 1, "cs_6_1", 0, 0, true, true, UINT_MAX),
    SM(Kind::Compute, 6, 2, "cs_6_2", 0, 0, true, true, UINT_MAX),
    SM(Kind::Compute, 6, 3, "cs_6_3", 0, 0, true, true, UINT_MAX),
    SM(Kind::Compute, 6, 4, "cs_6_4", 0, 0, true, true, UINT_MAX),
    SM(Kind::Compute, 6, 5, "cs_6_5", 0, 0, true, true, UINT_MAX),
    SM(Kind::Compute, 6, 6, "cs_6_6", 0, 0, true, true, UINT_MAX),
    SM(Kind::Compute, 6, 7, "cs_6_7", 0, 0, true, true, UINT_MAX),
    SM(Kind::Compute, 6, 8, "cs_6_8", 0, 0, true, true, UINT_MAX),
    SM(Kind::Compute, 6, 9, "cs_6_9", 0, 0, true, true, UINT_MAX),
    SM(Kind::Library, 6, 1, "lib_6_1", 32, 32, true, true, UINT_MAX),
    SM(Kind::Library, 6, 2, "lib_6_2", 32, 32, true, true, UINT_MAX),
    SM(Kind::Library, 6, 3, "lib_6_3", 32, 32, true, true, UINT_MAX),
    SM(Kind::Library, 6, 4, "lib_6_4", 32, 32, true, true, UINT_MAX),
    SM(Kind::Library, 6, 5, "lib_6_5", 32, 32, true, true, UINT_MAX),
    SM(Kind::Library, 6, 6, "lib_6_6", 32, 32, true, true, UINT_MAX),
    SM(Kind::Library, 6, 7, "lib_6_7", 32, 32, true, true, UINT_MAX),
    SM(Kind::Library, 6, 8, "lib_6_8", 32, 32, true, true, UINT_MAX),
    SM(Kind::Library, 6, 9, "lib_6_9", 32, 32, true, true, UINT_MAX),
    // lib_6_x is for offline linking only, and relaxes restrictions
    SM(Kind::Library, 6, kOfflineMinor, "lib_6_x", 32, 32, true, true,
       UINT_MAX),
    SM(Kind::Mesh, 6, 5, "ms_6_5", 0, 0, true, true, UINT_MAX),
    SM(Kind::Mesh, 6, 6, "ms_6_6", 0, 0, true, true, UINT_MAX),
    SM(Kind::Mesh, 6, 7, "ms_6_7", 0, 0, true, true, UINT_MAX),
    SM(Kind::Mesh, 6, 8, "ms_6_8", 0, 0, true, true, UINT_MAX),
    SM(Kind::Mesh, 6, 9, "ms_6_9", 0, 0, true, true, UINT_MAX),
    SM(Kind::Amplification, 6, 5, "as_6_5", 0, 0, true, true, UINT_MAX),
    SM(Kind::Amplification, 6, 6, "as_6_6", 0, 0, true, true, UINT_MAX),
    SM(Kind::Amplification, 6, 7, "as_6_7", 0, 0, true, true, UINT_MAX),
    SM(Kind::Amplification, 6, 8, "as_6_8", 0, 0, true, true, UINT_MAX),
    SM(Kind::Amplification, 6, 9, "as_6_9", 0, 0, true, true, UINT_MAX),
    // Values before Invalid must remain sorted by Kind, then Major, then Minor.
    SM(Kind::Invalid, 0, 0, "invalid", 0, 0, false, false, 0),
    // VALRULE-TEXT:END
};

static const char *NodeLaunchTypeNames[] = {"invalid", "broadcasting",
                                            "coalescing", "thread"};

const char *ShaderModel::GetNodeLaunchTypeName(DXIL::NodeLaunchType launchTy) {
  static_assert(static_cast<unsigned>(DXIL::NodeLaunchType::Thread) ==
                    _countof(NodeLaunchTypeNames) - 1,
                "Invalid launch type or names");
  return NodeLaunchTypeNames[static_cast<unsigned int>(launchTy)];
}

DXIL::NodeLaunchType ShaderModel::NodeLaunchTypeFromName(llvm::StringRef name) {
  return llvm::StringSwitch<DXIL::NodeLaunchType>(name.lower())
      .Case("broadcasting", DXIL::NodeLaunchType::Broadcasting)
      .Case("coalescing", DXIL::NodeLaunchType::Coalescing)
      .Case("thread", DXIL::NodeLaunchType::Thread)
      .Default(DXIL::NodeLaunchType::Invalid);
}

} // namespace hlsl
