//
// Copyright 2021 The ANGLE Project. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// rewrite_incides_shared:
// defines for provoking vertex rewriting.

#ifndef rewrite_indices_shared_h
#define rewrite_indices_shared_h
#define MTL_FIX_INDEX_BUFFER_KEY_FUNCTION_CONSTANT_INDEX 1

#define MtlFixIndexBufferKeyTypeMask 0x03U
#define MtlFixIndexBufferKeyInShift 0U
#define MtlFixIndexBufferKeyOutShift 2U
#define MtlFixIndexBufferKeyVoid 0U
#define MtlFixIndexBufferKeyUint16 2U
#define MtlFixIndexBufferKeyUint32 3U
#define MtlFixIndexBufferKeyModeMask 0x0FU
#define MtlFixIndexBufferKeyModeShift 4U
#define MtlFixIndexBufferKeyPoints 0x00U
#define MtlFixIndexBufferKeyLines 0x01U
#define MtlFixIndexBufferKeyLineLoop 0x02U
#define MtlFixIndexBufferKeyLineStrip 0x03U
#define MtlFixIndexBufferKeyTriangles 0x04U
#define MtlFixIndexBufferKeyTriangleStrip 0x05U
#define MtlFixIndexBufferKeyTriangleFan 0x06U
#define MtlFixIndexBufferKeyPrimRestart 0x00100U
#define MtlFixIndexBufferKeyProvokingVertexLast 0x00200U
#endif /* rewrite_indices_shared_h */
