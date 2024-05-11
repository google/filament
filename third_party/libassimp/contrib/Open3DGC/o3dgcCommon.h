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
#ifndef O3DGC_COMMON_H
#define O3DGC_COMMON_H

#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <math.h>

namespace o3dgc
{
    typedef float        Real;
    const double O3DGC_MAX_DOUBLE       = 1.79769e+308;
    const long O3DGC_MIN_LONG           = -2147483647;
    const long O3DGC_MAX_LONG           =  2147483647;
    const long O3DGC_MAX_UCHAR8         = 255;
    const long O3DGC_MAX_TFAN_SIZE      = 256;
    const unsigned long O3DGC_MAX_ULONG = 4294967295;

    const unsigned long O3DGC_SC3DMC_START_CODE               = 0x00001F1;
    const unsigned long O3DGC_DV_START_CODE                   = 0x00001F2;
    const unsigned long O3DGC_SC3DMC_MAX_NUM_FLOAT_ATTRIBUTES = 256;
    const unsigned long O3DGC_SC3DMC_MAX_NUM_INT_ATTRIBUTES   = 256;
    const unsigned long O3DGC_SC3DMC_MAX_DIM_ATTRIBUTES = 32;

    const unsigned long O3DGC_SC3DMC_MAX_PREDICTION_NEIGHBORS = 2;
    const unsigned long O3DGC_SC3DMC_MAX_PREDICTION_SYMBOLS   = 257;

    enum O3DGCEndianness
    {
        O3DGC_BIG_ENDIAN     = 0,
        O3DGC_LITTLE_ENDIAN  = 1
    };
    enum O3DGCErrorCode
    {
        O3DGC_OK,
        O3DGC_ERROR_BUFFER_FULL,
        O3DGC_ERROR_CREATE_FILE,
        O3DGC_ERROR_OPEN_FILE,
        O3DGC_ERROR_READ_FILE,
        O3DGC_ERROR_CORRUPTED_STREAM,
        O3DGC_ERROR_NON_SUPPORTED_FEATURE
    };
    enum O3DGCSC3DMCBinarization
    {
        O3DGC_SC3DMC_BINARIZATION_FL     = 0,            // Fixed Length (not supported)
        O3DGC_SC3DMC_BINARIZATION_BP     = 1,            // BPC (not supported)
        O3DGC_SC3DMC_BINARIZATION_FC     = 2,            // 4 bits Coding (not supported)
        O3DGC_SC3DMC_BINARIZATION_AC     = 3,            // Arithmetic Coding (not supported)
        O3DGC_SC3DMC_BINARIZATION_AC_EGC = 4,            // Arithmetic Coding & EGCk
        O3DGC_SC3DMC_BINARIZATION_ASCII  = 5             // Arithmetic Coding & EGCk
    };
    enum O3DGCStreamType
    {
        O3DGC_STREAM_TYPE_UNKOWN = 0,
        O3DGC_STREAM_TYPE_ASCII  = 1,
        O3DGC_STREAM_TYPE_BINARY = 2
    };
    enum O3DGCSC3DMCQuantizationMode
    {
        O3DGC_SC3DMC_DIAG_BB             = 0, // supported
        O3DGC_SC3DMC_MAX_ALL_DIMS        = 1, // supported
        O3DGC_SC3DMC_MAX_SEP_DIM         = 2  // supported
    };
    enum O3DGCSC3DMCPredictionMode
    {
        O3DGC_SC3DMC_NO_PREDICTION                    = 0, // supported
        O3DGC_SC3DMC_DIFFERENTIAL_PREDICTION          = 1, // supported
        O3DGC_SC3DMC_XOR_PREDICTION                   = 2, // not supported
        O3DGC_SC3DMC_ADAPTIVE_DIFFERENTIAL_PREDICTION = 3, // not supported
        O3DGC_SC3DMC_CIRCULAR_DIFFERENTIAL_PREDICTION = 4, // not supported
        O3DGC_SC3DMC_PARALLELOGRAM_PREDICTION         = 5,  // supported
        O3DGC_SC3DMC_SURF_NORMALS_PREDICTION          = 6   // supported
    };
    enum O3DGCSC3DMCEncodingMode
    {
        O3DGC_SC3DMC_ENCODE_MODE_QBCR       = 0,        // not supported
        O3DGC_SC3DMC_ENCODE_MODE_SVA        = 1,        // not supported
        O3DGC_SC3DMC_ENCODE_MODE_TFAN       = 2,        // supported
    };
    enum O3DGCDVEncodingMode
    {
        O3DGC_DYNAMIC_VECTOR_ENCODE_MODE_LIFT       = 0
    };
    enum O3DGCIFSFloatAttributeType
    {
        O3DGC_IFS_FLOAT_ATTRIBUTE_TYPE_UNKOWN   = 0,
        O3DGC_IFS_FLOAT_ATTRIBUTE_TYPE_POSITION = 1,
        O3DGC_IFS_FLOAT_ATTRIBUTE_TYPE_NORMAL   = 2,
        O3DGC_IFS_FLOAT_ATTRIBUTE_TYPE_COLOR    = 3,
        O3DGC_IFS_FLOAT_ATTRIBUTE_TYPE_TEXCOORD = 4,
        O3DGC_IFS_FLOAT_ATTRIBUTE_TYPE_WEIGHT   = 5
        
    };
    enum O3DGCIFSIntAttributeType
    {
        O3DGC_IFS_INT_ATTRIBUTE_TYPE_UNKOWN   = 0,
        O3DGC_IFS_INT_ATTRIBUTE_TYPE_INDEX    = 1,
        O3DGC_IFS_INT_ATTRIBUTE_TYPE_JOINT_ID = 2,
        O3DGC_IFS_INT_ATTRIBUTE_TYPE_INDEX_BUFFER_ID = 3
    };

    template<class T> 
    inline const T absolute(const T& a)
    {
        return (a < (T)(0)) ? -a : a;
    }
    template<class T> 
    inline const T min(const T& a, const T& b)
    {
        return (b < a) ? b : a;
    }
    template<class T> 
    inline const T max(const T& a, const T& b)
    {
        return (b > a) ? b : a;
    }
    template<class T> 
    inline void swap(T& a, T& b)
    {
        T tmp = a;
        a = b;
        b = tmp;
    }
    inline double log2( double n )  
    {  
        return log(n) / log(2.0);  
    }

    inline O3DGCEndianness SystemEndianness()
    {
        unsigned long num = 1;
        return ( *((char *)(&num)) == 1 )? O3DGC_LITTLE_ENDIAN : O3DGC_BIG_ENDIAN ;
    }
    class SC3DMCStats
    {
    public: 
                                    SC3DMCStats(void)
                                    {
                                        memset(this, 0, sizeof(SC3DMCStats));
                                    };
                                    ~SC3DMCStats(void){};
        
        double                      m_timeCoord;
        double                      m_timeNormal;
        double                      m_timeCoordIndex;
        double                      m_timeFloatAttribute[O3DGC_SC3DMC_MAX_NUM_FLOAT_ATTRIBUTES];
        double                      m_timeIntAttribute  [O3DGC_SC3DMC_MAX_NUM_INT_ATTRIBUTES  ];
        double                      m_timeReorder;

        unsigned long               m_streamSizeCoord;
        unsigned long               m_streamSizeNormal;
        unsigned long               m_streamSizeCoordIndex;
        unsigned long               m_streamSizeFloatAttribute[O3DGC_SC3DMC_MAX_NUM_FLOAT_ATTRIBUTES];
        unsigned long               m_streamSizeIntAttribute  [O3DGC_SC3DMC_MAX_NUM_INT_ATTRIBUTES  ];

    };
    typedef struct 
    {
        long          m_a;
        long          m_b;
        long          m_c;
    } SC3DMCTriplet;

    typedef struct 
    {
        SC3DMCTriplet m_id;
        long          m_pred[O3DGC_SC3DMC_MAX_DIM_ATTRIBUTES];
    } SC3DMCPredictor;

    inline bool operator< (const SC3DMCTriplet& lhs, const SC3DMCTriplet& rhs)
    {
          if (lhs.m_c != rhs.m_c)
          {
              return (lhs.m_c < rhs.m_c);
          }
          else if (lhs.m_b != rhs.m_b)
          {
              return (lhs.m_b < rhs.m_b);
          }
          return (lhs.m_a < rhs.m_a);
    }
    inline bool operator== (const SC3DMCTriplet& lhs, const SC3DMCTriplet& rhs)
    {
          return (lhs.m_c == rhs.m_c && lhs.m_b == rhs.m_b && lhs.m_a == rhs.m_a);
    }


    // fix me: optimize this function (e.g., binary search)
    inline unsigned long Insert(SC3DMCTriplet e, unsigned long & nPred, SC3DMCPredictor * const list)
    {
        unsigned long pos = 0xFFFFFFFF;
        bool foundOrInserted = false;
        for (unsigned long j = 0; j < nPred; ++j)
        {
            if (e == list[j].m_id)
            {
                foundOrInserted = true;
                break;
            }
            else if (e < list[j].m_id)
            {
                if (nPred < O3DGC_SC3DMC_MAX_PREDICTION_NEIGHBORS)
                {
                    ++nPred;
                }
                for (unsigned long h = nPred-1; h > j; --h)
                {
                    list[h] = list[h-1];
                }
                list[j].m_id = e;
                pos = j;
                foundOrInserted = true;
                break;
            }
        }
        if (!foundOrInserted && nPred < O3DGC_SC3DMC_MAX_PREDICTION_NEIGHBORS)
        {
            pos = nPred;
            list[nPred++].m_id = e;
        }
        return pos;
    }
    template <class T> 
    inline void SphereToCube(const T x, const T y, const T z, 
                             T & a, T & b, char & index)
    {
        T ax = absolute(x);
        T ay = absolute(y);
        T az = absolute(z);
        if (az >= ax && az >= ay)
        {
            if (z >= (T)(0))
            {
                index = 0;
                a = x;
                b = y;
            }
            else
            {
                index = 1;
                a = -x;
                b = -y;
            }
        }
        else if (ay >= ax && ay >= az)
        {
            if (y >= (T)(0))
            {
                index = 2;
                a = z;
                b = x;
            }
            else
            {
                index = 3;
                a = -z;
                b = -x;
            }
        }
        else if (ax >= ay && ax >= az)
        {
            if (x >= (T)(0))
            {
                index = 4;
                a = y;
                b = z;
            }
            else
            {
                index = 5;
                a = -y;
                b = -z;
            }
        }
    }
    inline void CubeToSphere(const Real a, const Real b, const char index,
                             Real & x, Real & y, Real & z)
    {
        switch( index )
        {
        case 0:
            x = a;
            y = b;
            z =  (Real) sqrt(max(0.0, 1.0 - x*x-y*y));
            break;
        case 1:
            x = -a;
            y = -b;
            z = -(Real) sqrt(max(0.0, 1.0 - x*x-y*y));
            break;
        case 2:
            z = a;
            x = b;
            y =  (Real) sqrt(max(0.0, 1.0 - x*x-z*z));
            break;
        case 3:
            z = -a;
            x = -b;
            y = -(Real) sqrt(max(0.0, 1.0 - x*x-z*z));
            break;
        case 4:
            y = a;
            z = b;
            x =  (Real) sqrt(max(0.0, 1.0 - y*y-z*z));
            break;
        case 5:
            y = -a;
            z = -b;
            x = -(Real) sqrt(max(0.0, 1.0 - y*y-z*z));
            break;
        }
    }
    inline unsigned long IntToUInt(long value)
    {
        return (value < 0)?(unsigned long) (-1 - (2 * value)):(unsigned long) (2 * value);
    }
    inline long UIntToInt(unsigned long uiValue)
    {
        return (uiValue & 1)?-((long) ((uiValue+1) >> 1)):((long) (uiValue >> 1));
    }
    inline void ComputeVectorMinMax(const Real * const tab, 
                                    unsigned long size, 
                                    unsigned long dim,
                                    unsigned long stride,
                                    Real * minTab,
                                    Real * maxTab,
                                    O3DGCSC3DMCQuantizationMode quantMode)
    {
        if (size == 0 || dim == 0)
        {
            return;
        }
        unsigned long p = 0;
        for(unsigned long d = 0; d < dim; ++d)
        {
            maxTab[d] = minTab[d] = tab[p++];
        }
        p = stride;
        for(unsigned long i = 1; i < size; ++i)
        {
            for(unsigned long d = 0; d < dim; ++d)
            {
                if (maxTab[d] < tab[p+d]) maxTab[d] = tab[p+d];
                if (minTab[d] > tab[p+d]) minTab[d] = tab[p+d];
            }
            p += stride;
        }

        if (quantMode == O3DGC_SC3DMC_DIAG_BB)
        {
            Real diag = Real( 0.0 );
            Real r;
            for(unsigned long d = 0; d < dim; ++d)
            {
                r     = (maxTab[d] - minTab[d]);
                diag += r*r;
            } 
            diag = static_cast<Real>(sqrt(diag));
            for(unsigned long d = 0; d < dim; ++d)
            {
                 maxTab[d] = minTab[d] + diag;
            } 
        }
        else if (quantMode == O3DGC_SC3DMC_MAX_ALL_DIMS)
        {            
            Real maxr = (maxTab[0] - minTab[0]);
            Real r;
            for(unsigned long d = 1; d < dim; ++d)
            {
                r = (maxTab[d] - minTab[d]);
                if ( r > maxr)
                {
                    maxr = r;
                }
            } 
            for(unsigned long d = 0; d < dim; ++d)
            {
                 maxTab[d] = minTab[d] + maxr;
            } 
        }
    }
}
#endif // O3DGC_COMMON_H

