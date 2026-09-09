///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// DxilNodeProps.cpp                                                         //
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
// This file is distributed under the University of Illinois Open Source     //
// License. See LICENSE.TXT for details.                                     //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#include "dxc/DXIL/DxilNodeProps.h"

namespace hlsl {

//------------------------------------------------------------------------------
//
// NodeFlags methods
//
NodeFlags::NodeFlags() : m_Flags(DXIL::NodeIOFlags::None) {}

NodeFlags::NodeFlags(DXIL::NodeIOFlags flags) : m_Flags(flags) {}

NodeFlags::NodeFlags(DXIL::NodeIOKind kind)
    : m_Flags((DXIL::NodeIOFlags)kind) {}

NodeFlags::NodeFlags(uint32_t F) : NodeFlags((DXIL::NodeIOFlags)F) {}

bool NodeFlags::operator==(const hlsl::NodeFlags &o) const {
  return m_Flags == o.m_Flags;
}

NodeFlags::operator uint32_t() const { return (uint32_t)m_Flags; }

DXIL::NodeIOKind NodeFlags::GetNodeIOKind() const {
  return (DXIL::NodeIOKind)((uint32_t)m_Flags &
                            (uint32_t)DXIL::NodeIOFlags::NodeIOKindMask);
}

DXIL::NodeIOFlags NodeFlags::GetNodeIOFlags() const { return m_Flags; }

bool NodeFlags::IsInputRecord() const {
  return ((uint32_t)m_Flags & (uint32_t)DXIL::NodeIOFlags::Input) != 0;
}

bool NodeFlags::IsOutput() const {
  return ((uint32_t)m_Flags & (uint32_t)DXIL::NodeIOFlags::Output) != 0;
}

bool NodeFlags::IsRecord() const {
  return ((uint32_t)m_Flags &
          (uint32_t)DXIL::NodeIOFlags::RecordGranularityMask) != 0;
}

bool NodeFlags::IsOutputNode() const { return IsOutput() && !IsRecord(); }

bool NodeFlags::IsOutputRecord() const { return IsOutput() && IsRecord(); }

bool NodeFlags::IsReadWrite() const {
  return ((uint32_t)m_Flags & (uint32_t)DXIL::NodeIOFlags::ReadWrite) != 0;
}

bool NodeFlags::IsEmpty() const {
  return ((uint32_t)m_Flags & (uint32_t)DXIL::NodeIOFlags::EmptyRecord) != 0;
}

bool NodeFlags::IsEmptyInput() const { return IsEmpty() && IsInputRecord(); }

bool NodeFlags::IsValidNodeKind() const {
  return GetNodeIOKind() != DXIL::NodeIOKind::Invalid;
}

bool NodeFlags::RecordTypeMatchesLaunchType(
    DXIL::NodeLaunchType launchType) const {
  DXIL::NodeIOFlags recordLaunchType = (DXIL::NodeIOFlags)(
      (uint32_t)m_Flags & (uint32_t)DXIL::NodeIOFlags::RecordGranularityMask);
  return (launchType == DXIL::NodeLaunchType::Broadcasting &&
          recordLaunchType == DXIL::NodeIOFlags::DispatchRecord) ||
         (launchType == DXIL::NodeLaunchType::Coalescing &&
          recordLaunchType == DXIL::NodeIOFlags::GroupRecord) ||
         (launchType == DXIL::NodeLaunchType::Thread &&
          recordLaunchType == DXIL::NodeIOFlags::ThreadRecord);
}

void NodeFlags::SetTrackRWInputSharing() {
  m_Flags = (DXIL::NodeIOFlags)(
      (uint32_t)m_Flags | (uint32_t)DXIL::NodeIOFlags::TrackRWInputSharing);
}

bool NodeFlags::GetTrackRWInputSharing() const {
  return ((uint32_t)m_Flags &
          (uint32_t)DXIL::NodeIOFlags::TrackRWInputSharing) != 0;
}

void NodeFlags::SetGloballyCoherent() {
  m_Flags = (DXIL::NodeIOFlags)((uint32_t)m_Flags |
                                (uint32_t)DXIL::NodeIOFlags::GloballyCoherent);
}

bool NodeFlags::GetGloballyCoherent() const {
  return ((uint32_t)m_Flags & (uint32_t)DXIL::NodeIOFlags::GloballyCoherent) !=
         0;
}

//------------------------------------------------------------------------------
//
// NodeIOProperties methods.
//

NodeInfo NodeIOProperties::GetNodeInfo() const {
  return NodeInfo(Flags.GetNodeIOFlags(), RecordType.size);
}

NodeRecordInfo NodeIOProperties::GetNodeRecordInfo() const {
  return NodeInfo(Flags.GetNodeIOFlags(), RecordType.size);
}

} // namespace hlsl
