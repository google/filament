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
#ifndef O3DGC_TRIANGLE_LIST_ENCODER_H
#define O3DGC_TRIANGLE_LIST_ENCODER_H

#include "o3dgcCommon.h"
#include "o3dgcAdjacencyInfo.h"
#include "o3dgcBinaryStream.h"
#include "o3dgcFIFO.h"
#include "o3dgcTriangleFans.h"

namespace o3dgc
{
    //! 
    template <class T>
    class TriangleListEncoder
    {
    public:    
        //! Constructor.
                                    TriangleListEncoder(void);
        //! Destructor.
                                    ~TriangleListEncoder(void);
        //! 
        O3DGCErrorCode              Encode(const T * const triangles,
                                           const unsigned long * const indexBufferIDs,
                                           const long numTriangles,
                                           const long numVertices,
                                           BinaryStream & bstream);
        O3DGCStreamType       GetStreamType() const { return m_streamType; }
        void                        SetStreamType(O3DGCStreamType streamType) { m_streamType = streamType; }
        const long *                GetInvVMap() const { return m_invVMap;}
        const long *                GetInvTMap() const { return m_invTMap;}
        const long *                GetVMap()    const { return m_vmap;}
        const long *                GetTMap()    const { return m_tmap;}
        const AdjacencyInfo &       GetVertexToTriangle() const { return m_vertexToTriangle;}

        private:
        O3DGCErrorCode              Init(const T * const triangles, 
                                         long numTriangles, 
                                         long numVertices);
        O3DGCErrorCode              CompueLocalConnectivityInfo(const long focusVertex);
        O3DGCErrorCode              ProcessVertex( long focusVertex);
        O3DGCErrorCode              ComputeTFANDecomposition(const long focusVertex);
        O3DGCErrorCode              CompressTFAN(const long focusVertex);

        long                        m_vertexCount;
        long                        m_triangleCount;
        long                        m_maxNumVertices;
        long                        m_maxNumTriangles;
        long                        m_numNonConqueredTriangles;
        long                        m_numConqueredTriangles;
        long                        m_numVisitedVertices;
        long                        m_numTriangles;
        long                        m_numVertices;
        long                        m_maxSizeVertexToTriangle;
        T const *                   m_triangles;
        long *                      m_vtags;
        long *                      m_ttags;
        long *                      m_vmap;
        long *                      m_invVMap;
        long *                      m_tmap;
        long *                      m_invTMap;
        long *                      m_count;
        long *                      m_nonConqueredTriangles;
        long *                      m_nonConqueredEdges;
        long *                      m_visitedVertices;
        long *                      m_visitedVerticesValence;
        FIFO<long>                  m_vfifo;
        AdjacencyInfo               m_vertexToTriangle;
        AdjacencyInfo               m_triangleToTriangle;
        AdjacencyInfo               m_triangleToTriangleInv;
        TriangleFans                m_tfans;
        CompressedTriangleFans      m_ctfans;
        O3DGCStreamType       m_streamType;
    };
}
#include "o3dgcTriangleListEncoder.inl"    // template implementation
#endif // O3DGC_TRIANGLE_LIST_ENCODER_H

