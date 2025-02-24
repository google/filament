/*
 * Copyright 2011 - 2014
 * Andr\xe9 Malo or his licensors, as applicable
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "cext.h"
EXT_INIT_FUNC;

#ifdef EXT3
typedef Py_UNICODE rchar;
#else
typedef unsigned char rchar;
#endif
#define U(c) ((rchar)(c))

typedef struct {
    const rchar *start;
    const rchar *sentinel;
    const rchar *tsentinel;
    Py_ssize_t at_group;
    int in_macie5;
    int in_rule;
    int keep_bang_comments;
} rcssmin_ctx_t;

typedef enum {
    NEED_SPACE_MAYBE = 0,
    NEED_SPACE_NEVER
} need_space_flag;


#define RCSSMIN_DULL_BIT         (1 << 0)
#define RCSSMIN_HEX_BIT          (1 << 1)
#define RCSSMIN_ESC_BIT          (1 << 2)
#define RCSSMIN_SPACE_BIT        (1 << 3)
#define RCSSMIN_STRING_DULL_BIT  (1 << 4)
#define RCSSMIN_NMCHAR_BIT       (1 << 5)
#define RCSSMIN_URI_DULL_BIT     (1 << 6)
#define RCSSMIN_PRE_CHAR_BIT     (1 << 7)
#define RCSSMIN_POST_CHAR_BIT    (1 << 8)

static const unsigned short rcssmin_charmask[128] = {
     21,  21,  21,  21,  21,  21,  21,  21,
     21,  28,   8,  21,   8,   8,  21,  21,
     21,  21,  21,  21,  21,  21,  21,  21,
     21,  21,  21,  21,  21,  21,  21,  21,
     28, 469,   4,  85,  85,  85,  85,   4,
    149, 277,  85, 469, 469, 117,  85,  84,
    115, 115, 115, 115, 115, 115, 115, 115,
    115, 115, 468, 340,  85, 469, 468,  85,
     84, 115, 115, 115, 115, 115, 115, 117,
    117, 117, 117, 117, 117, 117, 117, 117,
    117, 117, 117, 117, 117, 117, 117, 117,
    117, 117, 117, 213,   4, 341,  85, 117,
     85, 115, 115, 115, 115, 115, 115, 117,
    117, 117, 117, 117, 117, 117, 117, 117,
    117, 117, 117, 117, 117, 116, 117, 117,
    117, 117, 117, 468,  85, 468,  85,  21
};

#define RCSSMIN_IS_DULL(c) ((U(c) > 127) || \
    (rcssmin_charmask[U(c) & 0x7F] & RCSSMIN_DULL_BIT))

#define RCSSMIN_IS_HEX(c) ((U(c) <= 127) && \
    (rcssmin_charmask[U(c) & 0x7F] & RCSSMIN_HEX_BIT))

#define RCSSMIN_IS_ESC(c) ((U(c) > 127) || \
    (rcssmin_charmask[U(c) & 0x7F] & RCSSMIN_ESC_BIT))

#define RCSSMIN_IS_SPACE(c) ((U(c) <= 127) && \
    (rcssmin_charmask[U(c) & 0x7F] & RCSSMIN_SPACE_BIT))

#define RCSSMIN_IS_STRING_DULL(c) ((U(c) > 127) || \
    (rcssmin_charmask[U(c) & 0x7F] & RCSSMIN_STRING_DULL_BIT))

#define RCSSMIN_IS_NMCHAR(c) ((U(c) > 127) || \
    (rcssmin_charmask[U(c) & 0x7F] & RCSSMIN_NMCHAR_BIT))

#define RCSSMIN_IS_URI_DULL(c) ((U(c) > 127) || \
    (rcssmin_charmask[U(c) & 0x7F] & RCSSMIN_URI_DULL_BIT))

#define RCSSMIN_IS_PRE_CHAR(c) ((U(c) <= 127) && \
    (rcssmin_charmask[U(c) & 0x7F] & RCSSMIN_PRE_CHAR_BIT))

#define RCSSMIN_IS_POST_CHAR(c) ((U(c) <= 127) && \
    (rcssmin_charmask[U(c) & 0x7F] & RCSSMIN_POST_CHAR_BIT))


static const rchar pattern_url[] = {
    /*U('u'),*/ U('r'), U('l'), U('(')
};

static const rchar pattern_ie7[] = {
    /*U('>'),*/ U('/'), U('*'), U('*'), U('/')
};

static const rchar pattern_media[] = {
    U('m'), U('e'), U('d'), U('i'), U('a'),
    U('M'), U('E'), U('D'), U('I'), U('A')
};

static const rchar pattern_document[] = {
    U('d'), U('o'), U('c'), U('u'), U('m'), U('e'), U('n'), U('t'),
    U('D'), U('O'), U('C'), U('U'), U('M'), U('E'), U('N'), U('T')
};

static const rchar pattern_supports[] = {
    U('s'), U('u'), U('p'), U('p'), U('o'), U('r'), U('t'), U('s'),
    U('S'), U('U'), U('P'), U('P'), U('O'), U('R'), U('T'), U('S')
};

static const rchar pattern_keyframes[] = {
    U('k'), U('e'), U('y'), U('f'), U('r'), U('a'), U('m'), U('e'), U('s'),
    U('K'), U('E'), U('Y'), U('F'), U('R'), U('A'), U('M'), U('E'), U('S')
};

static const rchar pattern_vendor_o[] = {
    U('-'), U('o'), U('-'),
    U('-'), U('O'), U('-')
};

static const rchar pattern_vendor_moz[] = {
    U('-'), U('m'), U('o'), U('z'), U('-'),
    U('-'), U('M'), U('O'), U('Z'), U('-')
};

static const rchar pattern_vendor_webkit[] = {
    U('-'), U('w'), U('e'), U('b'), U('k'), U('i'), U('t'), U('-'),
    U('-'), U('W'), U('E'), U('B'), U('K'), U('I'), U('T'), U('-')
};

static const rchar pattern_vendor_ms[] = {
    U('-'), U('m'), U('s'), U('-'),
    U('-'), U('M'), U('S'), U('-')
};

static const rchar pattern_first[] = {
    U('f'), U('i'), U('r'), U('s'), U('t'), U('-'), U('l'),
    U('F'), U('I'), U('R'), U('S'), U('T'), U('-'), U('L')
};

static const rchar pattern_line[] = {
    U('i'), U('n'), U('e'),
    U('I'), U('N'), U('E'),
};

static const rchar pattern_letter[] = {
    U('e'), U('t'), U('t'), U('e'), U('r'),
    U('E'), U('T'), U('T'), U('E'), U('R')
};

static const rchar pattern_macie5_init[] = {
    U('/'), U('*'), U('\\'), U('*'), U('/')
};

static const rchar pattern_macie5_exit[] = {
    U('/'), U('*'), U('*'), U('/')
};

/*
 * Match a pattern (and copy immediately to target)
 */
#if defined(__GNUC__) && (__GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 6))
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wstrict-overflow"
#endif
static int
copy_match(const rchar *pattern, const rchar *psentinel,
           const rchar **source_, rchar **target_, rcssmin_ctx_t *ctx)
{
    const rchar *source = *source_;
    rchar *target = *target_;
    rchar c;

    while (pattern < psentinel
           && source < ctx->sentinel && target < ctx->tsentinel
           && ((c = *source++) == *pattern++))
        *target++ = c;

    *source_ = source;
    *target_ = target;

    return (pattern == psentinel);
}
#if defined(__GNUC__) && (__GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 6))
#pragma GCC diagnostic pop
#endif

#define MATCH(PAT, source, target, ctx) (                              \
    copy_match(pattern_##PAT,                                          \
               pattern_##PAT + sizeof(pattern_##PAT) / sizeof(rchar),  \
               source, target, ctx)                                    \
)


/*
 * Match a pattern (and copy immediately to target) - CI version
 */
static int
copy_imatch(const rchar *pattern, const rchar *psentinel,
            const rchar **source_, rchar **target_, rcssmin_ctx_t *ctx)
{
    const rchar *source = *source_, *pstart = pattern;
    rchar *target = *target_;
    rchar c;

    while (pattern < psentinel
           && source < ctx->sentinel && target < ctx->tsentinel
           && ((c = *source++) == *pattern
               || c == pstart[(pattern - pstart) + (psentinel - pstart)])) {
        ++pattern;
        *target++ = c;
    }

    *source_ = source;
    *target_ = target;

    return (pattern == psentinel);
}

#define IMATCH(PAT, source, target, ctx) (                                  \
    copy_imatch(pattern_##PAT,                                              \
                pattern_##PAT + sizeof(pattern_##PAT) / sizeof(rchar) / 2,  \
                source, target, ctx)                                        \
)


/*
 * Copy characters
 */
#if defined(__GNUC__) && (__GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 6))
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wstrict-overflow"
#endif
static int
copy(const rchar *source, const rchar *sentinel, rchar **target_,
     rcssmin_ctx_t *ctx)
{
    rchar *target = *target_;

    while (source < sentinel && target < ctx->tsentinel)
        *target++ = *source++;

    *target_ = target;

    return (source == sentinel);
}
#if defined(__GNUC__) && (__GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 6))
#pragma GCC diagnostic pop
#endif

#define COPY_PAT(PAT, target, ctx) (                             \
    copy(pattern_##PAT,                                          \
         pattern_##PAT + sizeof(pattern_##PAT) / sizeof(rchar),  \
         target, ctx)                                            \
)


/*
 * The ABORT macros work with known local variables!
 */
#define ABORT_(RET) do {                                         \
    if (source < ctx->sentinel && !(target < ctx->tsentinel)) {  \
        *source_ = source;                                       \
        *target_ = target;                                       \
    }                                                            \
    return RET;                                                  \
} while(0)


#define CRAPPY_C90_COMPATIBLE_EMPTY
#define ABORT ABORT_(CRAPPY_C90_COMPATIBLE_EMPTY)
#define RABORT(RET) ABORT_((RET))


/*
 * Copy escape
 */
static void
copy_escape(const rchar **source_, rchar **target_, rcssmin_ctx_t *ctx)
{
    const rchar *source = *source_, *hsentinel;
    rchar *target = *target_;
    rchar c;

    *target++ = U('\\');
    *target_ = target;

    if (source < ctx->sentinel && target < ctx->tsentinel) {
        c = *source++;
        if (RCSSMIN_IS_ESC(c)) {
            *target++ = c;
        }
        else if (RCSSMIN_IS_HEX(c)) {
            *target++ = c;

            /* 6 hex chars max, one we got already */
            if (ctx->sentinel - source > 5)
                hsentinel = source + 5;
            else
                hsentinel = ctx->sentinel;

            while (source < hsentinel && target < ctx->tsentinel
                   && (c = *source, RCSSMIN_IS_HEX(c))) {
                ++source;
                *target++ = c;
            }

            /* One optional space after */
            if (source < ctx->sentinel && target < ctx->tsentinel) {
                if (source == hsentinel)
                    c = *source;
                if (RCSSMIN_IS_SPACE(c)) {
                    ++source;
                    *target++ = U(' ');
                    if (c == U('\r') && source < ctx->sentinel
                        && *source == U('\n'))
                        ++source;
                }
            }
        }
    }

    *target_ = target;
    *source_ = source;
}


/*
 * Copy string
 */
static void
copy_string(const rchar **source_, rchar **target_, rcssmin_ctx_t *ctx)
{
    const rchar *source = *source_;
    rchar *target = *target_;
    rchar c, quote = source[-1];

    *target++ = quote;
    *target_ = target;

    while (source < ctx->sentinel && target < ctx->tsentinel) {
        c = *target++ = *source++;
        if (RCSSMIN_IS_STRING_DULL(c))
            continue;

        switch (c) {
        case U('\''): case U('"'):
            if (c == quote) {
                *target_ = target;
                *source_ = source;
                return;
            }
            continue;

        case U('\\'):
            if (source < ctx->sentinel && target < ctx->tsentinel) {
                c = *source++;
                switch (c) {
                case U('\r'):
                    if (source < ctx->sentinel && *source == U('\n'))
                        ++source;
                    /* fall through */

                case U('\n'): case U('\f'):
                    --target;
                    break;

                default:
                    *target++ = c;
                }
            }
            continue;
        }
        break; /* forbidden characters */
    }

    ABORT;
}


/*
 * Copy URI string
 */
static int
copy_uri_string(const rchar **source_, rchar **target_, rcssmin_ctx_t *ctx)
{
    const rchar *source = *source_;
    rchar *target = *target_;
    rchar c, quote = source[-1];

    *target++ = quote;
    *target_ = target;

    while (source < ctx->sentinel && target < ctx->tsentinel) {
        c = *source++;
        if (RCSSMIN_IS_SPACE(c))
            continue;
        *target++ = c;
        if (RCSSMIN_IS_STRING_DULL(c))
            continue;

        switch (c) {
        case U('\''): case U('"'):
            if (c == quote) {
                *target_ = target;
                *source_ = source;
                return 0;
            }
            continue;

        case U('\\'):
            if (source < ctx->sentinel && target < ctx->tsentinel) {
                c = *source;
                switch (c) {
                case U('\r'):
                    if ((source + 1) < ctx->sentinel && source[1] == U('\n'))
                        ++source;
                    /* fall through */

                case U('\n'): case U('\f'):
                    --target;
                    ++source;
                    break;

                default:
                    --target;
                    copy_escape(&source, &target, ctx);
                }
            }
            continue;
        }

        break; /* forbidden characters */
    }

    RABORT(-1);
}


/*
 * Copy URI (unquoted)
 */
static int
copy_uri_unquoted(const rchar **source_, rchar **target_, rcssmin_ctx_t *ctx)
{
    const rchar *source = *source_;
    rchar *target = *target_;
    rchar c;

    *target++ = source[-1];
    *target_ = target;

    while (source < ctx->sentinel && target < ctx->tsentinel) {
        c = *source++;
        if (RCSSMIN_IS_SPACE(c))
            continue;
        *target++ = c;
        if (RCSSMIN_IS_URI_DULL(c))
            continue;

        switch (c) {

        case U(')'):
            *target_ = target - 1;
            *source_ = source - 1;
            return 0;

        case U('\\'):
            if (source < ctx->sentinel && target < ctx->tsentinel) {
                c = *source;
                switch (c) {
                case U('\r'):
                    if ((source + 1) < ctx->sentinel && source[1] == U('\n'))
                        ++source;
                    /* fall through */

                case U('\n'): case U('\f'):
                    --target;
                    ++source;
                    break;

                default:
                    --target;
                    copy_escape(&source, &target, ctx);
                }
            }
            continue;
        }

        break; /* forbidden characters */
    }

    RABORT(-1);
}


/*
 * Copy url
 */
static void
copy_url(const rchar **source_, rchar **target_, rcssmin_ctx_t *ctx)
{
    const rchar *source = *source_;
    rchar *target = *target_;
    rchar c;

    *target++ = U('u');
    *target_ = target;

    /* Must not be inside an identifier */
    if ((source != ctx->start + 1) && RCSSMIN_IS_NMCHAR(source[-2]))
        return;

    if (!MATCH(url, &source, &target, ctx)
        || !(source < ctx->sentinel && target < ctx->tsentinel))
        ABORT;

    while (source < ctx->sentinel && RCSSMIN_IS_SPACE(*source))
        ++source;

    if (!(source < ctx->sentinel))
        ABORT;

    c = *source++;
    switch (c) {
    case U('"'): case U('\''):
        if (copy_uri_string(&source, &target, ctx) == -1)
            ABORT;

        while (source < ctx->sentinel && RCSSMIN_IS_SPACE(*source))
            ++source;
        break;

    default:
        if (copy_uri_unquoted(&source, &target, ctx) == -1)
            ABORT;
    }

    if (!(source < ctx->sentinel && target < ctx->tsentinel))
        ABORT;

    if ((*target++ = *source++) != U(')'))
        ABORT;

    *target_ = target;
    *source_ = source;
}


/*
 * Copy @-group
 */
static void
copy_at_group(const rchar **source_, rchar **target_, rcssmin_ctx_t *ctx)
{
    const rchar *source = *source_;
    rchar *target = *target_;

    *target++ = U('@');
    *target_ = target;

#define REMATCH(what) ( \
    source = *source_, \
    target = *target_, \
    IMATCH(what, &source, &target, ctx) \
)
#define CMATCH(what) IMATCH(what, &source, &target, ctx)

    if ((  !CMATCH(media)
        && !REMATCH(supports)
        && !REMATCH(document)
        && !REMATCH(keyframes)
        && !(REMATCH(vendor_webkit) && CMATCH(keyframes))
        && !(REMATCH(vendor_moz) && CMATCH(keyframes))
        && !(REMATCH(vendor_o) && CMATCH(keyframes))
        && !(REMATCH(vendor_ms) && CMATCH(keyframes)))
        || !(source < ctx->sentinel && target < ctx->tsentinel)
        || RCSSMIN_IS_NMCHAR(*source))
        ABORT;

#undef CMATCH
#undef REMATCH

    ++ctx->at_group;

    *target_ = target;
    *source_ = source;
}


/*
 * Skip space
 */
static const rchar *
skip_space(const rchar *source, rcssmin_ctx_t *ctx)
{
    const rchar *begin = source;
    int res;
    rchar c;

    while (source < ctx->sentinel) {
        c = *source;
        if (RCSSMIN_IS_SPACE(c)) {
            ++source;
            continue;
        }
        else if (c == U('/')) {
            ++source;
            if (!(source < ctx->sentinel && *source == U('*'))) {
                --source;
                break;
            }
            ++source;
            res = 0;
            while (source < ctx->sentinel) {
                c = *source++;
                if (c != U('*'))
                    continue;
                if (!(source < ctx->sentinel))
                    return begin;
                if (*source != U('/'))
                    continue;

                /* Comment complete */
                ++source;
                res = 1;
                break;
            }
            if (!res)
                return begin;

            continue;
        }

        break;
    }

    return source;
}


/*
 * Copy space
 */
static void
copy_space(const rchar **source_, rchar **target_, rcssmin_ctx_t *ctx,
           need_space_flag need_space)
{
    const rchar *source = *source_, *end, *comment;
    rchar *target = *target_;
    int res;
    rchar c;

    --source;
    if (need_space == NEED_SPACE_MAYBE
        && source > ctx->start
        && !RCSSMIN_IS_PRE_CHAR(source[-1])
        && (end = skip_space(source, ctx)) < ctx->sentinel
        && (!RCSSMIN_IS_POST_CHAR(*end)
            || (*end == U(':') && !ctx->in_rule && !ctx->at_group))) {

        if (!(target < ctx->tsentinel))
            ABORT;
        *target++ = U(' ');
    }

    while (source < ctx->sentinel) {
        switch (c = *source) {

        /* comment */
        case U('/'):
            comment = source++;
            if (!((source < ctx->sentinel && *source == U('*')))) {
                --source;
                break;
            }
            ++source;
            res = 0;
            while (source < ctx->sentinel) {
                c = *source++;
                if (c != U('*'))
                    continue;
                if (!(source < ctx->sentinel))
                    ABORT;
                if (*source != U('/'))
                    continue;

                /* Comment complete */
                ++source;
                res = 1;

                if (ctx->keep_bang_comments && comment[2] == U('!')) {
                    ctx->in_macie5 = (source[-3] == U('\\'));
                    if (!copy(comment, source, &target, ctx))
                        ABORT;
                }
                else if (source[-3] == U('\\')) {
                    if (!ctx->in_macie5) {
                        if (!COPY_PAT(macie5_init, &target, ctx))
                            ABORT;
                    }
                    ctx->in_macie5 = 1;
                }
                else if (ctx->in_macie5) {
                    if (!COPY_PAT(macie5_exit, &target, ctx))
                        ABORT;
                    ctx->in_macie5 = 0;
                }
                /* else don't copy anything */
                break;
            }
            if (!res)
                ABORT;
            continue;

        /* space */
        case U(' '): case U('\t'): case U('\r'): case U('\n'): case U('\f'):
            ++source;
            continue;
        }

        break;
    }

    *source_ = source;
    *target_ = target;
}


/*
 * Copy space if comment
 */
static int
copy_space_comment(const rchar **source_, rchar **target_,
                   rcssmin_ctx_t *ctx, need_space_flag need_space)
{
    const rchar *source = *source_;
    rchar *target = *target_;

    if (source < ctx->sentinel && *source == U('*')) {
        copy_space(source_, target_, ctx, need_space);
        if (*source_ > source)
            return 0;
    }
    if (!(target < ctx->tsentinel))
        RABORT(-1);

    *target++ = source[-1];

    /* *source_ = source; <-- unchanged */
    *target_ = target;

    return -1;
}


/*
 * Copy space if exists
 */
static int
copy_space_optional(const rchar **source_, rchar **target_,
                    rcssmin_ctx_t *ctx)
{
    const rchar *source = *source_;

    if (!(source < ctx->sentinel))
        return -1;

    if (*source == U('/')) {
        *source_ = source + 1;
        return copy_space_comment(source_, target_, ctx, NEED_SPACE_NEVER);
    }
    else if (RCSSMIN_IS_SPACE(*source)) {
        *source_ = source + 1;
        copy_space(source_, target_, ctx, NEED_SPACE_NEVER);
        return 0;
    }

    return -1;
}


/*
 * Copy :first-line|letter
 */
static void
copy_first(const rchar **source_, rchar **target_, rcssmin_ctx_t *ctx)
{
    const rchar *source = *source_, *next, *source_fork;
    rchar *target = *target_, *target_fork;

    *target++ = U(':');
    *target_ = target;

    if (!IMATCH(first, &source, &target, ctx)
        || !(source < ctx->sentinel && target < ctx->tsentinel))
        ABORT;

    source_fork = source;
    target_fork = target;

    if (!IMATCH(line, &source, &target, ctx)) {
        source = source_fork;
        target = target_fork;

        if (!IMATCH(letter, &source, &target, ctx)
            || !(source < ctx->sentinel && target < ctx->tsentinel))
            ABORT;
    }

    next = skip_space(source, ctx);
    if (!(next < ctx->sentinel && target < ctx->tsentinel
        && (*next == U('{') || *next == U(','))))
        ABORT;

    *target++ = U(' ');
    *target_ = target;
    *source_ = source;
    (void)copy_space_optional(source_, target_, ctx);
}


/*
 * Copy IE7 hack
 */
static void
copy_ie7hack(const rchar **source_, rchar **target_, rcssmin_ctx_t *ctx)
{
    const rchar *source = *source_;
    rchar *target = *target_;

    *target++ = U('>');
    *target_ = target;

    if (ctx->in_rule || ctx->at_group)
        return; /* abort */

    if (!MATCH(ie7, &source, &target, ctx))
        ABORT;

    ctx->in_macie5 = 0;

    *target_ = target;
    *source_ = source;

    (void)copy_space_optional(source_, target_, ctx);
}


/*
 * Copy semicolon; miss out duplicates or even this one (before '}')
 */
static void
copy_semicolon(const rchar **source_, rchar **target_, rcssmin_ctx_t *ctx)
{
    const rchar *source = *source_, *begin, *end;
    rchar *target = *target_;

    begin = source;
    while (source < ctx->sentinel) {
        end = skip_space(source, ctx);
        if (!(end < ctx->sentinel)) {
            if (!(target < ctx->tsentinel))
                ABORT;
            *target++ = U(';');
            break;
        }
        switch (*end) {
        case U(';'):
            source = end + 1;
            continue;

        case U('}'):
            if (ctx->in_rule)
                break;

            /* fall through */
        default:
            if (!(target < ctx->tsentinel))
                ABORT;
            *target++ = U(';');
            break;
        }

        break;
    }

    source = begin;
    *target_ = target;
    while (source < ctx->sentinel) {
        if (*source == U(';')) {
            ++source;
            continue;
        }

        if (copy_space_optional(&source, target_, ctx) == 0)
            continue;

        break;
    }

    *source_ = source;
}


/*
 * Main function
 *
 * The return value determines the result length (kept in the target buffer).
 * However, if the target buffer is too small, the return value is greater
 * than tlength. The difference to tlength is the number of unconsumed source
 * characters at the time the buffer was full. In this case you should resize
 * the target buffer to the return value and call rcssmin again. Repeat as
 * often as needed.
 */
static Py_ssize_t
rcssmin(const rchar *source, rchar *target, Py_ssize_t slength,
        Py_ssize_t tlength, int keep_bang_comments)
{
    rcssmin_ctx_t ctx_, *ctx = &ctx_;
    const rchar *tstart = target;
    rchar c;

    ctx->start = source;
    ctx->sentinel = source + slength;
    ctx->tsentinel = target + tlength;
    ctx->at_group = 0;
    ctx->in_macie5 = 0;
    ctx->in_rule = 0;
    ctx->keep_bang_comments = keep_bang_comments;

    while (source < ctx->sentinel && target < ctx->tsentinel) {
        c = *source++;
        if (RCSSMIN_IS_DULL(c)) {
            *target++ = c;
            continue;
        }
        else if (RCSSMIN_IS_SPACE(c)) {
            copy_space(&source, &target, ctx, NEED_SPACE_MAYBE);
            continue;
        }

        switch (c) {

        /* Escape */
        case U('\\'):
            copy_escape(&source, &target, ctx);
            continue;

        /* String */
        case U('"'): case U('\''):
            copy_string(&source, &target, ctx);
            continue;

        /* URL */
        case U('u'):
            copy_url(&source, &target, ctx);
            continue;

        /* IE7hack */
        case U('>'):
            copy_ie7hack(&source, &target, ctx);
            continue;

        /* @-group */
        case U('@'):
            copy_at_group(&source, &target, ctx);
            continue;

        /* ; */
        case U(';'):
            copy_semicolon(&source, &target, ctx);
            continue;

        /* :first-line|letter followed by [{,] */
        /* (apparently needed for IE6) */
        case U(':'):
            copy_first(&source, &target, ctx);
            continue;

        /* { */
        case U('{'):
            if (ctx->at_group)
                --ctx->at_group;
            else
                ++ctx->in_rule;
            *target++ = c;
            continue;

        /* } */
        case U('}'):
            if (ctx->in_rule)
                --ctx->in_rule;
            *target++ = c;
            continue;

        /* space starting with comment */
        case U('/'):
            (void)copy_space_comment(&source, &target, ctx, NEED_SPACE_MAYBE);
            continue;

        /* Fallback: copy character. Better safe than sorry. Should not be
         * reached, though */
        default:
            *target++ = c;
            continue;
        }
    }

    return
        (Py_ssize_t)(target - tstart) + (Py_ssize_t)(ctx->sentinel - source);
}


PyDoc_STRVAR(rcssmin_cssmin__doc__,
"cssmin(style, keep_bang_comments=False)\n\
\n\
Minify CSS.\n\
\n\
:Note: This is a hand crafted C implementation built on the regex\n\
       semantics.\n\
\n\
:Parameters:\n\
  `style` : ``str``\n\
    CSS to minify\n\
\n\
:Return: Minified style\n\
:Rtype: ``str``");

static PyObject *
rcssmin_cssmin(PyObject *self, PyObject *args, PyObject *kwds)
{
    PyObject *style, *keep_bang_comments_ = NULL, *result;
    static char *kwlist[] = {"style", "keep_bang_comments", NULL};
    Py_ssize_t rlength, slength, length;
    int keep_bang_comments;
#ifdef EXT2
    int uni;
#define UOBJ "O"
#endif
#ifdef EXT3
#define UOBJ "U"
#endif

    if (!PyArg_ParseTupleAndKeywords(args, kwds, UOBJ "|O", kwlist,
                                     &style, &keep_bang_comments_))
        return NULL;

    if (!keep_bang_comments_)
        keep_bang_comments = 0;
    else {
        keep_bang_comments = PyObject_IsTrue(keep_bang_comments_);
        if (keep_bang_comments == -1)
            return NULL;
    }

#ifdef EXT2
    if (PyUnicode_Check(style)) {
        if (!(style = PyUnicode_AsUTF8String(style)))
            return NULL;
        uni = 1;
    }
    else {
        if (!(style = PyObject_Str(style)))
            return NULL;
        uni = 0;
    }
#endif

#ifdef EXT3
    Py_INCREF(style);
#define PyString_GET_SIZE PyUnicode_GET_SIZE
#define PyString_AS_STRING PyUnicode_AS_UNICODE
#define _PyString_Resize PyUnicode_Resize
#define PyString_FromStringAndSize PyUnicode_FromUnicode
#endif

    rlength = slength = PyString_GET_SIZE(style);

again:
    if (!(result = PyString_FromStringAndSize(NULL, rlength))) {
        Py_DECREF(style);
        return NULL;
    }
    Py_BEGIN_ALLOW_THREADS
    length = rcssmin((rchar *)PyString_AS_STRING(style),
                     (rchar *)PyString_AS_STRING(result),
                     slength, rlength, keep_bang_comments);
    Py_END_ALLOW_THREADS

    if (length > rlength) {
        Py_DECREF(result);
        rlength = length;
        goto again;
    }

    Py_DECREF(style);
    if (length < 0) {
        Py_DECREF(result);
        return NULL;
    }
    if (length != rlength && _PyString_Resize(&result, length) == -1)
        return NULL;

#ifdef EXT2
    if (uni) {
        style = PyUnicode_DecodeUTF8(PyString_AS_STRING(result),
                                     PyString_GET_SIZE(result), "strict");
        Py_DECREF(result);
        if (!style)
            return NULL;
        result = style;
    }
#endif
    return result;
}

/* ------------------------ BEGIN MODULE DEFINITION ------------------------ */

EXT_METHODS = {
    {"cssmin",
        (PyCFunction)rcssmin_cssmin, METH_VARARGS | METH_KEYWORDS,
        rcssmin_cssmin__doc__},

    {NULL}  /* Sentinel */
};

PyDoc_STRVAR(EXT_DOCS_VAR,
"C implementation of rcssmin\n\
===========================\n\
\n\
C implementation of rcssmin.");


EXT_DEFINE(EXT_MODULE_NAME, EXT_METHODS_VAR, EXT_DOCS_VAR);

EXT_INIT_FUNC {
    PyObject *m;

    /* Create the module and populate stuff */
    if (!(m = EXT_CREATE(&EXT_DEFINE_VAR)))
        EXT_INIT_ERROR(NULL);

    EXT_ADD_UNICODE(m, "__author__", "Andr\xe9 Malo", "latin-1");
    EXT_ADD_STRING(m, "__docformat__", "restructuredtext en");

    EXT_INIT_RETURN(m);
}

/* ------------------------- END MODULE DEFINITION ------------------------- */
