/*
 * Copyright 2011 - 2015
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

#define RJSMIN_DULL_BIT           (1 << 0)
#define RJSMIN_PRE_REGEX_BIT      (1 << 1)
#define RJSMIN_REGEX_DULL_BIT     (1 << 2)
#define RJSMIN_REGEX_CC_DULL_BIT  (1 << 3)
#define RJSMIN_ID_LIT_BIT         (1 << 4)
#define RJSMIN_ID_LIT_O_BIT       (1 << 5)
#define RJSMIN_ID_LIT_C_BIT       (1 << 6)
#define RJSMIN_STRING_DULL_BIT    (1 << 7)
#define RJSMIN_SPACE_BIT          (1 << 8)
#define RJSMIN_POST_REGEX_OFF_BIT (1 << 9)

#ifdef EXT3
typedef Py_UNICODE rchar;
#else
typedef unsigned char rchar;
#endif
#define U(c) ((rchar)(c))

#define RJSMIN_IS_DULL(c) ((U(c) > 127) || \
    (rjsmin_charmask[U(c) & 0x7F] & RJSMIN_DULL_BIT))

#define RJSMIN_IS_REGEX_DULL(c) ((U(c) > 127) || \
    (rjsmin_charmask[U(c) & 0x7F] & RJSMIN_REGEX_DULL_BIT))

#define RJSMIN_IS_REGEX_CC_DULL(c) ((U(c) > 127) || \
    (rjsmin_charmask[U(c) & 0x7F] & RJSMIN_REGEX_CC_DULL_BIT))

#define RJSMIN_IS_STRING_DULL(c) ((U(c) > 127) || \
    (rjsmin_charmask[U(c) & 0x7F] & RJSMIN_STRING_DULL_BIT))

#define RJSMIN_IS_ID_LITERAL(c) ((U(c) > 127) || \
    (rjsmin_charmask[U(c) & 0x7F] & RJSMIN_ID_LIT_BIT))

#define RJSMIN_IS_ID_LITERAL_OPEN(c) ((U(c) > 127) || \
    (rjsmin_charmask[U(c) & 0x7F] & RJSMIN_ID_LIT_O_BIT))

#define RJSMIN_IS_ID_LITERAL_CLOSE(c) ((U(c) > 127) || \
    (rjsmin_charmask[U(c) & 0x7F] & RJSMIN_ID_LIT_C_BIT))

#define RJSMIN_IS_POST_REGEX_OFF(c) ((U(c) > 127) || \
    (rjsmin_charmask[U(c) & 0x7F] & RJSMIN_POST_REGEX_OFF_BIT))

#define RJSMIN_IS_SPACE(c) ((U(c) <= 127) && \
    (rjsmin_charmask[U(c) & 0x7F] & RJSMIN_SPACE_BIT))

#define RJSMIN_IS_PRE_REGEX_1(c) ((U(c) <= 127) && \
    (rjsmin_charmask[U(c) & 0x7F] & RJSMIN_PRE_REGEX_BIT))


static const unsigned short rjsmin_charmask[128] = {
    396, 396, 396, 396, 396, 396, 396, 396,
    396, 396,   2, 396, 396,   2, 396, 396,
    396, 396, 396, 396, 396, 396, 396, 396,
    396, 396, 396, 396, 396, 396, 396, 396,
    396, 687, 588, 653, 765, 653, 143, 588,
    687, 205, 653, 237, 143, 237, 141, 648,
    765, 765, 765, 765, 765, 765, 765, 765,
    765, 765, 143, 143, 653, 143, 653, 143,
    653, 765, 765, 765, 765, 765, 765, 765,
    765, 765, 765, 765, 765, 765, 765, 765,
    765, 765, 765, 765, 765, 765, 765, 765,
    765, 765, 765, 683, 513, 197, 653, 765,
    653, 765, 765, 765, 765, 765, 765, 765,
    765, 765, 765, 765, 765, 765, 765, 765,
    765, 765, 765, 765, 765, 765, 765, 765,
    765, 765, 765, 687, 143, 207, 653, 765
};

static Py_ssize_t
rjsmin(const rchar *source, rchar *target, Py_ssize_t length,
       int keep_bang_comments)
{
    const rchar *reset, *pcreset = NULL, *pctoken = NULL, *xtarget,
                *sentinel = source + length;
    rchar *tstart = target;
    int post_regex = 0;
    rchar c, quote, spaced = U(' ');

    while (source < sentinel) {
        c = *source++;
        if (RJSMIN_IS_DULL(c)) {
            if (post_regex) post_regex = 0;
            if (pctoken) pctoken = NULL;
            if (spaced == U('\n')) spaced = U(' ');

            *target++ = c;
            continue;
        }
        switch (c) {

        /* String */
        case U('\''): case U('"'):
            if (post_regex) post_regex = 0;
            if (pctoken) pctoken = NULL;
            if (spaced == U('\n')) spaced = U(' ');

            reset = source;
            *target++ = quote = c;
            while (source < sentinel) {
                c = *source++;
                *target++ = c;
                if (RJSMIN_IS_STRING_DULL(c))
                    continue;
                switch (c) {
                case U('\''): case U('"'):
                    if (c == quote)
                        goto cont;
                    continue;
                case U('\\'):
                    if (source < sentinel) {
                        c = *source++;
                        *target++ = c;
                        if (c == U('\r') && source < sentinel
                            && *source == U('\n'))
                            *target++ = *source++;
                    }
                    continue;
                }
                break;
            }
            target -= source - reset;
            source = reset;
            continue;

        /* Comment or Regex or something else entirely */
        case U('/'):
            if (!(source < sentinel)) {
                if (post_regex) post_regex = 0;
                if (pctoken) pctoken = NULL;
                if (spaced == U('\n')) spaced = U(' ');

                *target++ = c;
            }
            else {
                switch (*source) {
            /* Comment */
                case U('*'): case U('/'):
                    goto skip_or_copy_ws;

                default:
                    xtarget = NULL;
                    if (   target == tstart
                        || RJSMIN_IS_PRE_REGEX_1(*((pctoken ? pctoken : target)
                                                   - 1))
                        || (
                            (xtarget = pctoken ? pctoken : target)
                            && (xtarget - tstart >= 6)
                            && *(xtarget - 1) == U('n')
                            && *(xtarget - 2) == U('r')
                            && *(xtarget - 3) == U('u')
                            && *(xtarget - 4) == U('t')
                            && *(xtarget - 5) == U('e')
                            && *(xtarget - 6) == U('r')
                            && (
                                   xtarget - tstart == 6
                                || !RJSMIN_IS_ID_LITERAL(*(xtarget - 7))
                            )
                        )) {

            /* Regex */
                        if (post_regex) post_regex = 0;
                        if (pctoken) pctoken = NULL;

                        reset = source;
                        if (spaced == U('\n')) {
                            spaced = U(' ');
                            if (xtarget)
                                *target++ = U('\n');
                        }

                        *target++ = U('/');
                        while (source < sentinel) {
                            c = *source++;
                            *target++ = c;
                            if (RJSMIN_IS_REGEX_DULL(c))
                                continue;
                            switch (c) {
                            case U('/'):
                                post_regex = 1;
                                goto cont;
                            case U('\\'):
                                if (source < sentinel) {
                                    c = *source++;
                                    *target++ = c;
                                    if (c == U('\r') || c == U('\n'))
                                        break;
                                }
                                continue;
                            case U('['):
                                while (source < sentinel) {
                                    c = *source++;
                                    *target++ = c;
                                    if (RJSMIN_IS_REGEX_CC_DULL(c))
                                        continue;
                                    switch (c) {
                                    case U('\\'):
                                        if (source < sentinel) {
                                            c = *source++;
                                            *target++ = c;
                                            if (c == U('\r') || c == U('\n'))
                                                break;
                                        }
                                        continue;
                                    case U(']'):
                                        goto cont_regex;
                                    }
                                }
                                break;
                            }
                            break;
                        cont_regex:
                            continue;
                        }
                        target -= source - reset;
                        source = reset;
                    }
                    else {
            /* Just a slash */
                        if (post_regex) post_regex = 0;
                        if (pctoken) pctoken = NULL;
                        if (spaced == U('\n')) spaced = U(' ');

                        *target++ = c;
                    }
                    continue;
                }
            }
            continue;

        /* Whitespace */
        default:
        skip_or_copy_ws:
            quote = U(' ');
            --source;
            while (source < sentinel) {
                c = *source++;
                if (RJSMIN_IS_SPACE(c))
                    continue;
                switch (c) {
                case U('\r'): case U('\n'):
                    quote = U('\n');
                    continue;
                case U('/'):
                    if (source < sentinel) {
                        switch (*source) {
                        case U('*'):
                            reset = source++;
                            /* copy bang comment, if requested */
                            if (   keep_bang_comments && source < sentinel
                                && *source == U('!')) {
                                if (!pctoken) {
                                    pctoken = target;
                                    pcreset = reset;
                                }

                                *target++ = U('/');
                                *target++ = U('*');
                                *target++ = *source++;
                                while (source < sentinel) {
                                    c = *source++;
                                    *target++ = c;
                                    if (c == U('*') && source < sentinel
                                        && *source == U('/')) {
                                        *target++ = *source++;
                                        reset = NULL;
                                        break;
                                    }
                                }
                                if (!reset)
                                    continue;

                                target -= source - reset;
                                source = reset;
                                if (pcreset == reset) {
                                    pctoken = NULL;
                                    pcreset = NULL;
                                }

                            }
                            /* strip regular comment */
                            else {
                                while (source < sentinel) {
                                    c = *source++;
                                    if (c == U('*') && source < sentinel
                                        && *source == U('/')) {
                                        ++source;
                                        reset = NULL;
                                        break;
                                    }
                                }
                                if (!reset)
                                    continue;
                                source = reset;
                                *target++ = U('/');
                            }
                            goto cont;
                        case U('/'):
                            ++source;
                            while (source < sentinel) {
                                c = *source++;
                                switch (c) {
                                case U('\n'):
                                    break;
                                case U('\r'):
                                    if (source < sentinel
                                        && *source == U('\n'))
                                        ++source;
                                    break;
                                default:
                                    continue;
                                }
                                break;
                            }
                            quote = U('\n');
                            continue;
                        }
                    }
                }
                --source;
                break;
            }

            if ((tstart < (pctoken ? pctoken : target) && source < sentinel)
                && ((quote == U('\n')
                     && ((RJSMIN_IS_ID_LITERAL_CLOSE(*((pctoken ?
                                                        pctoken : target) - 1))
                          && RJSMIN_IS_ID_LITERAL_OPEN(*source))
                         || (post_regex
                             && RJSMIN_IS_POST_REGEX_OFF(*source)
                             && !(post_regex = 0))))
                    ||
                    (quote == U(' ') && !pctoken
                     && ((RJSMIN_IS_ID_LITERAL(*(target - 1))
                          && RJSMIN_IS_ID_LITERAL(*source))
                         || (source < sentinel
                             && ((*(target - 1) == U('+')
                                  && *source == U('+'))
                                 || (*(target - 1) == U('-')
                                     && *source == U('-')))))))) {
                *target++ = quote;
            }

            pcreset = NULL;
            spaced = quote;
        }
    cont:
        continue;
    }
    return (Py_ssize_t)(target - tstart);
}


PyDoc_STRVAR(rjsmin_jsmin__doc__,
"jsmin(script, keep_bang_comments=False)\n\
\n\
Minify javascript based on `jsmin.c by Douglas Crockford`_\\.\n\
\n\
Instead of parsing the stream char by char, it uses a regular\n\
expression approach which minifies the whole script with one big\n\
substitution regex.\n\
\n\
.. _jsmin.c by Douglas Crockford:\n\
   http://www.crockford.com/javascript/jsmin.c\n\
\n\
:Note: This is a hand crafted C implementation built on the regex\n\
       semantics.\n\
\n\
:Parameters:\n\
  `script` : ``str``\n\
    Script to minify\n\
\n\
  `keep_bang_comments` : ``bool``\n\
    Keep comments starting with an exclamation mark? (``/*!...*/``)\n\
\n\
:Return: Minified script\n\
:Rtype: ``str``");

static PyObject *
rjsmin_jsmin(PyObject *self, PyObject *args, PyObject *kwds)
{
    PyObject *script, *keep_bang_comments_ = NULL, *result;
    static char *kwlist[] = {"script", "keep_bang_comments", NULL};
    Py_ssize_t slength, length;
    int keep_bang_comments;
#ifdef EXT2
    int uni;
#define UOBJ "O"
#endif
#ifdef EXT3
#define UOBJ "U"
#endif

    if (!PyArg_ParseTupleAndKeywords(args, kwds, UOBJ "|O", kwlist,
                                     &script, &keep_bang_comments_))
        return NULL;

    if (!keep_bang_comments_)
        keep_bang_comments = 0;
    else {
        keep_bang_comments = PyObject_IsTrue(keep_bang_comments_);
        if (keep_bang_comments == -1)
            return NULL;
    }

#ifdef EXT2
    if (PyUnicode_Check(script)) {
        if (!(script = PyUnicode_AsUTF8String(script)))
            return NULL;
        uni = 1;
    }
    else {
        if (!(script = PyObject_Str(script)))
            return NULL;
        uni = 0;
    }
#endif

#ifdef EXT3
    Py_INCREF(script);
#define PyString_GET_SIZE PyUnicode_GET_SIZE
#define PyString_AS_STRING PyUnicode_AS_UNICODE
#define _PyString_Resize PyUnicode_Resize
#define PyString_FromStringAndSize PyUnicode_FromUnicode
#endif

    slength = PyString_GET_SIZE(script);
    if (!(result = PyString_FromStringAndSize(NULL, slength))) {
        Py_DECREF(script);
        return NULL;
    }
    Py_BEGIN_ALLOW_THREADS
    length = rjsmin((rchar *)PyString_AS_STRING(script),
                    (rchar *)PyString_AS_STRING(result),
                    slength, keep_bang_comments);
    Py_END_ALLOW_THREADS

    Py_DECREF(script);
    if (length < 0) {
        Py_DECREF(result);
        return NULL;
    }
    if (length != slength && _PyString_Resize(&result, length) == -1)
        return NULL;

#ifdef EXT2
    if (uni) {
        script = PyUnicode_DecodeUTF8(PyString_AS_STRING(result),
                                      PyString_GET_SIZE(result), "strict");
        Py_DECREF(result);
        if (!script)
            return NULL;
        result = script;
    }
#endif
    return result;
}

/* ------------------------ BEGIN MODULE DEFINITION ------------------------ */

EXT_METHODS = {
    {"jsmin",
        (PyCFunction)rjsmin_jsmin, METH_VARARGS | METH_KEYWORDS,
        rjsmin_jsmin__doc__},

    {NULL}  /* Sentinel */
};

PyDoc_STRVAR(EXT_DOCS_VAR,
"C implementation of rjsmin\n\
==========================\n\
\n\
C implementation of rjsmin.");


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
