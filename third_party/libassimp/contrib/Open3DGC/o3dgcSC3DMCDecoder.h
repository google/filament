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
#ifndef O3DGC_SC3DMC_DECODER_H
#define O3DGC_SC3DMC_DECODER_H

#include "o3dgcCommon.h"
#include "o3dgcBinaryStream.h"
#include "o3dgcIndexedFaceSet.h"
#include "o3dgcSC3DMCEncodeParams.h"
#include "o3dgcTriangleListDecoder.h"

namespace o3dgc
{    
    //! 
    template <class T>
    class SC3DMCDecoder
    {
    public:    
        //! Constructor.
                                    SC3DMCDecoder(void)
                                    {
                                        m_iterator            = 0;
                                        m_streamSize          = 0;
                                        m_quantFloatArray     = 0;
                                        m_quantFloatArraySize = 0;
                                        m_normals             = 0;
                                        m_normalsSize         = 0;
                                        m_streamType          = O3DGC_STREAM_TYPE_UNKOWN;
                                    };
        //! Destructor.
                                    ~SC3DMCDecoder(void)
                                    {
                                        delete [] m_normals;
                                        delete [] m_quantFloatArray;
                                    }
        //!
        O3DGCErrorCode              DecodeHeader(IndexedFaceSet<T> & ifs,
                                                 const BinaryStream & bstream);
        //!                         
		O3DGCErrorCode              DecodePayload(IndexedFaceSet<T> & ifs,
                                                  const BinaryStream & bstream);
        const SC3DMCStats &         GetStats()    const { return m_stats;}
        unsigned long               GetIterator() const { return m_iterator;}
		O3DGCErrorCode              SetIterator(unsigned long iterator) { m_iterator = iterator; return O3DGC_OK; }
        

    private:                        
        O3DGCErrorCode              DecodeFloatArray(Real * const floatArray,
                                                     unsigned long numfloatArraySize,
                                                     unsigned long dimfloatArraySize,
                                                     unsigned long stride,
                                                     const Real * const minfloatArray,
                                                     const Real * const maxfloatArray,
                                                     unsigned long nQBits,
                                                     const IndexedFaceSet<T> & ifs,
                                                     O3DGCSC3DMCPredictionMode & predMode,
                                                     const BinaryStream & bstream);
        O3DGCErrorCode              IQuantizeFloatArray(Real * const floatArray,
                                                       unsigned long numfloatArraySize,
                                                       unsigned long dimfloatArraySize,
                                                       unsigned long stride,
                                                       const Real * const minfloatArray,
                                                       const Real * const maxfloatArray,
                                                       unsigned long nQBits);
        O3DGCErrorCode              DecodeIntArray(long * const intArray, 
                                                   unsigned long numIntArraySize,
                                                   unsigned long dimIntArraySize,
                                                   unsigned long stride,
                                                   const IndexedFaceSet<T> & ifs,
                                                   O3DGCSC3DMCPredictionMode & predMode,
                                                   const BinaryStream & bstream);
        O3DGCErrorCode              ProcessNormals(const IndexedFaceSet<T> & ifs);

        unsigned long               m_iterator;
        unsigned long               m_streamSize;
        SC3DMCEncodeParams          m_params;
        TriangleListDecoder<T>      m_triangleListDecoder;
        long *                      m_quantFloatArray;
        unsigned long               m_quantFloatArraySize;
        Vector<char>                m_orientation;
        Real *                      m_normals;
        unsigned long               m_normalsSize;
        SC3DMCStats                 m_stats;
        O3DGCStreamType             m_streamType;
    };
}
#include "o3dgcSC3DMCDecoder.inl"    // template implementation
#endif // O3DGC_SC3DMC_DECODER_H

