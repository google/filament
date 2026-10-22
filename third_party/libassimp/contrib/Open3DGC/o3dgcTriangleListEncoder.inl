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
#ifndef O3DGC_TRIANGLE_LIST_ENCODER_INL
#define O3DGC_TRIANGLE_LIST_ENCODER_INL

namespace o3dgc
{
    // extract opposite edge
    template <class T>
    inline void CompueOppositeEdge(const long focusVertex, 
                                   const T * triangle,
                                   long & a, long & b)
    {                
        if ((long) triangle[0] == focusVertex)
        {
            a = triangle[1];
            b = triangle[2];
        }
        else if ((long) triangle[1] == focusVertex)
        {
            a = triangle[2];
            b = triangle[0];
        }
        else
        {
            a = triangle[0];
            b = triangle[1];
        }
    }
    inline bool IsCase0(long degree, long numIndices, const long * const ops, const long * const indices)
    {
        // ops: 1000001 vertices: -1 -2
        if ((numIndices != 2) || (degree < 2)) {
            return false;
        }
        if ((indices[0] != -1) ||(indices[1] != -2) || 
            (ops[0] != 1)       ||(ops[degree-1] != 1)  ) return false;
        for (long u = 1; u < degree-1; u++) {
            if (ops[u] != 0) return false;
        }
        return true;
    }
    inline bool IsCase1(long degree, long numIndices, const long * const ops, const long * const indices)
    {
        // ops: 1xxxxxx1 indices: -1 x x x x x -2
        if ((degree < 2) || (numIndices < 1))
        {
            return false;
        }
        if ((indices[0] != -1) ||(indices[numIndices-1] != -2) || 
            (ops[0] != 1)       ||(ops[degree-1] != 1)  ) return false;
        return true;
    }
    inline bool IsCase2(long degree, long numIndices, const long * const ops, const long * const indices)
    {
        // ops: 00000001 indices: -1
        if ((degree < 2) || (numIndices!= 1))
        {
            return false;
        }
        if ((indices[0] != -1) || (ops[degree-1] != 1)  ) return false;
        for (long u = 0; u < degree-1; u++) {
            if (ops[u] != 0) return false;
        }
        return true;
    }
    inline bool IsCase3(long degree, long numIndices, const long * const ops, const long * const indices)
    {
        // ops: 00000001 indices: -2
        if ((degree < 2) || (numIndices!= 1))
        {
            return false;
        }
        if ((indices[0] != -2) || (ops[degree-1] != 1)  ) return false;
        for (long u = 0; u < degree-1; u++) {
            if (ops[u] != 0) return false;
        }
        return true;
    }
    inline bool IsCase4(long degree, long numIndices, const long * const ops, const long * const indices)
    {
        // ops: 10000000 indices: -1
        if ((degree < 2) || (numIndices!= 1)) 
        {
            return false;
        }
        if ((indices[0] != -1) || (ops[0] != 1)  ) return false;
        for (long u = 1; u < degree; u++) 
        {
            if (ops[u] != 0) return false;
        }
        return true;
    }
    inline bool IsCase5(long degree, long numIndices, const long * const ops, const long * const indices)
    {
        // ops: 10000000 indices: -2
        if ((degree < 2) || (numIndices!= 1)) 
        {
            return false;
        }
        if ((indices[0] != -2) || (ops[0] != 1)  ) return false;
        for (long u = 1; u < degree; u++) {
            if (ops[u] != 0) return false;
        }
        return true;
    }
    inline bool IsCase6(long degree, long numIndices, const long * const ops, const long * const /*indices*/)
    {
        // ops: 0000000 indices: 
        if (numIndices!= 0) 
        {
            return false;
        }
        for (long u = 0; u < degree; u++) 
        {
            if (ops[u] != 0) return false;
        }
        return true;
    }
    inline bool IsCase7(long degree, long numIndices, const long * const ops, const long * const indices)
    {
        // ops: 1000001 indices: -2 -1
        if ((numIndices!= 2) || (degree < 2)) 
        {
            return false;
        }
        if ((indices[0] != -2) ||(indices[1] != -1) || 
            (ops[0] != 1)      ||(ops[degree-1] != 1)  ) return false;
        for (long u = 1; u < degree-1; u++) 
        {
            if (ops[u] != 0) return false;
        }
        return true;
    }
    inline bool IsCase8(long degree, long numIndices, const long * const ops, const long * const indices)
    {
        // ops: 1xxxxxx1 indices: -1 x x x x x -2
        if ((degree < 2) || (numIndices < 1)) 
        {
            return false;
        }
        if ((indices[0] != -2) ||(indices[numIndices-1] != -1) || 
            (ops[0] != 1)      ||(ops[degree-1] != 1)  ) return false;
        return true;
    }
    template <class T>
    TriangleListEncoder<T>::TriangleListEncoder(void)
    {
        m_vtags                   = 0;
        m_ttags                   = 0;
        m_tmap                    = 0;
        m_vmap                    = 0;
        m_count                   = 0;
        m_invVMap                 = 0;
        m_invTMap                 = 0;
        m_nonConqueredTriangles   = 0;
        m_nonConqueredEdges       = 0;
        m_visitedVertices         = 0;
        m_visitedVerticesValence  = 0;
        m_vertexCount             = 0;
        m_triangleCount           = 0;
        m_maxNumVertices          = 0;
        m_maxNumTriangles         = 0;
        m_numTriangles            = 0;
        m_numVertices             = 0;
        m_triangles               = 0;
        m_maxSizeVertexToTriangle = 0;
        m_streamType              = O3DGC_STREAM_TYPE_UNKOWN;
    }
    template <class T>
    TriangleListEncoder<T>::~TriangleListEncoder()
    {
        delete [] m_vtags;
        delete [] m_vmap;
        delete [] m_invVMap;
        delete [] m_invTMap;
        delete [] m_visitedVerticesValence;
        delete [] m_visitedVertices;
        delete [] m_ttags;
        delete [] m_tmap;
        delete [] m_count;
        delete [] m_nonConqueredTriangles;
        delete [] m_nonConqueredEdges;
    }
    template <class T>
    O3DGCErrorCode TriangleListEncoder<T>::Init(const T * const triangles, 
                                             long numTriangles, 
                                             long numVertices)
    {
        assert(numVertices  > 0);
        assert(numTriangles > 0);

        m_numTriangles  = numTriangles;
        m_numVertices   = numVertices;
        m_triangles     = triangles;
        m_vertexCount   = 0;
        m_triangleCount = 0;
        
        if  (m_numVertices > m_maxNumVertices)
        {
            delete [] m_vtags;
            delete [] m_vmap;
            delete [] m_invVMap;
            delete [] m_visitedVerticesValence;
            delete [] m_visitedVertices;
            m_maxNumVertices         = m_numVertices;
            m_vtags                  = new long [m_numVertices];
            m_vmap                   = new long [m_numVertices];
            m_invVMap                = new long [m_numVertices];
            m_visitedVerticesValence = new long [m_numVertices];
            m_visitedVertices        = new long [m_numVertices];
        }
        
        if  (m_numTriangles > m_maxNumTriangles)
        {
            delete [] m_ttags;
            delete [] m_tmap;
            delete [] m_invTMap;
            delete [] m_nonConqueredTriangles;
            delete [] m_nonConqueredEdges;
            delete [] m_count;
            m_maxNumTriangles       = m_numTriangles;
            m_ttags                 = new long [m_numTriangles];
            m_tmap                  = new long [m_numTriangles];
            m_invTMap               = new long [m_numTriangles];
            m_count                 = new long [m_numTriangles+1];
            m_nonConqueredTriangles = new long [m_numTriangles];
            m_nonConqueredEdges     = new long [2*m_numTriangles];
        }

        memset(m_vtags  , 0x00, sizeof(long) * m_numVertices );
        memset(m_vmap   , 0xFF, sizeof(long) * m_numVertices );
        memset(m_invVMap, 0xFF, sizeof(long) * m_numVertices );
        memset(m_ttags  , 0x00, sizeof(long) * m_numTriangles);
        memset(m_tmap   , 0xFF, sizeof(long) * m_numTriangles);
        memset(m_invTMap, 0xFF, sizeof(long) * m_numTriangles);
        memset(m_count  , 0x00, sizeof(long) * (m_numTriangles+1));

        m_vfifo.Allocate(m_numVertices);
        m_ctfans.SetStreamType(m_streamType);
        m_ctfans.Allocate(m_numVertices, m_numTriangles);

        // compute vertex-to-triangle adjacency information
        m_vertexToTriangle.AllocateNumNeighborsArray(numVertices);
        m_vertexToTriangle.ClearNumNeighborsArray();
        long * numNeighbors = m_vertexToTriangle.GetNumNeighborsBuffer();
        for(long i = 0, t = 0; i < m_numTriangles; ++i, t+=3)
        {
            ++numNeighbors[ triangles[t  ] ];
            ++numNeighbors[ triangles[t+1] ];
            ++numNeighbors[ triangles[t+2] ];
        }
        m_maxSizeVertexToTriangle = 0;
        for(long i = 0; i < numVertices; ++i)
        {
            if (m_maxSizeVertexToTriangle < numNeighbors[i])
            {
                m_maxSizeVertexToTriangle = numNeighbors[i];
            }
        }
        m_vertexToTriangle.AllocateNeighborsArray();
        m_vertexToTriangle.ClearNeighborsArray();
        for(long i = 0, t = 0; i < m_numTriangles; ++i, t+=3)
        {
            m_vertexToTriangle.AddNeighbor(triangles[t  ], i);
            m_vertexToTriangle.AddNeighbor(triangles[t+1], i);
            m_vertexToTriangle.AddNeighbor(triangles[t+2], i);
        }
        return O3DGC_OK;
    }
    template <class T>
    O3DGCErrorCode TriangleListEncoder<T>::Encode(const T * const triangles, 
                                                  const unsigned long * const indexBufferIDs,
                                                  const long numTriangles,
                                                  const long numVertices, 
                                                  BinaryStream & bstream)
    {
        assert(numVertices > 0);
        assert(numTriangles > 0);
        
        Init(triangles, numTriangles, numVertices);
        unsigned char mask = 0;
        bool encodeTrianglesOrder = (indexBufferIDs != 0);

        
        if (encodeTrianglesOrder)
        {
            long numBufferIDs = 0;
            for (long t = 0; t < numTriangles; t++)
            {
                if (numBufferIDs <= (long) indexBufferIDs[t])
                {
                    ++numBufferIDs;
                    assert(numBufferIDs <= numTriangles);
                }
                ++m_count[indexBufferIDs[t]+1];
            }
            for (long i = 2; i <= numBufferIDs; i++)
            {
                m_count[i] += m_count[i-1];
            }
            mask += 2; // preserved triangles order
        }
        bstream.WriteUChar(mask, m_streamType); 
        bstream.WriteUInt32(m_maxSizeVertexToTriangle, m_streamType);

        long v0;
        for (long v = 0; v < m_numVertices; v++)
        {
            if (!m_vtags[v]) 
            {
                m_vfifo.PushBack(v);
                m_vtags[v] = 1; 
                m_vmap[v] = m_vertexCount++;
                m_invVMap[m_vmap[v]] = v;
                while (m_vfifo.GetSize() > 0 )
                {
                    v0 = m_vfifo.PopFirst();
                    ProcessVertex(v0);
                }
            }
        }
        if (encodeTrianglesOrder)
        {
            long t, prev = 0;
            long pred;
            for (long i = 0; i < numTriangles; ++i)
            {
                t = m_invTMap[i];
                m_tmap[t] = m_count[ indexBufferIDs[t] ]++;
                pred = m_tmap[t] - prev;
                m_ctfans.PushTriangleIndex(pred);
                prev = m_tmap[t] + 1;
            }
            for (long t = 0; t < numTriangles; ++t)
            {
                m_invTMap[m_tmap[t]] = t;
            }
        }
        m_ctfans.Save(bstream, encodeTrianglesOrder, m_streamType);
        return O3DGC_OK;
    }
    template <class T>
    O3DGCErrorCode TriangleListEncoder<T>::CompueLocalConnectivityInfo(const long focusVertex)
    {
        long t, v, p;
        m_numNonConqueredTriangles = 0;
        m_numConqueredTriangles    = 0;
        m_numVisitedVertices       = 0;
        for(long i = m_vertexToTriangle.Begin(focusVertex); i < m_vertexToTriangle.End(focusVertex); ++i)
        {
            t = m_vertexToTriangle.GetNeighbor(i);

            if ( m_ttags[t] == 0) // non-processed triangle
            {
                m_nonConqueredTriangles[m_numNonConqueredTriangles] = t;
                CompueOppositeEdge( focusVertex, 
                                    m_triangles + (3*t), 
                                    m_nonConqueredEdges[m_numNonConqueredTriangles*2],
                                    m_nonConqueredEdges[m_numNonConqueredTriangles*2+1]);
                ++m_numNonConqueredTriangles;
            }
            else                // triangle already processed
            {
                m_numConqueredTriangles++;
                p = 3*t;
                // extract visited vertices
                for(long k = 0; k < 3; ++k)
                {
                    v = m_triangles[p+k];
                    if (m_vmap[v] > m_vmap[focusVertex]) // vertices are insertices by increasing traversal order
                    {
                        bool foundOrInserted = false;
                        for (long j = 0; j < m_numVisitedVertices; ++j)
                        {

                            if (m_vmap[v] == m_visitedVertices[j])
                            {
                                m_visitedVerticesValence[j]++;
                                foundOrInserted = true;
                                break;
                            }
                            else if (m_vmap[v] < m_visitedVertices[j])
                            {
                                ++m_numVisitedVertices;
                                for (long h = m_numVisitedVertices-1; h > j; --h)
                                {
                                    m_visitedVertices[h]        = m_visitedVertices[h-1];
                                    m_visitedVerticesValence[h] = m_visitedVerticesValence[h-1];
                                }
                                m_visitedVertices[j]        = m_vmap[v];
                                m_visitedVerticesValence[j] = 1;
                                foundOrInserted = true;
                                break;
                            }
                        }
                        if (!foundOrInserted)
                        {
                            m_visitedVertices[m_numVisitedVertices] = m_vmap[v];
                            m_visitedVerticesValence[m_numVisitedVertices] = 1;
                            m_numVisitedVertices++;
                        }
                    }
                }
            }            
        }
        // re-order visited vertices by taking into account their valence (i.e., # of conquered triangles incident to each vertex)
        // in order to avoid config. 9
        if (m_numVisitedVertices > 2)
        {
            long y;
            for(long x = 1; x < m_numVisitedVertices; ++x)
            {

                if (m_visitedVerticesValence[x] == 1)
                {
                    y = x;
                    while( (y > 0) && (m_visitedVerticesValence[y] < m_visitedVerticesValence[y-1]) )
                    {
                        swap(m_visitedVerticesValence[y], m_visitedVerticesValence[y-1]);
                        swap(m_visitedVertices[y], m_visitedVertices[y-1]);
                        --y;
                    }
                }
            }
        }
        if (m_numNonConqueredTriangles > 0)
        {
            // compute triangle-to-triangle adjacency information
            m_triangleToTriangle.AllocateNumNeighborsArray(m_numNonConqueredTriangles);
            m_triangleToTriangle.ClearNumNeighborsArray();
            m_triangleToTriangleInv.AllocateNumNeighborsArray(m_numNonConqueredTriangles);
            m_triangleToTriangleInv.ClearNumNeighborsArray();
            long * const numNeighbors    = m_triangleToTriangle.GetNumNeighborsBuffer();
            long * const invNumNeighbors = m_triangleToTriangleInv.GetNumNeighborsBuffer();
            for(long i = 0; i < m_numNonConqueredTriangles; ++i)
            {
                for(long j = i+1; j < m_numNonConqueredTriangles; ++j)
                {
                    if (m_nonConqueredEdges[2*i+1] == m_nonConqueredEdges[2*j]) // edge i is connected to edge j
                    {
                        ++numNeighbors[i];
                        ++invNumNeighbors[j];
                    }
                    if (m_nonConqueredEdges[2*i] == m_nonConqueredEdges[2*j+1]) // edge i is connected to edge j
                    {
                        ++numNeighbors[j];
                        ++invNumNeighbors[i];
                    }
                }
            }
            m_triangleToTriangle.AllocateNeighborsArray();
            m_triangleToTriangle.ClearNeighborsArray();
            m_triangleToTriangleInv.AllocateNeighborsArray();
            m_triangleToTriangleInv.ClearNeighborsArray();
            for(long i = 0; i < m_numNonConqueredTriangles; ++i)
            {
                for(long j = 1; j < m_numNonConqueredTriangles; ++j)
                {
                    if (m_nonConqueredEdges[2*i+1] == m_nonConqueredEdges[2*j]) // edge i is connected to edge j
                    {
                        m_triangleToTriangle.AddNeighbor(i, j);
                        m_triangleToTriangleInv.AddNeighbor(j, i);
                    }
                    if (m_nonConqueredEdges[2*i] == m_nonConqueredEdges[2*j+1]) // edge i is connected to edge j
                    {
                        m_triangleToTriangle.AddNeighbor(j, i);
                        m_triangleToTriangleInv.AddNeighbor(i, j);
                    }
                }
            }
        }
        return O3DGC_OK;
    }
    template <class T>
    O3DGCErrorCode TriangleListEncoder<T>::ComputeTFANDecomposition(const long focusVertex)
    {
        long processedTriangles = 0;
        long minNumInputEdges;
        long numInputEdges;
        long indexSeedTriangle;
        long seedTriangle;
        long currentIndex;
        long currentTriangle;
        long i0, i1, index;

        m_tfans.Clear();
        while (processedTriangles != m_numNonConqueredTriangles)
        {
            // find non processed triangle with lowest number of inputs
            minNumInputEdges   = m_numTriangles;
            indexSeedTriangle = -1;
            for(long i = 0; i < m_numNonConqueredTriangles; ++i)
            {
                numInputEdges = m_triangleToTriangleInv.GetNumNeighbors(i);
                if ( !m_ttags[m_nonConqueredTriangles[i]] && 
                      numInputEdges < minNumInputEdges            )
                {
                    minNumInputEdges  = numInputEdges;
                    indexSeedTriangle = i;
                    if (minNumInputEdges == 0) // found boundary triangle
                    {
                        break;
                    }
                }
            }
            assert(indexSeedTriangle >= 0);
            seedTriangle = m_nonConqueredTriangles[indexSeedTriangle];
            m_tfans.AddTFAN();
            m_tfans.AddVertex( focusVertex );
            m_tfans.AddVertex( m_nonConqueredEdges[indexSeedTriangle*2] );
            m_tfans.AddVertex( m_nonConqueredEdges[indexSeedTriangle*2 + 1] );
            m_ttags[ seedTriangle ]         = 1; // mark triangle as processed
            m_tmap[seedTriangle]            = m_triangleCount++;
            m_invTMap[m_tmap[seedTriangle]] = seedTriangle;
            ++processedTriangles;
            currentIndex                    = indexSeedTriangle;
            currentTriangle                 = seedTriangle;
            do
            {
                // find next triangle            
                i0 = m_triangleToTriangle.Begin(currentIndex);
                i1 = m_triangleToTriangle.End(currentIndex);
                currentIndex = -1;
                for(long i = i0; i < i1; ++i)
                {
                    index           = m_triangleToTriangle.GetNeighbor(i);
                    currentTriangle = m_nonConqueredTriangles[index];
                    if ( !m_ttags[currentTriangle] )
                    {
                        currentIndex = index;
                        m_tfans.AddVertex( m_nonConqueredEdges[currentIndex*2+1] );
                        m_ttags[currentTriangle]            = 1; // mark triangle as processed
                        m_tmap [currentTriangle]            = m_triangleCount++;
                        m_invTMap[m_tmap [currentTriangle]] = currentTriangle;
                        ++processedTriangles;
                        break;
                    }
                }
            } while (currentIndex != -1);
        }

        return O3DGC_OK;
    }
    template <class T>
    O3DGCErrorCode TriangleListEncoder<T>::CompressTFAN(const long focusVertex)
    {
        m_ctfans.PushNumTFans(m_tfans.GetNumTFANs());    

        const long ntfans = m_tfans.GetNumTFANs();
        long degree;
        long k0, k1;
        long v0;
        long ops[O3DGC_MAX_TFAN_SIZE];
        long indices[O3DGC_MAX_TFAN_SIZE];

        long numOps;
        long numIndices;
        long pos;
        long found;

        if (m_tfans.GetNumTFANs() > 0) 
        {
            for(long f = 0; f != ntfans; f++) 
            {
                degree = m_tfans.GetTFANSize(f) - 1;
                m_ctfans.PushDegree(degree-2+ m_numConqueredTriangles);
                numOps     = 0;
                numIndices = 0;
                k0 = 1 + m_tfans.Begin(f);
                k1 = m_tfans.End(f);
                for(long k = k0; k < k1; k++) 
                {
                    v0 = m_tfans.GetVertex(k);
                    if (m_vtags[v0] == 0)
                    {
                        ops[numOps++] = 0;
                        m_vtags[v0] = 1;
                        m_vmap[v0] = m_vertexCount++;
                        m_invVMap[m_vmap[v0]] = v0;
                        m_vfifo.PushBack(v0);
                        m_visitedVertices[m_numVisitedVertices++] = m_vmap[v0];
                    }
                    else 
                    {
                        ops[numOps++] = 1;
                        pos = 0;
                        found = 0;
                        for(long u=0; u < m_numVisitedVertices; ++u)
                        {
                            pos++;
                            if (m_visitedVertices[u] == m_vmap[v0]) 
                            {
                                found = 1;
                                break;
                            }
                        }    
                        if (found == 1)
                        {
                            indices[numIndices++] = -pos;
                        }
                        else 
                        {
                            indices[numIndices++] = m_vmap[v0] - m_vmap[focusVertex];
                        }
                    }
                }
                //-----------------------------------------------
                if (IsCase0(degree, numIndices, ops, indices))
                { 
                    // ops: 1000001 vertices: -1 -2
                    m_ctfans.PushConfig(0);
                }
                else if (IsCase1(degree, numIndices, ops, indices))
                {
                    // ops: 1xxxxxx1 vertices: -1 x x x x x -2
                    long u = 1;
                    for(u = 1; u < degree-1; u++)
                    {
                        m_ctfans.PushOperation(ops[u]);
                    }
                    for(u =1; u < numIndices-1; u++)
                    {
                        m_ctfans.PushIndex(indices[u]);
                    }
                    m_ctfans.PushConfig(1);
                }
                else if (IsCase2(degree, numIndices, ops, indices))
                {
                    // ops: 00000001 vertices: -1
                    m_ctfans.PushConfig(2);
                }
                else if (IsCase3(degree, numIndices, ops, indices))
                {
                    // ops: 00000001 vertices: -2
                    m_ctfans.PushConfig(3);
                }            
                else if (IsCase4(degree, numIndices, ops, indices))
                {
                    // ops: 10000000 vertices: -1
                    m_ctfans.PushConfig(4);
                }
                else if (IsCase5(degree, numIndices, ops, indices))
                {
                    // ops: 10000000 vertices: -2
                    m_ctfans.PushConfig(5);
                }            
                else if (IsCase6(degree, numIndices, ops, indices))
                {
                    // ops: 00000000 vertices:
                    m_ctfans.PushConfig(6);
                }
                else if (IsCase7(degree, numIndices, ops, indices))
                {
                    // ops: 1000001 vertices: -1 -2
                    m_ctfans.PushConfig(7);
                }
                else if (IsCase8(degree, numIndices, ops, indices))
                {
                    // ops: 1xxxxxx1 vertices: -2 x x x x x -1
                    long u = 1;
                    for(u =1; u < degree-1; u++)
                    {
                        m_ctfans.PushOperation(ops[u]);
                    }
                    for(u =1; u < numIndices-1; u++)
                    {
                        m_ctfans.PushIndex(indices[u]);
                    }
                    m_ctfans.PushConfig(8);
                }
                else 
                {
                    long u = 0;
                    for(u =0; u < degree; u++)
                    {
                        m_ctfans.PushOperation(ops[u]);
                    }
                    for(u =0; u < numIndices; u++)
                    {
                        m_ctfans.PushIndex(indices[u]);
                    }
                    m_ctfans.PushConfig(9);
                }
            }
        }
        return O3DGC_OK;
    }
    template <class T>
    O3DGCErrorCode TriangleListEncoder<T>::ProcessVertex(const long focusVertex)
    {
        CompueLocalConnectivityInfo(focusVertex);
        ComputeTFANDecomposition(focusVertex);
        CompressTFAN(focusVertex);
        return O3DGC_OK;
    }
}
#endif //O3DGC_TRIANGLE_LIST_ENCODER_INL
