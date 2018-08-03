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
#include "o3dgcDynamicVectorDecoder.h"
#include "o3dgcArithmeticCodec.h"


//#define DEBUG_VERBOSE

namespace o3dgc
{
#ifdef DEBUG_VERBOSE
        FILE * g_fileDebugDVCDec = NULL;
#endif //DEBUG_VERBOSE

    O3DGCErrorCode IUpdate(long * const data, const long size)
    {   
        assert(size > 1);
        const long size1 = size - 1;
        long p = 2;
        data[0] -= data[1] >> 1;
        while(p < size1)
        {
            data[p] -= (data[p-1] + data[p+1] + 2) >> 2;
            p += 2;
        }
        if ( p == size1)
        {
            data[p] -= data[p-1]>>1;
        }
        return O3DGC_OK;
    }
    O3DGCErrorCode IPredict(long * const data, const long size)
    {   
        assert(size > 1);
        const long size1 = size - 1;
        long p = 1;
        while(p < size1)
        {
            data[p] += (data[p-1] + data[p+1] + 1) >> 1;
            p += 2;
        }
        if ( p == size1)
        {
            data[p] += data[p-1];
        }
        return O3DGC_OK;
    }
    O3DGCErrorCode Merge(long * const data, const long size)
    {
        assert(size > 1);
        const long h = (size >> 1) + (size & 1);
        long       a = h-1;
        long       b = h;
        while (a > 0)
        {
            for (long i = a; i < b; i += 2)
            {
                swap(data[i], data[i+1]);
            }
            --a;
            ++b;
        }
        return O3DGC_OK;
    }
    inline O3DGCErrorCode ITransform(long * const data, const unsigned long size)
    {   
        unsigned long n    = size;
        unsigned long even = 0;
        unsigned long k    = 0;
        even += ((n&1) << k++);
        while(n > 1)
        {
            n = (n >> 1) + (n & 1);
            even += ((n&1) << k++);
        }
        for(long i = k-2; i >= 0; --i)
        {
            n = (n << 1) - ((even>>i) & 1);
            Merge  (data, n);
            IUpdate (data, n);
            IPredict(data, n);
        }
        return O3DGC_OK;
    }
    DynamicVectorDecoder::DynamicVectorDecoder(void)
    {
        m_streamSize    = 0;
        m_maxNumVectors = 0;
        m_numVectors    = 0;
        m_dimVectors    = 0;
        m_quantVectors  = 0;
        m_iterator      = 0;
        m_streamType    = O3DGC_STREAM_TYPE_UNKOWN;
    }
    DynamicVectorDecoder::~DynamicVectorDecoder()
    {
        delete [] m_quantVectors;
    }    
    O3DGCErrorCode DynamicVectorDecoder::DecodeHeader(DynamicVector & dynamicVector,
                                                      const BinaryStream & bstream)
    {
        unsigned long iterator0 = m_iterator;
        unsigned long start_code = bstream.ReadUInt32(m_iterator, O3DGC_STREAM_TYPE_BINARY);
        if (start_code != O3DGC_DV_START_CODE)
        {
            m_iterator = iterator0;
            start_code = bstream.ReadUInt32(m_iterator, O3DGC_STREAM_TYPE_ASCII);
            if (start_code != O3DGC_DV_START_CODE)
            {
                return O3DGC_ERROR_CORRUPTED_STREAM;
            }
            else
            {
                m_streamType = O3DGC_STREAM_TYPE_ASCII;
            }
        }
        else
        {
            m_streamType = O3DGC_STREAM_TYPE_BINARY;
        }
        m_streamSize = bstream.ReadUInt32(m_iterator, m_streamType);
        m_params.SetEncodeMode( (O3DGCDVEncodingMode) bstream.ReadUChar(m_iterator, m_streamType));
        dynamicVector.SetNVector   ( bstream.ReadUInt32(m_iterator, m_streamType) );
          
        if (dynamicVector.GetNVector() > 0)
        {
            dynamicVector.SetDimVector( bstream.ReadUInt32(m_iterator, m_streamType) );
            m_params.SetQuantBits(bstream.ReadUChar(m_iterator, m_streamType));
        }
        return O3DGC_OK;
    }
    O3DGCErrorCode DynamicVectorDecoder::DecodePlayload(DynamicVector & dynamicVector,
                                                        const BinaryStream & bstream)
    {
        O3DGCErrorCode ret = O3DGC_OK;
#ifdef DEBUG_VERBOSE
        g_fileDebugDVCDec = fopen("dv_dec.txt", "w");
#endif //DEBUG_VERBOSE
        unsigned long       start            = m_iterator;
        unsigned long       streamSize       = bstream.ReadUInt32(m_iterator, m_streamType);        // bitsream size

        const unsigned long dim  = dynamicVector.GetDimVector();
        const unsigned long num  = dynamicVector.GetNVector();
        const unsigned long size = dim * num;
        for(unsigned long j=0 ; j < dynamicVector.GetDimVector() ; ++j)
        {
            dynamicVector.SetMin(j, (Real) bstream.ReadFloat32(m_iterator, m_streamType));
            dynamicVector.SetMax(j, (Real) bstream.ReadFloat32(m_iterator, m_streamType));
        }
        Arithmetic_Codec acd;
        Static_Bit_Model bModel0;
        Adaptive_Bit_Model bModel1;
        unsigned char *     buffer           = 0;
        streamSize                          -= (m_iterator - start);
        unsigned int        exp_k            = 0;
        unsigned int        M                = 0;        
        if (m_streamType == O3DGC_STREAM_TYPE_BINARY)
        {
            bstream.GetBuffer(m_iterator, buffer);
            m_iterator += streamSize;
            acd.set_buffer(streamSize, buffer);
            acd.start_decoder();
            exp_k = acd.ExpGolombDecode(0, bModel0, bModel1);
            M     = acd.ExpGolombDecode(0, bModel0, bModel1);
        }
        Adaptive_Data_Model mModelValues(M+2);

        if (m_maxNumVectors < size)
        {
            delete [] m_quantVectors;
            m_maxNumVectors = size;
            m_quantVectors  = new long [size];
        }
        if (m_streamType == O3DGC_STREAM_TYPE_ASCII)
        {
            for(unsigned long v = 0; v < num; ++v)
            {
                for(unsigned long d = 0; d < dim; ++d)
                {
                    m_quantVectors[d * num + v] = bstream.ReadIntASCII(m_iterator);
                }
            }
        }
        else
        {
            for(unsigned long v = 0; v < num; ++v)
            {
                for(unsigned long d = 0; d < dim; ++d)
                {
                    m_quantVectors[d * num + v] = DecodeIntACEGC(acd, mModelValues, bModel0, bModel1, exp_k, M);
                }
            }
        }
        #ifdef DEBUG_VERBOSE
        printf("IntArray (%i, %i)\n", num, dim);
        fprintf(g_fileDebugDVCDec, "IntArray (%i, %i)\n", num, dim);
        for(unsigned long v = 0; v < num; ++v)
        {
            for(unsigned long d = 0; d < dim; ++d)
            {
                printf("%i\t %i \t %i\n", d * num + v, m_quantVectors[d * num + v], IntToUInt(m_quantVectors[d * num + v]));
                fprintf(g_fileDebugDVCDec, "%i\t %i \t %i\n", d * num + v, m_quantVectors[d * num + v], IntToUInt(m_quantVectors[d * num + v]));
            }
        }
        fflush(g_fileDebugDVCDec);
        #endif //DEBUG_VERBOSE
        for(unsigned long d = 0; d < dim; ++d)
        {
            ITransform(m_quantVectors + d * num, num);
        }
        IQuantize(dynamicVector.GetVectors(), 
                  num, 
                  dim,
                  dynamicVector.GetStride(),
                  dynamicVector.GetMin(),
                  dynamicVector.GetMax(),
                  m_params.GetQuantBits());

#ifdef DEBUG_VERBOSE
        fclose(g_fileDebugDVCDec);
#endif //DEBUG_VERBOSE
        return ret;
    }
    O3DGCErrorCode DynamicVectorDecoder::IQuantize(Real * const floatArray, 
                                                   unsigned long numFloatArray,
                                                   unsigned long dimFloatArray,
                                                   unsigned long stride,
                                                   const Real * const minFloatArray,
                                                   const Real * const maxFloatArray,
                                                   unsigned long nQBits)
    {
        const unsigned long size = numFloatArray * dimFloatArray;
        Real r;
        if (m_maxNumVectors < size)
        {
            delete [] m_quantVectors;
            m_maxNumVectors = size;
            m_quantVectors = new long [m_maxNumVectors];
        }
        Real idelta;
        for(unsigned long d = 0; d < dimFloatArray; ++d)
        {
            r = maxFloatArray[d] - minFloatArray[d];
            if (r > 0.0f)
            {
                idelta = (float)(r) / ((1 << nQBits) - 1);
            }
            else
            {
                idelta = 1.0f;
            }
            for(unsigned long v = 0; v < numFloatArray; ++v)
            {
                floatArray[v * stride + d] = m_quantVectors[v + d * numFloatArray] * idelta + minFloatArray[d];
            }
        }
        return O3DGC_OK;
    }
}
