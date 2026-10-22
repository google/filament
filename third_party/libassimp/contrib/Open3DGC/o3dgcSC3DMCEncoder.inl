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
#ifndef O3DGC_SC3DMC_ENCODER_INL
#define O3DGC_SC3DMC_ENCODER_INL


#include "o3dgcArithmeticCodec.h"
#include "o3dgcTimer.h"
#include "o3dgcVector.h"
#include "o3dgcBinaryStream.h"
#include "o3dgcCommon.h"

//#define DEBUG_VERBOSE

namespace o3dgc
{
#ifdef DEBUG_VERBOSE
        FILE * g_fileDebugSC3DMCEnc = NULL;
#endif //DEBUG_VERBOSE

    template <class T>
    O3DGCErrorCode SC3DMCEncoder<T>::Encode(const SC3DMCEncodeParams & params, 
                                            const IndexedFaceSet<T> & ifs, 
                                            BinaryStream & bstream)
    {
        // Encode header
        unsigned long start = bstream.GetSize();
        EncodeHeader(params, ifs, bstream);
        // Encode payload
        EncodePayload(params, ifs, bstream);
        bstream.WriteUInt32(m_posSize, bstream.GetSize() - start, m_streamType);
        return O3DGC_OK;
    }
    template <class T>
    O3DGCErrorCode SC3DMCEncoder<T>::EncodeHeader(const SC3DMCEncodeParams & params, 
                                               const IndexedFaceSet<T> & ifs, 
                                               BinaryStream & bstream)
    {
        m_streamType = params.GetStreamType();
        bstream.WriteUInt32(O3DGC_SC3DMC_START_CODE, m_streamType);
        m_posSize = bstream.GetSize();
        bstream.WriteUInt32(0, m_streamType); // to be filled later

        bstream.WriteUChar(O3DGC_SC3DMC_ENCODE_MODE_TFAN, m_streamType);
        bstream.WriteFloat32((float)ifs.GetCreaseAngle(), m_streamType);
          
        unsigned char mask = 0;
        bool markerBit0 = false;
        bool markerBit1 = false;
        bool markerBit2 = false;
        bool markerBit3 = false;

        mask += (ifs.GetCCW()                  );
        mask += (ifs.GetSolid()            << 1);
        mask += (ifs.GetConvex()           << 2);
        mask += (ifs.GetIsTriangularMesh() << 3);
        mask += (markerBit0                << 4);
        mask += (markerBit1                << 5);
        mask += (markerBit2                << 6);
        mask += (markerBit3                << 7);

        bstream.WriteUChar(mask, m_streamType);

        bstream.WriteUInt32(ifs.GetNCoord(), m_streamType);
        bstream.WriteUInt32(ifs.GetNNormal(), m_streamType);
        bstream.WriteUInt32(ifs.GetNumFloatAttributes(), m_streamType);
        bstream.WriteUInt32(ifs.GetNumIntAttributes(), m_streamType);

        if (ifs.GetNCoord() > 0)
        {
            bstream.WriteUInt32(ifs.GetNCoordIndex(), m_streamType);
            for(int j=0 ; j<3 ; ++j)
            {
                bstream.WriteFloat32((float) ifs.GetCoordMin(j), m_streamType);
                bstream.WriteFloat32((float) ifs.GetCoordMax(j), m_streamType);
            }            
            bstream.WriteUChar((unsigned char) params.GetCoordQuantBits(), m_streamType);
        }
        if (ifs.GetNNormal() > 0)
        {
            bstream.WriteUInt32(0, m_streamType);
             for(int j=0 ; j<3 ; ++j)
            {
                bstream.WriteFloat32((float) ifs.GetNormalMin(j), m_streamType);
                bstream.WriteFloat32((float) ifs.GetNormalMax(j), m_streamType);
            }
            bstream.WriteUChar(true, m_streamType); //(unsigned char) ifs.GetNormalPerVertex()
            bstream.WriteUChar((unsigned char) params.GetNormalQuantBits(), m_streamType);
        }
        for(unsigned long a = 0; a < ifs.GetNumFloatAttributes(); ++a)
        {
            bstream.WriteUInt32(ifs.GetNFloatAttribute(a), m_streamType);
            if (ifs.GetNFloatAttribute(a) > 0)
            {
                assert(ifs.GetFloatAttributeDim(a) < (unsigned long) O3DGC_MAX_UCHAR8);
                bstream.WriteUInt32(0, m_streamType);
                unsigned char d = (unsigned char) ifs.GetFloatAttributeDim(a);
                bstream.WriteUChar(d, m_streamType);
                for(unsigned char j = 0 ; j < d ; ++j)
                {
                    bstream.WriteFloat32((float) ifs.GetFloatAttributeMin(a, j), m_streamType);
                    bstream.WriteFloat32((float) ifs.GetFloatAttributeMax(a, j), m_streamType);
                }
                bstream.WriteUChar(true, m_streamType); //(unsigned char) ifs.GetFloatAttributePerVertex(a)
                bstream.WriteUChar((unsigned char) ifs.GetFloatAttributeType(a), m_streamType);
                bstream.WriteUChar((unsigned char) params.GetFloatAttributeQuantBits(a), m_streamType);
            }
        }
        for(unsigned long a = 0; a < ifs.GetNumIntAttributes(); ++a)
        {
            bstream.WriteUInt32(ifs.GetNIntAttribute(a), m_streamType);
            if (ifs.GetNIntAttribute(a) > 0)
            {
                assert(ifs.GetFloatAttributeDim(a) < (unsigned long) O3DGC_MAX_UCHAR8);
                bstream.WriteUInt32(0, m_streamType);
                bstream.WriteUChar((unsigned char) ifs.GetIntAttributeDim(a), m_streamType);
                bstream.WriteUChar(true, m_streamType); // (unsigned char) ifs.GetIntAttributePerVertex(a)
                bstream.WriteUChar((unsigned char) ifs.GetIntAttributeType(a), m_streamType);
            }
        }    
        return O3DGC_OK;
    }
    template <class T>
    O3DGCErrorCode SC3DMCEncoder<T>::QuantizeFloatArray(const Real * const floatArray, 
                                                   unsigned long numFloatArray,
                                                   unsigned long dimFloatArray,
                                                   unsigned long stride,
                                                   const Real * const minFloatArray,
                                                   const Real * const maxFloatArray,
                                                   unsigned long nQBits)
    {
        const unsigned long size = numFloatArray * dimFloatArray;
        Real delta[O3DGC_SC3DMC_MAX_DIM_ATTRIBUTES];
        Real r;
        for(unsigned long d = 0; d < dimFloatArray; d++)
        {
            r = maxFloatArray[d] - minFloatArray[d];
            if (r > 0.0f)
            {
                delta[d] = (float)((1 << nQBits) - 1) / r;
            }
            else
            {
                delta[d] = 1.0f;
            }
        }        
        if (m_quantFloatArraySize < size)
        {
            delete [] m_quantFloatArray;
            m_quantFloatArraySize = size;
            m_quantFloatArray     = new long [size];
        }                                  
        for(unsigned long v = 0; v < numFloatArray; ++v)
        {
            for(unsigned long d = 0; d < dimFloatArray; ++d)
            {
                m_quantFloatArray[v * stride + d] = (long)((floatArray[v * stride + d]-minFloatArray[d]) * delta[d] + 0.5f);
            }
        }
        return O3DGC_OK;
    }
    template <class T>
    O3DGCErrorCode SC3DMCEncoder<T>::EncodeFloatArray(const Real * const floatArray, 
                                                      unsigned long numFloatArray,
                                                      unsigned long dimFloatArray,
                                                      unsigned long stride,
                                                      const Real * const minFloatArray,
                                                      const Real * const maxFloatArray,
                                                      unsigned long nQBits,
                                                      const IndexedFaceSet<T> & ifs,
                                                      O3DGCSC3DMCPredictionMode predMode,
                                                      BinaryStream & bstream)
    {
        assert(dimFloatArray <  O3DGC_SC3DMC_MAX_DIM_ATTRIBUTES);
        long predResidual, v, uPredResidual;
        unsigned long nPred;
        Arithmetic_Codec ace;
        Static_Bit_Model bModel0;
        Adaptive_Bit_Model bModel1;

        const AdjacencyInfo & v2T         = m_triangleListEncoder.GetVertexToTriangle();
        const long * const    vmap        = m_triangleListEncoder.GetVMap();
        const long * const    invVMap     = m_triangleListEncoder.GetInvVMap();
        const T * const       triangles   = ifs.GetCoordIndex();
        const long            nvert       = (long) numFloatArray;
        unsigned long         start       = bstream.GetSize();
        unsigned char         mask        = predMode & 7;
        const unsigned long   M           = O3DGC_SC3DMC_MAX_PREDICTION_SYMBOLS - 1;
        unsigned long         nSymbols    = O3DGC_SC3DMC_MAX_PREDICTION_SYMBOLS;
        unsigned long         nPredictors = O3DGC_SC3DMC_MAX_PREDICTION_NEIGHBORS;
        

        Adaptive_Data_Model mModelValues(M+2);
        Adaptive_Data_Model mModelPreds(O3DGC_SC3DMC_MAX_PREDICTION_NEIGHBORS+1);

        memset(m_freqSymbols, 0, sizeof(unsigned long) * O3DGC_SC3DMC_MAX_PREDICTION_SYMBOLS);
        memset(m_freqPreds  , 0, sizeof(unsigned long) * O3DGC_SC3DMC_MAX_PREDICTION_NEIGHBORS);
        if (m_streamType == O3DGC_STREAM_TYPE_ASCII)
        {
            mask += (O3DGC_SC3DMC_BINARIZATION_ASCII & 7)<<4;
            m_predictors.Allocate(nvert);
            m_predictors.Clear();
        }
        else
        {
            mask += (O3DGC_SC3DMC_BINARIZATION_AC_EGC & 7)<<4;
            const unsigned int NMAX = numFloatArray * dimFloatArray * 8 + 100;
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
        }
        bstream.WriteUInt32(0, m_streamType);
        bstream.WriteUChar(mask, m_streamType);

#ifdef DEBUG_VERBOSE
        printf("FloatArray (%i, %i)\n", numFloatArray, dimFloatArray);
        fprintf(g_fileDebugSC3DMCEnc, "FloatArray (%i, %i)\n", numFloatArray, dimFloatArray);
#endif //DEBUG_VERBOSE

        if (predMode == O3DGC_SC3DMC_SURF_NORMALS_PREDICTION)
        {
            const Real minFloatArray[2] = {(Real)(-2.0),(Real)(-2.0)};
            const Real maxFloatArray[2] = {(Real)(2.0),(Real)(2.0)};
            if (m_streamType == O3DGC_STREAM_TYPE_ASCII)
            {
                for(unsigned long i = 0; i < numFloatArray; ++i)
                {
                    bstream.WriteIntASCII(m_predictors[i]);
                }
            }
            else
            {
                Adaptive_Data_Model dModel(12);
                for(unsigned long i = 0; i < numFloatArray; ++i)
                {
                    ace.encode(IntToUInt(m_predictors[i]), dModel);
                }
            }
            QuantizeFloatArray(floatArray, numFloatArray, dimFloatArray, stride, minFloatArray, maxFloatArray, nQBits+1);
        }
        else
        {
            QuantizeFloatArray(floatArray, numFloatArray, dimFloatArray, stride, minFloatArray, maxFloatArray, nQBits);
        }

        for (long vm=0; vm < nvert; ++vm) 
        {
            nPred = 0;
            v     = invVMap[vm];
            assert( v >= 0 && v < nvert);
            if ( v2T.GetNumNeighbors(v) > 0 && 
                 predMode != O3DGC_SC3DMC_NO_PREDICTION)
            {
                int u0 = v2T.Begin(v);
                int u1 = v2T.End(v);
                for (long u = u0; u < u1; u++) 
                {
                    long ta = v2T.GetNeighbor(u);
                    if ( predMode == O3DGC_SC3DMC_PARALLELOGRAM_PREDICTION )
                    {
                        long a,b;
                        if ((long) triangles[ta*3] == v)
                        {
                            a = triangles[ta*3 + 1];
                            b = triangles[ta*3 + 2];
                        }
                        else if ((long) triangles[ta*3 + 1] == v)
                        {
                            a = triangles[ta*3 + 0];
                            b = triangles[ta*3 + 2];
                        }
                        else
                        {
                            a = triangles[ta*3 + 0];
                            b = triangles[ta*3 + 1];
                        }
                        if ( vmap[a] < vm && vmap[b] < vm)
                        {
                            int u0 = v2T.Begin(a);
                            int u1 = v2T.End(a);
                            for (long u = u0; u < u1; u++) 
                            {
                                long tb = v2T.GetNeighbor(u);
                                long c = -1;
                                bool foundB = false;
                                for(long k = 0; k < 3; ++k)
                                {
                                    long x = triangles[tb*3 + k];
                                    if (x == b)
                                    {
                                        foundB = true;
                                    }
                                    if (vmap[x] < vm && x != a && x != b)
                                    {
                                        c = x;
                                    }
                                }
                                if (c != -1 && foundB)
                                {
                                    SC3DMCTriplet id = {min(vmap[a], vmap[b]), max(vmap[a], vmap[b]), -vmap[c]-1};
                                    unsigned long p = Insert(id, nPred, m_neighbors);
                                    if (p != 0xFFFFFFFF)
                                    {
                                        for (unsigned long i = 0; i < dimFloatArray; i++) 
                                        {
                                            m_neighbors[p].m_pred[i] = m_quantFloatArray[a*stride+i] + 
                                                                       m_quantFloatArray[b*stride+i] - 
                                                                       m_quantFloatArray[c*stride+i];
                                        } 
                                    }
                                }
                            }
                        }
                    }
                    if ( predMode == O3DGC_SC3DMC_SURF_NORMALS_PREDICTION  ||
                         predMode == O3DGC_SC3DMC_PARALLELOGRAM_PREDICTION ||
                         predMode == O3DGC_SC3DMC_DIFFERENTIAL_PREDICTION )
                    {
                        for(long k = 0; k < 3; ++k)
                        {
                            long w = triangles[ta*3 + k];
                            if ( vmap[w] < vm )
                            {
                                SC3DMCTriplet id = {-1, -1, vmap[w]};
                                unsigned long p = Insert(id, nPred, m_neighbors);
                                if (p != 0xFFFFFFFF)
                                {
                                    for (unsigned long i = 0; i < dimFloatArray; i++) 
                                    {
                                        m_neighbors[p].m_pred[i] = m_quantFloatArray[w*stride+i];
                                    } 
                                }
                            }
                        }
                    }        
                }
            }
            if (nPred > 1)
            {
                // find best predictor
                unsigned long bestPred = 0xFFFFFFFF;
                double bestCost = O3DGC_MAX_DOUBLE;
                double cost;
#ifdef DEBUG_VERBOSE1
                    printf("\t\t vm %i\n", vm);
                    fprintf(g_fileDebugSC3DMCEnc, "\t\t vm %i\n", vm);
#endif //DEBUG_VERBOSE

                for (unsigned long p = 0; p < nPred; ++p)
                {
#ifdef DEBUG_VERBOSE1
                    printf("\t\t pred a = %i b = %i c = %i \n", m_neighbors[p].m_id.m_a, m_neighbors[p].m_id.m_b, m_neighbors[p].m_id.m_c);
                    fprintf(g_fileDebugSC3DMCEnc, "\t\t pred a = %i b = %i c = %i \n", m_neighbors[p].m_id.m_a, m_neighbors[p].m_id.m_b, m_neighbors[p].m_id.m_c);
#endif //DEBUG_VERBOSE
                    cost = -log2((m_freqPreds[p]+1.0) / nPredictors );
                    for (unsigned long i = 0; i < dimFloatArray; ++i) 
                    {
#ifdef DEBUG_VERBOSE1
                        printf("\t\t\t %i\n", m_neighbors[p].m_pred[i]);
                        fprintf(g_fileDebugSC3DMCEnc, "\t\t\t %i\n", m_neighbors[p].m_pred[i]);
#endif //DEBUG_VERBOSE

                        predResidual = (long) IntToUInt(m_quantFloatArray[v*stride+i] - m_neighbors[p].m_pred[i]);
                        if (predResidual < (long) M) 
                        {
                            cost += -log2((m_freqSymbols[predResidual]+1.0) / nSymbols );
                        }
                        else 
                        {
                            cost += -log2((m_freqSymbols[M] + 1.0) / nSymbols ) + log2((double) (predResidual-M));
                        }
                    }
                    if (cost < bestCost)
                    {
                        bestCost = cost;
                        bestPred = p;
                    }
                }
                if (m_streamType == O3DGC_STREAM_TYPE_ASCII)
                {
                    m_predictors.PushBack((unsigned char) bestPred);
                }
                else
                {
                    ace.encode(bestPred, mModelPreds);
                }
#ifdef DEBUG_VERBOSE1
                    printf("best (%i, %i, %i) \t pos %i\n", m_neighbors[bestPred].m_id.m_a, m_neighbors[bestPred].m_id.m_b, m_neighbors[bestPred].m_id.m_c, bestPred);
                    fprintf(g_fileDebugSC3DMCEnc, "best (%i, %i, %i) \t pos %i\n", m_neighbors[bestPred].m_id.m_a, m_neighbors[bestPred].m_id.m_b, m_neighbors[bestPred].m_id.m_c, bestPred);
#endif //DEBUG_VERBOSE
                // use best predictor
                for (unsigned long i = 0; i < dimFloatArray; ++i) 
                {
                    predResidual  = m_quantFloatArray[v*stride+i] - m_neighbors[bestPred].m_pred[i];
                    uPredResidual = IntToUInt(predResidual);
                    ++m_freqSymbols[(uPredResidual < (long) M)? uPredResidual : M];

#ifdef DEBUG_VERBOSE
                    printf("%i \t %i \t [%i]\n", vm*dimFloatArray+i, predResidual, m_neighbors[bestPred].m_pred[i]);
                    fprintf(g_fileDebugSC3DMCEnc, "%i \t %i \t [%i]\n", vm*dimFloatArray+i, predResidual, m_neighbors[bestPred].m_pred[i]);
#endif //DEBUG_VERBOSE

                    if (m_streamType == O3DGC_STREAM_TYPE_ASCII)
                    {
                        bstream.WriteIntASCII(predResidual);
                    }
                    else
                    {
                        EncodeIntACEGC(predResidual, ace, mModelValues, bModel0, bModel1, M);
                    }
                }
                ++m_freqPreds[bestPred];
                nSymbols += dimFloatArray;
                ++nPredictors;
            }
            else if ( vm > 0 && predMode != O3DGC_SC3DMC_NO_PREDICTION)
            {
                long prev = invVMap[vm-1];
                for (unsigned long i = 0; i < dimFloatArray; i++) 
                {
                    predResidual = m_quantFloatArray[v*stride+i] - m_quantFloatArray[prev*stride+i];
                    if (m_streamType == O3DGC_STREAM_TYPE_ASCII)
                    {
                        bstream.WriteIntASCII(predResidual);
                    }
                    else
                    {
                        EncodeIntACEGC(predResidual, ace, mModelValues, bModel0, bModel1, M);
                    }
#ifdef DEBUG_VERBOSE
                    printf("%i \t %i\n", vm*dimFloatArray+i, predResidual);
                    fprintf(g_fileDebugSC3DMCEnc, "%i \t %i\n", vm*dimFloatArray+i, predResidual);
#endif //DEBUG_VERBOSE
                }
            }
            else
            {
                for (unsigned long i = 0; i < dimFloatArray; i++) 
                {
                    predResidual = m_quantFloatArray[v*stride+i];
                    if (m_streamType == O3DGC_STREAM_TYPE_ASCII)
                    {
                        bstream.WriteUIntASCII(predResidual);
                    }
                    else
                    {
                        EncodeUIntACEGC(predResidual, ace, mModelValues, bModel0, bModel1, M);
                    }
#ifdef DEBUG_VERBOSE
                    printf("%i \t %i\n", vm*dimFloatArray+i, predResidual);
                    fprintf(g_fileDebugSC3DMCEnc, "%i \t %i\n", vm*dimFloatArray+i, predResidual);
#endif //DEBUG_VERBOSE
                }
            }
        }
        if (m_streamType != O3DGC_STREAM_TYPE_ASCII)
        {
            unsigned long encodedBytes = ace.stop_encoder();
            for(unsigned long i = 0; i < encodedBytes; ++i)
            {
                bstream.WriteUChar8Bin(m_bufferAC[i]);
            }
        }
        bstream.WriteUInt32(start, bstream.GetSize() - start, m_streamType);

        if (m_streamType == O3DGC_STREAM_TYPE_ASCII)
        {
            unsigned long start = bstream.GetSize();
            bstream.WriteUInt32ASCII(0);
            const unsigned long size       = m_predictors.GetSize();
            for(unsigned long i = 0; i < size; ++i)
            {
                bstream.WriteUCharASCII((unsigned char) m_predictors[i]);
            }
            bstream.WriteUInt32ASCII(start, bstream.GetSize() - start);
        }
#ifdef DEBUG_VERBOSE
        fflush(g_fileDebugSC3DMCEnc);
#endif //DEBUG_VERBOSE
        return O3DGC_OK;
    }

    template <class T>
    O3DGCErrorCode SC3DMCEncoder<T>::EncodeIntArray(const long * const intArray, 
                                                    unsigned long numIntArray,
                                                    unsigned long dimIntArray,
                                                    unsigned long stride,
                                                    const IndexedFaceSet<T> & ifs,
                                                    O3DGCSC3DMCPredictionMode predMode,
                                                    BinaryStream & bstream)
    {
        assert(dimIntArray <  O3DGC_SC3DMC_MAX_DIM_ATTRIBUTES);
        long predResidual, v, uPredResidual;
        unsigned long nPred;
        Arithmetic_Codec ace;
        Static_Bit_Model bModel0;
        Adaptive_Bit_Model bModel1;

        const AdjacencyInfo & v2T         = m_triangleListEncoder.GetVertexToTriangle();
        const long * const    vmap        = m_triangleListEncoder.GetVMap();
        const long * const    invVMap     = m_triangleListEncoder.GetInvVMap();
        const T * const       triangles   = ifs.GetCoordIndex();
        const long            nvert       = (long) numIntArray;
        unsigned long         start       = bstream.GetSize();
        unsigned char         mask        = predMode & 7;
        const unsigned long   M           = O3DGC_SC3DMC_MAX_PREDICTION_SYMBOLS - 1;
        unsigned long         nSymbols    = O3DGC_SC3DMC_MAX_PREDICTION_SYMBOLS;
        unsigned long         nPredictors = O3DGC_SC3DMC_MAX_PREDICTION_NEIGHBORS;
        

        Adaptive_Data_Model mModelValues(M+2);
        Adaptive_Data_Model mModelPreds(O3DGC_SC3DMC_MAX_PREDICTION_NEIGHBORS+1);

        memset(m_freqSymbols, 0, sizeof(unsigned long) * O3DGC_SC3DMC_MAX_PREDICTION_SYMBOLS);
        memset(m_freqPreds  , 0, sizeof(unsigned long) * O3DGC_SC3DMC_MAX_PREDICTION_NEIGHBORS);
        if (m_streamType == O3DGC_STREAM_TYPE_ASCII)
        {
            mask += (O3DGC_SC3DMC_BINARIZATION_ASCII & 7)<<4;
            m_predictors.Allocate(nvert);
            m_predictors.Clear();
        }
        else
        {
            mask += (O3DGC_SC3DMC_BINARIZATION_AC_EGC & 7)<<4;
            const unsigned int NMAX = numIntArray * dimIntArray * 8 + 100;
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
        }
        bstream.WriteUInt32(0, m_streamType);
        bstream.WriteUChar(mask, m_streamType);

#ifdef DEBUG_VERBOSE
        printf("IntArray (%i, %i)\n", numIntArray, dimIntArray);
        fprintf(g_fileDebugSC3DMCEnc, "IntArray (%i, %i)\n", numIntArray, dimIntArray);
#endif //DEBUG_VERBOSE

        for (long vm=0; vm < nvert; ++vm) 
        {
            nPred = 0;
            v     = invVMap[vm];
            assert( v >= 0 && v < nvert);
            if ( v2T.GetNumNeighbors(v) > 0 && 
                 predMode != O3DGC_SC3DMC_NO_PREDICTION)
            {
                int u0 = v2T.Begin(v);
                int u1 = v2T.End(v);
                for (long u = u0; u < u1; u++) 
                {
                    long ta = v2T.GetNeighbor(u);
                    for(long k = 0; k < 3; ++k)
                    {
                        long w = triangles[ta*3 + k];
                        if ( vmap[w] < vm )
                        {
                            SC3DMCTriplet id = {-1, -1, vmap[w]};
                            unsigned long p = Insert(id, nPred, m_neighbors);
                            if (p != 0xFFFFFFFF)
                            {
                                for (unsigned long i = 0; i < dimIntArray; i++) 
                                {
                                    m_neighbors[p].m_pred[i] = intArray[w*stride+i];
                                } 
                            }
                        }
                    }
                }
            }
            if (nPred > 1)
            {
                // find best predictor
                unsigned long bestPred = 0xFFFFFFFF;
                double bestCost = O3DGC_MAX_DOUBLE;
                double cost;
#ifdef DEBUG_VERBOSE1
                    printf("\t\t vm %i\n", vm);
                    fprintf(g_fileDebugSC3DMCEnc, "\t\t vm %i\n", vm);
#endif //DEBUG_VERBOSE

                for (unsigned long p = 0; p < nPred; ++p)
                {
#ifdef DEBUG_VERBOSE1
                    printf("\t\t pred a = %i b = %i c = %i \n", m_neighbors[p].m_id.m_a, m_neighbors[p].m_id.m_b, m_neighbors[p].m_id.m_c);
                    fprintf(g_fileDebugSC3DMCEnc, "\t\t pred a = %i b = %i c = %i \n", m_neighbors[p].m_id.m_a, m_neighbors[p].m_id.m_b, m_neighbors[p].m_id.m_c);
#endif //DEBUG_VERBOSE
                    cost = -log2((m_freqPreds[p]+1.0) / nPredictors );
                    for (unsigned long i = 0; i < dimIntArray; ++i) 
                    {
#ifdef DEBUG_VERBOSE1
                        printf("\t\t\t %i\n", m_neighbors[p].m_pred[i]);
                        fprintf(g_fileDebugSC3DMCEnc, "\t\t\t %i\n", m_neighbors[p].m_pred[i]);
#endif //DEBUG_VERBOSE

                        predResidual = (long) IntToUInt(intArray[v*stride+i] - m_neighbors[p].m_pred[i]);
                        if (predResidual < (long) M) 
                        {
                            cost += -log2((m_freqSymbols[predResidual]+1.0) / nSymbols );
                        }
                        else 
                        {
                            cost += -log2((m_freqSymbols[M] + 1.0) / nSymbols ) + log2((double) (predResidual-M));
                        }
                    }
                    if (cost < bestCost)
                    {
                        bestCost = cost;
                        bestPred = p;
                    }
                }
                if (m_streamType == O3DGC_STREAM_TYPE_ASCII)
                {
                    m_predictors.PushBack((unsigned char) bestPred);
                }
                else
                {
                    ace.encode(bestPred, mModelPreds);
                }
#ifdef DEBUG_VERBOSE1
                    printf("best (%i, %i, %i) \t pos %i\n", m_neighbors[bestPred].m_id.m_a, m_neighbors[bestPred].m_id.m_b, m_neighbors[bestPred].m_id.m_c, bestPred);
                    fprintf(g_fileDebugSC3DMCEnc, "best (%i, %i, %i) \t pos %i\n", m_neighbors[bestPred].m_id.m_a, m_neighbors[bestPred].m_id.m_b, m_neighbors[bestPred].m_id.m_c, bestPred);
#endif //DEBUG_VERBOSE
                // use best predictor
                for (unsigned long i = 0; i < dimIntArray; ++i) 
                {
                    predResidual  = intArray[v*stride+i] - m_neighbors[bestPred].m_pred[i];
                    uPredResidual = IntToUInt(predResidual);
                    ++m_freqSymbols[(uPredResidual < (long) M)? uPredResidual : M];

#ifdef DEBUG_VERBOSE
                    printf("%i \t %i \t [%i]\n", vm*dimIntArray+i, predResidual, m_neighbors[bestPred].m_pred[i]);
                    fprintf(g_fileDebugSC3DMCEnc, "%i \t %i \t [%i]\n", vm*dimIntArray+i, predResidual, m_neighbors[bestPred].m_pred[i]);
#endif //DEBUG_VERBOSE

                    if (m_streamType == O3DGC_STREAM_TYPE_ASCII)
                    {
                        bstream.WriteIntASCII(predResidual);
                    }
                    else
                    {
                        EncodeIntACEGC(predResidual, ace, mModelValues, bModel0, bModel1, M);
                    }
                }
                ++m_freqPreds[bestPred];
                nSymbols += dimIntArray;
                ++nPredictors;
            }
            else if ( vm > 0 && predMode != O3DGC_SC3DMC_NO_PREDICTION)
            {
                long prev = invVMap[vm-1];
                for (unsigned long i = 0; i < dimIntArray; i++) 
                {
                    predResidual = intArray[v*stride+i] - intArray[prev*stride+i];
                    if (m_streamType == O3DGC_STREAM_TYPE_ASCII)
                    {
                        bstream.WriteIntASCII(predResidual);
                    }
                    else
                    {
                        EncodeIntACEGC(predResidual, ace, mModelValues, bModel0, bModel1, M);
                    }
#ifdef DEBUG_VERBOSE
                    printf("%i \t %i\n", vm*dimIntArray+i, predResidual);
                    fprintf(g_fileDebugSC3DMCEnc, "%i \t %i\n", vm*dimIntArray+i, predResidual);
#endif //DEBUG_VERBOSE
                }
            }
            else
            {
                for (unsigned long i = 0; i < dimIntArray; i++) 
                {
                    predResidual = intArray[v*stride+i];
                    if (m_streamType == O3DGC_STREAM_TYPE_ASCII)
                    {
                        bstream.WriteUIntASCII(predResidual);
                    }
                    else
                    {
                        EncodeUIntACEGC(predResidual, ace, mModelValues, bModel0, bModel1, M);
                    }
#ifdef DEBUG_VERBOSE
                    printf("%i \t %i\n", vm*dimIntArray+i, predResidual);
                    fprintf(g_fileDebugSC3DMCEnc, "%i \t %i\n", vm*dimIntArray+i, predResidual);
#endif //DEBUG_VERBOSE
                }
            }
        }
        if (m_streamType != O3DGC_STREAM_TYPE_ASCII)
        {
            unsigned long encodedBytes = ace.stop_encoder();
            for(unsigned long i = 0; i < encodedBytes; ++i)
            {
                bstream.WriteUChar8Bin(m_bufferAC[i]);
            }
        }
        bstream.WriteUInt32(start, bstream.GetSize() - start, m_streamType);

        if (m_streamType == O3DGC_STREAM_TYPE_ASCII)
        {
            unsigned long start = bstream.GetSize();
            bstream.WriteUInt32ASCII(0);
            const unsigned long size       = m_predictors.GetSize();
            for(unsigned long i = 0; i < size; ++i)
            {
                bstream.WriteUCharASCII((unsigned char) m_predictors[i]);
            }
            bstream.WriteUInt32ASCII(start, bstream.GetSize() - start);
        }
#ifdef DEBUG_VERBOSE
        fflush(g_fileDebugSC3DMCEnc);
#endif //DEBUG_VERBOSE
        return O3DGC_OK;
    }
    template <class T>
    O3DGCErrorCode SC3DMCEncoder<T>::ProcessNormals(const IndexedFaceSet<T> & ifs)
    {
        const long nvert               = (long) ifs.GetNNormal();
        const unsigned long normalSize = ifs.GetNNormal() * 2;
        if (m_normalsSize < normalSize)
        {
            delete [] m_normals;
            m_normalsSize = normalSize;
            m_normals     = new Real [normalSize];
        }                                  
        const AdjacencyInfo & v2T          = m_triangleListEncoder.GetVertexToTriangle();
        const long * const    invVMap      = m_triangleListEncoder.GetInvVMap();
        const T * const       triangles    = ifs.GetCoordIndex();
        const Real * const originalNormals = ifs.GetNormal();
        Vec3<long> p1, p2, p3, n0, nt;
        Vec3<Real> n1;
        long na0 = 0, nb0 = 0;
        Real rna0, rnb0, na1 = 0, nb1 = 0, norm0, norm1;
        char ni0 = 0, ni1 = 0;
        long a, b, c, v;
        m_predictors.Clear();
        for (long i=0; i < nvert; ++i) 
        {
            v = invVMap[i];
            n0.X() = 0;
            n0.Y() = 0;
            n0.Z() = 0;
            int u0 = v2T.Begin(v);
            int u1 = v2T.End(v);
            for (long u = u0; u < u1; u++) 
            {
                long ta = v2T.GetNeighbor(u);
                a = triangles[ta*3 + 0];
                b = triangles[ta*3 + 1];
                c = triangles[ta*3 + 2];
                p1.X() = m_quantFloatArray[3*a];
                p1.Y() = m_quantFloatArray[3*a+1];
                p1.Z() = m_quantFloatArray[3*a+2];
                p2.X() = m_quantFloatArray[3*b];
                p2.Y() = m_quantFloatArray[3*b+1];
                p2.Z() = m_quantFloatArray[3*b+2];
                p3.X() = m_quantFloatArray[3*c];
                p3.Y() = m_quantFloatArray[3*c+1];
                p3.Z() = m_quantFloatArray[3*c+2];
                nt  = (p2-p1)^(p3-p1);
                n0 += nt;
            }
            norm0 = (Real) n0.GetNorm();
            if (norm0 == 0.0)
            {
                norm0 = 1.0;
            }
            SphereToCube(n0.X(), n0.Y(), n0.Z(), na0, nb0, ni0);
            rna0 = na0 / norm0;
            rnb0 = nb0 / norm0;

            n1.X() = originalNormals[3*v];
            n1.Y() = originalNormals[3*v+1];
            n1.Z() = originalNormals[3*v+2];
            norm1 = (Real) n1.GetNorm();
            if (norm1 != 0.0)
            {
                n1.X() /= norm1;
                n1.Y() /= norm1;
                n1.Z() /= norm1;
            }
            SphereToCube(n1.X(), n1.Y(), n1.Z(), na1, nb1, ni1);
            m_predictors.PushBack(ni1 - ni0);
            if ( (ni1 >> 1) != (ni0 >> 1) )
            {
                rna0 = (Real)0.0;
                rnb0 = (Real)0.0;
            }
            m_normals[2*v]   = na1 - rna0;
            m_normals[2*v+1] = nb1 - rnb0;

#ifdef DEBUG_VERBOSE1
            printf("n0 \t %i \t %i \t %i \t %i (%f, %f)\n", i, n0.X(), n0.Y(), n0.Z(), rna0, rnb0);
            fprintf(g_fileDebugSC3DMCEnc,"n0 \t %i \t %i \t %i \t %i (%f, %f)\n", i, n0.X(), n0.Y(), n0.Z(), rna0, rnb0);
#endif //DEBUG_VERBOSE

#ifdef DEBUG_VERBOSE1
            printf("normal \t %i \t %f \t %f \t %f \t (%i, %f, %f) \t (%f, %f)\n", i, n1.X(), n1.Y(), n1.Z(), ni1, na1, nb1, rna0, rnb0);
            fprintf(g_fileDebugSC3DMCEnc, "normal \t %i \t %f \t %f \t %f \t (%i, %f, %f) \t (%f, %f)\n", i, n1.X(), n1.Y(), n1.Z(), ni1, na1, nb1, rna0, rnb0);
#endif //DEBUG_VERBOSE

        }
        return O3DGC_OK;
    }

    template <class T>
    O3DGCErrorCode SC3DMCEncoder<T>::EncodePayload(const SC3DMCEncodeParams & params, 
                                                   const IndexedFaceSet<T> & ifs, 
                                                   BinaryStream & bstream)
    {
#ifdef DEBUG_VERBOSE
        g_fileDebugSC3DMCEnc = fopen("tfans_enc_main.txt", "w");
#endif //DEBUG_VERBOSE

        // encode triangle list        
        m_triangleListEncoder.SetStreamType(params.GetStreamType());
        m_stats.m_streamSizeCoordIndex = bstream.GetSize();
        Timer timer;
        timer.Tic();
        m_triangleListEncoder.Encode(ifs.GetCoordIndex(), ifs.GetIndexBufferID(), ifs.GetNCoordIndex(), ifs.GetNCoord(), bstream);
        timer.Toc();
        m_stats.m_timeCoordIndex       = timer.GetElapsedTime();
        m_stats.m_streamSizeCoordIndex = bstream.GetSize() - m_stats.m_streamSizeCoordIndex;

        // encode coord
        m_stats.m_streamSizeCoord = bstream.GetSize();
        timer.Tic();
        if (ifs.GetNCoord() > 0)
        {
            EncodeFloatArray(ifs.GetCoord(), ifs.GetNCoord(), 3, 3, ifs.GetCoordMin(), ifs.GetCoordMax(), 
                                params.GetCoordQuantBits(), ifs, params.GetCoordPredMode(), bstream);
        }
        timer.Toc();
        m_stats.m_timeCoord       = timer.GetElapsedTime();
        m_stats.m_streamSizeCoord = bstream.GetSize() - m_stats.m_streamSizeCoord;


        // encode Normal
        m_stats.m_streamSizeNormal = bstream.GetSize();
        timer.Tic();
        if (ifs.GetNNormal() > 0)
        {
            if (params.GetNormalPredMode() == O3DGC_SC3DMC_SURF_NORMALS_PREDICTION)
            {
                ProcessNormals(ifs);
                EncodeFloatArray(m_normals, ifs.GetNNormal(), 2, 2, ifs.GetNormalMin(), ifs.GetNormalMax(), 
                params.GetNormalQuantBits(), ifs, params.GetNormalPredMode(), bstream);
            }
            else
            {
                EncodeFloatArray(ifs.GetNormal(), ifs.GetNNormal(), 3, 3, ifs.GetNormalMin(), ifs.GetNormalMax(), 
                params.GetNormalQuantBits(), ifs, params.GetNormalPredMode(), bstream);
            }
        }
        timer.Toc();
        m_stats.m_timeNormal       = timer.GetElapsedTime();
        m_stats.m_streamSizeNormal = bstream.GetSize() - m_stats.m_streamSizeNormal;


        // encode FloatAttribute
        for(unsigned long a = 0; a < ifs.GetNumFloatAttributes(); ++a)
        {
            m_stats.m_streamSizeFloatAttribute[a] = bstream.GetSize();
            timer.Tic();
            EncodeFloatArray(ifs.GetFloatAttribute(a), ifs.GetNFloatAttribute(a), 
                             ifs.GetFloatAttributeDim(a), ifs.GetFloatAttributeDim(a),
                             ifs.GetFloatAttributeMin(a), ifs.GetFloatAttributeMax(a), 
                             params.GetFloatAttributeQuantBits(a), ifs, 
                             params.GetFloatAttributePredMode(a), bstream);
            timer.Toc();
            m_stats.m_timeFloatAttribute[a]       = timer.GetElapsedTime();
            m_stats.m_streamSizeFloatAttribute[a] = bstream.GetSize() - m_stats.m_streamSizeFloatAttribute[a];
        }

        // encode IntAttribute
        for(unsigned long a = 0; a < ifs.GetNumIntAttributes(); ++a)
        {
            m_stats.m_streamSizeIntAttribute[a] = bstream.GetSize();
            timer.Tic();
            EncodeIntArray(ifs.GetIntAttribute(a), ifs.GetNIntAttribute(a), ifs.GetIntAttributeDim(a), 
                           ifs.GetIntAttributeDim(a), ifs, params.GetIntAttributePredMode(a), bstream);
            timer.Toc();
            m_stats.m_timeIntAttribute[a]       = timer.GetElapsedTime();
            m_stats.m_streamSizeIntAttribute[a] = bstream.GetSize() - m_stats.m_streamSizeIntAttribute[a];
        }
#ifdef DEBUG_VERBOSE
        fclose(g_fileDebugSC3DMCEnc);
#endif //DEBUG_VERBOSE
        return O3DGC_OK;
    }
}
#endif // O3DGC_SC3DMC_ENCODER_INL


