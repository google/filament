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
#ifndef O3DGC_DYNAMIC_VECTOR_DECODER_H
#define O3DGC_DYNAMIC_VECTOR_DECODER_H

#include "o3dgcCommon.h"
#include "o3dgcBinaryStream.h"
#include "o3dgcDVEncodeParams.h"
#include "o3dgcDynamicVector.h"

namespace o3dgc
{
    //! 
    class DynamicVectorDecoder
    {
    public:    
        //! Constructor.
                                    DynamicVectorDecoder(void);
        //! Destructor.
                                    ~DynamicVectorDecoder(void);
        //! 
        //!
        O3DGCErrorCode              DecodeHeader(DynamicVector & dynamicVector,
                                                 const BinaryStream & bstream);
        //!                         
        O3DGCErrorCode              DecodePlayload(DynamicVector & dynamicVector, 
                                                   const BinaryStream & bstream);

        O3DGCStreamType             GetStreamType() const { return m_streamType; }
        void                        SetStreamType(O3DGCStreamType streamType) { m_streamType = streamType; }
        unsigned long               GetIterator() const { return m_iterator;}
        O3DGCErrorCode              SetIterator(unsigned long iterator) { m_iterator = iterator; return O3DGC_OK; }

        private:
        O3DGCErrorCode              IQuantize(Real * const floatArray, 
                                              unsigned long numFloatArray,
                                              unsigned long dimFloatArray,
                                              unsigned long stride,
                                              const Real * const minFloatArray,
                                              const Real * const maxFloatArray,
                                              unsigned long nQBits);

        unsigned long               m_streamSize;
        unsigned long               m_maxNumVectors;
        unsigned long               m_numVectors;
        unsigned long               m_dimVectors;
        unsigned long               m_iterator;
        long *                      m_quantVectors;
        DVEncodeParams              m_params;
        O3DGCStreamType             m_streamType;
    };
}
#endif // O3DGC_DYNAMIC_VECTOR_DECODER_H

