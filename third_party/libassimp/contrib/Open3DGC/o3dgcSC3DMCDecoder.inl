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
#ifndef O3DGC_SC3DMC_DECODER_INL
#define O3DGC_SC3DMC_DECODER_INL

#include "o3dgcArithmeticCodec.h"
#include "o3dgcTimer.h"

//#define DEBUG_VERBOSE

namespace o3dgc
{
#ifdef DEBUG_VERBOSE
        FILE * g_fileDebugSC3DMCDec = NULL;
#endif //DEBUG_VERBOSE

    template<class T>
    O3DGCErrorCode SC3DMCDecoder<T>::DecodeHeader(IndexedFaceSet<T> & ifs, 
                                                  const BinaryStream & bstream)
    {
        unsigned long iterator0 = m_iterator;
        unsigned long start_code = bstream.ReadUInt32(m_iterator, O3DGC_STREAM_TYPE_BINARY);
        if (start_code != O3DGC_SC3DMC_START_CODE)
        {
            m_iterator = iterator0;
            start_code = bstream.ReadUInt32(m_iterator, O3DGC_STREAM_TYPE_ASCII);
            if (start_code != O3DGC_SC3DMC_START_CODE)
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
        m_params.SetEncodeMode( (O3DGCSC3DMCEncodingMode) bstream.ReadUChar(m_iterator, m_streamType));

        ifs.SetCreaseAngle((Real) bstream.ReadFloat32(m_iterator, m_streamType));
          
        unsigned char mask = bstream.ReadUChar(m_iterator, m_streamType);

        ifs.SetCCW             ((mask & 1) == 1);
        ifs.SetSolid           ((mask & 2) == 1);
        ifs.SetConvex          ((mask & 4) == 1);
        ifs.SetIsTriangularMesh((mask & 8) == 1);
        //bool markerBit0 = (mask & 16 ) == 1;
        //bool markerBit1 = (mask & 32 ) == 1;
        //bool markerBit2 = (mask & 64 ) == 1;
        //bool markerBit3 = (mask & 128) == 1;
       
        ifs.SetNCoord         (bstream.ReadUInt32(m_iterator, m_streamType));
        ifs.SetNNormal        (bstream.ReadUInt32(m_iterator, m_streamType));


        ifs.SetNumFloatAttributes(bstream.ReadUInt32(m_iterator, m_streamType));
        ifs.SetNumIntAttributes  (bstream.ReadUInt32(m_iterator, m_streamType));
                              
        if (ifs.GetNCoord() > 0)
        {
            ifs.SetNCoordIndex(bstream.ReadUInt32(m_iterator, m_streamType));
            for(int j=0 ; j<3 ; ++j)
            {
                ifs.SetCoordMin(j, (Real) bstream.ReadFloat32(m_iterator, m_streamType));
                ifs.SetCoordMax(j, (Real) bstream.ReadFloat32(m_iterator, m_streamType));
            }
            m_params.SetCoordQuantBits( bstream.ReadUChar(m_iterator, m_streamType) );
        }
        if (ifs.GetNNormal() > 0)
        {
            ifs.SetNNormalIndex(bstream.ReadUInt32(m_iterator, m_streamType));
            for(int j=0 ; j<3 ; ++j)
            {
                ifs.SetNormalMin(j, (Real) bstream.ReadFloat32(m_iterator, m_streamType));
                ifs.SetNormalMax(j, (Real) bstream.ReadFloat32(m_iterator, m_streamType));
            }
            ifs.SetNormalPerVertex(bstream.ReadUChar(m_iterator, m_streamType) == 1);
            m_params.SetNormalQuantBits(bstream.ReadUChar(m_iterator, m_streamType));
        }

        for(unsigned long a = 0; a < ifs.GetNumFloatAttributes(); ++a)
        {
            ifs.SetNFloatAttribute(a, bstream.ReadUInt32(m_iterator, m_streamType));    
            if (ifs.GetNFloatAttribute(a) > 0)
            {
                ifs.SetNFloatAttributeIndex(a, bstream.ReadUInt32(m_iterator, m_streamType));
                unsigned char d = bstream.ReadUChar(m_iterator, m_streamType);
                ifs.SetFloatAttributeDim(a, d);
                for(unsigned char j = 0 ; j < d ; ++j)
                {
                    ifs.SetFloatAttributeMin(a, j, (Real) bstream.ReadFloat32(m_iterator, m_streamType));
                    ifs.SetFloatAttributeMax(a, j, (Real) bstream.ReadFloat32(m_iterator, m_streamType));
                }
                ifs.SetFloatAttributePerVertex(a, bstream.ReadUChar(m_iterator, m_streamType) == 1);
                ifs.SetFloatAttributeType(a, (O3DGCIFSFloatAttributeType) bstream.ReadUChar(m_iterator, m_streamType));
                m_params.SetFloatAttributeQuantBits(a, bstream.ReadUChar(m_iterator, m_streamType));
            }
        }
        for(unsigned long a = 0; a < ifs.GetNumIntAttributes(); ++a)
        {
            ifs.SetNIntAttribute(a, bstream.ReadUInt32(m_iterator, m_streamType));
            if (ifs.GetNIntAttribute(a) > 0)
            {
                ifs.SetNIntAttributeIndex(a, bstream.ReadUInt32(m_iterator, m_streamType));
                ifs.SetIntAttributeDim(a, bstream.ReadUChar(m_iterator, m_streamType));
                ifs.SetIntAttributePerVertex(a, bstream.ReadUChar(m_iterator, m_streamType) == 1);
                ifs.SetIntAttributeType(a, (O3DGCIFSIntAttributeType) bstream.ReadUChar(m_iterator, m_streamType));
            }
        }    
        return O3DGC_OK;
    }
    template<class T>
	O3DGCErrorCode SC3DMCDecoder<T>::DecodePayload(IndexedFaceSet<T> & ifs,
                                                    const BinaryStream & bstream)
    {
        O3DGCErrorCode ret = O3DGC_OK;
#ifdef DEBUG_VERBOSE
        g_fileDebugSC3DMCDec = fopen("tfans_dec_main.txt", "w");
#endif //DEBUG_VERBOSE

        m_triangleListDecoder.SetStreamType(m_streamType);
        m_stats.m_streamSizeCoordIndex = m_iterator;
        Timer timer;
        timer.Tic();
        m_triangleListDecoder.Decode(ifs.GetCoordIndex(), ifs.GetNCoordIndex(), ifs.GetNCoord(), bstream, m_iterator);
        timer.Toc();
        m_stats.m_timeCoordIndex       = timer.GetElapsedTime();
        m_stats.m_streamSizeCoordIndex = m_iterator - m_stats.m_streamSizeCoordIndex;

        // decode coord
        m_stats.m_streamSizeCoord = m_iterator;
        timer.Tic();
        if (ifs.GetNCoord() > 0)
        {
            ret = DecodeFloatArray(ifs.GetCoord(), ifs.GetNCoord(), 3, 3, ifs.GetCoordMin(), ifs.GetCoordMax(),
                                   m_params.GetCoordQuantBits(), ifs, m_params.GetCoordPredMode(), bstream);
        }
        if (ret != O3DGC_OK)
        {
            return ret;
        }
        timer.Toc();
        m_stats.m_timeCoord       = timer.GetElapsedTime();
        m_stats.m_streamSizeCoord = m_iterator - m_stats.m_streamSizeCoord;

        // decode Normal
        m_stats.m_streamSizeNormal = m_iterator;
        timer.Tic();
        if (ifs.GetNNormal() > 0)
        {
            DecodeFloatArray(ifs.GetNormal(), ifs.GetNNormal(), 3, 3, ifs.GetNormalMin(), ifs.GetNormalMax(),
                                m_params.GetNormalQuantBits(), ifs, m_params.GetNormalPredMode(), bstream);
        }
        if (ret != O3DGC_OK)
        {
            return ret;
        }
        timer.Toc();
        m_stats.m_timeNormal       = timer.GetElapsedTime();
        m_stats.m_streamSizeNormal = m_iterator - m_stats.m_streamSizeNormal;

        // decode FloatAttributes
        for(unsigned long a = 0; a < ifs.GetNumFloatAttributes(); ++a)
        {
            m_stats.m_streamSizeFloatAttribute[a] = m_iterator;
            timer.Tic();
            DecodeFloatArray(ifs.GetFloatAttribute(a), ifs.GetNFloatAttribute(a), ifs.GetFloatAttributeDim(a), ifs.GetFloatAttributeDim(a), 
                                ifs.GetFloatAttributeMin(a), ifs.GetFloatAttributeMax(a), 
                                m_params.GetFloatAttributeQuantBits(a), ifs, m_params.GetFloatAttributePredMode(a), bstream);
            timer.Toc();
            m_stats.m_timeFloatAttribute[a]       = timer.GetElapsedTime();
            m_stats.m_streamSizeFloatAttribute[a] = m_iterator - m_stats.m_streamSizeFloatAttribute[a];
        }
        if (ret != O3DGC_OK)
        {
            return ret;
        }

        // decode IntAttributes
        for(unsigned long a = 0; a < ifs.GetNumIntAttributes(); ++a)
        {
            m_stats.m_streamSizeIntAttribute[a] = m_iterator;
            timer.Tic();
            DecodeIntArray(ifs.GetIntAttribute(a), ifs.GetNIntAttribute(a), ifs.GetIntAttributeDim(a), ifs.GetIntAttributeDim(a), 
                           ifs, m_params.GetIntAttributePredMode(a), bstream);
            timer.Toc();
            m_stats.m_timeIntAttribute[a]       = timer.GetElapsedTime();
            m_stats.m_streamSizeIntAttribute[a] = m_iterator - m_stats.m_streamSizeIntAttribute[a];
        }
        if (ret != O3DGC_OK)
        {
            return ret;
        }

        timer.Tic();
        m_triangleListDecoder.Reorder();
        timer.Toc();
        m_stats.m_timeReorder       = timer.GetElapsedTime();

#ifdef DEBUG_VERBOSE
        fclose(g_fileDebugSC3DMCDec);
#endif //DEBUG_VERBOSE
        return ret;
    }
    template<class T>
    O3DGCErrorCode SC3DMCDecoder<T>::DecodeIntArray(long * const intArray, 
                                                    unsigned long numIntArray,
                                                    unsigned long dimIntArray,
                                                    unsigned long stride,
                                                    const IndexedFaceSet<T> & ifs,
                                                    O3DGCSC3DMCPredictionMode & predMode,
                                                    const BinaryStream & bstream)
    {
        assert(dimIntArray <  O3DGC_SC3DMC_MAX_DIM_ATTRIBUTES);
        long predResidual;
        SC3DMCPredictor m_neighbors  [O3DGC_SC3DMC_MAX_PREDICTION_NEIGHBORS];
        Arithmetic_Codec acd;
        Static_Bit_Model bModel0;
        Adaptive_Bit_Model bModel1;
        Adaptive_Data_Model mModelPreds(O3DGC_SC3DMC_MAX_PREDICTION_NEIGHBORS+1);
        unsigned long nPred;

        const AdjacencyInfo & v2T            = m_triangleListDecoder.GetVertexToTriangle();
        const T * const     triangles        = ifs.GetCoordIndex();
        const long          nvert            = (long) numIntArray;
        unsigned char *     buffer           = 0;
        unsigned long       start            = m_iterator;
        unsigned long       streamSize       = bstream.ReadUInt32(m_iterator, m_streamType);        // bitsream size
        unsigned char mask                   = bstream.ReadUChar(m_iterator, m_streamType);
        O3DGCSC3DMCBinarization binarization = (O3DGCSC3DMCBinarization)((mask >> 4) & 7);
        predMode                             = (O3DGCSC3DMCPredictionMode)(mask & 7);
        streamSize                          -= (m_iterator - start);
        unsigned long       iteratorPred     = m_iterator + streamSize;
        unsigned int        exp_k            = 0;
        unsigned int        M                = 0;
        if (m_streamType != O3DGC_STREAM_TYPE_ASCII)
        {
            if (binarization != O3DGC_SC3DMC_BINARIZATION_AC_EGC)
            {
                return O3DGC_ERROR_CORRUPTED_STREAM;
            }
            bstream.GetBuffer(m_iterator, buffer);
            m_iterator += streamSize;
            acd.set_buffer(streamSize, buffer);
            acd.start_decoder();
            exp_k = acd.ExpGolombDecode(0, bModel0, bModel1);
            M     = acd.ExpGolombDecode(0, bModel0, bModel1);
        }
        else
        {
            if (binarization != O3DGC_SC3DMC_BINARIZATION_ASCII)
            {
                return O3DGC_ERROR_CORRUPTED_STREAM;
            }
            bstream.ReadUInt32(iteratorPred, m_streamType);        // predictors bitsream size
        }
        Adaptive_Data_Model mModelValues(M+2);

#ifdef DEBUG_VERBOSE
        printf("IntArray (%i, %i)\n", numIntArray, dimIntArray);
        fprintf(g_fileDebugSC3DMCDec, "IntArray (%i, %i)\n", numIntArray, dimIntArray);
#endif //DEBUG_VERBOSE

        for (long v=0; v < nvert; ++v) 
        {
            nPred = 0;
            if ( v2T.GetNumNeighbors(v) > 0 && 
                 predMode != O3DGC_SC3DMC_NO_PREDICTION)
            {
                int u0 = v2T.Begin(v);
                int u1 = v2T.End(v);
                for (long u = u0; u < u1; u++) 
                {
                    long ta = v2T.GetNeighbor(u);
                    if (ta < 0)
                    {
                        break;
                    }
                    for(long k = 0; k < 3; ++k)
                    {
                        long w = triangles[ta*3 + k];
                        if ( w < v )
                        {
                            SC3DMCTriplet id = {-1, -1, w};
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
#ifdef DEBUG_VERBOSE1
                printf("\t\t vm %i\n", v);
                fprintf(g_fileDebugSC3DMCDec, "\t\t vm %i\n", v);
                for (unsigned long p = 0; p < nPred; ++p)
                {
                    printf("\t\t pred a = %i b = %i c = %i \n", m_neighbors[p].m_id.m_a, m_neighbors[p].m_id.m_b, m_neighbors[p].m_id.m_c);
                    fprintf(g_fileDebugSC3DMCDec, "\t\t pred a = %i b = %i c = %i \n", m_neighbors[p].m_id.m_a, m_neighbors[p].m_id.m_b, m_neighbors[p].m_id.m_c);
                    for (unsigned long i = 0; i < dimIntArray; ++i) 
                    {
                        printf("\t\t\t %i\n", m_neighbors[p].m_pred[i]);
                        fprintf(g_fileDebugSC3DMCDec, "\t\t\t %i\n", m_neighbors[p].m_pred[i]);
                    }
                }
#endif //DEBUG_VERBOSE
                unsigned long bestPred;
                if (m_streamType == O3DGC_STREAM_TYPE_ASCII)
                {
                    bestPred = bstream.ReadUCharASCII(iteratorPred);
                }
                else
                {
                    bestPred = acd.decode(mModelPreds);
                }
#ifdef DEBUG_VERBOSE1
                    printf("best (%i, %i, %i) \t pos %i\n", m_neighbors[bestPred].m_id.m_a, m_neighbors[bestPred].m_id.m_b, m_neighbors[bestPred].m_id.m_c, bestPred);
                    fprintf(g_fileDebugSC3DMCDec, "best (%i, %i, %i) \t pos %i\n", m_neighbors[bestPred].m_id.m_a, m_neighbors[bestPred].m_id.m_b, m_neighbors[bestPred].m_id.m_c, bestPred);
#endif //DEBUG_VERBOSE
                for (unsigned long i = 0; i < dimIntArray; i++) 
                {
                    if (m_streamType == O3DGC_STREAM_TYPE_ASCII)
                    {
                        predResidual = bstream.ReadIntASCII(m_iterator);
                    }
                    else
                    {
                        predResidual = DecodeIntACEGC(acd, mModelValues, bModel0, bModel1, exp_k, M);
                    }
                    intArray[v*stride+i] = predResidual + m_neighbors[bestPred].m_pred[i];
#ifdef DEBUG_VERBOSE
                    printf("%i \t %i \t [%i]\n", v*dimIntArray+i, predResidual, m_neighbors[bestPred].m_pred[i]);
                    fprintf(g_fileDebugSC3DMCDec, "%i \t %i \t [%i]\n", v*dimIntArray+i, predResidual, m_neighbors[bestPred].m_pred[i]);
#endif //DEBUG_VERBOSE
                }
            }
            else if (v > 0 && predMode != O3DGC_SC3DMC_NO_PREDICTION)
            {
                for (unsigned long i = 0; i < dimIntArray; i++) 
                {
                    if (m_streamType == O3DGC_STREAM_TYPE_ASCII)
                    {
                        predResidual = bstream.ReadIntASCII(m_iterator);
                    }
                    else
                    {
                        predResidual = DecodeIntACEGC(acd, mModelValues, bModel0, bModel1, exp_k, M);
                    }
                    intArray[v*stride+i] = predResidual + intArray[(v-1)*stride+i];
#ifdef DEBUG_VERBOSE
                    printf("%i \t %i\n", v*dimIntArray+i, predResidual);
                    fprintf(g_fileDebugSC3DMCDec, "%i \t %i\n", v*dimIntArray+i, predResidual);
#endif //DEBUG_VERBOSE
                }
            }
            else
            {
                for (unsigned long i = 0; i < dimIntArray; i++) 
                {
                    if (m_streamType == O3DGC_STREAM_TYPE_ASCII)
                    {
                        predResidual = bstream.ReadUIntASCII(m_iterator);
                    }
                    else
                    {
                        predResidual = DecodeUIntACEGC(acd, mModelValues, bModel0, bModel1, exp_k, M);
                    }
                    intArray[v*stride+i] = predResidual;
#ifdef DEBUG_VERBOSE
                    printf("%i \t %i\n", v*dimIntArray+i, predResidual);
                    fprintf(g_fileDebugSC3DMCDec, "%i \t %i\n", v*dimIntArray+i, predResidual);
#endif //DEBUG_VERBOSE
                }
            }
        }
        m_iterator  = iteratorPred;
#ifdef DEBUG_VERBOSE
        fflush(g_fileDebugSC3DMCDec);
#endif //DEBUG_VERBOSE
        return O3DGC_OK;
    }
    template <class T>
    O3DGCErrorCode SC3DMCDecoder<T>::ProcessNormals(const IndexedFaceSet<T> & ifs)
    {
        const long nvert               = (long) ifs.GetNNormal();
        const unsigned long normalSize = ifs.GetNNormal() * 2;
        if (m_normalsSize < normalSize)
        {
            delete [] m_normals;
            m_normalsSize = normalSize;
            m_normals     = new Real [normalSize];
        }                                  
        const AdjacencyInfo & v2T          = m_triangleListDecoder.GetVertexToTriangle();
        const T * const       triangles    = ifs.GetCoordIndex();        
        Vec3<long> p1, p2, p3, n0, nt;
        long na0 = 0, nb0 = 0;
        Real rna0, rnb0, norm0;
        char ni0 = 0, ni1 = 0;
        long a, b, c;
        for (long v=0; v < nvert; ++v) 
        {
            n0.X() = 0;
            n0.Y() = 0;
            n0.Z() = 0;
            int u0 = v2T.Begin(v);
            int u1 = v2T.End(v);
            for (long u = u0; u < u1; u++) 
            {
                long ta = v2T.GetNeighbor(u);
                if (ta == -1)
                {
                    break;
                }
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
            ni1  = ni0 + m_orientation[v];
            m_orientation[v] = ni1;
            if ( (ni1 >> 1) != (ni0 >> 1) )
            {
                rna0 = Real(0.0);
                rnb0 = Real(0.0);
            }
            m_normals[2*v]   = rna0;
            m_normals[2*v+1] = rnb0;

#ifdef DEBUG_VERBOSE1
            printf("n0 \t %i \t %i \t %i \t %i (%f, %f)\n", v, n0.X(), n0.Y(), n0.Z(), rna0, rnb0);
            fprintf(g_fileDebugSC3DMCDec, "n0 \t %i \t %i \t %i \t %i (%f, %f)\n", v, n0.X(), n0.Y(), n0.Z(), rna0, rnb0);
#endif //DEBUG_VERBOSE

        }
        return O3DGC_OK;
    }
    template<class T>
    O3DGCErrorCode SC3DMCDecoder<T>::DecodeFloatArray(Real * const floatArray, 
                                                   unsigned long numFloatArray,
                                                   unsigned long dimFloatArray,
                                                   unsigned long stride,
                                                   const Real * const minFloatArray,
                                                   const Real * const maxFloatArray,
                                                   unsigned long nQBits,
                                                   const IndexedFaceSet<T> & ifs,
                                                   O3DGCSC3DMCPredictionMode & predMode,
                                                   const BinaryStream & bstream)
    {
        assert(dimFloatArray <  O3DGC_SC3DMC_MAX_DIM_ATTRIBUTES);
        long predResidual;
        SC3DMCPredictor m_neighbors  [O3DGC_SC3DMC_MAX_PREDICTION_NEIGHBORS];
        Arithmetic_Codec acd;
        Static_Bit_Model bModel0;
        Adaptive_Bit_Model bModel1;
        Adaptive_Data_Model mModelPreds(O3DGC_SC3DMC_MAX_PREDICTION_NEIGHBORS+1);
        unsigned long nPred;

        const AdjacencyInfo & v2T            = m_triangleListDecoder.GetVertexToTriangle();
        const T * const     triangles        = ifs.GetCoordIndex();       
        const long          nvert            = (long) numFloatArray;
        const unsigned long size             = numFloatArray * dimFloatArray;
        unsigned char *     buffer           = 0;
        unsigned long       start            = m_iterator;
        unsigned long       streamSize       = bstream.ReadUInt32(m_iterator, m_streamType);        // bitsream size
        unsigned char mask                   = bstream.ReadUChar(m_iterator, m_streamType);
        O3DGCSC3DMCBinarization binarization = (O3DGCSC3DMCBinarization)((mask >> 4) & 7);
        predMode                             = (O3DGCSC3DMCPredictionMode)(mask & 7);
        streamSize                          -= (m_iterator - start);
        unsigned long       iteratorPred     = m_iterator + streamSize;
        unsigned int        exp_k            = 0;
        unsigned int        M                = 0;
        if (m_streamType != O3DGC_STREAM_TYPE_ASCII)
        {
            if (binarization != O3DGC_SC3DMC_BINARIZATION_AC_EGC)
            {
                return O3DGC_ERROR_CORRUPTED_STREAM;
            }
            bstream.GetBuffer(m_iterator, buffer);
            m_iterator += streamSize;
            acd.set_buffer(streamSize, buffer);
            acd.start_decoder();
            exp_k = acd.ExpGolombDecode(0, bModel0, bModel1);
            M     = acd.ExpGolombDecode(0, bModel0, bModel1);
        }
        else
        {
            if (binarization != O3DGC_SC3DMC_BINARIZATION_ASCII)
            {
                return O3DGC_ERROR_CORRUPTED_STREAM;
            }
            bstream.ReadUInt32(iteratorPred, m_streamType);        // predictors bitsream size
        }
        Adaptive_Data_Model mModelValues(M+2);


        if (predMode == O3DGC_SC3DMC_SURF_NORMALS_PREDICTION)
        {
            m_orientation.Allocate(size);
            m_orientation.Clear();
            if (m_streamType == O3DGC_STREAM_TYPE_ASCII)
            {
                for(unsigned long i = 0; i < numFloatArray; ++i)
                {
                    m_orientation.PushBack((unsigned char) bstream.ReadIntASCII(m_iterator));
                }
            }
            else
            {
                Adaptive_Data_Model dModel(12);
                for(unsigned long i = 0; i < numFloatArray; ++i)
                {
                    m_orientation.PushBack((unsigned char) UIntToInt(acd.decode(dModel)));
                }
            }
            ProcessNormals(ifs);
            dimFloatArray = 2;
        }
#ifdef DEBUG_VERBOSE
        printf("FloatArray (%i, %i)\n", numFloatArray, dimFloatArray);
        fprintf(g_fileDebugSC3DMCDec, "FloatArray (%i, %i)\n", numFloatArray, dimFloatArray);
#endif //DEBUG_VERBOSE

        if (m_quantFloatArraySize < size)
        {
            delete [] m_quantFloatArray;
            m_quantFloatArraySize = size;
            m_quantFloatArray     = new long [size];
        }
        for (long v=0; v < nvert; ++v) 
        {
            nPred = 0;
            if ( v2T.GetNumNeighbors(v) > 0 && 
                 predMode != O3DGC_SC3DMC_NO_PREDICTION)
            {
                int u0 = v2T.Begin(v);
                int u1 = v2T.End(v);
                for (long u = u0; u < u1; u++) 
                {
                    long ta = v2T.GetNeighbor(u);
                    if (ta < 0)
                    {
                        break;
                    }
                    if (predMode == O3DGC_SC3DMC_PARALLELOGRAM_PREDICTION)
                    {
                        long a,b;
                        if ((long) triangles[ta*3] == v)
                        {
                            a = triangles[ta*3 + 1];
                            b = triangles[ta*3 + 2];
                        }
                        else if ((long)triangles[ta*3 + 1] == v)
                        {
                            a = triangles[ta*3 + 0];
                            b = triangles[ta*3 + 2];
                        }
                        else
                        {
                            a = triangles[ta*3 + 0];
                            b = triangles[ta*3 + 1];
                        }
                        if ( a < v && b < v)
                        {
                            int u0 = v2T.Begin(a);
                            int u1 = v2T.End(a);
                            for (long u = u0; u < u1; u++) 
                            {
                                long tb = v2T.GetNeighbor(u);
                                if (tb < 0)
                                {
                                    break;
                                }
                                long c = -1;
                                bool foundB = false;
                                for(long k = 0; k < 3; ++k)
                                {
                                    long x = triangles[tb*3 + k];
                                    if (x == b)
                                    {
                                        foundB = true;
                                    }
                                    if (x < v && x != a && x != b)
                                    {
                                        c = x;
                                    }
                                }
                                if (c != -1 && foundB)
                                {
                                    SC3DMCTriplet id = {min(a, b), max(a, b), -c-1};
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
                            if ( w < v )
                            {
                                SC3DMCTriplet id = {-1, -1, w};
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
#ifdef DEBUG_VERBOSE1
                printf("\t\t vm %i\n", v);
                fprintf(g_fileDebugSC3DMCDec, "\t\t vm %i\n", v);
                for (unsigned long p = 0; p < nPred; ++p)
                {
                    printf("\t\t pred a = %i b = %i c = %i \n", m_neighbors[p].m_id.m_a, m_neighbors[p].m_id.m_b, m_neighbors[p].m_id.m_c);
                    fprintf(g_fileDebugSC3DMCDec, "\t\t pred a = %i b = %i c = %i \n", m_neighbors[p].m_id.m_a, m_neighbors[p].m_id.m_b, m_neighbors[p].m_id.m_c);
                    for (unsigned long i = 0; i < dimFloatArray; ++i) 
                    {
                        printf("\t\t\t %i\n", m_neighbors[p].m_pred[i]);
                        fprintf(g_fileDebugSC3DMCDec, "\t\t\t %i\n", m_neighbors[p].m_pred[i]);
                    }
                }
#endif //DEBUG_VERBOSE
                unsigned long bestPred;
                if (m_streamType == O3DGC_STREAM_TYPE_ASCII)
                {
                    bestPred = bstream.ReadUCharASCII(iteratorPred);
                }
                else
                {
                    bestPred = acd.decode(mModelPreds);
                }
#ifdef DEBUG_VERBOSE1
                    printf("best (%i, %i, %i) \t pos %i\n", m_neighbors[bestPred].m_id.m_a, m_neighbors[bestPred].m_id.m_b, m_neighbors[bestPred].m_id.m_c, bestPred);
                    fprintf(g_fileDebugSC3DMCDec, "best (%i, %i, %i) \t pos %i\n", m_neighbors[bestPred].m_id.m_a, m_neighbors[bestPred].m_id.m_b, m_neighbors[bestPred].m_id.m_c, bestPred);
#endif //DEBUG_VERBOSE
                for (unsigned long i = 0; i < dimFloatArray; i++) 
                {
                    if (m_streamType == O3DGC_STREAM_TYPE_ASCII)
                    {
                        predResidual = bstream.ReadIntASCII(m_iterator);
                    }
                    else
                    {
                        predResidual = DecodeIntACEGC(acd, mModelValues, bModel0, bModel1, exp_k, M);
                    }
                    m_quantFloatArray[v*stride+i] = predResidual + m_neighbors[bestPred].m_pred[i];
#ifdef DEBUG_VERBOSE
                    printf("%i \t %i \t [%i]\n", v*dimFloatArray+i, predResidual, m_neighbors[bestPred].m_pred[i]);
                    fprintf(g_fileDebugSC3DMCDec, "%i \t %i \t [%i]\n", v*dimFloatArray+i, predResidual, m_neighbors[bestPred].m_pred[i]);
#endif //DEBUG_VERBOSE
                }
            }
            else if (v > 0 && predMode != O3DGC_SC3DMC_NO_PREDICTION)
            {
                for (unsigned long i = 0; i < dimFloatArray; i++) 
                {
                    if (m_streamType == O3DGC_STREAM_TYPE_ASCII)
                    {
                        predResidual = bstream.ReadIntASCII(m_iterator);
                    }
                    else
                    {
                        predResidual = DecodeIntACEGC(acd, mModelValues, bModel0, bModel1, exp_k, M);
                    }
                    m_quantFloatArray[v*stride+i] = predResidual + m_quantFloatArray[(v-1)*stride+i];
#ifdef DEBUG_VERBOSE
                    printf("%i \t %i\n", v*dimFloatArray+i, predResidual);
                    fprintf(g_fileDebugSC3DMCDec, "%i \t %i\n", v*dimFloatArray+i, predResidual);
#endif //DEBUG_VERBOSE
                }
            }
            else
            {
                for (unsigned long i = 0; i < dimFloatArray; i++) 
                {
                    if (m_streamType == O3DGC_STREAM_TYPE_ASCII)
                    {
                        predResidual = bstream.ReadUIntASCII(m_iterator);
                    }
                    else
                    {
                        predResidual = DecodeUIntACEGC(acd, mModelValues, bModel0, bModel1, exp_k, M);
                    }
                    m_quantFloatArray[v*stride+i] = predResidual;
#ifdef DEBUG_VERBOSE
                    printf("%i \t %i\n", v*dimFloatArray+i, predResidual);
                    fprintf(g_fileDebugSC3DMCDec, "%i \t %i\n", v*dimFloatArray+i, predResidual);
#endif //DEBUG_VERBOSE
                }
            }
        }
        m_iterator  = iteratorPred;
        if (predMode == O3DGC_SC3DMC_SURF_NORMALS_PREDICTION)
        {
            const Real minNormal[2] = {(Real)(-2),(Real)(-2)};
            const Real maxNormal[2] = {(Real)(2),(Real)(2)};
            Real na1, nb1;
            Real na0, nb0;
            char ni1;
            IQuantizeFloatArray(floatArray, numFloatArray, dimFloatArray, stride, minNormal, maxNormal, nQBits+1);
            for (long v=0; v < nvert; ++v) 
            {
                na0 = m_normals[2*v];
                nb0 = m_normals[2*v+1];
                na1 = floatArray[stride*v]   + na0;
                nb1 = floatArray[stride*v+1] + nb0;
                ni1 = m_orientation[v];

                CubeToSphere(na1, nb1, ni1,
                             floatArray[stride*v], 
                             floatArray[stride*v+1], 
                             floatArray[stride*v+2]);

#ifdef DEBUG_VERBOSE1
                printf("normal \t %i \t %f \t %f \t %f \t (%i, %f, %f) \t (%f, %f)\n", 
                                               v, 
                                               floatArray[stride*v], 
                                               floatArray[stride*v+1], 
                                               floatArray[stride*v+2], 
                                               ni1, na1, nb1,
                                               na0, nb0);
                fprintf(g_fileDebugSC3DMCDec, "normal \t %i \t %f \t %f \t %f \t (%i, %f, %f) \t (%f, %f)\n", 
                                               v, 
                                               floatArray[stride*v], 
                                               floatArray[stride*v+1], 
                                               floatArray[stride*v+2], 
                                               ni1, na1, nb1,
                                               na0, nb0);
#endif //DEBUG_VERBOSE
            }
        }
        else
        {
            IQuantizeFloatArray(floatArray, numFloatArray, dimFloatArray, stride, minFloatArray, maxFloatArray, nQBits);
        }
#ifdef DEBUG_VERBOSE
        fflush(g_fileDebugSC3DMCDec);
#endif //DEBUG_VERBOSE
        return O3DGC_OK;
    }
    template<class T>
    O3DGCErrorCode SC3DMCDecoder<T>::IQuantizeFloatArray(Real * const floatArray, 
                                                      unsigned long numFloatArray,
                                                      unsigned long dimFloatArray,
                                                      unsigned long stride,
                                                      const Real * const minFloatArray,
                                                      const Real * const maxFloatArray,
                                                      unsigned long nQBits)
    {
        
        Real idelta[O3DGC_SC3DMC_MAX_DIM_ATTRIBUTES];
        Real r;
        for(unsigned long d = 0; d < dimFloatArray; d++)
        {
            r = maxFloatArray[d] - minFloatArray[d];
            if (r > 0.0f)
            {
                idelta[d] = r/(float)((1 << nQBits) - 1);
            }
            else 
            {
                idelta[d] = 1.0f;
            }
        }        
        for(unsigned long v = 0; v < numFloatArray; ++v)
        {
            for(unsigned long d = 0; d < dimFloatArray; ++d)
            {
//                floatArray[v * stride + d] = m_quantFloatArray[v * stride + d];
                floatArray[v * stride + d] = m_quantFloatArray[v * stride + d] * idelta[d] + minFloatArray[d];
            }
        }
        return O3DGC_OK;
    }
}
#endif // O3DGC_SC3DMC_DECODER_INL


