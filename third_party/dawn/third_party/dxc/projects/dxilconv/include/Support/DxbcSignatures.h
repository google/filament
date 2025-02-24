///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// DxbcSignatures.h                                                          //
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
// This file is distributed under the University of Illinois Open Source     //
// License. See LICENSE.TXT for details.                                     //
//                                                                           //
// Declaration of shader parameter structs in DXBC container.                //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#pragma once

typedef D3D_NAME D3D10_NAME;
typedef D3D_REGISTER_COMPONENT_TYPE D3D10_REGISTER_COMPONENT_TYPE;

typedef struct _D3D11_INTERNALSHADER_PARAMETER_FOR_GS {
  UINT Stream; // Stream index (parameters must appear in non-decreasing stream
               // order)
  UINT SemanticName;                           // Offset to LPCSTR
  UINT SemanticIndex;                          // Semantic Index
  D3D10_NAME SystemValue;                      // Internally defined enumeration
  D3D10_REGISTER_COMPONENT_TYPE ComponentType; // Type of  of bits
  UINT Register;                               // Register Index
  BYTE Mask; // Combination of D3D10_COMPONENT_MASK values

  // The following unioned fields, NeverWrites_Mask and AlwaysReads_Mask, are
  // exclusively used for output signatures or input signatures, respectively.
  //
  // For an output signature, NeverWrites_Mask indicates that the shader the
  // signature belongs to never writes to the masked components of the output
  // register.  Meaningful bits are the ones set in Mask above.
  //
  // For an input signature, AlwaysReads_Mask indicates that the shader the
  // signature belongs to always reads the masked components of the input
  // register.  Meaningful bits are the ones set in the Mask above.
  //
  // This allows many shaders to share similar signatures even though some of
  // them may not happen to use all of the inputs/outputs - something which may
  // not be obvious when authored.  The NeverWrites_Mask and AlwaysReads_Mask
  // can be checked in a debug layer at runtime for the one interesting case:
  // that a shader that always reads a value is fed by a shader that always
  // writes it.  Cases where shaders may read values or may not cannot be
  // validated unfortunately.
  //
  // In scenarios where a signature is being passed around standalone (so it
  // isn't tied to input or output of a given shader), this union can be zeroed
  // out in the absence of more information.  This effectively forces off
  // linkage validation errors with the signature, since if interpreted as a
  // input or output signature somehow, since the meaning on output would be
  // "everything is always written" and on input it would be "nothing is always
  // read".
  union {
    BYTE NeverWrites_Mask; // For an output signature, the shader the signature
                           // belongs to never writes the masked components of
                           // the output register.
    BYTE AlwaysReads_Mask; // For an input signature, the shader the signature
                           // belongs to always reads the masked components of
                           // the input register.
  };
} D3D11_INTERNALSHADER_PARAMETER_FOR_GS,
    *LPD3D11_INTERNALSHADER_PARAMETER_FOR_GS;

typedef struct _D3D11_INTERNALSHADER_PARAMETER_11_1 {
  UINT Stream; // Stream index (parameters must appear in non-decreasing stream
               // order)
  UINT SemanticName;                           // Offset to LPCSTR
  UINT SemanticIndex;                          // Semantic Index
  D3D10_NAME SystemValue;                      // Internally defined enumeration
  D3D10_REGISTER_COMPONENT_TYPE ComponentType; // Type of  of bits
  UINT Register;                               // Register Index
  BYTE Mask; // Combination of D3D10_COMPONENT_MASK values

  // The following unioned fields, NeverWrites_Mask and AlwaysReads_Mask, are
  // exclusively used for output signatures or input signatures, respectively.
  //
  // For an output signature, NeverWrites_Mask indicates that the shader the
  // signature belongs to never writes to the masked components of the output
  // register.  Meaningful bits are the ones set in Mask above.
  //
  // For an input signature, AlwaysReads_Mask indicates that the shader the
  // signature belongs to always reads the masked components of the input
  // register.  Meaningful bits are the ones set in the Mask above.
  //
  // This allows many shaders to share similar signatures even though some of
  // them may not happen to use all of the inputs/outputs - something which may
  // not be obvious when authored.  The NeverWrites_Mask and AlwaysReads_Mask
  // can be checked in a debug layer at runtime for the one interesting case:
  // that a shader that always reads a value is fed by a shader that always
  // writes it.  Cases where shaders may read values or may not cannot be
  // validated unfortunately.
  //
  // In scenarios where a signature is being passed around standalone (so it
  // isn't tied to input or output of a given shader), this union can be zeroed
  // out in the absence of more information.  This effectively forces off
  // linkage validation errors with the signature, since if interpreted as a
  // input or output signature somehow, since the meaning on output would be
  // "everything is always written" and on input it would be "nothing is always
  // read".
  union {
    BYTE NeverWrites_Mask; // For an output signature, the shader the signature
                           // belongs to never writes the masked components of
                           // the output register.
    BYTE AlwaysReads_Mask; // For an input signature, the shader the signature
                           // belongs to always reads the masked components of
                           // the input register.
  };

  D3D_MIN_PRECISION MinPrecision; // Minimum precision of input/output data
} D3D11_INTERNALSHADER_PARAMETER_11_1, *LPD3D11_INTERNALSHADER_PARAMETER_11_1;

typedef struct _D3D10_INTERNALSHADER_SIGNATURE {
  UINT Parameters;    // Number of parameters
  UINT ParameterInfo; // Offset to D3D10_INTERNALSHADER_PARAMETER[Parameters]
} D3D10_INTERNALSHADER_SIGNATURE, *LPD3D10_INTERNALSHADER_SIGNATURE;

typedef struct _D3D10_INTERNALSHADER_PARAMETER {
  UINT SemanticName;                           // Offset to LPCSTR
  UINT SemanticIndex;                          // Semantic Index
  D3D10_NAME SystemValue;                      // Internally defined enumeration
  D3D10_REGISTER_COMPONENT_TYPE ComponentType; // Type of  of bits
  UINT Register;                               // Register Index
  BYTE Mask; // Combination of D3D10_COMPONENT_MASK values

  // The following unioned fields, NeverWrites_Mask and AlwaysReads_Mask, are
  // exclusively used for output signatures or input signatures, respectively.
  //
  // For an output signature, NeverWrites_Mask indicates that the shader the
  // signature belongs to never writes to the masked components of the output
  // register.  Meaningful bits are the ones set in Mask above.
  //
  // For an input signature, AlwaysReads_Mask indicates that the shader the
  // signature belongs to always reads the masked components of the input
  // register.  Meaningful bits are the ones set in the Mask above.
  //
  // This allows many shaders to share similar signatures even though some of
  // them may not happen to use all of the inputs/outputs - something which may
  // not be obvious when authored.  The NeverWrites_Mask and AlwaysReads_Mask
  // can be checked in a debug layer at runtime for the one interesting case:
  // that a shader that always reads a value is fed by a shader that always
  // writes it.  Cases where shaders may read values or may not cannot be
  // validated unfortunately.
  //
  // In scenarios where a signature is being passed around standalone (so it
  // isn't tied to input or output of a given shader), this union can be zeroed
  // out in the absence of more information.  This effectively forces off
  // linkage validation errors with the signature, since if interpreted as a
  // input or output signature somehow, since the meaning on output would be
  // "everything is always written" and on input it would be "nothing is always
  // read".
  union {
    BYTE NeverWrites_Mask; // For an output signature, the shader the signature
                           // belongs to never writes the masked components of
                           // the output register.
    BYTE AlwaysReads_Mask; // For an input signature, the shader the signature
                           // belongs to always reads the masked components of
                           // the input register.
  };
} D3D10_INTERNALSHADER_PARAMETER, *LPD3D10_INTERNALSHADER_PARAMETER;
