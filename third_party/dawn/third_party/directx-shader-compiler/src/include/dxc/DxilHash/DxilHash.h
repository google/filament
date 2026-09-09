///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// DxilHash.h                                                                //
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
// This file is distributed under the University of Illinois Open Source     //
// License. See LICENSE.TXT for details.                                     //
//                                                                           //
//                                                                           //
// DXBC/DXIL container hashing functions                                     //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#pragma once

#define DXIL_CONTAINER_HASH_SIZE 16

// Prototype for hash computing function
// pOutHash must always return a DXIL_CONTAINER_HASH_SIZE byte hash result.
typedef void HASH_FUNCTION_PROTO(const BYTE *pData, UINT32 byteCount,
                                 BYTE *pOutHash);

// **************************************************************************************
// **** DO NOT USE THESE ROUTINES TO PROVIDE FUNCTIONALITY THAT NEEDS TO BE
// SECURE!!! ***
// **************************************************************************************
// Derived from: RSA Data Security, Inc. M
//                                       D
//                                       5 Message-Digest Algorithm
// Computes a 128-bit hash of pData (size byteCount), returning 16 BYTE output
void ComputeHashRetail(const BYTE *pData, UINT32 byteCount, BYTE *pOutHash);
void ComputeHashDebug(const BYTE *pData, UINT32 byteCount, BYTE *pOutHash);
// **************************************************************************************
// **** DO NOT USE THESE ROUTINES TO PROVIDE FUNCTIONALITY THAT NEEDS TO BE
// SECURE!!! ***
// **************************************************************************************
