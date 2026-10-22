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
#include "o3dgcDVEncodeParams.h"
#include "o3dgcDynamicVectorEncoder.h"
#include "o3dgcArithmeticCodec.h"
#include "o3dgcBinaryStream.h"

//#define DEBUG_VERBOSE

namespace o3dgc
{
#ifdef DEBUG_VERBOSE
        FILE * g_fileDebugDVEnc = NULL;
#endif //DEBUG_VERBOSE

    inline O3DGCErrorCode Update(long * const data, const long size)
    {
        assert(size > 1);
        const long size1 = size - 1;
        long p = 2;
        data[0] += data[1] >> 1;
        while(p < size1)
        {
            data[p] += (data[p-1] + data[p+1] + 2) >> 2;
            p += 2;
        }
        if ( p == size1)
        {
            data[p] += data[p-1]>>1;
        }
        return O3DGC_OK;
    }
    inline O3DGCErrorCode Predict(long * const data, const long size)
    {   
        assert(size > 1);
        const long size1 = size - 1;
        long p = 1;
        while(p < size1)
        {
            data[p] -= (data[p-1] + data[p+1] + 1) >> 1;
            p += 2;
        }
        if ( p == size1)
        {
            data[p] -= data[p-1];
        }
        return O3DGC_OK;
    }
    inline O3DGCErrorCode Split(long * const data, const long size)
    {   
        assert(size > 1);
        long a = 1;
        long b = size-1;
        while (a < b) 
        {
            for (long i = a; i < b; i += 2) 
            {
                swap(data[i], data[i+1]);
            }
            ++a;
            --b;
        }
        return O3DGC_OK;
    }
    inline O3DGCErrorCode Transform(long * const data, const unsigned long size)
    {   
        unsigned long n = size;
        while(n > 1)
        {
            Predict(data, n);
            Update (data, n);
            Split(data, n);
            n = (n >> 1) + (n & 1);
        }
        return O3DGC_OK;
    }
    DynamicVectorEncoder::DynamicVectorEncoder(void)
    {
        m_maxNumVectors = 0;
        m_numVectors    = 0;
        m_dimVectors    = 0;
        m_quantVectors  = 0;
        m_sizeBufferAC  = 0;
        m_bufferAC      = 0;
        m_posSize       = 0;
        m_streamType    = O3DGC_STREAM_TYPE_UNKOWN;
    }
    DynamicVectorEncoder::~DynamicVectorEncoder()
    {
        delete [] m_quantVectors;
        delete [] m_bufferAC;
    }
    O3DGCErrorCode DynamicVectorEncoder::Encode(const DVEncodeParams & params,
                                                const DynamicVector & dynamicVector,
                                                BinaryStream & bstream)
    {
        assert(params.GetQuantBits() > 0);
        assert(dynamicVector.GetNVector()   > 0);
        assert(dynamicVector.GetDimVector() > 0);
        assert(dynamicVector.GetStride()    >= dynamicVector.GetDimVector());
        assert(dynamicVector.GetVectors() && dynamicVector.GetMin() && dynamicVector.GetMax());
        assert(m_streamType != O3DGC_STREAM_TYPE_UNKOWN);
        // Encode header
        unsigned long start = bstream.GetSize();
        EncodeHeader(params, dynamicVector, bstream);
        // Encode payload
        EncodePayload(params, dynamicVector, bstream);
        bstream.WriteUInt32(m_posSize, bstream.GetSize() - start, m_streamType);
        return O3DGC_OK;

    }
    O3DGCErrorCode DynamicVectorEncoder::EncodeHeader(const DVEncodeParams & params,
                                                      const DynamicVector & dynamicVector,
                                                      BinaryStream & bstream)
    {
        m_streamType = params.GetStreamType();
        bstream.WriteUInt32(O3DGC_DV_START_CODE, m_streamType);
        m_posSize = bstream.GetSize();
        bstream.WriteUInt32(0, m_streamType); // to be filled later
        bstream.WriteUChar((unsigned char) params.GetEncodeMode(), m_streamType);
        bstream.WriteUInt32(dynamicVector.GetNVector() , m_streamType);
        if (dynamicVector.GetNVector() > 0)
        {
            bstream.WriteUInt32(dynamicVector.GetDimVector(), m_streamType);
            bstream.WriteUChar ((unsigned char) params.GetQuantBits(), m_streamType);
        }
        return O3DGC_OK;
    }
    O3DGCErrorCode DynamicVectorEncoder::EncodeAC(unsigned long num, 
                                                  unsigned long dim, 
                                                  unsigned long M, 
                                                  unsigned long & encodedBytes)
    {
        Arithmetic_Codec ace;
        Static_Bit_Model bModel0;
        Adaptive_Bit_Model bModel1;
        Adaptive_Data_Model mModelValues(M+2);
        const unsigned int NMAX = num * dim * 8 + 100;
        if ( m_sizeBufferAC < NMAX )
        {
            delete [] m_bufferAC;
            m_sizeBufferAC = NMAX;
            m_bufferAC     = new unsigned char [m_sizeBufferAC];
        }
        ace.set_buffer(NMAX, m_bufferAC);
        ace.start_encoder();
        ace.ExpGolombEncode(0, 0, bModel0, bModel1);
        ace.ExpGolombEncode(M, 0, bModel0, bModel1);
        for(unsigned long v = 0; v < num; ++v)
        {
            for(unsigned long d = 0; d < dim; ++d)
            {
                EncodeIntACEGC(m_quantVectors[d * num + v], ace, mModelValues, bModel0, bModel1, M);
            }
        }
        encodedBytes = ace.stop_encoder();
        return O3DGC_OK;
    }

    O3DGCErrorCode DynamicVectorEncoder::EncodePayload(const DVEncodeParams & params,
                                                       const DynamicVector & dynamicVector,
                                                       BinaryStream & bstream)
    {
#ifdef DEBUG_VERBOSE
        g_fileDebugDVEnc = fopen("dv_enc.txt", "w");
#endif //DEBUG_VERBOSE
        unsigned long      start = bstream.GetSize();
        const unsigned long dim  = dynamicVector.GetDimVector();
        const unsigned long num  = dynamicVector.GetNVector();

        bstream.WriteUInt32(0, m_streamType);

        for(unsigned long j=0 ; j<dynamicVector.GetDimVector() ; ++j)
        {
            bstream.WriteFloat32((float) dynamicVector.GetMin(j), m_streamType);
            bstream.WriteFloat32((float) dynamicVector.GetMax(j), m_streamType);
        }
        Quantize(dynamicVector.GetVectors(), 
                 num, 
                 dim,
                 dynamicVector.GetStride(),
                 dynamicVector.GetMin(),
                 dynamicVector.GetMax(),
                 params.GetQuantBits());
        for(unsigned long d = 0; d < dim; ++d)
        {
            Transform(m_quantVectors + d * num, num);
        }
        #ifdef DEBUG_VERBOSE
        printf("IntArray (%i, %i)\n", num, dim);
        fprintf(g_fileDebugDVEnc, "IntArray (%i, %i)\n", num, dim);
        for(unsigned long v = 0; v < num; ++v)
        {
            for(unsigned long d = 0; d < dim; ++d)
            {
                printf("%i\t %i \t %i\n", d * num + v, m_quantVectors[d * num + v], IntToUInt(m_quantVectors[d * num + v]));
                fprintf(g_fileDebugDVEnc, "%i\t %i \t %i\n", d * num + v, m_quantVectors[d * num + v], IntToUInt(m_quantVectors[d * num + v]));
            }
        }
        #endif //DEBUG_VERBOSE

        if (m_streamType == O3DGC_STREAM_TYPE_ASCII)
        {
            for(unsigned long v = 0; v < num; ++v)
            {
                for(unsigned long d = 0; d < dim; ++d)
                {
                    bstream.WriteIntASCII(m_quantVectors[d * num + v]);
                }
            }
        }
        else
        {
            unsigned long encodedBytes = 0;
            unsigned long bestEncodedBytes = O3DGC_MAX_ULONG;
            unsigned long M = 1;
            unsigned long bestM = 1;
            while (M < 1024)
            {
                EncodeAC(num, dim, M, encodedBytes);
                if (encodedBytes > bestEncodedBytes)
                {
                    break;
                }
                bestM = M;
                bestEncodedBytes = encodedBytes;
                M *= 2;
            }
            EncodeAC(num, dim, bestM, encodedBytes);
            for(unsigned long i = 0; i < encodedBytes; ++i)
            {
                bstream.WriteUChar8Bin(m_bufferAC[i]);
            }
        }
        bstream.WriteUInt32(start, bstream.GetSize() - start, m_streamType);
#ifdef DEBUG_VERBOSE
        fclose(g_fileDebugDVEnc);
#endif //DEBUG_VERBOSE
        return O3DGC_OK;
    }
    O3DGCErrorCode DynamicVectorEncoder::Quantize(const Real * const floatArray, 
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
        Real delta;
        for(unsigned long d = 0; d < dimFloatArray; ++d)
        {
            r = maxFloatArray[d] - minFloatArray[d];
            if (r > 0.0f)
            {
                delta = (float)((1 << nQBits) - 1) / r;
            }
            else
            {
                delta = 1.0f;
            }
            for(unsigned long v = 0; v < numFloatArray; ++v)
            {
                m_quantVectors[v + d * numFloatArray] = (long)((floatArray[v * stride + d]-minFloatArray[d]) * delta + 0.5f);
            }
        }
        return O3DGC_OK;
    }
}
