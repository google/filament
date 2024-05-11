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
#ifndef O3DGC_ADJACENCY_INFO_H
#define O3DGC_ADJACENCY_INFO_H

#include "o3dgcCommon.h"

namespace o3dgc
{
    const long O3DGC_MIN_NEIGHBORS_SIZE     = 128;
    const long O3DGC_MIN_NUM_NEIGHBORS_SIZE = 16;
    //! 
    class AdjacencyInfo
    {
    public:
        //! Constructor.
                                AdjacencyInfo(long numNeighborsSize = O3DGC_MIN_NUM_NEIGHBORS_SIZE,
                                              long neighborsSize    = O3DGC_MIN_NUM_NEIGHBORS_SIZE)
                                {
                                    m_numElements      = 0;
                                    m_neighborsSize    = neighborsSize; 
                                    m_numNeighborsSize = numNeighborsSize;
                                    m_numNeighbors     = new long [m_numNeighborsSize];
                                    m_neighbors        = new long [m_neighborsSize   ];
                                };
        //! Destructor.
                                ~AdjacencyInfo(void)
                                {
                                    delete [] m_neighbors;
                                    delete [] m_numNeighbors;
                                };
        O3DGCErrorCode          Allocate(long numNeighborsSize, long neighborsSize)
                                {
                                    m_numElements = numNeighborsSize;
                                    if (neighborsSize > m_neighborsSize)
                                    {
                                        delete [] m_numNeighbors;
                                        m_neighborsSize    = neighborsSize;
                                        m_numNeighbors     = new long [m_numNeighborsSize];
                                    }
                                    if (numNeighborsSize > m_numNeighborsSize)
                                    {
                                        delete [] m_neighbors;
                                        m_numNeighborsSize = numNeighborsSize;
                                        m_neighbors        = new long [m_neighborsSize];
                                    }
                                    return O3DGC_OK;
                                }
        O3DGCErrorCode          AllocateNumNeighborsArray(long numElements)
                                {
                                    if (numElements > m_numNeighborsSize)
                                    {
                                        delete [] m_numNeighbors;
                                        m_numNeighborsSize = numElements;
                                        m_numNeighbors = new long [m_numNeighborsSize];
                                    }
                                    m_numElements = numElements;
                                    return O3DGC_OK;
                                }
        O3DGCErrorCode          AllocateNeighborsArray()
                                {
                                    for(long i = 1; i < m_numElements; ++i)
                                    {
                                        m_numNeighbors[i] += m_numNeighbors[i-1];
                                    }
                                    if (m_numNeighbors[m_numElements-1] > m_neighborsSize)
                                    {
                                        delete [] m_neighbors;
                                        m_neighborsSize = m_numNeighbors[m_numElements-1];
                                        m_neighbors = new long [m_neighborsSize];
                                    }
                                    return O3DGC_OK;
                                }
        O3DGCErrorCode          ClearNumNeighborsArray()
                                {
                                    memset(m_numNeighbors, 0x00, sizeof(long) * m_numElements);
                                    return O3DGC_OK;
                                }
        O3DGCErrorCode          ClearNeighborsArray()
                                {
                                    memset(m_neighbors, 0xFF, sizeof(long) * m_neighborsSize);
                                    return O3DGC_OK;
                                }
        O3DGCErrorCode          AddNeighbor(long element, long neighbor)
                                {
                                    assert(m_numNeighbors[element] <= m_numNeighbors[m_numElements-1]);
                                    long p0 = Begin(element);
                                    long p1 = End(element);
                                    for(long p = p0; p < p1; p++)
                                    {
                                        if (m_neighbors[p] == -1)
                                        {
                                            m_neighbors[p] = neighbor;
                                            return O3DGC_OK;
                                        }
                                    }
                                    return O3DGC_ERROR_BUFFER_FULL;
                                }
        long                    Begin(long element) const 
                                {
                                    assert(element < m_numElements);
                                    assert(element >= 0);
                                    return (element>0)?m_numNeighbors[element-1]:0;
                                }
        long                    End(long element) const
                                {
                                    assert(element < m_numElements);
                                    assert(element >= 0);
                                    return m_numNeighbors[element];
                                }
        long                    GetNeighbor(long element) const
                                {
                                    assert(element < m_neighborsSize);
                                    assert(element >= 0);
                                    return m_neighbors[element];
                                }    
        long                    GetNumNeighbors(long element)  const 
                                { 
                                    return End(element) - Begin(element);
                                }
        long *                  GetNumNeighborsBuffer() { return m_numNeighbors;}
        long *                  GetNeighborsBuffer()    { return m_neighbors;}

    private:
        long                    m_neighborsSize;    // actual allocated size for m_neighbors
        long                    m_numNeighborsSize; // actual allocated size for m_numNeighbors
        long                    m_numElements;      // number of elements 
        long *                  m_neighbors;        // 
        long *                  m_numNeighbors;     //         
    };
}
#endif // O3DGC_ADJACENCY_INFO_H

