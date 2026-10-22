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
#ifndef O3DGC_DYNAMIC_VECTOR_SET_H
#define O3DGC_DYNAMIC_VECTOR_SET_H

#include "o3dgcCommon.h"

namespace o3dgc
{
    class DynamicVector
    {
    public:
        //! Constructor.
                                    DynamicVector(void)
                                    {
                                        m_num               = 0;
                                        m_dim               = 0;
                                        m_stride            = 0;
                                        m_max               = 0;
                                        m_min               = 0;
                                        m_vectors           = 0;
                                    };
        //! Destructor.
                                    ~DynamicVector(void) {};

        unsigned long               GetNVector()       const { return m_num;}
        unsigned long               GetDimVector()     const { return m_dim;}
        unsigned long               GetStride()        const { return m_stride;}
        const Real *                GetMin()           const { return m_min;}
        const Real *                GetMax()           const { return m_max;}
        const Real *                GetVectors()       const { return m_vectors;}
        Real *                      GetVectors()             { return m_vectors;}
        Real                        GetMin(unsigned long j) const { return m_min[j];}
        Real                        GetMax(unsigned long j) const { return m_max[j];}

        void                        SetNVector     (unsigned long num        ) { m_num       = num      ;}
        void                        SetDimVector   (unsigned long dim        ) { m_dim       = dim      ;}
        void                        SetStride      (unsigned long stride     ) { m_stride    = stride   ;}
        void                        SetMin         (Real * const min         ) { m_min       = min      ;}
        void                        SetMax         (Real * const max         ) { m_max       = max      ;}
        void                        SetMin         (unsigned long j, Real min) { m_min[j]    = min      ;}
        void                        SetMax         (unsigned long j, Real max) { m_max[j]    = max      ;}
        void                        SetVectors     (Real * const vectors)      { m_vectors   = vectors  ;}

        void                        ComputeMinMax(O3DGCSC3DMCQuantizationMode quantMode)
                                    {
                                        assert( m_max && m_min && m_vectors && m_stride && m_dim && m_num);
                                        ComputeVectorMinMax(m_vectors, m_num , m_dim, m_stride, m_min , m_max , quantMode);
                                    }

    private:
        unsigned long               m_num;
        unsigned long               m_dim;
        unsigned long               m_stride;
        Real *                      m_max;
        Real *                      m_min;
        Real *                      m_vectors;
    };

}
#endif // O3DGC_DYNAMIC_VECTOR_SET_H

