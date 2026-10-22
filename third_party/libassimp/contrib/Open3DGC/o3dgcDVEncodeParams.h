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
#ifndef O3DGC_DV_ENCODE_PARAMS_H
#define O3DGC_DV_ENCODE_PARAMS_H

#include "o3dgcCommon.h"

namespace o3dgc
{
    class DVEncodeParams
    {
    public:
        //! Constructor.
                                    DVEncodeParams(void)
                                    {
                                        m_quantBits         = 10;
                                        m_streamTypeMode    = O3DGC_STREAM_TYPE_ASCII;
                                        m_encodeMode        = O3DGC_DYNAMIC_VECTOR_ENCODE_MODE_LIFT;
                                    };
        //! Destructor.
                                    ~DVEncodeParams(void) {};

        unsigned long               GetQuantBits()     const { return m_quantBits;}
        O3DGCStreamType             GetStreamType()    const { return m_streamTypeMode;}
        O3DGCDVEncodingMode         GetEncodeMode()    const { return m_encodeMode;}

        void                        SetQuantBits   (unsigned long quantBits  ) { m_quantBits = quantBits;}

        void                        SetStreamType(O3DGCStreamType     streamTypeMode) { m_streamTypeMode = streamTypeMode;}
        void                        SetEncodeMode(O3DGCDVEncodingMode encodeMode    ) { m_encodeMode     = encodeMode    ;}


    private:
        unsigned long               m_quantBits;
        O3DGCStreamType             m_streamTypeMode;
        O3DGCDVEncodingMode         m_encodeMode;
    };
}
#endif // O3DGC_DV_ENCODE_PARAMS_H

