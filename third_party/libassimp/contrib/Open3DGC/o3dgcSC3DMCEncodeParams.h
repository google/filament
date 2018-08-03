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
#ifndef O3DGC_SC3DMC_ENCODE_PARAMS_H
#define O3DGC_SC3DMC_ENCODE_PARAMS_H

#include "o3dgcCommon.h"

namespace o3dgc
{
    class SC3DMCEncodeParams
    {
    public:
        //! Constructor.
                                    SC3DMCEncodeParams(void)
                                    {
                                        memset(this, 0, sizeof(SC3DMCEncodeParams));
                                        m_encodeMode        = O3DGC_SC3DMC_ENCODE_MODE_TFAN;
                                        m_streamTypeMode    = O3DGC_STREAM_TYPE_ASCII;
                                        m_coordQuantBits    = 14;
                                        m_normalQuantBits   = 8;
                                        m_coordPredMode     = O3DGC_SC3DMC_PARALLELOGRAM_PREDICTION;
                                        m_normalPredMode    = O3DGC_SC3DMC_SURF_NORMALS_PREDICTION;
                                        for(unsigned long a = 0; a < O3DGC_SC3DMC_MAX_NUM_FLOAT_ATTRIBUTES; ++a)
                                        {
                                            m_floatAttributePredMode[a] = O3DGC_SC3DMC_DIFFERENTIAL_PREDICTION;
                                        }
                                        for(unsigned long a = 0; a < O3DGC_SC3DMC_MAX_NUM_INT_ATTRIBUTES; ++a)
                                        {
                                            m_intAttributePredMode[a] = O3DGC_SC3DMC_NO_PREDICTION;
                                        }
                                    };
        //! Destructor.
                                    ~SC3DMCEncodeParams(void) {};

        O3DGCStreamType             GetStreamType()    const { return m_streamTypeMode;}
        O3DGCSC3DMCEncodingMode     GetEncodeMode()    const { return m_encodeMode;}

        unsigned long               GetNumFloatAttributes() const { return m_numFloatAttributes;}
        unsigned long               GetNumIntAttributes()   const { return m_numIntAttributes;}
        unsigned long               GetCoordQuantBits()     const { return m_coordQuantBits;}
        unsigned long               GetNormalQuantBits()    const { return m_normalQuantBits;}
        unsigned long               GetFloatAttributeQuantBits(unsigned long a) const
                                    {
                                       assert(a < O3DGC_SC3DMC_MAX_NUM_FLOAT_ATTRIBUTES);
                                       return m_floatAttributeQuantBits[a];
                                    }
        O3DGCSC3DMCPredictionMode   GetCoordPredMode()    const { return m_coordPredMode; }
        O3DGCSC3DMCPredictionMode   GetNormalPredMode()   const { return m_normalPredMode; }
        O3DGCSC3DMCPredictionMode   GetFloatAttributePredMode(unsigned long a) const
                                    {
                                       assert(a < O3DGC_SC3DMC_MAX_NUM_FLOAT_ATTRIBUTES);
                                       return m_floatAttributePredMode[a];
                                    }
        O3DGCSC3DMCPredictionMode   GetIntAttributePredMode(unsigned long a) const
                                    { 
                                        assert(a < O3DGC_SC3DMC_MAX_NUM_INT_ATTRIBUTES);
                                        return m_intAttributePredMode[a];
                                    }
        O3DGCSC3DMCPredictionMode & GetCoordPredMode()    { return m_coordPredMode; }
        O3DGCSC3DMCPredictionMode & GetNormalPredMode()   { return m_normalPredMode; }
        O3DGCSC3DMCPredictionMode & GetFloatAttributePredMode(unsigned long a)
                                    {
                                       assert(a < O3DGC_SC3DMC_MAX_NUM_FLOAT_ATTRIBUTES);
                                       return m_floatAttributePredMode[a];
                                    }
        O3DGCSC3DMCPredictionMode & GetIntAttributePredMode(unsigned long a)
                                    {
                                        assert(a < O3DGC_SC3DMC_MAX_NUM_INT_ATTRIBUTES);
                                        return m_intAttributePredMode[a];
                                    }
        void                        SetStreamType(O3DGCStreamType streamTypeMode)  { m_streamTypeMode = streamTypeMode;}
        void                        SetEncodeMode(O3DGCSC3DMCEncodingMode encodeMode)  { m_encodeMode = encodeMode;}
        void                        SetNumFloatAttributes(unsigned long numFloatAttributes) 
                                    { 
                                        assert(numFloatAttributes < O3DGC_SC3DMC_MAX_NUM_FLOAT_ATTRIBUTES);
                                        m_numFloatAttributes = numFloatAttributes;
                                    }
        void                        SetNumIntAttributes  (unsigned long numIntAttributes)
                                    { 
                                        assert(numIntAttributes < O3DGC_SC3DMC_MAX_NUM_INT_ATTRIBUTES);
                                        m_numIntAttributes   = numIntAttributes;
                                    }
        void                        SetCoordQuantBits   (unsigned int coordQuantBits   ) { m_coordQuantBits    = coordQuantBits   ; }
        void                        SetNormalQuantBits  (unsigned int normalQuantBits  ) { m_normalQuantBits   = normalQuantBits  ; }
        void                        SetFloatAttributeQuantBits(unsigned long a, unsigned long q) 
                                    { 
                                       assert(a < O3DGC_SC3DMC_MAX_NUM_FLOAT_ATTRIBUTES);
                                       m_floatAttributeQuantBits[a] = q;
                                    }
        void                        SetCoordPredMode   (O3DGCSC3DMCPredictionMode coordPredMode   ) { m_coordPredMode    = coordPredMode   ; }
        void                        SetNormalPredMode  (O3DGCSC3DMCPredictionMode normalPredMode  ) { m_normalPredMode   = normalPredMode  ; }
        void                        SetFloatAttributePredMode(unsigned long a, O3DGCSC3DMCPredictionMode p) 
                                    {
                                       assert(a < O3DGC_SC3DMC_MAX_NUM_FLOAT_ATTRIBUTES);
                                       m_floatAttributePredMode[a] = p;
                                    }                       
        void                        SetIntAttributePredMode(unsigned long a, O3DGCSC3DMCPredictionMode p) 
                                    { 
                                        assert(a < O3DGC_SC3DMC_MAX_NUM_INT_ATTRIBUTES);
                                        m_intAttributePredMode[a] = p;
                                    }
    private:
        unsigned long               m_numFloatAttributes;
        unsigned long               m_numIntAttributes;
        unsigned long               m_coordQuantBits;
        unsigned long               m_normalQuantBits;
        unsigned long               m_floatAttributeQuantBits[O3DGC_SC3DMC_MAX_NUM_FLOAT_ATTRIBUTES];
        
        O3DGCSC3DMCPredictionMode   m_coordPredMode;
        O3DGCSC3DMCPredictionMode   m_normalPredMode; 
        O3DGCSC3DMCPredictionMode   m_floatAttributePredMode[O3DGC_SC3DMC_MAX_NUM_FLOAT_ATTRIBUTES];
        O3DGCSC3DMCPredictionMode   m_intAttributePredMode  [O3DGC_SC3DMC_MAX_NUM_INT_ATTRIBUTES];
        O3DGCStreamType             m_streamTypeMode;
        O3DGCSC3DMCEncodingMode     m_encodeMode;
    };
}
#endif // O3DGC_SC3DMC_ENCODE_PARAMS_H

