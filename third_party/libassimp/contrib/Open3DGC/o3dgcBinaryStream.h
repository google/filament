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
#ifndef O3DGC_BINARY_STREAM_H
#define O3DGC_BINARY_STREAM_H

#include "o3dgcCommon.h"
#include "o3dgcVector.h"

namespace o3dgc
{
    const unsigned long O3DGC_BINARY_STREAM_DEFAULT_SIZE       = 4096;
    const unsigned long O3DGC_BINARY_STREAM_BITS_PER_SYMBOL0   = 7;
    const unsigned long O3DGC_BINARY_STREAM_MAX_SYMBOL0        = (1 << O3DGC_BINARY_STREAM_BITS_PER_SYMBOL0) - 1;
    const unsigned long O3DGC_BINARY_STREAM_BITS_PER_SYMBOL1   = 6;
    const unsigned long O3DGC_BINARY_STREAM_MAX_SYMBOL1        = (1 << O3DGC_BINARY_STREAM_BITS_PER_SYMBOL1) - 1;
    const unsigned long O3DGC_BINARY_STREAM_NUM_SYMBOLS_UINT32 = (32+O3DGC_BINARY_STREAM_BITS_PER_SYMBOL0-1) / 
                                                                 O3DGC_BINARY_STREAM_BITS_PER_SYMBOL0;

    //! 
    class BinaryStream
    {
    public:    
        //! Constructor.
                                BinaryStream(unsigned long size = O3DGC_BINARY_STREAM_DEFAULT_SIZE)
                                {
                                    m_endianness = SystemEndianness();
                                    m_stream.Allocate(size);
                                };
        //! Destructor.
                                ~BinaryStream(void){};

        void                    WriteFloat32(float value, O3DGCStreamType streamType)
                                {
                                    if (streamType == O3DGC_STREAM_TYPE_ASCII)
                                    {
                                        WriteFloat32ASCII(value);
                                    }
                                    else
                                    {
                                        WriteFloat32Bin(value);
                                    }
                                }
        void                    WriteUInt32(unsigned long position, unsigned long value, O3DGCStreamType streamType)
                                {
                                    if (streamType == O3DGC_STREAM_TYPE_ASCII)
                                    {
                                        WriteUInt32ASCII(position, value);
                                    }
                                    else
                                    {
                                        WriteUInt32Bin(position, value);
                                    }
                                }
        void                    WriteUInt32(unsigned long value, O3DGCStreamType streamType)
                                {
                                    if (streamType == O3DGC_STREAM_TYPE_ASCII)
                                    {
                                        WriteUInt32ASCII(value);
                                    }
                                    else
                                    {
                                        WriteUInt32Bin(value);
                                    }
                                }
        void                    WriteUChar(unsigned int position, unsigned char value, O3DGCStreamType streamType)
                                {
                                    if (streamType == O3DGC_STREAM_TYPE_ASCII)
                                    {
                                        WriteUInt32ASCII(position, value);
                                    }
                                    else
                                    {
                                        WriteUInt32Bin(position, value);
                                    }
                                }
        void                    WriteUChar(unsigned char value, O3DGCStreamType streamType)
                                {
                                    if (streamType == O3DGC_STREAM_TYPE_ASCII)
                                    {
                                        WriteUCharASCII(value);
                                    }
                                    else
                                    {
                                        WriteUChar8Bin(value);
                                    }
                                }
        float                   ReadFloat32(unsigned long & position, O3DGCStreamType streamType) const
                                {
                                    float value;
                                    if (streamType == O3DGC_STREAM_TYPE_ASCII)
                                    {
                                        value = ReadFloat32ASCII(position);
                                    }
                                    else
                                    {
                                        value = ReadFloat32Bin(position);
                                    }
                                    return value;
                                }
        unsigned long           ReadUInt32(unsigned long & position, O3DGCStreamType streamType) const
                                {
                                    unsigned long value;
                                    if (streamType == O3DGC_STREAM_TYPE_ASCII)
                                    {
                                        value = ReadUInt32ASCII(position);
                                    }
                                    else
                                    {
                                        value = ReadUInt32Bin(position);
                                    }
                                    return value;
                                }
        unsigned char           ReadUChar(unsigned long & position, O3DGCStreamType streamType) const
                                {
                                    unsigned char value;
                                    if (streamType == O3DGC_STREAM_TYPE_ASCII)
                                    {
                                        value = ReadUCharASCII(position);
                                    }
                                    else
                                    {
                                        value = ReadUChar8Bin(position);
                                    }
                                    return value;
                                }

        void                    WriteFloat32Bin(unsigned long position, float value) 
                                {
                                    assert(position < m_stream.GetSize() - 4);
                                    unsigned char * ptr = (unsigned char *) (&value);
                                    if (m_endianness == O3DGC_BIG_ENDIAN)
                                    {
                                        m_stream[position++] = ptr[3];
                                        m_stream[position++] = ptr[2];
                                        m_stream[position++] = ptr[1];
                                        m_stream[position  ] = ptr[0];
                                    }
                                    else
                                    {
                                        m_stream[position++] = ptr[0];
                                        m_stream[position++] = ptr[1];
                                        m_stream[position++] = ptr[2];
                                        m_stream[position  ] = ptr[3];
                                    }
                                }
        void                    WriteFloat32Bin(float value) 
                                {
                                    unsigned char * ptr = (unsigned char *) (&value);
                                    if (m_endianness == O3DGC_BIG_ENDIAN)
                                    {
                                        m_stream.PushBack(ptr[3]);
                                        m_stream.PushBack(ptr[2]);
                                        m_stream.PushBack(ptr[1]);
                                        m_stream.PushBack(ptr[0]);
                                    }
                                    else
                                    {
                                        m_stream.PushBack(ptr[0]);
                                        m_stream.PushBack(ptr[1]);
                                        m_stream.PushBack(ptr[2]);
                                        m_stream.PushBack(ptr[3]);
                                    }
                                }
        void                    WriteUInt32Bin(unsigned long position, unsigned long value) 
                                {
                                    assert(position < m_stream.GetSize() - 4);
                                    unsigned char * ptr = (unsigned char *) (&value);
                                    if (m_endianness == O3DGC_BIG_ENDIAN)
                                    {
                                        m_stream[position++] = ptr[3];
                                        m_stream[position++] = ptr[2];
                                        m_stream[position++] = ptr[1];
                                        m_stream[position  ] = ptr[0];
                                    }
                                    else
                                    {
                                        m_stream[position++] = ptr[0];
                                        m_stream[position++] = ptr[1];
                                        m_stream[position++] = ptr[2];
                                        m_stream[position  ] = ptr[3];
                                    }
                                }
        void                    WriteUInt32Bin(unsigned long value) 
                                {
                                    unsigned char * ptr = (unsigned char *) (&value);
                                    if (m_endianness == O3DGC_BIG_ENDIAN)
                                    {
                                        m_stream.PushBack(ptr[3]);
                                        m_stream.PushBack(ptr[2]);
                                        m_stream.PushBack(ptr[1]);
                                        m_stream.PushBack(ptr[0]);
                                    }
                                    else
                                    {
                                        m_stream.PushBack(ptr[0]);
                                        m_stream.PushBack(ptr[1]);
                                        m_stream.PushBack(ptr[2]);
                                        m_stream.PushBack(ptr[3]);
                                    }
                                }
        void                    WriteUChar8Bin(unsigned int position, unsigned char value) 
                                {
                                    m_stream[position] = value;
                                }
        void                    WriteUChar8Bin(unsigned char value) 
                                {
                                    m_stream.PushBack(value);
                                }
        float                   ReadFloat32Bin(unsigned long & position) const
                                {
                                    unsigned long value = ReadUInt32Bin(position);
                                    float fvalue;
                                    memcpy(&fvalue, &value, 4);
                                    return fvalue;
                                }
        unsigned long           ReadUInt32Bin(unsigned long & position)  const
                                {
                                    assert(position < m_stream.GetSize() - 4);
                                    unsigned long value = 0;
                                    if (m_endianness == O3DGC_BIG_ENDIAN)
                                    {
                                        value += (m_stream[position++]<<24);
                                        value += (m_stream[position++]<<16);
                                        value += (m_stream[position++]<<8);
                                        value += (m_stream[position++]);
                                    }
                                    else
                                    {
                                        value += (m_stream[position++]);
                                        value += (m_stream[position++]<<8);
                                        value += (m_stream[position++]<<16);
                                        value += (m_stream[position++]<<24);
                                    }
                                    return value;
                                }
        unsigned char           ReadUChar8Bin(unsigned long & position) const
                                {
                                    return m_stream[position++];
                                }

        void                    WriteFloat32ASCII(float value) 
                                {
                                    unsigned long uiValue;
                                    memcpy(&uiValue, &value, 4);
                                    WriteUInt32ASCII(uiValue);
                                }
        void                    WriteUInt32ASCII(unsigned long position, unsigned long value) 
                                {
                                    assert(position < m_stream.GetSize() - O3DGC_BINARY_STREAM_NUM_SYMBOLS_UINT32);
                                    unsigned long value0 = value;
                                    for(unsigned long i = 0; i < O3DGC_BINARY_STREAM_NUM_SYMBOLS_UINT32; ++i)
                                    {
                                        m_stream[position++] = (value0 & O3DGC_BINARY_STREAM_MAX_SYMBOL0);
                                        value0 >>= O3DGC_BINARY_STREAM_BITS_PER_SYMBOL0;
                                    }
                                }
        void                    WriteUInt32ASCII(unsigned long value) 
                                {
                                    for(unsigned long i = 0; i < O3DGC_BINARY_STREAM_NUM_SYMBOLS_UINT32; ++i)
                                    {
                                        m_stream.PushBack(value & O3DGC_BINARY_STREAM_MAX_SYMBOL0);
                                        value >>= O3DGC_BINARY_STREAM_BITS_PER_SYMBOL0;
                                    }
                                }
        void                    WriteIntASCII(long value) 
                                {
                                    WriteUIntASCII(IntToUInt(value));
                                }
        void                    WriteUIntASCII(unsigned long value) 
                                {
                                    if (value >= O3DGC_BINARY_STREAM_MAX_SYMBOL0)
                                    {
                                        m_stream.PushBack(O3DGC_BINARY_STREAM_MAX_SYMBOL0);
                                        value -= O3DGC_BINARY_STREAM_MAX_SYMBOL0;
                                        unsigned char a, b;
                                        do
                                        {
                                            a  = ((value & O3DGC_BINARY_STREAM_MAX_SYMBOL1) << 1);
                                            b  = ( (value >>= O3DGC_BINARY_STREAM_BITS_PER_SYMBOL1) > 0);
                                            a += b;
                                            m_stream.PushBack(a);
                                        } while (b);
                                    }
                                    else
                                    {
                                        m_stream.PushBack((unsigned char) value);
                                    }
                                }
        void                    WriteUCharASCII(unsigned char value) 
                                {
                                    assert(value <= O3DGC_BINARY_STREAM_MAX_SYMBOL0);
                                    m_stream.PushBack(value);
                                }
        float                   ReadFloat32ASCII(unsigned long & position) const
                                {
                                    unsigned long value = ReadUInt32ASCII(position);
                                    float fvalue;
                                    memcpy(&fvalue, &value, 4);
                                    return fvalue;
                                }
        unsigned long           ReadUInt32ASCII(unsigned long & position)  const
                                {
                                    assert(position < m_stream.GetSize() - O3DGC_BINARY_STREAM_NUM_SYMBOLS_UINT32);
                                    unsigned long value = 0;
                                    unsigned long shift = 0;
                                    for(unsigned long i = 0; i < O3DGC_BINARY_STREAM_NUM_SYMBOLS_UINT32; ++i)
                                    {
                                        value  += (m_stream[position++] << shift);
                                        shift  += O3DGC_BINARY_STREAM_BITS_PER_SYMBOL0;
                                    }
                                    return value;
                                }
        long                    ReadIntASCII(unsigned long & position) const
                                {
                                    return UIntToInt(ReadUIntASCII(position));
                                }
        unsigned long           ReadUIntASCII(unsigned long & position) const
                                {
                                    unsigned long value = m_stream[position++];
                                    if (value == O3DGC_BINARY_STREAM_MAX_SYMBOL0)
                                    {
                                        long x;
                                        unsigned long i = 0;
                                        do
                                        {
                                            x = m_stream[position++];
                                            value += ( (x>>1) << i);
                                            i += O3DGC_BINARY_STREAM_BITS_PER_SYMBOL1;
                                        } while (x & 1);
                                    }
                                    return value;
                                }
        unsigned char           ReadUCharASCII(unsigned long & position) const
                                {
                                    return m_stream[position++];
                                }
        O3DGCErrorCode          Save(const char * const fileName) 
                                {
                                    FILE * fout = fopen(fileName, "wb");
                                    if (!fout)
                                    {
                                        return O3DGC_ERROR_CREATE_FILE;
                                    }
                                    fwrite(m_stream.GetBuffer(), 1, m_stream.GetSize(), fout);
                                    fclose(fout);
                                    return O3DGC_OK;
                                }
        O3DGCErrorCode          Load(const char * const fileName) 
                                {
                                    FILE * fin = fopen(fileName, "rb");
                                    if (!fin)
                                    {
                                        return O3DGC_ERROR_OPEN_FILE;
                                    }
                                    fseek(fin, 0, SEEK_END);
                                    unsigned long size = ftell(fin);
                                    m_stream.Allocate(size);
                                    rewind(fin);
                                    unsigned int nread = (unsigned int) fread((void *) m_stream.GetBuffer(), 1, size, fin);
                                    m_stream.SetSize(size);
                                    if (nread != size)
                                    {
                                        return O3DGC_ERROR_READ_FILE;
                                    }
                                    fclose(fin);
                                    return O3DGC_OK;
                                }
        O3DGCErrorCode          LoadFromBuffer(unsigned char * buffer, unsigned long bufferSize)
                                {
                                    m_stream.Allocate(bufferSize);
                                    memcpy(m_stream.GetBuffer(), buffer, bufferSize);
                                    m_stream.SetSize(bufferSize);
                                    return O3DGC_OK;
                                }
        unsigned long           GetSize() const
                                {
                                    return m_stream.GetSize();
                                }
    const unsigned char *       GetBuffer(unsigned long position) const
                                {
                                    return m_stream.GetBuffer() + position;
                                }
    unsigned char *             GetBuffer(unsigned long position)
                                {
                                    return (m_stream.GetBuffer() + position);
                                }                                
    unsigned char *             GetBuffer()
                                {
                                    return m_stream.GetBuffer();
                                }                                
    void                        GetBuffer(unsigned long position, unsigned char * & buffer) const
                                {
                                    buffer = (unsigned char *) (m_stream.GetBuffer() + position); // fix me: ugly!
                                }
    void                        SetSize(unsigned long size)
                                { 
                                    m_stream.SetSize(size);
                                };
    void                        Allocate(unsigned long size)
                                {
                                    m_stream.Allocate(size);
                                }

    private:
        Vector<unsigned char>   m_stream;
        O3DGCEndianness         m_endianness;
    };

}
#endif // O3DGC_BINARY_STREAM_H

