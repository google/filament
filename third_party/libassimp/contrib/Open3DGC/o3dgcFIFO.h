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
#ifndef O3DGC_FIFO_H
#define O3DGC_FIFO_H

#include "o3dgcCommon.h"

namespace o3dgc
{
    //! 
    template < typename T > class FIFO
    {
    public:
        //! Constructor.
                                FIFO()
                                {
                                    m_buffer    = 0;
                                    m_allocated = 0;
                                    m_size      = 0;
                                    m_start     = 0;
                                    m_end       = 0;
                                };
        //! Destructor.
                                ~FIFO(void)
                                {
                                    delete [] m_buffer;
                                };
        O3DGCErrorCode          Allocate(unsigned long size)
                                {
                                    assert(size > 0);
                                    if (size > m_allocated)
                                    {
                                        delete [] m_buffer;
                                        m_allocated = size;
                                        m_buffer = new T [m_allocated];
                                    }
                                    Clear();
                                    return O3DGC_OK;
                                }
        const T &               PopFirst()
                                {
                                    assert(m_size > 0);
                                    --m_size;
                                    unsigned long current = m_start++;
                                    if (m_start == m_allocated) 
                                    {
                                        m_end = 0;
                                    }
                                    return m_buffer[current];
                                };
        void                    PushBack(const T & value)
                                {
                                    assert( m_size < m_allocated);
                                    m_buffer[m_end] = value;
                                    ++m_size;
                                    ++m_end;
                                    if (m_end == m_allocated) 
                                    {
                                        m_end = 0;
                                    }
                                }
        unsigned long           GetSize()          const { return m_size;};
        unsigned long           GetAllocatedSize() const { return m_allocated;};
        void                    Clear() { m_start = m_end = m_size = 0;};

    private:
        T *                     m_buffer;
        unsigned long           m_allocated;
        unsigned long           m_size;
        unsigned long           m_start;
        unsigned long           m_end;
    };
}
#endif // O3DGC_FIFO_H

