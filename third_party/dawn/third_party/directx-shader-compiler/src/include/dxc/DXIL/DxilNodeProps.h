///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// DxilNodeProps.h                                                           //
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
// This file is distributed under the University of Illinois Open Source     //
// License. See LICENSE.TXT for details.                                     //
//                                                                           //
// Representation of DXIL nodes and node records properties                  //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#pragma once

#include "DxilConstants.h"
#include <string>

namespace llvm {
class StringRef;
}

namespace hlsl {

//------------------------------------------------------------------------------
//
// NodeID
//
struct NodeID {
  std::string Name;
  unsigned Index;
};

//------------------------------------------------------------------------------
//
// SVDispatchGrid
//
struct SVDispatchGrid {
  unsigned ByteOffset;
  DXIL::ComponentType ComponentType;
  unsigned NumComponents;
};

//------------------------------------------------------------------------------
//
// NodeRecordType
//
struct NodeRecordType {
  unsigned size;
  unsigned alignment;
  SVDispatchGrid SV_DispatchGrid;
};

//------------------------------------------------------------------------------
//
// NodeInfo
//
struct NodeInfo {
  NodeInfo() : NodeInfo(DXIL::NodeIOFlags::None) {}
  NodeInfo(DXIL::NodeIOFlags flags, unsigned recordSize = 0)
      : IOFlags((unsigned)flags), RecordSize(recordSize) {}
  NodeInfo(DXIL::NodeIOKind kind, unsigned recordSize = 0)
      : NodeInfo((DXIL::NodeIOFlags)kind, recordSize) {}

  unsigned IOFlags;
  unsigned RecordSize; // 0 if EmptyNodeOutput
};

//------------------------------------------------------------------------------
//
// NodeRecordInfo
//
typedef NodeInfo NodeRecordInfo;

//------------------------------------------------------------------------------
//
// NodeProps
//
struct NodeProps {
  unsigned MetadataIdx;
  NodeInfo Info;
};

//------------------------------------------------------------------------------
//
// NodeInputRecerdProps
//
struct NodeInputRecordProps {
  unsigned MetadataIdx;
  NodeRecordInfo RecordInfo;
};

//------------------------------------------------------------------------------
//
// NodeFlags - helper class for working with DXIL::NodeIOFlags and
// DXIL::NodeIOKind
//
struct NodeFlags {
public:
  NodeFlags();
  NodeFlags(DXIL::NodeIOFlags flags);
  NodeFlags(DXIL::NodeIOKind kind);
  NodeFlags(uint32_t F);

  bool operator==(const NodeFlags &o) const;
  operator uint32_t() const;

  DXIL::NodeIOKind GetNodeIOKind() const;
  DXIL::NodeIOFlags GetNodeIOFlags() const;

  bool IsInputRecord() const;
  bool IsRecord() const;
  bool IsOutput() const;
  bool IsOutputNode() const;
  bool IsOutputRecord() const;
  bool IsReadWrite() const;
  bool IsEmpty() const;
  bool IsEmptyInput() const;
  bool IsValidNodeKind() const;
  bool RecordTypeMatchesLaunchType(DXIL::NodeLaunchType launchType) const;

  void SetTrackRWInputSharing();
  bool GetTrackRWInputSharing() const;

  void SetGloballyCoherent();
  bool GetGloballyCoherent() const;

private:
  DXIL::NodeIOFlags m_Flags;

}; // end of NodeFlags

//------------------------------------------------------------------------------
//
// NodeIOProperties
//
struct NodeIOProperties {
  NodeFlags Flags = NodeFlags();
  NodeRecordType RecordType = {};
  NodeID OutputID = {};
  unsigned MaxRecords = 0;
  int MaxRecordsSharedWith = -1;
  unsigned OutputArraySize = 0;
  bool AllowSparseNodes = false;

public:
  NodeIOProperties() {}
  NodeIOProperties(NodeFlags flags) : Flags(flags) {}

  NodeInfo GetNodeInfo() const;
  NodeRecordInfo GetNodeRecordInfo() const;
};

} // namespace hlsl
