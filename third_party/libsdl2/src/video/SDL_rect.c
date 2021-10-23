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
#include "../SDL_internal.h"

#include "SDL_rect.h"
#include "SDL_rect_c.h"

SDL_bool
SDL_HasIntersection(const SDL_Rect * A, const SDL_Rect * B)
{
    int Amin, Amax, Bmin, Bmax;

    if (!A) {
        SDL_InvalidParamError("A");
        return SDL_FALSE;
    }

    if (!B) {
        SDL_InvalidParamError("B");
        return SDL_FALSE;
    }

    /* Special cases for empty rects */
    if (SDL_RectEmpty(A) || SDL_RectEmpty(B)) {
        return SDL_FALSE;
    }

    /* Horizontal intersection */
    Amin = A->x;
    Amax = Amin + A->w;
    Bmin = B->x;
    Bmax = Bmin + B->w;
    if (Bmin > Amin)
        Amin = Bmin;
    if (Bmax < Amax)
        Amax = Bmax;
    if (Amax <= Amin)
        return SDL_FALSE;

    /* Vertical intersection */
    Amin = A->y;
    Amax = Amin + A->h;
    Bmin = B->y;
    Bmax = Bmin + B->h;
    if (Bmin > Amin)
        Amin = Bmin;
    if (Bmax < Amax)
        Amax = Bmax;
    if (Amax <= Amin)
        return SDL_FALSE;

    return SDL_TRUE;
}

SDL_bool
SDL_IntersectRect(const SDL_Rect * A, const SDL_Rect * B, SDL_Rect * result)
{
    int Amin, Amax, Bmin, Bmax;

    if (!A) {
        SDL_InvalidParamError("A");
        return SDL_FALSE;
    }

    if (!B) {
        SDL_InvalidParamError("B");
        return SDL_FALSE;
    }

    if (!result) {
        SDL_InvalidParamError("result");
        return SDL_FALSE;
    }

    /* Special cases for empty rects */
    if (SDL_RectEmpty(A) || SDL_RectEmpty(B)) {
        result->w = 0;
        result->h = 0;
        return SDL_FALSE;
    }

    /* Horizontal intersection */
    Amin = A->x;
    Amax = Amin + A->w;
    Bmin = B->x;
    Bmax = Bmin + B->w;
    if (Bmin > Amin)
        Amin = Bmin;
    result->x = Amin;
    if (Bmax < Amax)
        Amax = Bmax;
    result->w = Amax - Amin;

    /* Vertical intersection */
    Amin = A->y;
    Amax = Amin + A->h;
    Bmin = B->y;
    Bmax = Bmin + B->h;
    if (Bmin > Amin)
        Amin = Bmin;
    result->y = Amin;
    if (Bmax < Amax)
        Amax = Bmax;
    result->h = Amax - Amin;

    return !SDL_RectEmpty(result);
}

void
SDL_UnionRect(const SDL_Rect * A, const SDL_Rect * B, SDL_Rect * result)
{
    int Amin, Amax, Bmin, Bmax;

    if (!A) {
        SDL_InvalidParamError("A");
        return;
    }

    if (!B) {
        SDL_InvalidParamError("B");
        return;
    }

    if (!result) {
        SDL_InvalidParamError("result");
        return;
    }

    /* Special cases for empty Rects */
    if (SDL_RectEmpty(A)) {
      if (SDL_RectEmpty(B)) {
       /* A and B empty */
       return;
      } else {
       /* A empty, B not empty */
       *result = *B;
       return;
      }
    } else {
      if (SDL_RectEmpty(B)) {
       /* A not empty, B empty */
       *result = *A;
       return;
      }
    }

    /* Horizontal union */
    Amin = A->x;
    Amax = Amin + A->w;
    Bmin = B->x;
    Bmax = Bmin + B->w;
    if (Bmin < Amin)
        Amin = Bmin;
    result->x = Amin;
    if (Bmax > Amax)
        Amax = Bmax;
    result->w = Amax - Amin;

    /* Vertical union */
    Amin = A->y;
    Amax = Amin + A->h;
    Bmin = B->y;
    Bmax = Bmin + B->h;
    if (Bmin < Amin)
        Amin = Bmin;
    result->y = Amin;
    if (Bmax > Amax)
        Amax = Bmax;
    result->h = Amax - Amin;
}

SDL_bool
SDL_EnclosePoints(const SDL_Point * points, int count, const SDL_Rect * clip,
                  SDL_Rect * result)
{
    int minx = 0;
    int miny = 0;
    int maxx = 0;
    int maxy = 0;
    int x, y, i;

    if (!points) {
        SDL_InvalidParamError("points");
        return SDL_FALSE;
    }

    if (count < 1) {
        SDL_InvalidParamError("count");
        return SDL_FALSE;
    }

    if (clip) {
        SDL_bool added = SDL_FALSE;
        const int clip_minx = clip->x;
        const int clip_miny = clip->y;
        const int clip_maxx = clip->x+clip->w-1;
        const int clip_maxy = clip->y+clip->h-1;

        /* Special case for empty rectangle */
        if (SDL_RectEmpty(clip)) {
            return SDL_FALSE;
        }

        for (i = 0; i < count; ++i) {
            x = points[i].x;
            y = points[i].y;

            if (x < clip_minx || x > clip_maxx ||
                y < clip_miny || y > clip_maxy) {
                continue;
            }
            if (!added) {
                /* Special case: if no result was requested, we are done */
                if (result == NULL) {
                    return SDL_TRUE;
                }

                /* First point added */
                minx = maxx = x;
                miny = maxy = y;
                added = SDL_TRUE;
                continue;
            }
            if (x < minx) {
                minx = x;
            } else if (x > maxx) {
                maxx = x;
            }
            if (y < miny) {
                miny = y;
            } else if (y > maxy) {
                maxy = y;
            }
        }
        if (!added) {
            return SDL_FALSE;
        }
    } else {
        /* Special case: if no result was requested, we are done */
        if (result == NULL) {
            return SDL_TRUE;
        }

        /* No clipping, always add the first point */
        minx = maxx = points[0].x;
        miny = maxy = points[0].y;

        for (i = 1; i < count; ++i) {
            x = points[i].x;
            y = points[i].y;

            if (x < minx) {
                minx = x;
            } else if (x > maxx) {
                maxx = x;
            }
            if (y < miny) {
                miny = y;
            } else if (y > maxy) {
                maxy = y;
            }
        }
    }

    if (result) {
        result->x = minx;
        result->y = miny;
        result->w = (maxx-minx)+1;
        result->h = (maxy-miny)+1;
    }
    return SDL_TRUE;
}

/* Use the Cohen-Sutherland algorithm for line clipping */
#define CODE_BOTTOM 1
#define CODE_TOP    2
#define CODE_LEFT   4
#define CODE_RIGHT  8

static int
ComputeOutCode(const SDL_Rect * rect, int x, int y)
{
    int code = 0;
    if (y < rect->y) {
        code |= CODE_TOP;
    } else if (y >= rect->y + rect->h) {
        code |= CODE_BOTTOM;
    }
    if (x < rect->x) {
        code |= CODE_LEFT;
    } else if (x >= rect->x + rect->w) {
        code |= CODE_RIGHT;
    }
    return code;
}

SDL_bool
SDL_IntersectRectAndLine(const SDL_Rect * rect, int *X1, int *Y1, int *X2,
                         int *Y2)
{
    int x = 0;
    int y = 0;
    int x1, y1;
    int x2, y2;
    int rectx1;
    int recty1;
    int rectx2;
    int recty2;
    int outcode1, outcode2;

    if (!rect) {
        SDL_InvalidParamError("rect");
        return SDL_FALSE;
    }

    if (!X1) {
        SDL_InvalidParamError("X1");
        return SDL_FALSE;
    }

    if (!Y1) {
        SDL_InvalidParamError("Y1");
        return SDL_FALSE;
    }

    if (!X2) {
        SDL_InvalidParamError("X2");
        return SDL_FALSE;
    }

    if (!Y2) {
        SDL_InvalidParamError("Y2");
        return SDL_FALSE;
    }

    /* Special case for empty rect */
    if (SDL_RectEmpty(rect)) {
        return SDL_FALSE;
    }

    x1 = *X1;
    y1 = *Y1;
    x2 = *X2;
    y2 = *Y2;
    rectx1 = rect->x;
    recty1 = rect->y;
    rectx2 = rect->x + rect->w - 1;
    recty2 = rect->y + rect->h - 1;

    /* Check to see if entire line is inside rect */
    if (x1 >= rectx1 && x1 <= rectx2 && x2 >= rectx1 && x2 <= rectx2 &&
        y1 >= recty1 && y1 <= recty2 && y2 >= recty1 && y2 <= recty2) {
        return SDL_TRUE;
    }

    /* Check to see if entire line is to one side of rect */
    if ((x1 < rectx1 && x2 < rectx1) || (x1 > rectx2 && x2 > rectx2) ||
        (y1 < recty1 && y2 < recty1) || (y1 > recty2 && y2 > recty2)) {
        return SDL_FALSE;
    }

    if (y1 == y2) {
        /* Horizontal line, easy to clip */
        if (x1 < rectx1) {
            *X1 = rectx1;
        } else if (x1 > rectx2) {
            *X1 = rectx2;
        }
        if (x2 < rectx1) {
            *X2 = rectx1;
        } else if (x2 > rectx2) {
            *X2 = rectx2;
        }
        return SDL_TRUE;
    }

    if (x1 == x2) {
        /* Vertical line, easy to clip */
        if (y1 < recty1) {
            *Y1 = recty1;
        } else if (y1 > recty2) {
            *Y1 = recty2;
        }
        if (y2 < recty1) {
            *Y2 = recty1;
        } else if (y2 > recty2) {
            *Y2 = recty2;
        }
        return SDL_TRUE;
    }

    /* More complicated Cohen-Sutherland algorithm */
    outcode1 = ComputeOutCode(rect, x1, y1);
    outcode2 = ComputeOutCode(rect, x2, y2);
    while (outcode1 || outcode2) {
        if (outcode1 & outcode2) {
            return SDL_FALSE;
        }

        if (outcode1) {
            if (outcode1 & CODE_TOP) {
                y = recty1;
                x = x1 + ((x2 - x1) * (y - y1)) / (y2 - y1);
            } else if (outcode1 & CODE_BOTTOM) {
                y = recty2;
                x = x1 + ((x2 - x1) * (y - y1)) / (y2 - y1);
            } else if (outcode1 & CODE_LEFT) {
                x = rectx1;
                y = y1 + ((y2 - y1) * (x - x1)) / (x2 - x1);
            } else if (outcode1 & CODE_RIGHT) {
                x = rectx2;
                y = y1 + ((y2 - y1) * (x - x1)) / (x2 - x1);
            }
            x1 = x;
            y1 = y;
            outcode1 = ComputeOutCode(rect, x, y);
        } else {
            if (outcode2 & CODE_TOP) {
                y = recty1;
                x = x1 + ((x2 - x1) * (y - y1)) / (y2 - y1);
            } else if (outcode2 & CODE_BOTTOM) {
                y = recty2;
                x = x1 + ((x2 - x1) * (y - y1)) / (y2 - y1);
            } else if (outcode2 & CODE_LEFT) {
                /* If this assertion ever fires, here's the static analysis that warned about it:
                   http://buildbot.libsdl.org/sdl-static-analysis/sdl-macosx-static-analysis/sdl-macosx-static-analysis-1101/report-b0d01a.html#EndPath */
                SDL_assert(x2 != x1);  /* if equal: division by zero. */
                x = rectx1;
                y = y1 + ((y2 - y1) * (x - x1)) / (x2 - x1);
            } else if (outcode2 & CODE_RIGHT) {
                /* If this assertion ever fires, here's the static analysis that warned about it:
                   http://buildbot.libsdl.org/sdl-static-analysis/sdl-macosx-static-analysis/sdl-macosx-static-analysis-1101/report-39b114.html#EndPath */
                SDL_assert(x2 != x1);  /* if equal: division by zero. */
                x = rectx2;
                y = y1 + ((y2 - y1) * (x - x1)) / (x2 - x1);
            }
            x2 = x;
            y2 = y;
            outcode2 = ComputeOutCode(rect, x, y);
        }
    }
    *X1 = x1;
    *Y1 = y1;
    *X2 = x2;
    *Y2 = y2;
    return SDL_TRUE;
}

SDL_bool
SDL_GetSpanEnclosingRect(int width, int height,
                         int numrects, const SDL_Rect * rects, SDL_Rect *span)
{
    int i;
    int span_y1, span_y2;
    int rect_y1, rect_y2;

    if (width < 1) {
        SDL_InvalidParamError("width");
        return SDL_FALSE;
    }

    if (height < 1) {
        SDL_InvalidParamError("height");
        return SDL_FALSE;
    }

    if (!rects) {
        SDL_InvalidParamError("rects");
        return SDL_FALSE;
    }

    if (!span) {
        SDL_InvalidParamError("span");
        return SDL_FALSE;
    }

    if (numrects < 1) {
        SDL_InvalidParamError("numrects");
        return SDL_FALSE;
    }

    /* Initialize to empty rect */
    span_y1 = height;
    span_y2 = 0;

    for (i = 0; i < numrects; ++i) {
        rect_y1 = rects[i].y;
        rect_y2 = rect_y1 + rects[i].h;

        /* Clip out of bounds rectangles, and expand span rect */
        if (rect_y1 < 0) {
            span_y1 = 0;
        } else if (rect_y1 < span_y1) {
            span_y1 = rect_y1;
        }
        if (rect_y2 > height) {
            span_y2 = height;
        } else if (rect_y2 > span_y2) {
            span_y2 = rect_y2;
        }
    }
    if (span_y2 > span_y1) {
        span->x = 0;
        span->y = span_y1;
        span->w = width;
        span->h = (span_y2 - span_y1);
        return SDL_TRUE;
    }
    return SDL_FALSE;
}

/* vi: set ts=4 sw=4 expandtab: */
