///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// RDATDumper.cpp                                                            //
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
// This file is distributed under the University of Illinois Open Source     //
// License. See LICENSE.TXT for details.                                     //
//                                                                           //
// Use this to dump DxilRuntimeData (RDAT) for testing.                      //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#include "dxc/Test/RDATDumper.h"
#include "dxc/Support/Global.h"

using namespace hlsl;
using namespace RDAT;

namespace hlsl {
namespace dump {

void DumpRuntimeData(const RDAT::DxilRuntimeData &RDAT, DumpContext &d) {
  const RDATContext &ctx = RDAT.GetContext();
  d.WriteLn("DxilRuntimeData (size = ", RDAT.GetDataSize(), " bytes):");
  d.Indent();

  d.WriteLn("StringBuffer (size = ", ctx.StringBuffer.Size(), " bytes)");
  d.WriteLn("IndexTable (size = ", ctx.IndexTable.Count() * 4, " bytes)");
  d.WriteLn("RawBytes (size = ", ctx.RawBytes.Size(), " bytes)");

// Once per table.
#define RDAT_STRUCT_TABLE(type, table)                                         \
  DumpRecordTable<type>(ctx, d, #table, ctx.Table(RecordTableIndex::table));
#define DEF_RDAT_TYPES DEF_RDAT_DEFAULTS
#include "dxc/DxilContainer/RDAT_Macros.inl"

  d.Dedent();
}

template <typename RecordType>
void DumpRecordTable(const RDAT::RDATContext &ctx, DumpContext &d,
                     const char *tableName, const RDAT::TableReader &table) {
  if (!table.Count())
    return;
  d.WriteLn("RecordTable (stride = ", table.Stride(), " bytes) ", tableName,
            "[", table.Count(), "] = {");
  d.Indent();
  for (unsigned i = 0; i < table.Count(); i++) {
    DumpRecordTableEntry<RecordType>(ctx, d, i);
  }
  d.Dedent();
  d.WriteLn("}");
}

template <typename RecordType>
void DumpRecordTableEntry(const RDAT::RDATContext &ctx, DumpContext &d,
                          uint32_t i) {
  // Visit() will prevent recursive/repeated reference expansion.  Resetting
  // for each top-level table entry prevents a record from dumping differently
  // depending on differences in other unrelated records.
  d.VisitReset();

  // RecordRefDumper handles derived types.
  RecordRefDumper<RecordType> rrDumper(i);

  d.WriteLn("<", i, ":", rrDumper.TypeName(ctx), "> = {");
  rrDumper.Dump(ctx, d);
  d.WriteLn("}");
}

template <typename _T>
void DumpRecordValue(const hlsl::RDAT::RDATContext &ctx, DumpContext &d,
                     const char *tyName, const char *memberName,
                     const _T *memberPtr) {
  d.WriteLn(memberName, ": <", tyName, ">");
  DumpWithBase(ctx, d, memberPtr);
}

template <typename _T>
void DumpRecordRef(const hlsl::RDAT::RDATContext &ctx, DumpContext &d,
                   const char *tyName, const char *memberName,
                   hlsl::RDAT::RecordRef<_T> rr) {
  RecordRefDumper<_T> rrDumper(rr.Index);
  const char *storedTypeName = rrDumper.TypeName(ctx);
  if (nullptr == storedTypeName)
    storedTypeName = tyName;
  // Unique visit location is based on end of struct so derived are not skipped
  if (rr.Get(ctx)) {
    if (d.Visit(rr.Get(ctx))) {
      d.WriteLn(memberName, ": <", rr.Index, ":", storedTypeName, "> = {");
      rrDumper.Dump(ctx, d);
      d.WriteLn("}");
    } else {
      d.WriteLn(memberName, ": <", rr.Index, ":", storedTypeName, ">");
    }
  } else {
    d.WriteLn(memberName, ": <", rr.Index, ":", storedTypeName,
              "> = <nullptr>");
  }
}

template <typename _T>
void DumpRecordArrayRef(const hlsl::RDAT::RDATContext &ctx, DumpContext &d,
                        const char *tyName, const char *memberName,
                        hlsl::RDAT::RecordArrayRef<_T> rar) {
  auto row = ctx.IndexTable.getRow(rar.Index);
  if (row.Count()) {
    d.WriteLn(memberName, ": <", rar.Index, ":RecordArrayRef<", tyName, ">[",
              row.Count(), "]>  = {");
    d.Indent();
    for (uint32_t i = 0; i < row.Count(); ++i) {
      RecordRefDumper<_T> rrDumper(row.At(i));
      if (rrDumper.Get(ctx)) {
        if (d.Visit(rrDumper.Get(ctx))) {
          d.WriteLn("[", i, "]: <", rrDumper.Index, ":", rrDumper.TypeName(ctx),
                    "> = {");
          rrDumper.Dump(ctx, d);
          d.WriteLn("}");
        } else {
          d.WriteLn("[", i, "]: <", rrDumper.Index, ":", rrDumper.TypeName(ctx),
                    ">");
        }
      } else {
        d.WriteLn("[", i, "]: <", row.At(i), ":", tyName, "> = <nullptr>");
      }
    }
    d.Dedent();
    d.WriteLn("}");
  } else {
    d.WriteLn(memberName, ": <RecordArrayRef<", tyName, ">[0]> = {}");
  }
}

void DumpStringArray(const hlsl::RDAT::RDATContext &ctx, DumpContext &d,
                     const char *memberName, hlsl::RDAT::RDATStringArray sa) {
  auto sar = sa.Get(ctx);
  if (sar && sar.Count()) {
    d.WriteLn(memberName, ": <", sa.Index, ":string[", sar.Count(), "]> = {");
    d.Indent();
    for (uint32_t _i = 0; _i < (uint32_t)sar.Count(); ++_i) {
      const char *str = sar[_i];
      if (str) {
        d.WriteLn("[", _i, "]: ", QuotedStringValue(str));
      } else {
        d.WriteLn("[", _i, "]: <nullptr>");
      }
    }
    d.Dedent();
    d.WriteLn("}");
  } else {
    d.WriteLn(memberName, ": <string[0]> = {}");
  }
}

// Currently dumps index array inline
void DumpIndexArray(const hlsl::RDAT::RDATContext &ctx, DumpContext &d,
                    const char *memberName, uint32_t index) {
  auto indexArray = ctx.IndexTable.getRow(index);
  unsigned arraySize = indexArray.Count();
  std::ostringstream oss;
  std::ostream &os = oss;
  d.Write(os, memberName, ": <", index, ":array[", arraySize, "]> = { ");
  for (unsigned i = 0; i < arraySize; ++i) {
    d.Write(os, indexArray[i]);
    if (i < arraySize - 1)
      d.Write(os, ", ");
  }
  d.Write(os, " }");
  d.WriteLn(oss.str());
}

void DumpBytesRef(const hlsl::RDAT::RDATContext &ctx, DumpContext &d,
                  const char *memberName, hlsl::RDAT::BytesRef bytesRef) {
  d.WriteLn(memberName, ": <", bytesRef.Offset, ":bytes[", bytesRef.Size, "]>");
}

template <typename _T>
void DumpValueArray(DumpContext &d, const char *memberName,
                    const char *typeName, const void *valueArray,
                    unsigned arraySize) {
  d.WriteLn(memberName, ": ", typeName, "[", arraySize, "] = { ");
  for (unsigned i = 0; i < arraySize; i++) {
    d.Write(((const _T *)valueArray)[i]);
    if (i < arraySize - 1)
      d.Write(", ");
  }
  d.WriteLn(" }");
}

#define DEF_RDAT_TYPES DEF_RDAT_DUMP_IMPL
#include "dxc/DxilContainer/RDAT_Macros.inl"

} // namespace dump
} // namespace hlsl
