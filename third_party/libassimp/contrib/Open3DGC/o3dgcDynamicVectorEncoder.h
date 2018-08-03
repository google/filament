/*
Copyright (c) 2013 Khaled Mammou - Advanced Micro Devices, Inc.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/


#pragma once
#ifndef O3DGC_DYNAMIC_VECTOR_ENCODER_H
#define O3DGC_DYNAMIC_VECTOR_ENCODER_H

#include "o3dgcCommon.h"
#include "o3dgcBinaryStream.h"
#include "o3dgcDynamicVector.h"

namespace o3dgc
{
    //! 
    class DynamicVectorEncoder
    {
    public:    
        //! Constructor.
                                    DynamicVectorEncoder(void);
        //! Destructor.
                                    ~DynamicVectorEncoder(void);
        //! 
        O3DGCErrorCode              Encode(const DVEncodeParams & params,
                                           const DynamicVector & dynamicVector,
                                           BinaryStream & bstream);
        O3DGCStreamType             GetStreamType() const { return m_streamType; }
        void                        SetStreamType(O3DGCStreamType streamType) { m_streamType = streamType; }

        private:
        O3DGCErrorCode              EncodeHeader(const DVEncodeParams & params,
                                                 const DynamicVector & dynamicVector,
                                                 BinaryStream & bstream);
        O3DGCErrorCode              EncodePayload(const DVEncodeParams & params, 
                                                  const DynamicVector & dynamicVector,
                                                  BinaryStream & bstream);
        O3DGCErrorCode              Quantize(const Real * const floatArray, 
                                             unsigned long numFloatArray,
                                             unsigned long dimFloatArray,
                                             unsigned long stride,
                                             const Real * const minFloatArray,
                                             const Real * const maxFloatArray,
                                             unsigned long nQBits);
        O3DGCErrorCode              EncodeAC(unsigned long num, 
                                             unsigned long dim, 
                                             unsigned long M, 
                                             unsigned long & encodedBytes);

        unsigned long               m_posSize;
        unsigned long               m_sizeBufferAC;
        unsigned long               m_maxNumVectors;
        unsigned long               m_numVectors;
        unsigned long               m_dimVectors;
        unsigned char *             m_bufferAC;
        long *                      m_quantVectors;
        O3DGCStreamType             m_streamType;
    };
}
#endif // O3DGC_DYNAMIC_VECTOR_ENCODER_H

