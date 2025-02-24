/* Copyright (c) 2020-2022 Hans-Kristian Arntzen
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
 const ivec2 etc1_color_modifier_table[8] = ivec2[](
    ivec2(2, 8),
    ivec2(5, 17),
    ivec2(9, 29),
    ivec2(13, 42),
    ivec2(18, 60),
    ivec2(24, 80),
    ivec2(33, 106),
    ivec2(47, 183));

const ivec4 etc2_alpha_modifier_table[16] = ivec4[](
    ivec4(2, 5, 8, 14),
    ivec4(2, 6, 9, 12),
    ivec4(1, 4, 7, 12),
    ivec4(1, 3, 5, 12),
    ivec4(2, 5, 7, 11),
    ivec4(2, 6, 8, 10),
    ivec4(3, 6, 7, 10),
    ivec4(2, 4, 7, 10),
    ivec4(1, 5, 7, 9),
    ivec4(1, 4, 7, 9),
    ivec4(1, 3, 7, 9),
    ivec4(1, 4, 6, 9),
    ivec4(2, 3, 6, 9),
    ivec4(0, 1, 2, 9),
    ivec4(3, 5, 7, 8),
    ivec4(2, 4, 6, 8)
);

const int etc2_distance_table[8] = int[](3, 6, 11, 16, 23, 32, 41, 64);

int decode_etc2_alpha(uvec2 payload, int linear_pixel)
{
    int bit_offset = 45 - 3 * linear_pixel;
#if R11
    int base = isSigned !=0 ? bitfieldExtract(int(payload.y), 24, 8) : int(bitfieldExtract(payload.y, 24, 8));
#else
    int base = int(bitfieldExtract(payload.y, 24, 8));
#endif
    int multiplier = int(bitfieldExtract(payload.y, 20, 4));
    int table = int(bitfieldExtract(payload.y, 16, 4));

    int lsb_index = int(bitfieldExtract(payload[bit_offset >> 5], bit_offset & 31, 2));
    bit_offset += 2;
    int msb = int((payload[bit_offset >> 5] >> (bit_offset & 31)) & 1u);
    int mod = etc2_alpha_modifier_table[table][lsb_index] ^ (msb - 1);
#if R11
    int a = base * 8 + 4;
    a += multiplier != 0 ? mod * multiplier * 8 : mod;
    int minValue = isSigned !=0 ? -1023: 0;
    int maxValue = isSigned !=0 ? 1023 : 2047;
    a = clamp(a, minValue, maxValue);
    float scale = isSigned !=0 ? 127.0f : 255.0f;
    return int(a/float(maxValue) * scale + 0.5f);
#else
    int a = base + mod * multiplier;
    return clamp(a, 0, 0xff);
#endif
}

 ivec4 DecodeRGB(ivec2 pixel_coord, uvec2 color_payload, int linear_pixel, inout bool punchthrough) {
    int alpha_result = 0xff;
    ivec3 rgb_result;
    ivec3 base_rgb;
    uint flip = color_payload.y & 1u;
    uint subblock = uint((pixel_coord[flip] & 2) >> 1);
    bool etc1_compat = false;

    if (alphaBits != 1 && (color_payload.y & 2u) == 0u)
    {
        // Individual mode (ETC1)
        etc1_compat = true;
        base_rgb = ivec3(color_payload.yyy >> (uvec3(28, 20, 12) - 4u * subblock));
        base_rgb &= 0xf;
        base_rgb *= 0x11;
    }
    else
    {
        int r = int(bitfieldExtract(color_payload.y, 27, 5));
        int rd = bitfieldExtract(int(color_payload.y), 24, 3);
        int g = int(bitfieldExtract(color_payload.y, 19, 5));
        int gd = bitfieldExtract(int(color_payload.y), 16, 3);
        int b = int(bitfieldExtract(color_payload.y, 11, 5));
        int bd = bitfieldExtract(int(color_payload.y), 8, 3);

        int r1 = r + rd;
        int g1 = g + gd;
        int b1 = b + bd;

        if (uint(r1) > 31u) // T mode
        {
            int r1 = int(bitfieldExtract(color_payload.y, 56 - 32, 2)) |
                    (int(bitfieldExtract(color_payload.y, 59 - 32, 2)) << 2);
            int g1 = int(bitfieldExtract(color_payload.y, 52 - 32, 4));
            int b1 = int(bitfieldExtract(color_payload.y, 48 - 32, 4));
            int r2 = int(bitfieldExtract(color_payload.y, 44 - 32, 4));
            int g2 = int(bitfieldExtract(color_payload.y, 40 - 32, 4));
            int b2 = int(bitfieldExtract(color_payload.y, 36 - 32, 4));
            uint da = (bitfieldExtract(color_payload.y, 34 - 32, 2) << 1) |
                    (color_payload.y & 1u);
            int dist = etc2_distance_table[da];

            int msb = int((color_payload.x >> (15 + linear_pixel)) & 2u);
            int lsb = int((color_payload.x >> linear_pixel) & 1u);
            int index = msb | lsb;

            if (punchthrough)
                punchthrough = index == 2;

            if (index == 0)
            {
                rgb_result = ivec3(r1, g1, b1);
                rgb_result *= 0x11;
            }
            else
            {
                int mod = 2 - index;
                ivec3 rgb = ivec3(r2, g2, b2) * 0x11 + mod * dist;
                rgb_result = ivec3(clamp(rgb, ivec3(0), ivec3(255)));
            }
        }
        else if (uint(g1) > 31u) // H mode
        {
            int r1 = int(bitfieldExtract(color_payload.y, 59 - 32, 4));
            int g1 = (int(bitfieldExtract(color_payload.y, 56 - 32, 3)) << 1) |
                    int((color_payload.y >> 20u) & 1u);
            int b1 = int(bitfieldExtract(color_payload.y, 47 - 32, 3)) |
                    int((color_payload.y >> 16u) & 8u);
            int r2 = int(bitfieldExtract(color_payload.y, 43 - 32, 4));
            int g2 = int(bitfieldExtract(color_payload.y, 39 - 32, 4));
            int b2 = int(bitfieldExtract(color_payload.y, 35 - 32, 4));
            uint da = color_payload.y & 4u;
            uint db = color_payload.y & 1u;
            uint d = da + 2u * db;
            d += uint((r1 * 0x10000 + g1 * 0x100 + b1) >= (r2 * 0x10000 + g2 * 0x100 + b2));
            int dist = etc2_distance_table[d];
            int msb = int((color_payload.x >> (15 + linear_pixel)) & 2u);
            int lsb = int((color_payload.x >> linear_pixel) & 1u);

            if (punchthrough)
                punchthrough = (msb + lsb) == 2;

            ivec3 base = msb != 0 ? ivec3(r2, g2, b2) : ivec3(r1, g1, b1);
            base *= 0x11;
            int mod = 1 - 2 * lsb;
            base += mod * dist;
            rgb_result = ivec3(clamp(base, ivec3(0), ivec3(0xff)));
        }
        else if (uint(b1) > 31u) // plane mode
        {
            // Planar mode
            int r = int(bitfieldExtract(color_payload.y, 57 - 32, 6));
            int g = int(bitfieldExtract(color_payload.y, 49 - 32, 6)) |
                    (int(color_payload.y >> 18) & 0x40);
            int b = int(bitfieldExtract(color_payload.y, 39 - 32, 3)) |
                    (int(bitfieldExtract(color_payload.y, 43 - 32, 2)) << 3) |
                    (int(color_payload.y >> 11) & 0x20);
            int rh = int(color_payload.y & 1u) |
                    (int(bitfieldExtract(color_payload.y, 2, 5)) << 1);
            int rv = int(bitfieldExtract(color_payload.x, 13, 6));
            int gh = int(bitfieldExtract(color_payload.x, 25, 7));
            int gv = int(bitfieldExtract(color_payload.x, 6, 7));
            int bh = int(bitfieldExtract(color_payload.x, 19, 6));
            int bv = int(bitfieldExtract(color_payload.x, 0, 6));

            r = (r << 2) | (r >> 4);
            rh = (rh << 2) | (rh >> 4);
            rv = (rv << 2) | (rv >> 4);
            g = (g << 1) | (g >> 6);
            gh = (gh << 1) | (gh >> 6);
            gv = (gv << 1) | (gv >> 6);
            b = (b << 2) | (b >> 4);
            bh = (bh << 2) | (bh >> 4);
            bv = (bv << 2) | (bv >> 4);

            ivec3 rgb = ivec3(r, g, b);
            ivec3 dx = ivec3(rh, gh, bh) - rgb;
            ivec3 dy = ivec3(rv, gv, bv) - rgb;
            dx *= int(pixel_coord.x);
            dy *= int(pixel_coord.y);
            rgb = rgb + ((dx + dy + 2) >> 2);
            rgb = clamp(rgb, ivec3(0), ivec3(255));
            rgb_result = ivec3(rgb);

            punchthrough = false;

        }
        else // diff mode
        {
            // Differential mode (ETC1)
            etc1_compat = true;
            base_rgb = ivec3(r, g, b) + int(subblock) * ivec3(rd, gd, bd);
            base_rgb = (base_rgb << 3) | (base_rgb >> 2);
        }
    }

    if (etc1_compat)
    {
        uint etc1_table_index = bitfieldExtract(color_payload.y, 5 - 3 * int(subblock != 0u), 3);
        int msb = int((color_payload.x >> (15 + linear_pixel)) & 2u);
        int lsb = int((color_payload.x >> linear_pixel) & 1u);
        int sgn = 1 - msb;

        if (punchthrough)
        {
            sgn *= lsb;
            punchthrough = (msb + lsb) == 2;
        }

        int offset = etc1_color_modifier_table[etc1_table_index][lsb] * sgn;
        base_rgb = clamp(base_rgb + offset, ivec3(0), ivec3(255));
        rgb_result = ivec3(base_rgb);
    }

    if (alphaBits == 1 && punchthrough)
    {
        rgb_result = ivec3(0);
        alpha_result = 0;
    }

    return ivec4(rgb_result.r, rgb_result.g, rgb_result.b, alpha_result);
}