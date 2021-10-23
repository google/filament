/*
  Simple DirectMedia Layer
  Copyright (C) 1997-2021 Sam Lantinga <slouken@libsdl.org>

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.
*/

#if defined(__clang_analyzer__) && !defined(SDL_DISABLE_ANALYZE_MACROS)
#define SDL_DISABLE_ANALYZE_MACROS 1
#endif

#include "../SDL_internal.h"

/* This file contains portable iconv functions for SDL */

#include "SDL_stdinc.h"
#include "SDL_endian.h"

#if defined(HAVE_ICONV) && defined(HAVE_ICONV_H)
#ifdef __FreeBSD__
/* Define LIBICONV_PLUG to use iconv from the base instead of ports and avoid linker errors. */
#define LIBICONV_PLUG 1
#endif
#include <iconv.h>

/* Depending on which standard the iconv() was implemented with,
   iconv() may or may not use const char ** for the inbuf param.
   If we get this wrong, it's just a warning, so no big deal.
*/
#if defined(_XGP6) || defined(__APPLE__) || defined(__RISCOS__) || defined(__FREEBSD__) || \
    defined(__EMSCRIPTEN__) || \
    (defined(__GLIBC__) && ((__GLIBC__ > 2) || (__GLIBC__ == 2 && __GLIBC_MINOR__ >= 2)) || \
    (defined(_NEWLIB_VERSION)))
#define ICONV_INBUF_NONCONST
#endif

#include <errno.h>

SDL_COMPILE_TIME_ASSERT(iconv_t, sizeof (iconv_t) <= sizeof (SDL_iconv_t));

SDL_iconv_t
SDL_iconv_open(const char *tocode, const char *fromcode)
{
    return (SDL_iconv_t) ((uintptr_t) iconv_open(tocode, fromcode));
}

int
SDL_iconv_close(SDL_iconv_t cd)
{
    return iconv_close((iconv_t) ((uintptr_t) cd));
}

size_t
SDL_iconv(SDL_iconv_t cd,
          const char **inbuf, size_t * inbytesleft,
          char **outbuf, size_t * outbytesleft)
{
    size_t retCode;
#ifdef ICONV_INBUF_NONCONST
    retCode = iconv((iconv_t) ((uintptr_t) cd), (char **) inbuf, inbytesleft, outbuf, outbytesleft);
#else
    retCode = iconv((iconv_t) ((uintptr_t) cd), inbuf, inbytesleft, outbuf, outbytesleft);
#endif
    if (retCode == (size_t) - 1) {
        switch (errno) {
        case E2BIG:
            return SDL_ICONV_E2BIG;
        case EILSEQ:
            return SDL_ICONV_EILSEQ;
        case EINVAL:
            return SDL_ICONV_EINVAL;
        default:
            return SDL_ICONV_ERROR;
        }
    }
    return retCode;
}

#else

/* Lots of useful information on Unicode at:
    http://www.cl.cam.ac.uk/~mgk25/unicode.html
*/

#define UNICODE_BOM    0xFEFF

#define UNKNOWN_ASCII    '?'
#define UNKNOWN_UNICODE    0xFFFD

enum
{
    ENCODING_UNKNOWN,
    ENCODING_ASCII,
    ENCODING_LATIN1,
    ENCODING_UTF8,
    ENCODING_UTF16,             /* Needs byte order marker */
    ENCODING_UTF16BE,
    ENCODING_UTF16LE,
    ENCODING_UTF32,             /* Needs byte order marker */
    ENCODING_UTF32BE,
    ENCODING_UTF32LE,
    ENCODING_UCS2BE,
    ENCODING_UCS2LE,
    ENCODING_UCS4BE,
    ENCODING_UCS4LE,
};
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
#define ENCODING_UTF16NATIVE    ENCODING_UTF16BE
#define ENCODING_UTF32NATIVE    ENCODING_UTF32BE
#define ENCODING_UCS2NATIVE     ENCODING_UCS2BE
#define ENCODING_UCS4NATIVE     ENCODING_UCS4BE
#else
#define ENCODING_UTF16NATIVE    ENCODING_UTF16LE
#define ENCODING_UTF32NATIVE    ENCODING_UTF32LE
#define ENCODING_UCS2NATIVE     ENCODING_UCS2LE
#define ENCODING_UCS4NATIVE     ENCODING_UCS4LE
#endif

struct _SDL_iconv_t
{
    int src_fmt;
    int dst_fmt;
};

static struct
{
    const char *name;
    int format;
} encodings[] = {
/* *INDENT-OFF* */
    { "ASCII", ENCODING_ASCII },
    { "US-ASCII", ENCODING_ASCII },
    { "8859-1", ENCODING_LATIN1 },
    { "ISO-8859-1", ENCODING_LATIN1 },
    { "UTF8", ENCODING_UTF8 },
    { "UTF-8", ENCODING_UTF8 },
    { "UTF16", ENCODING_UTF16 },
    { "UTF-16", ENCODING_UTF16 },
    { "UTF16BE", ENCODING_UTF16BE },
    { "UTF-16BE", ENCODING_UTF16BE },
    { "UTF16LE", ENCODING_UTF16LE },
    { "UTF-16LE", ENCODING_UTF16LE },
    { "UTF32", ENCODING_UTF32 },
    { "UTF-32", ENCODING_UTF32 },
    { "UTF32BE", ENCODING_UTF32BE },
    { "UTF-32BE", ENCODING_UTF32BE },
    { "UTF32LE", ENCODING_UTF32LE },
    { "UTF-32LE", ENCODING_UTF32LE },
    { "UCS2", ENCODING_UCS2BE },
    { "UCS-2", ENCODING_UCS2BE },
    { "UCS-2LE", ENCODING_UCS2LE },
    { "UCS-2BE", ENCODING_UCS2BE },
    { "UCS-2-INTERNAL", ENCODING_UCS2NATIVE },
    { "UCS4", ENCODING_UCS4BE },
    { "UCS-4", ENCODING_UCS4BE },
    { "UCS-4LE", ENCODING_UCS4LE },
    { "UCS-4BE", ENCODING_UCS4BE },
    { "UCS-4-INTERNAL", ENCODING_UCS4NATIVE },
/* *INDENT-ON* */
};

static const char *
getlocale(char *buffer, size_t bufsize)
{
    const char *lang;
    char *ptr;

    lang = SDL_getenv("LC_ALL");
    if (!lang) {
        lang = SDL_getenv("LC_CTYPE");
    }
    if (!lang) {
        lang = SDL_getenv("LC_MESSAGES");
    }
    if (!lang) {
        lang = SDL_getenv("LANG");
    }
    if (!lang || !*lang || SDL_strcmp(lang, "C") == 0) {
        lang = "ASCII";
    }

    /* We need to trim down strings like "en_US.UTF-8@blah" to "UTF-8" */
    ptr = SDL_strchr(lang, '.');
    if (ptr != NULL) {
        lang = ptr + 1;
    }

    SDL_strlcpy(buffer, lang, bufsize);
    ptr = SDL_strchr(buffer, '@');
    if (ptr != NULL) {
        *ptr = '\0';            /* chop end of string. */
    }

    return buffer;
}

SDL_iconv_t
SDL_iconv_open(const char *tocode, const char *fromcode)
{
    int src_fmt = ENCODING_UNKNOWN;
    int dst_fmt = ENCODING_UNKNOWN;
    int i;
    char fromcode_buffer[64];
    char tocode_buffer[64];

    if (!fromcode || !*fromcode) {
        fromcode = getlocale(fromcode_buffer, sizeof(fromcode_buffer));
    }
    if (!tocode || !*tocode) {
        tocode = getlocale(tocode_buffer, sizeof(tocode_buffer));
    }
    for (i = 0; i < SDL_arraysize(encodings); ++i) {
        if (SDL_strcasecmp(fromcode, encodings[i].name) == 0) {
            src_fmt = encodings[i].format;
            if (dst_fmt != ENCODING_UNKNOWN) {
                break;
            }
        }
        if (SDL_strcasecmp(tocode, encodings[i].name) == 0) {
            dst_fmt = encodings[i].format;
            if (src_fmt != ENCODING_UNKNOWN) {
                break;
            }
        }
    }
    if (src_fmt != ENCODING_UNKNOWN && dst_fmt != ENCODING_UNKNOWN) {
        SDL_iconv_t cd = (SDL_iconv_t) SDL_malloc(sizeof(*cd));
        if (cd) {
            cd->src_fmt = src_fmt;
            cd->dst_fmt = dst_fmt;
            return cd;
        }
    }
    return (SDL_iconv_t) - 1;
}

size_t
SDL_iconv(SDL_iconv_t cd,
          const char **inbuf, size_t * inbytesleft,
          char **outbuf, size_t * outbytesleft)
{
    /* For simplicity, we'll convert everything to and from UCS-4 */
    const char *src;
    char *dst;
    size_t srclen, dstlen;
    Uint32 ch = 0;
    size_t total;

    if (!inbuf || !*inbuf) {
        /* Reset the context */
        return 0;
    }
    if (!outbuf || !*outbuf || !outbytesleft || !*outbytesleft) {
        return SDL_ICONV_E2BIG;
    }
    src = *inbuf;
    srclen = (inbytesleft ? *inbytesleft : 0);
    dst = *outbuf;
    dstlen = *outbytesleft;

    switch (cd->src_fmt) {
    case ENCODING_UTF16:
        /* Scan for a byte order marker */
        {
            Uint8 *p = (Uint8 *) src;
            size_t n = srclen / 2;
            while (n) {
                if (p[0] == 0xFF && p[1] == 0xFE) {
                    cd->src_fmt = ENCODING_UTF16BE;
                    break;
                } else if (p[0] == 0xFE && p[1] == 0xFF) {
                    cd->src_fmt = ENCODING_UTF16LE;
                    break;
                }
                p += 2;
                --n;
            }
            if (n == 0) {
                /* We can't tell, default to host order */
                cd->src_fmt = ENCODING_UTF16NATIVE;
            }
        }
        break;
    case ENCODING_UTF32:
        /* Scan for a byte order marker */
        {
            Uint8 *p = (Uint8 *) src;
            size_t n = srclen / 4;
            while (n) {
                if (p[0] == 0xFF && p[1] == 0xFE &&
                    p[2] == 0x00 && p[3] == 0x00) {
                    cd->src_fmt = ENCODING_UTF32BE;
                    break;
                } else if (p[0] == 0x00 && p[1] == 0x00 &&
                           p[2] == 0xFE && p[3] == 0xFF) {
                    cd->src_fmt = ENCODING_UTF32LE;
                    break;
                }
                p += 4;
                --n;
            }
            if (n == 0) {
                /* We can't tell, default to host order */
                cd->src_fmt = ENCODING_UTF32NATIVE;
            }
        }
        break;
    }

    switch (cd->dst_fmt) {
    case ENCODING_UTF16:
        /* Default to host order, need to add byte order marker */
        if (dstlen < 2) {
            return SDL_ICONV_E2BIG;
        }
        *(Uint16 *) dst = UNICODE_BOM;
        dst += 2;
        dstlen -= 2;
        cd->dst_fmt = ENCODING_UTF16NATIVE;
        break;
    case ENCODING_UTF32:
        /* Default to host order, need to add byte order marker */
        if (dstlen < 4) {
            return SDL_ICONV_E2BIG;
        }
        *(Uint32 *) dst = UNICODE_BOM;
        dst += 4;
        dstlen -= 4;
        cd->dst_fmt = ENCODING_UTF32NATIVE;
        break;
    }

    total = 0;
    while (srclen > 0) {
        /* Decode a character */
        switch (cd->src_fmt) {
        case ENCODING_ASCII:
            {
                Uint8 *p = (Uint8 *) src;
                ch = (Uint32) (p[0] & 0x7F);
                ++src;
                --srclen;
            }
            break;
        case ENCODING_LATIN1:
            {
                Uint8 *p = (Uint8 *) src;
                ch = (Uint32) p[0];
                ++src;
                --srclen;
            }
            break;
        case ENCODING_UTF8:    /* RFC 3629 */
            {
                Uint8 *p = (Uint8 *) src;
                size_t left = 0;
                SDL_bool overlong = SDL_FALSE;
                if (p[0] >= 0xF0) {
                    if ((p[0] & 0xF8) != 0xF0) {
                        /* Skip illegal sequences
                           return SDL_ICONV_EILSEQ;
                         */
                        ch = UNKNOWN_UNICODE;
                    } else {
                        if (p[0] == 0xF0 && srclen > 1 && (p[1] & 0xF0) == 0x80) {
                            overlong = SDL_TRUE;
                        }
                        ch = (Uint32) (p[0] & 0x07);
                        left = 3;
                    }
                } else if (p[0] >= 0xE0) {
                    if ((p[0] & 0xF0) != 0xE0) {
                        /* Skip illegal sequences
                           return SDL_ICONV_EILSEQ;
                         */
                        ch = UNKNOWN_UNICODE;
                    } else {
                        if (p[0] == 0xE0 && srclen > 1 && (p[1] & 0xE0) == 0x80) {
                            overlong = SDL_TRUE;
                        }
                        ch = (Uint32) (p[0] & 0x0F);
                        left = 2;
                    }
                } else if (p[0] >= 0xC0) {
                    if ((p[0] & 0xE0) != 0xC0) {
                        /* Skip illegal sequences
                           return SDL_ICONV_EILSEQ;
                         */
                        ch = UNKNOWN_UNICODE;
                    } else {
                        if ((p[0] & 0xDE) == 0xC0) {
                            overlong = SDL_TRUE;
                        }
                        ch = (Uint32) (p[0] & 0x1F);
                        left = 1;
                    }
                } else {
                    if ((p[0] & 0x80) != 0x00) {
                        /* Skip illegal sequences
                           return SDL_ICONV_EILSEQ;
                         */
                        ch = UNKNOWN_UNICODE;
                    } else {
                        ch = (Uint32) p[0];
                    }
                }
                ++src;
                --srclen;
                if (srclen < left) {
                    return SDL_ICONV_EINVAL;
                }
                while (left--) {
                    ++p;
                    if ((p[0] & 0xC0) != 0x80) {
                        /* Skip illegal sequences
                           return SDL_ICONV_EILSEQ;
                         */
                        ch = UNKNOWN_UNICODE;
                        break;
                    }
                    ch <<= 6;
                    ch |= (p[0] & 0x3F);
                    ++src;
                    --srclen;
                }
                if (overlong) {
                    /* Potential security risk
                       return SDL_ICONV_EILSEQ;
                     */
                    ch = UNKNOWN_UNICODE;
                }
                if ((ch >= 0xD800 && ch <= 0xDFFF) ||
                    (ch == 0xFFFE || ch == 0xFFFF) || ch > 0x10FFFF) {
                    /* Skip illegal sequences
                       return SDL_ICONV_EILSEQ;
                     */
                    ch = UNKNOWN_UNICODE;
                }
            }
            break;
        case ENCODING_UTF16BE: /* RFC 2781 */
            {
                Uint8 *p = (Uint8 *) src;
                Uint16 W1, W2;
                if (srclen < 2) {
                    return SDL_ICONV_EINVAL;
                }
                W1 = ((Uint16) p[0] << 8) | (Uint16) p[1];
                src += 2;
                srclen -= 2;
                if (W1 < 0xD800 || W1 > 0xDFFF) {
                    ch = (Uint32) W1;
                    break;
                }
                if (W1 > 0xDBFF) {
                    /* Skip illegal sequences
                       return SDL_ICONV_EILSEQ;
                     */
                    ch = UNKNOWN_UNICODE;
                    break;
                }
                if (srclen < 2) {
                    return SDL_ICONV_EINVAL;
                }
                p = (Uint8 *) src;
                W2 = ((Uint16) p[0] << 8) | (Uint16) p[1];
                src += 2;
                srclen -= 2;
                if (W2 < 0xDC00 || W2 > 0xDFFF) {
                    /* Skip illegal sequences
                       return SDL_ICONV_EILSEQ;
                     */
                    ch = UNKNOWN_UNICODE;
                    break;
                }
                ch = (((Uint32) (W1 & 0x3FF) << 10) |
                      (Uint32) (W2 & 0x3FF)) + 0x10000;
            }
            break;
        case ENCODING_UTF16LE: /* RFC 2781 */
            {
                Uint8 *p = (Uint8 *) src;
                Uint16 W1, W2;
                if (srclen < 2) {
                    return SDL_ICONV_EINVAL;
                }
                W1 = ((Uint16) p[1] << 8) | (Uint16) p[0];
                src += 2;
                srclen -= 2;
                if (W1 < 0xD800 || W1 > 0xDFFF) {
                    ch = (Uint32) W1;
                    break;
                }
                if (W1 > 0xDBFF) {
                    /* Skip illegal sequences
                       return SDL_ICONV_EILSEQ;
                     */
                    ch = UNKNOWN_UNICODE;
                    break;
                }
                if (srclen < 2) {
                    return SDL_ICONV_EINVAL;
                }
                p = (Uint8 *) src;
                W2 = ((Uint16) p[1] << 8) | (Uint16) p[0];
                src += 2;
                srclen -= 2;
                if (W2 < 0xDC00 || W2 > 0xDFFF) {
                    /* Skip illegal sequences
                       return SDL_ICONV_EILSEQ;
                     */
                    ch = UNKNOWN_UNICODE;
                    break;
                }
                ch = (((Uint32) (W1 & 0x3FF) << 10) |
                      (Uint32) (W2 & 0x3FF)) + 0x10000;
            }
            break;
        case ENCODING_UCS2LE:
            {
                Uint8 *p = (Uint8 *) src;
                if (srclen < 2) {
                    return SDL_ICONV_EINVAL;
                }
                ch = ((Uint32) p[1] << 8) | (Uint32) p[0];
                src += 2;
                srclen -= 2;
            }
            break;
        case ENCODING_UCS2BE:
            {
                Uint8 *p = (Uint8 *) src;
                if (srclen < 2) {
                    return SDL_ICONV_EINVAL;
                }
                ch = ((Uint32) p[0] << 8) | (Uint32) p[1];
                src += 2;
                srclen -= 2;
            }
            break;
        case ENCODING_UCS4BE:
        case ENCODING_UTF32BE:
            {
                Uint8 *p = (Uint8 *) src;
                if (srclen < 4) {
                    return SDL_ICONV_EINVAL;
                }
                ch = ((Uint32) p[0] << 24) |
                    ((Uint32) p[1] << 16) |
                    ((Uint32) p[2] << 8) | (Uint32) p[3];
                src += 4;
                srclen -= 4;
            }
            break;
        case ENCODING_UCS4LE:
        case ENCODING_UTF32LE:
            {
                Uint8 *p = (Uint8 *) src;
                if (srclen < 4) {
                    return SDL_ICONV_EINVAL;
                }
                ch = ((Uint32) p[3] << 24) |
                    ((Uint32) p[2] << 16) |
                    ((Uint32) p[1] << 8) | (Uint32) p[0];
                src += 4;
                srclen -= 4;
            }
            break;
        }

        /* Encode a character */
        switch (cd->dst_fmt) {
        case ENCODING_ASCII:
            {
                Uint8 *p = (Uint8 *) dst;
                if (dstlen < 1) {
                    return SDL_ICONV_E2BIG;
                }
                if (ch > 0x7F) {
                    *p = UNKNOWN_ASCII;
                } else {
                    *p = (Uint8) ch;
                }
                ++dst;
                --dstlen;
            }
            break;
        case ENCODING_LATIN1:
            {
                Uint8 *p = (Uint8 *) dst;
                if (dstlen < 1) {
                    return SDL_ICONV_E2BIG;
                }
                if (ch > 0xFF) {
                    *p = UNKNOWN_ASCII;
                } else {
                    *p = (Uint8) ch;
                }
                ++dst;
                --dstlen;
            }
            break;
        case ENCODING_UTF8:    /* RFC 3629 */
            {
                Uint8 *p = (Uint8 *) dst;
                if (ch > 0x10FFFF) {
                    ch = UNKNOWN_UNICODE;
                }
                if (ch <= 0x7F) {
                    if (dstlen < 1) {
                        return SDL_ICONV_E2BIG;
                    }
                    *p = (Uint8) ch;
                    ++dst;
                    --dstlen;
                } else if (ch <= 0x7FF) {
                    if (dstlen < 2) {
                        return SDL_ICONV_E2BIG;
                    }
                    p[0] = 0xC0 | (Uint8) ((ch >> 6) & 0x1F);
                    p[1] = 0x80 | (Uint8) (ch & 0x3F);
                    dst += 2;
                    dstlen -= 2;
                } else if (ch <= 0xFFFF) {
                    if (dstlen < 3) {
                        return SDL_ICONV_E2BIG;
                    }
                    p[0] = 0xE0 | (Uint8) ((ch >> 12) & 0x0F);
                    p[1] = 0x80 | (Uint8) ((ch >> 6) & 0x3F);
                    p[2] = 0x80 | (Uint8) (ch & 0x3F);
                    dst += 3;
                    dstlen -= 3;
                } else {
                    if (dstlen < 4) {
                        return SDL_ICONV_E2BIG;
                    }
                    p[0] = 0xF0 | (Uint8) ((ch >> 18) & 0x07);
                    p[1] = 0x80 | (Uint8) ((ch >> 12) & 0x3F);
                    p[2] = 0x80 | (Uint8) ((ch >> 6) & 0x3F);
                    p[3] = 0x80 | (Uint8) (ch & 0x3F);
                    dst += 4;
                    dstlen -= 4;
                }
            }
            break;
        case ENCODING_UTF16BE: /* RFC 2781 */
            {
                Uint8 *p = (Uint8 *) dst;
                if (ch > 0x10FFFF) {
                    ch = UNKNOWN_UNICODE;
                }
                if (ch < 0x10000) {
                    if (dstlen < 2) {
                        return SDL_ICONV_E2BIG;
                    }
                    p[0] = (Uint8) (ch >> 8);
                    p[1] = (Uint8) ch;
                    dst += 2;
                    dstlen -= 2;
                } else {
                    Uint16 W1, W2;
                    if (dstlen < 4) {
                        return SDL_ICONV_E2BIG;
                    }
                    ch = ch - 0x10000;
                    W1 = 0xD800 | (Uint16) ((ch >> 10) & 0x3FF);
                    W2 = 0xDC00 | (Uint16) (ch & 0x3FF);
                    p[0] = (Uint8) (W1 >> 8);
                    p[1] = (Uint8) W1;
                    p[2] = (Uint8) (W2 >> 8);
                    p[3] = (Uint8) W2;
                    dst += 4;
                    dstlen -= 4;
                }
            }
            break;
        case ENCODING_UTF16LE: /* RFC 2781 */
            {
                Uint8 *p = (Uint8 *) dst;
                if (ch > 0x10FFFF) {
                    ch = UNKNOWN_UNICODE;
                }
                if (ch < 0x10000) {
                    if (dstlen < 2) {
                        return SDL_ICONV_E2BIG;
                    }
                    p[1] = (Uint8) (ch >> 8);
                    p[0] = (Uint8) ch;
                    dst += 2;
                    dstlen -= 2;
                } else {
                    Uint16 W1, W2;
                    if (dstlen < 4) {
                        return SDL_ICONV_E2BIG;
                    }
                    ch = ch - 0x10000;
                    W1 = 0xD800 | (Uint16) ((ch >> 10) & 0x3FF);
                    W2 = 0xDC00 | (Uint16) (ch & 0x3FF);
                    p[1] = (Uint8) (W1 >> 8);
                    p[0] = (Uint8) W1;
                    p[3] = (Uint8) (W2 >> 8);
                    p[2] = (Uint8) W2;
                    dst += 4;
                    dstlen -= 4;
                }
            }
            break;
        case ENCODING_UCS2BE:
            {
                Uint8 *p = (Uint8 *) dst;
                if (ch > 0xFFFF) {
                    ch = UNKNOWN_UNICODE;
                }
                if (dstlen < 2) {
                    return SDL_ICONV_E2BIG;
                }
                p[0] = (Uint8) (ch >> 8);
                p[1] = (Uint8) ch;
                dst += 2;
                dstlen -= 2;
            }
            break;
        case ENCODING_UCS2LE:
            {
                Uint8 *p = (Uint8 *) dst;
                if (ch > 0xFFFF) {
                    ch = UNKNOWN_UNICODE;
                }
                if (dstlen < 2) {
                    return SDL_ICONV_E2BIG;
                }
                p[1] = (Uint8) (ch >> 8);
                p[0] = (Uint8) ch;
                dst += 2;
                dstlen -= 2;
            }
            break;
        case ENCODING_UTF32BE:
            if (ch > 0x10FFFF) {
                ch = UNKNOWN_UNICODE;
            }
            /* fallthrough */
        case ENCODING_UCS4BE:
            if (ch > 0x7FFFFFFF) {
                ch = UNKNOWN_UNICODE;
            }
            {
                Uint8 *p = (Uint8 *) dst;
                if (dstlen < 4) {
                    return SDL_ICONV_E2BIG;
                }
                p[0] = (Uint8) (ch >> 24);
                p[1] = (Uint8) (ch >> 16);
                p[2] = (Uint8) (ch >> 8);
                p[3] = (Uint8) ch;
                dst += 4;
                dstlen -= 4;
            }
            break;
        case ENCODING_UTF32LE:
            if (ch > 0x10FFFF) {
                ch = UNKNOWN_UNICODE;
            }
            /* fallthrough */
        case ENCODING_UCS4LE:
            if (ch > 0x7FFFFFFF) {
                ch = UNKNOWN_UNICODE;
            }
            {
                Uint8 *p = (Uint8 *) dst;
                if (dstlen < 4) {
                    return SDL_ICONV_E2BIG;
                }
                p[3] = (Uint8) (ch >> 24);
                p[2] = (Uint8) (ch >> 16);
                p[1] = (Uint8) (ch >> 8);
                p[0] = (Uint8) ch;
                dst += 4;
                dstlen -= 4;
            }
            break;
        }

        /* Update state */
        *inbuf = src;
        *inbytesleft = srclen;
        *outbuf = dst;
        *outbytesleft = dstlen;
        ++total;
    }
    return total;
}

int
SDL_iconv_close(SDL_iconv_t cd)
{
    if (cd != (SDL_iconv_t)-1) {
        SDL_free(cd);
    }
    return 0;
}

#endif /* !HAVE_ICONV */

char *
SDL_iconv_string(const char *tocode, const char *fromcode, const char *inbuf,
                 size_t inbytesleft)
{
    SDL_iconv_t cd;
    char *string;
    size_t stringsize;
    char *outbuf;
    size_t outbytesleft;
    size_t retCode = 0;

    cd = SDL_iconv_open(tocode, fromcode);
    if (cd == (SDL_iconv_t) - 1) {
        /* See if we can recover here (fixes iconv on Solaris 11) */
        if (!tocode || !*tocode) {
            tocode = "UTF-8";
        }
        if (!fromcode || !*fromcode) {
            fromcode = "UTF-8";
        }
        cd = SDL_iconv_open(tocode, fromcode);
    }
    if (cd == (SDL_iconv_t) - 1) {
        return NULL;
    }

    stringsize = inbytesleft > 4 ? inbytesleft : 4;
    string = (char *) SDL_malloc(stringsize);
    if (!string) {
        SDL_iconv_close(cd);
        return NULL;
    }
    outbuf = string;
    outbytesleft = stringsize;
    SDL_memset(outbuf, 0, 4);

    while (inbytesleft > 0) {
        const size_t oldinbytesleft = inbytesleft;
        retCode = SDL_iconv(cd, &inbuf, &inbytesleft, &outbuf, &outbytesleft);
        switch (retCode) {
        case SDL_ICONV_E2BIG:
            {
                char *oldstring = string;
                stringsize *= 2;
                string = (char *) SDL_realloc(string, stringsize);
                if (!string) {
                    SDL_iconv_close(cd);
                    return NULL;
                }
                outbuf = string + (outbuf - oldstring);
                outbytesleft = stringsize - (outbuf - string);
                SDL_memset(outbuf, 0, 4);
            }
            break;
        case SDL_ICONV_EILSEQ:
            /* Try skipping some input data - not perfect, but... */
            ++inbuf;
            --inbytesleft;
            break;
        case SDL_ICONV_EINVAL:
        case SDL_ICONV_ERROR:
            /* We can't continue... */
            inbytesleft = 0;
            break;
        }
        /* Avoid infinite loops when nothing gets converted */
        if (oldinbytesleft == inbytesleft)
        {
            break;
        }
    }
    SDL_iconv_close(cd);

    return string;
}

/* vi: set ts=4 sw=4 expandtab: */
