/*
Open Asset Import Library (assimp)
----------------------------------------------------------------------

Copyright (c) 2006-2017, assimp team

All rights reserved.

Redistribution and use of this software in source and binary forms,
with or without modification, are permitted provided that the
following conditions are met:

* Redistributions of source code must retain the above
  copyright notice, this list of conditions and the
  following disclaimer.

* Redistributions in binary form must reproduce the above
  copyright notice, this list of conditions and the
  following disclaimer in the documentation and/or other
  materials provided with the distribution.

* Neither the name of the assimp team, nor the names of its
  contributors may be used to endorse or promote products
  derived from this software without specific prior
  written permission of the assimp team.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

----------------------------------------------------------------------
*/

/** @file  STEPFileEncoding.cpp
 *  @brief STEP character handling, string un-escaping
 */
#include "STEPFileEncoding.h"
#include "fast_atof.h"
#include <contrib/utf8cpp/source/utf8.h>

#include <memory>

using namespace Assimp;

// roman1 to utf16 table
static const uint16_t mac_codetable[] = {
    // 0x20 unassig./nonprint. slots
     0x0020 ,
     0x0021 ,
     0x0022 ,
     0x0023 ,
     0x0024 ,
     0x0025 ,
     0x0026 ,
     0x0027 ,
     0x0028 ,
     0x0029 ,
     0x002A ,
     0x002B ,
     0x002C ,
     0x002D ,
     0x002E ,
     0x002F ,
     0x0030 ,
     0x0031 ,
     0x0032 ,
     0x0033 ,
     0x0034 ,
     0x0035 ,
     0x0036 ,
     0x0037 ,
     0x0038 ,
     0x0039 ,
     0x003A ,
     0x003B ,
     0x003C ,
     0x003D ,
     0x003E ,
     0x003F ,
     0x0040 ,
     0x0041 ,
     0x0042 ,
     0x0043 ,
     0x0044 ,
     0x0045 ,
     0x0046 ,
     0x0047 ,
     0x0048 ,
     0x0049 ,
     0x004A ,
     0x004B ,
     0x004C ,
     0x004D ,
     0x004E ,
     0x004F ,
     0x0050 ,
     0x0051 ,
     0x0052 ,
     0x0053 ,
     0x0054 ,
     0x0055 ,
     0x0056 ,
     0x0057 ,
     0x0058 ,
     0x0059 ,
     0x005A ,
     0x005B ,
     0x005C ,
     0x005D ,
     0x005E ,
     0x005F ,
     0x0060 ,
     0x0061 ,
     0x0062 ,
     0x0063 ,
     0x0064 ,
     0x0065 ,
     0x0066 ,
     0x0067 ,
     0x0068 ,
     0x0069 ,
     0x006A ,
     0x006B ,
     0x006C ,
     0x006D ,
     0x006E ,
     0x006F ,
     0x0070 ,
     0x0071 ,
     0x0072 ,
     0x0073 ,
     0x0074 ,
     0x0075 ,
     0x0076 ,
     0x0077 ,
     0x0078 ,
     0x0079 ,
     0x007A ,
     0x007B ,
     0x007C ,
     0x007D ,
     0x007E ,
     0x0000 , // unassig.
     0x00C4 ,
     0x00C5 ,
     0x00C7 ,
     0x00C9 ,
     0x00D1 ,
     0x00D6 ,
     0x00DC ,
     0x00E1 ,
     0x00E0 ,
     0x00E2 ,
     0x00E4 ,
     0x00E3 ,
     0x00E5 ,
     0x00E7 ,
     0x00E9 ,
     0x00E8 ,
     0x00EA ,
     0x00EB ,
     0x00ED ,
     0x00EC ,
     0x00EE ,
     0x00EF ,
     0x00F1 ,
     0x00F3 ,
     0x00F2 ,
     0x00F4 ,
     0x00F6 ,
     0x00F5 ,
     0x00FA ,
     0x00F9 ,
     0x00FB ,
     0x00FC ,
     0x2020 ,
     0x00B0 ,
     0x00A2 ,
     0x00A3 ,
     0x00A7 ,
     0x2022 ,
     0x00B6 ,
     0x00DF ,
     0x00AE ,
     0x00A9 ,
     0x2122 ,
     0x00B4 ,
     0x00A8 ,
     0x2260 ,
     0x00C6 ,
     0x00D8 ,
     0x221E ,
     0x00B1 ,
     0x2264 ,
     0x2265 ,
     0x00A5 ,
     0x00B5 ,
     0x2202 ,
     0x2211 ,
     0x220F ,
     0x03C0 ,
     0x222B ,
     0x00AA ,
     0x00BA ,
     0x03A9 ,
     0x00E6 ,
     0x00F8 ,
     0x00BF ,
     0x00A1 ,
     0x00AC ,
     0x221A ,
     0x0192 ,
     0x2248 ,
     0x2206 ,
     0x00AB ,
     0x00BB ,
     0x2026 ,
     0x00A0 ,
     0x00C0 ,
     0x00C3 ,
     0x00D5 ,
     0x0152 ,
     0x0153 ,
     0x2013 ,
     0x2014 ,
     0x201C ,
     0x201D ,
     0x2018 ,
     0x2019 ,
     0x00F7 ,
     0x25CA ,
     0x00FF ,
     0x0178 ,
     0x2044 ,
     0x20AC ,
     0x2039 ,
     0x203A ,
     0xFB01 ,
     0xFB02 ,
     0x2021 ,
     0x00B7 ,
     0x201A ,
     0x201E ,
     0x2030 ,
     0x00C2 ,
     0x00CA ,
     0x00C1 ,
     0x00CB ,
     0x00C8 ,
     0x00CD ,
     0x00CE ,
     0x00CF ,
     0x00CC ,
     0x00D3 ,
     0x00D4 ,
     0xF8FF ,
     0x00D2 ,
     0x00DA ,
     0x00DB ,
     0x00D9 ,
     0x0131 ,
     0x02C6 ,
     0x02DC ,
     0x00AF ,
     0x02D8 ,
     0x02D9 ,
     0x02DA ,
     0x00B8 ,
     0x02DD ,
     0x02DB ,
     0x02C7
};

// ------------------------------------------------------------------------------------------------
bool STEP::StringToUTF8(std::string& s)
{
    // very basic handling for escaped string sequences
    // http://doc.spatial.com/index.php?title=InterOp:Connect/STEP&redirect=no

    for (size_t i = 0; i < s.size(); ) {
        if (s[i] == '\\') {
            // \S\X - cp1252 (X is the character remapped to [0,127])
            if (i+3 < s.size() && s[i+1] == 'S' && s[i+2] == '\\') {
                // http://stackoverflow.com/questions/5586214/how-to-convert-char-from-iso-8859-1-to-utf-8-in-c-multiplatformly
                ai_assert((uint8_t)s[i+3] < 0x80);
                const uint8_t ch = s[i+3] + 0x80;

                s[i] = 0xc0 | (ch & 0xc0) >> 6;
                s[i+1] =  0x80 | (ch & 0x3f);

                s.erase(i + 2,2);
                ++i;
            }
            // \X\xx - mac/roman (xx is a hex sequence)
            else if (i+4 < s.size() && s[i+1] == 'X' && s[i+2] == '\\') {

                const uint8_t macval = HexOctetToDecimal(s.c_str() + i + 3);
                if(macval < 0x20) {
                    return false;
                }

                ai_assert(sizeof(mac_codetable) / sizeof(mac_codetable[0]) == 0x100-0x20);

                const uint32_t unival = mac_codetable[macval - 0x20], *univalp = &unival;

                unsigned char temp[5], *tempp = temp;
                ai_assert(sizeof( unsigned char ) == 1);

                utf8::utf32to8( univalp, univalp + 1, tempp );

                const size_t outcount = static_cast<size_t>(tempp-temp);

                s.erase(i,5);
                s.insert(i, reinterpret_cast<char*>(temp), outcount);
                i += outcount;
            }
            // \Xn\ .. \X0\ - various unicode encodings (n=2: utf16; n=4: utf32)
            else if (i+3 < s.size() && s[i+1] == 'X' && s[i+2] >= '0' && s[i+2] <= '9') {
                switch(s[i+2]) {
                    // utf16
                case '2':
                    // utf32
                case '4':
                    if (s[i+3] == '\\') {
                        const size_t basei = i+4;
                        size_t j = basei, jend = s.size()-3;

                        for (; j < jend; ++j) {
                            if (s[j] == '\\' && s[j+1] == 'X' && s[j+2] == '0' && s[j+3] == '\\') {
                                break;
                            }
                        }
                        if (j == jend) {
                            return false;
                        }

                        if (j == basei) {
                            s.erase(i,8);
                            continue;
                        }

                        if (s[i+2] == '2') {
                            if (((j - basei) % 4) != 0) {
                                return false;
                            }

                            const size_t count = (j-basei)/4;
                            std::unique_ptr<uint16_t[]> src(new uint16_t[count]);

                            const char* cur = s.c_str() + basei;
                            for (size_t k = 0; k < count; ++k, cur += 4) {
                                src[k] = (static_cast<uint16_t>(HexOctetToDecimal(cur)) << 8u)  |
                                     static_cast<uint16_t>(HexOctetToDecimal(cur+2));
                            }

                            const size_t dcount = count * 3; // this is enough to hold all possible outputs
                            std::unique_ptr<unsigned char[]> dest(new unsigned char[dcount]);

                            const uint16_t* srct = src.get();
                            unsigned char* destt = dest.get();
                            utf8::utf16to8( srct, srct + count, destt );

                            const size_t outcount = static_cast<size_t>(destt-dest.get());

                            s.erase(i,(j+4-i));

                            ai_assert(sizeof(unsigned char) == 1);
                            s.insert(i, reinterpret_cast<char*>(dest.get()), outcount);

                            i += outcount;
                            continue;
                        }
                        else if (s[i+2] == '4') {
                            if (((j - basei) % 8) != 0) {
                                return false;
                            }

                            const size_t count = (j-basei)/8;
                            std::unique_ptr<uint32_t[]> src(new uint32_t[count]);

                            const char* cur = s.c_str() + basei;
                            for (size_t k = 0; k < count; ++k, cur += 8) {
                                src[k] = (static_cast<uint32_t>(HexOctetToDecimal(cur  )) << 24u) |
                                         (static_cast<uint32_t>(HexOctetToDecimal(cur+2)) << 16u) |
                                         (static_cast<uint32_t>(HexOctetToDecimal(cur+4)) << 8u)  |
                                         (static_cast<uint32_t>(HexOctetToDecimal(cur+6)));
                            }

                            const size_t dcount = count * 5; // this is enough to hold all possible outputs
                            std::unique_ptr<unsigned char[]> dest(new unsigned char[dcount]);

                            const uint32_t* srct = src.get();
                            unsigned char* destt = dest.get();
                            utf8::utf32to8( srct, srct + count, destt );

                            const size_t outcount = static_cast<size_t>(destt-dest.get());

                            s.erase(i,(j+4-i));

                            ai_assert(sizeof(unsigned char) == 1);
                            s.insert(i, reinterpret_cast<char*>(dest.get()), outcount);

                            i += outcount;
                            continue;
                        }
                    }
                    break;

                    // TODO: other encoding patterns?

                default:
                    return false;
                }
            }
        }
        ++i;
    }
    return true;
}
