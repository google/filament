///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// RDAT_PdbInfoTypes.inl                                                     //
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
// This file is distributed under the University of Illinois Open Source     //
// License. See LICENSE.TXT for details.                                     //
//                                                                           //
// Defines macros use to define types Dxil PDBInfo data.                     //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#ifdef DEF_RDAT_TYPES

#define RECORD_TYPE DxilPdbInfoLibrary
RDAT_STRUCT_TABLE(DxilPdbInfoLibrary, DxilPdbInfoLibraryTable)
RDAT_STRING(Name)
RDAT_BYTES(Data)
RDAT_STRUCT_END()
#undef RECORD_TYPE

#define RECORD_TYPE DxilPdbInfoSource
RDAT_STRUCT_TABLE(DxilPdbInfoSource, DxilPdbInfoSourceTable)
RDAT_STRING(Name)
RDAT_STRING(Content)
RDAT_STRUCT_END()
#undef RECORD_TYPE

#define RECORD_TYPE DxilPdbInfo
RDAT_STRUCT_TABLE(DxilPdbInfo, DxilPdbInfoTable)
RDAT_RECORD_ARRAY_REF(DxilPdbInfoSource, Sources)
RDAT_RECORD_ARRAY_REF(DxilPdbInfoLibrary, Libraries)
RDAT_STRING_ARRAY_REF(ArgPairs)
RDAT_BYTES(Hash)
RDAT_STRING(PdbName)
RDAT_VALUE(uint32_t, CustomToolchainId)
RDAT_BYTES(CustomToolchainData)
RDAT_BYTES(WholeDxil)
RDAT_STRUCT_END()
#undef RECORD_TYPE

#endif // DEF_RDAT_TYPES
