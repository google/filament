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
#ifndef O3DGC_TRIANGLE_LIST_DECODER_H
#define O3DGC_TRIANGLE_LIST_DECODER_H

#include "o3dgcCommon.h"
#include "o3dgcTriangleFans.h"
#include "o3dgcBinaryStream.h"
#include "o3dgcAdjacencyInfo.h"

namespace o3dgc
{
    
    //! 
    template <class T>
    class TriangleListDecoder
    {
    public:    
        //! Constructor.
                                    TriangleListDecoder(void)
                                    {
                                        m_vertexCount            = 0;
                                        m_triangleCount          = 0;
                                        m_numTriangles           = 0;
                                        m_numVertices            = 0;
                                        m_triangles              = 0;
                                        m_numConqueredTriangles  = 0;
                                        m_numVisitedVertices     = 0;
                                        m_visitedVertices        = 0;
                                        m_visitedVerticesValence = 0;
                                        m_maxNumVertices         = 0;
                                        m_maxNumTriangles        = 0;
                                        m_itNumTFans             = 0;
                                        m_itDegree               = 0;
                                        m_itConfig               = 0;
                                        m_itOperation            = 0;
                                        m_itIndex                = 0;
                                        m_tempTriangles          = 0;
                                        m_tempTrianglesSize      = 0;
                                        m_decodeTrianglesOrder   = false;
                                        m_decodeVerticesOrder    = false;
                                    };
        //! Destructor.
                                    ~TriangleListDecoder(void)
                                    {
                                        delete [] m_tempTriangles;
                                    };

        O3DGCStreamType       GetStreamType()       const { return m_streamType; }
        bool                        GetReorderTriangles() const { return m_decodeTrianglesOrder; }        
        bool                        GetReorderVertices()  const { return m_decodeVerticesOrder; }        
        void                        SetStreamType(O3DGCStreamType streamType) { m_streamType = streamType; }
        const AdjacencyInfo &       GetVertexToTriangle() const { return m_vertexToTriangle;}
        O3DGCErrorCode              Decode(T * const triangles,
                                           const long numTriangles,
                                           const long numVertices,
                                           const BinaryStream & bstream,
                                           unsigned long & iterator)
                                    {
                                        unsigned char compressionMask = bstream.ReadUChar(iterator, m_streamType); 
                                        m_decodeTrianglesOrder = ( (compressionMask&2) != 0);
                                        m_decodeVerticesOrder = ( (compressionMask&1) != 0); 
                                        if (m_decodeVerticesOrder)  // vertices reordering not supported
                                        {
                                            return O3DGC_ERROR_NON_SUPPORTED_FEATURE;
                                        }
                                        unsigned long maxSizeV2T = bstream.ReadUInt32(iterator, m_streamType);
                                        Init(triangles, numTriangles, numVertices, maxSizeV2T);
                                        m_ctfans.Load(bstream, iterator, m_decodeTrianglesOrder, m_streamType);
                                        Decompress();
                                        return O3DGC_OK;
                                    }
        O3DGCErrorCode              Reorder();

        private:
        O3DGCErrorCode              Init(T * const triangles, 
                                         const long numTriangles,
                                         const long numVertices,
                                         const long maxSizeV2T);
        O3DGCErrorCode              Decompress();
        O3DGCErrorCode              CompueLocalConnectivityInfo(const long focusVertex);
        O3DGCErrorCode              DecompressTFAN(const long focusVertex);

        unsigned long               m_itNumTFans;
        unsigned long               m_itDegree;
        unsigned long               m_itConfig;
        unsigned long               m_itOperation;
        unsigned long               m_itIndex;
        long                        m_maxNumVertices;
        long                        m_maxNumTriangles;
        long                        m_numTriangles;
        long                        m_numVertices;
        long                        m_tempTrianglesSize;
        T *                         m_triangles;
        T *                         m_tempTriangles;
        long                        m_vertexCount;
        long                        m_triangleCount;
        long                        m_numConqueredTriangles;
        long                        m_numVisitedVertices;
        long *                      m_visitedVertices;
        long *                      m_visitedVerticesValence;
        AdjacencyInfo               m_vertexToTriangle;
        CompressedTriangleFans      m_ctfans;
        TriangleFans                m_tfans;
        O3DGCStreamType       m_streamType;
        bool                        m_decodeTrianglesOrder;
        bool                        m_decodeVerticesOrder;
    };
}
#include "o3dgcTriangleListDecoder.inl"    // template implementation
#endif // O3DGC_TRIANGLE_LIST_DECODER_H

