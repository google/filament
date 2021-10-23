/**
 * Original code: automated SDL rect test written by Edgar Simo "bobbens"
 * New/updated tests: aschiffler at ferzkopp dot net
 */

#include <stdio.h>

#include "SDL.h"
#include "SDL_test.h"

/* ================= Test Case Implementation ================== */

/* Helper functions */

/* !
 * \brief Private helper to check SDL_IntersectRectAndLine results
 */
void _validateIntersectRectAndLineResults(
    SDL_bool intersection, SDL_bool expectedIntersection,
    SDL_Rect *rect, SDL_Rect * refRect,
    int x1, int y1, int x2, int y2,
    int x1Ref, int y1Ref, int x2Ref, int y2Ref)
{
    SDLTest_AssertCheck(intersection == expectedIntersection,
        "Check for correct intersection result: expected %s, got %s intersecting rect (%d,%d,%d,%d) with line (%d,%d - %d,%d)",
        (expectedIntersection == SDL_TRUE) ? "SDL_TRUE" : "SDL_FALSE",
        (intersection == SDL_TRUE) ? "SDL_TRUE" : "SDL_FALSE",
        refRect->x, refRect->y, refRect->w, refRect->h,
        x1Ref, y1Ref, x2Ref, y2Ref);
    SDLTest_AssertCheck(rect->x == refRect->x && rect->y == refRect->y && rect->w == refRect->w && rect->h == refRect->h,
        "Check that source rectangle was not modified: got (%d,%d,%d,%d) expected (%d,%d,%d,%d)",
        rect->x, rect->y, rect->w, rect->h,
        refRect->x, refRect->y, refRect->w, refRect->h);
    SDLTest_AssertCheck(x1 == x1Ref && y1 == y1Ref && x2 == x2Ref && y2 == y2Ref,
        "Check if line was incorrectly clipped or modified: got (%d,%d - %d,%d) expected (%d,%d - %d,%d)",
        x1, y1, x2, y2,
        x1Ref, y1Ref, x2Ref, y2Ref);
}

/* Test case functions */

/* !
 * \brief Tests SDL_IntersectRectAndLine() clipping cases
 *
 * \sa
 * http://wiki.libsdl.org/SDL_IntersectRectAndLine
 */
int
rect_testIntersectRectAndLine (void *arg)
{
    SDL_Rect refRect = { 0, 0, 32, 32 };
    SDL_Rect rect;
    int x1, y1;
    int x2, y2;
    SDL_bool intersected;

    int xLeft = -SDLTest_RandomIntegerInRange(1, refRect.w);
    int xRight = refRect.w + SDLTest_RandomIntegerInRange(1, refRect.w);
    int yTop = -SDLTest_RandomIntegerInRange(1, refRect.h);
    int yBottom = refRect.h + SDLTest_RandomIntegerInRange(1, refRect.h);

    x1 = xLeft;
    y1 = 15;
    x2 = xRight;
    y2 = 15;
    rect = refRect;
    intersected = SDL_IntersectRectAndLine(&rect, &x1, &y1, &x2, &y2);
    _validateIntersectRectAndLineResults(intersected, SDL_TRUE, &rect, &refRect, x1, y1, x2, y2, 0, 15, 31, 15);

    x1 = 15;
    y1 = yTop;
    x2 = 15;
    y2 = yBottom;
    rect = refRect;
    intersected = SDL_IntersectRectAndLine(&rect, &x1, &y1, &x2, &y2);
    _validateIntersectRectAndLineResults(intersected, SDL_TRUE, &rect, &refRect, x1, y1, x2, y2, 15, 0, 15, 31);

    x1 = -refRect.w;
    y1 = -refRect.h;
    x2 = 2*refRect.w;
    y2 = 2*refRect.h;
    rect = refRect;
    intersected = SDL_IntersectRectAndLine(&rect, &x1, &y1, &x2, &y2);
     _validateIntersectRectAndLineResults(intersected, SDL_TRUE, &rect, &refRect, x1, y1, x2, y2, 0, 0, 31, 31);

    x1 = 2*refRect.w;
    y1 = 2*refRect.h;
    x2 = -refRect.w;
    y2 = -refRect.h;
    rect = refRect;
    intersected = SDL_IntersectRectAndLine(&rect, &x1, &y1, &x2, &y2);
    _validateIntersectRectAndLineResults(intersected, SDL_TRUE, &rect, &refRect, x1, y1, x2, y2, 31, 31, 0, 0);

    x1 = -1;
    y1 = 32;
    x2 = 32;
    y2 = -1;
    rect = refRect;
    intersected = SDL_IntersectRectAndLine(&rect, &x1, &y1, &x2, &y2);
    _validateIntersectRectAndLineResults(intersected, SDL_TRUE, &rect, &refRect, x1, y1, x2, y2, 0, 31, 31, 0);

    x1 = 32;
    y1 = -1;
    x2 = -1;
    y2 = 32;
    rect = refRect;
    intersected = SDL_IntersectRectAndLine(&rect, &x1, &y1, &x2, &y2);
    _validateIntersectRectAndLineResults(intersected, SDL_TRUE, &rect, &refRect, x1, y1, x2, y2, 31, 0, 0, 31);

    return TEST_COMPLETED;
}

/* !
 * \brief Tests SDL_IntersectRectAndLine() non-clipping case line inside
 *
 * \sa
 * http://wiki.libsdl.org/SDL_IntersectRectAndLine
 */
int
rect_testIntersectRectAndLineInside (void *arg)
{
    SDL_Rect refRect = { 0, 0, 32, 32 };
    SDL_Rect rect;
    int x1, y1;
    int x2, y2;
    SDL_bool intersected;

    int xmin = refRect.x;
    int xmax = refRect.x + refRect.w - 1;
    int ymin = refRect.y;
    int ymax = refRect.y + refRect.h - 1;
    int x1Ref = SDLTest_RandomIntegerInRange(xmin + 1, xmax - 1);
    int y1Ref = SDLTest_RandomIntegerInRange(ymin + 1, ymax - 1);
    int x2Ref = SDLTest_RandomIntegerInRange(xmin + 1, xmax - 1);
    int y2Ref = SDLTest_RandomIntegerInRange(ymin + 1, ymax - 1);

    x1 = x1Ref;
    y1 = y1Ref;
    x2 = x2Ref;
    y2 = y2Ref;
    rect = refRect;
    intersected = SDL_IntersectRectAndLine(&rect, &x1, &y1, &x2, &y2);
    _validateIntersectRectAndLineResults(intersected, SDL_TRUE, &rect, &refRect, x1, y1, x2, y2, x1Ref, y1Ref, x2Ref, y2Ref);

    x1 = x1Ref;
    y1 = y1Ref;
    x2 = xmax;
    y2 = ymax;
    rect = refRect;
    intersected = SDL_IntersectRectAndLine(&rect, &x1, &y1, &x2, &y2);
    _validateIntersectRectAndLineResults(intersected, SDL_TRUE, &rect, &refRect, x1, y1, x2, y2, x1Ref, y1Ref, xmax, ymax);

    x1 = xmin;
    y1 = ymin;
    x2 = x2Ref;
    y2 = y2Ref;
    rect = refRect;
    intersected = SDL_IntersectRectAndLine(&rect, &x1, &y1, &x2, &y2);
    _validateIntersectRectAndLineResults(intersected, SDL_TRUE, &rect, &refRect, x1, y1, x2, y2, xmin, ymin, x2Ref, y2Ref);

    x1 = xmin;
    y1 = ymin;
    x2 = xmax;
    y2 = ymax;
    rect = refRect;
    intersected = SDL_IntersectRectAndLine(&rect, &x1, &y1, &x2, &y2);
    _validateIntersectRectAndLineResults(intersected, SDL_TRUE, &rect, &refRect, x1, y1, x2, y2, xmin, ymin, xmax, ymax);

    x1 = xmin;
    y1 = ymax;
    x2 = xmax;
    y2 = ymin;
    rect = refRect;
    intersected = SDL_IntersectRectAndLine(&rect, &x1, &y1, &x2, &y2);
    _validateIntersectRectAndLineResults(intersected, SDL_TRUE, &rect, &refRect, x1, y1, x2, y2, xmin, ymax, xmax, ymin);

    return TEST_COMPLETED;
}

/* !
 * \brief Tests SDL_IntersectRectAndLine() non-clipping cases outside
 *
 * \sa
 * http://wiki.libsdl.org/SDL_IntersectRectAndLine
 */
int
rect_testIntersectRectAndLineOutside (void *arg)
{
    SDL_Rect refRect = { 0, 0, 32, 32 };
    SDL_Rect rect;
    int x1, y1;
    int x2, y2;
    SDL_bool intersected;

    int xLeft = -SDLTest_RandomIntegerInRange(1, refRect.w);
    int xRight = refRect.w + SDLTest_RandomIntegerInRange(1, refRect.w);
    int yTop = -SDLTest_RandomIntegerInRange(1, refRect.h);
    int yBottom = refRect.h + SDLTest_RandomIntegerInRange(1, refRect.h);

    x1 = xLeft;
    y1 = 0;
    x2 = xLeft;
    y2 = 31;
    rect = refRect;
    intersected = SDL_IntersectRectAndLine(&rect, &x1, &y1, &x2, &y2);
    _validateIntersectRectAndLineResults(intersected, SDL_FALSE, &rect, &refRect, x1, y1, x2, y2, xLeft, 0, xLeft, 31);

    x1 = xRight;
    y1 = 0;
    x2 = xRight;
    y2 = 31;
    rect = refRect;
    intersected = SDL_IntersectRectAndLine(&rect, &x1, &y1, &x2, &y2);
    _validateIntersectRectAndLineResults(intersected, SDL_FALSE, &rect, &refRect, x1, y1, x2, y2, xRight, 0, xRight, 31);

    x1 = 0;
    y1 = yTop;
    x2 = 31;
    y2 = yTop;
    rect = refRect;
    intersected = SDL_IntersectRectAndLine(&rect, &x1, &y1, &x2, &y2);
    _validateIntersectRectAndLineResults(intersected, SDL_FALSE, &rect, &refRect, x1, y1, x2, y2, 0, yTop, 31, yTop);

    x1 = 0;
    y1 = yBottom;
    x2 = 31;
    y2 = yBottom;
    rect = refRect;
    intersected = SDL_IntersectRectAndLine(&rect, &x1, &y1, &x2, &y2);
    _validateIntersectRectAndLineResults(intersected, SDL_FALSE, &rect, &refRect, x1, y1, x2, y2, 0, yBottom, 31, yBottom);

    return TEST_COMPLETED;
}

/* !
 * \brief Tests SDL_IntersectRectAndLine() with empty rectangle
 *
 * \sa
 * http://wiki.libsdl.org/SDL_IntersectRectAndLine
 */
int
rect_testIntersectRectAndLineEmpty (void *arg)
{
    SDL_Rect refRect;
    SDL_Rect rect;
    int x1, y1, x1Ref, y1Ref;
    int x2, y2, x2Ref, y2Ref;
    SDL_bool intersected;

    refRect.x = SDLTest_RandomIntegerInRange(1, 1024);
    refRect.y = SDLTest_RandomIntegerInRange(1, 1024);
    refRect.w = 0;
    refRect.h = 0;
    x1Ref = refRect.x;
    y1Ref = refRect.y;
    x2Ref = SDLTest_RandomIntegerInRange(1, 1024);
    y2Ref = SDLTest_RandomIntegerInRange(1, 1024);

    x1 = x1Ref;
    y1 = y1Ref;
    x2 = x2Ref;
    y2 = y2Ref;
    rect = refRect;
    intersected = SDL_IntersectRectAndLine(&rect, &x1, &y1, &x2, &y2);
    _validateIntersectRectAndLineResults(intersected, SDL_FALSE, &rect, &refRect, x1, y1, x2, y2, x1Ref, y1Ref, x2Ref, y2Ref);

    return TEST_COMPLETED;
}

/* !
 * \brief Negative tests against SDL_IntersectRectAndLine() with invalid parameters
 *
 * \sa
 * http://wiki.libsdl.org/SDL_IntersectRectAndLine
 */
int
rect_testIntersectRectAndLineParam (void *arg)
{
    SDL_Rect rect = { 0, 0, 32, 32 };
    int x1 = rect.w / 2;
    int y1 = rect.h / 2;
    int x2 = x1;
    int y2 = 2 * rect.h;
    SDL_bool intersected;

    intersected = SDL_IntersectRectAndLine(&rect, &x1, &y1, &x2, &y2);
    SDLTest_AssertCheck(intersected == SDL_TRUE, "Check that intersection result was SDL_TRUE");

    intersected = SDL_IntersectRectAndLine((SDL_Rect *)NULL, &x1, &y1, &x2, &y2);
    SDLTest_AssertCheck(intersected == SDL_FALSE, "Check that function returns SDL_FALSE when 1st parameter is NULL");
    intersected = SDL_IntersectRectAndLine(&rect, (int *)NULL, &y1, &x2, &y2);
    SDLTest_AssertCheck(intersected == SDL_FALSE, "Check that function returns SDL_FALSE when 2nd parameter is NULL");
    intersected = SDL_IntersectRectAndLine(&rect, &x1, (int *)NULL, &x2, &y2);
    SDLTest_AssertCheck(intersected == SDL_FALSE, "Check that function returns SDL_FALSE when 3rd parameter is NULL");
    intersected = SDL_IntersectRectAndLine(&rect, &x1, &y1, (int *)NULL, &y2);
    SDLTest_AssertCheck(intersected == SDL_FALSE, "Check that function returns SDL_FALSE when 4th parameter is NULL");
    intersected = SDL_IntersectRectAndLine(&rect, &x1, &y1, &x2, (int *)NULL);
    SDLTest_AssertCheck(intersected == SDL_FALSE, "Check that function returns SDL_FALSE when 5th parameter is NULL");
    intersected = SDL_IntersectRectAndLine((SDL_Rect *)NULL, (int *)NULL, (int *)NULL, (int *)NULL, (int *)NULL);
    SDLTest_AssertCheck(intersected == SDL_FALSE, "Check that function returns SDL_FALSE when all parameters are NULL");

    return TEST_COMPLETED;
}

/* !
 * \brief Private helper to check SDL_HasIntersection results
 */
void _validateHasIntersectionResults(
    SDL_bool intersection, SDL_bool expectedIntersection,
    SDL_Rect *rectA, SDL_Rect *rectB, SDL_Rect *refRectA, SDL_Rect *refRectB)
{
    SDLTest_AssertCheck(intersection == expectedIntersection,
        "Check intersection result: expected %s, got %s intersecting A (%d,%d,%d,%d) with B (%d,%d,%d,%d)",
        (expectedIntersection == SDL_TRUE) ? "SDL_TRUE" : "SDL_FALSE",
        (intersection == SDL_TRUE) ? "SDL_TRUE" : "SDL_FALSE",
        rectA->x, rectA->y, rectA->w, rectA->h,
        rectB->x, rectB->y, rectB->w, rectB->h);
    SDLTest_AssertCheck(rectA->x == refRectA->x && rectA->y == refRectA->y && rectA->w == refRectA->w && rectA->h == refRectA->h,
        "Check that source rectangle A was not modified: got (%d,%d,%d,%d) expected (%d,%d,%d,%d)",
        rectA->x, rectA->y, rectA->w, rectA->h,
        refRectA->x, refRectA->y, refRectA->w, refRectA->h);
    SDLTest_AssertCheck(rectB->x == refRectB->x && rectB->y == refRectB->y && rectB->w == refRectB->w && rectB->h == refRectB->h,
        "Check that source rectangle B was not modified: got (%d,%d,%d,%d) expected (%d,%d,%d,%d)",
        rectB->x, rectB->y, rectB->w, rectB->h,
        refRectB->x, refRectB->y, refRectB->w, refRectB->h);
}

/* !
 * \brief Private helper to check SDL_IntersectRect results
 */
void _validateIntersectRectResults(
    SDL_bool intersection, SDL_bool expectedIntersection,
    SDL_Rect *rectA, SDL_Rect *rectB, SDL_Rect *refRectA, SDL_Rect *refRectB,
    SDL_Rect *result, SDL_Rect *expectedResult)
{
    _validateHasIntersectionResults(intersection, expectedIntersection, rectA, rectB, refRectA, refRectB);
    if (result && expectedResult) {
        SDLTest_AssertCheck(result->x == expectedResult->x && result->y == expectedResult->y && result->w == expectedResult->w && result->h == expectedResult->h,
            "Check that intersection of rectangles A (%d,%d,%d,%d) and B (%d,%d,%d,%d) was correctly calculated, got (%d,%d,%d,%d) expected (%d,%d,%d,%d)",
            rectA->x, rectA->y, rectA->w, rectA->h,
            rectB->x, rectB->y, rectB->w, rectB->h,
            result->x, result->y, result->w, result->h,
            expectedResult->x, expectedResult->y, expectedResult->w, expectedResult->h);
    }
}

/* !
 * \brief Private helper to check SDL_UnionRect results
 */
void _validateUnionRectResults(
    SDL_Rect *rectA, SDL_Rect *rectB, SDL_Rect *refRectA, SDL_Rect *refRectB,
    SDL_Rect *result, SDL_Rect *expectedResult)
{
    SDLTest_AssertCheck(rectA->x == refRectA->x && rectA->y == refRectA->y && rectA->w == refRectA->w && rectA->h == refRectA->h,
        "Check that source rectangle A was not modified: got (%d,%d,%d,%d) expected (%d,%d,%d,%d)",
        rectA->x, rectA->y, rectA->w, rectA->h,
        refRectA->x, refRectA->y, refRectA->w, refRectA->h);
    SDLTest_AssertCheck(rectB->x == refRectB->x && rectB->y == refRectB->y && rectB->w == refRectB->w && rectB->h == refRectB->h,
        "Check that source rectangle B was not modified: got (%d,%d,%d,%d) expected (%d,%d,%d,%d)",
        rectB->x, rectB->y, rectB->w, rectB->h,
        refRectB->x, refRectB->y, refRectB->w, refRectB->h);
    SDLTest_AssertCheck(result->x == expectedResult->x && result->y == expectedResult->y && result->w == expectedResult->w && result->h == expectedResult->h,
        "Check that union of rectangles A (%d,%d,%d,%d) and B (%d,%d,%d,%d) was correctly calculated, got (%d,%d,%d,%d) expected (%d,%d,%d,%d)",
        rectA->x, rectA->y, rectA->w, rectA->h,
        rectB->x, rectB->y, rectB->w, rectB->h,
        result->x, result->y, result->w, result->h,
        expectedResult->x, expectedResult->y, expectedResult->w, expectedResult->h);
}

/* !
 * \brief Private helper to check SDL_RectEmpty results
 */
void _validateRectEmptyResults(
    SDL_bool empty, SDL_bool expectedEmpty,
    SDL_Rect *rect, SDL_Rect *refRect)
{
    SDLTest_AssertCheck(empty == expectedEmpty,
        "Check for correct empty result: expected %s, got %s testing (%d,%d,%d,%d)",
        (expectedEmpty == SDL_TRUE) ? "SDL_TRUE" : "SDL_FALSE",
        (empty == SDL_TRUE) ? "SDL_TRUE" : "SDL_FALSE",
        rect->x, rect->y, rect->w, rect->h);
    SDLTest_AssertCheck(rect->x == refRect->x && rect->y == refRect->y && rect->w == refRect->w && rect->h == refRect->h,
        "Check that source rectangle was not modified: got (%d,%d,%d,%d) expected (%d,%d,%d,%d)",
        rect->x, rect->y, rect->w, rect->h,
        refRect->x, refRect->y, refRect->w, refRect->h);
}

/* !
 * \brief Private helper to check SDL_RectEquals results
 */
void _validateRectEqualsResults(
    SDL_bool equals, SDL_bool expectedEquals,
    SDL_Rect *rectA, SDL_Rect *rectB, SDL_Rect *refRectA, SDL_Rect *refRectB)
{
    SDLTest_AssertCheck(equals == expectedEquals,
        "Check for correct equals result: expected %s, got %s testing (%d,%d,%d,%d) and (%d,%d,%d,%d)",
        (expectedEquals == SDL_TRUE) ? "SDL_TRUE" : "SDL_FALSE",
        (equals == SDL_TRUE) ? "SDL_TRUE" : "SDL_FALSE",
        rectA->x, rectA->y, rectA->w, rectA->h,
        rectB->x, rectB->y, rectB->w, rectB->h);
    SDLTest_AssertCheck(rectA->x == refRectA->x && rectA->y == refRectA->y && rectA->w == refRectA->w && rectA->h == refRectA->h,
        "Check that source rectangle A was not modified: got (%d,%d,%d,%d) expected (%d,%d,%d,%d)",
        rectA->x, rectA->y, rectA->w, rectA->h,
        refRectA->x, refRectA->y, refRectA->w, refRectA->h);
    SDLTest_AssertCheck(rectB->x == refRectB->x && rectB->y == refRectB->y && rectB->w == refRectB->w && rectB->h == refRectB->h,
        "Check that source rectangle B was not modified: got (%d,%d,%d,%d) expected (%d,%d,%d,%d)",
        rectB->x, rectB->y, rectB->w, rectB->h,
        refRectB->x, refRectB->y, refRectB->w, refRectB->h);
}

/* !
 * \brief Tests SDL_IntersectRect() with B fully inside A
 *
 * \sa
 * http://wiki.libsdl.org/SDL_IntersectRect
 */
int rect_testIntersectRectInside (void *arg)
{
    SDL_Rect refRectA = { 0, 0, 32, 32 };
    SDL_Rect refRectB;
    SDL_Rect rectA;
    SDL_Rect rectB;
    SDL_Rect result;
    SDL_bool intersection;

    /* rectB fully contained in rectA */
    refRectB.x = 0;
    refRectB.y = 0;
    refRectB.w = SDLTest_RandomIntegerInRange(refRectA.x + 1, refRectA.x + refRectA.w - 1);
    refRectB.h = SDLTest_RandomIntegerInRange(refRectA.y + 1, refRectA.y + refRectA.h - 1);
    rectA = refRectA;
    rectB = refRectB;
    intersection = SDL_IntersectRect(&rectA, &rectB, &result);
    _validateIntersectRectResults(intersection, SDL_TRUE, &rectA, &rectB, &refRectA, &refRectB, &result, &refRectB);

    return TEST_COMPLETED;
}

/* !
 * \brief Tests SDL_IntersectRect() with B fully outside A
 *
 * \sa
 * http://wiki.libsdl.org/SDL_IntersectRect
 */
int rect_testIntersectRectOutside (void *arg)
{
    SDL_Rect refRectA = { 0, 0, 32, 32 };
    SDL_Rect refRectB;
    SDL_Rect rectA;
    SDL_Rect rectB;
    SDL_Rect result;
    SDL_bool intersection;

    /* rectB fully outside of rectA */
    refRectB.x = refRectA.x + refRectA.w + SDLTest_RandomIntegerInRange(1, 10);
    refRectB.y = refRectA.y + refRectA.h + SDLTest_RandomIntegerInRange(1, 10);
    refRectB.w = refRectA.w;
    refRectB.h = refRectA.h;
    rectA = refRectA;
    rectB = refRectB;
    intersection = SDL_IntersectRect(&rectA, &rectB, &result);
    _validateIntersectRectResults(intersection, SDL_FALSE, &rectA, &rectB, &refRectA, &refRectB, (SDL_Rect *)NULL, (SDL_Rect *)NULL);

    return TEST_COMPLETED;
}

/* !
 * \brief Tests SDL_IntersectRect() with B partially intersecting A
 *
 * \sa
 * http://wiki.libsdl.org/SDL_IntersectRect
 */
int rect_testIntersectRectPartial (void *arg)
{
    SDL_Rect refRectA = { 0, 0, 32, 32 };
    SDL_Rect refRectB;
    SDL_Rect rectA;
    SDL_Rect rectB;
    SDL_Rect result;
    SDL_Rect expectedResult;
    SDL_bool intersection;

    /* rectB partially contained in rectA */
    refRectB.x = SDLTest_RandomIntegerInRange(refRectA.x + 1, refRectA.x + refRectA.w - 1);
    refRectB.y = SDLTest_RandomIntegerInRange(refRectA.y + 1, refRectA.y + refRectA.h - 1);
    refRectB.w = refRectA.w;
    refRectB.h = refRectA.h;
    rectA = refRectA;
    rectB = refRectB;
    expectedResult.x = refRectB.x;
    expectedResult.y = refRectB.y;
    expectedResult.w = refRectA.w - refRectB.x;
    expectedResult.h = refRectA.h - refRectB.y;
    intersection = SDL_IntersectRect(&rectA, &rectB, &result);
    _validateIntersectRectResults(intersection, SDL_TRUE, &rectA, &rectB, &refRectA, &refRectB, &result, &expectedResult);

    /* rectB right edge */
    refRectB.x = rectA.w - 1;
    refRectB.y = rectA.y;
    refRectB.w = SDLTest_RandomIntegerInRange(1, refRectA.w - 1);
    refRectB.h = SDLTest_RandomIntegerInRange(1, refRectA.h - 1);
    rectA = refRectA;
    rectB = refRectB;
    expectedResult.x = refRectB.x;
    expectedResult.y = refRectB.y;
    expectedResult.w = 1;
    expectedResult.h = refRectB.h;
    intersection = SDL_IntersectRect(&rectA, &rectB, &result);
    _validateIntersectRectResults(intersection, SDL_TRUE, &rectA, &rectB, &refRectA, &refRectB, &result, &expectedResult);

    /* rectB left edge */
    refRectB.x = 1 - rectA.w;
    refRectB.y = rectA.y;
    refRectB.w = refRectA.w;
    refRectB.h = SDLTest_RandomIntegerInRange(1, refRectA.h - 1);
    rectA = refRectA;
    rectB = refRectB;
    expectedResult.x = 0;
    expectedResult.y = refRectB.y;
    expectedResult.w = 1;
    expectedResult.h = refRectB.h;
    intersection = SDL_IntersectRect(&rectA, &rectB, &result);
    _validateIntersectRectResults(intersection, SDL_TRUE, &rectA, &rectB, &refRectA, &refRectB, &result, &expectedResult);

    /* rectB bottom edge */
    refRectB.x = rectA.x;
    refRectB.y = rectA.h - 1;
    refRectB.w = SDLTest_RandomIntegerInRange(1, refRectA.w - 1);
    refRectB.h = SDLTest_RandomIntegerInRange(1, refRectA.h - 1);
    rectA = refRectA;
    rectB = refRectB;
    expectedResult.x = refRectB.x;
    expectedResult.y = refRectB.y;
    expectedResult.w = refRectB.w;
    expectedResult.h = 1;
    intersection = SDL_IntersectRect(&rectA, &rectB, &result);
    _validateIntersectRectResults(intersection, SDL_TRUE, &rectA, &rectB, &refRectA, &refRectB, &result, &expectedResult);

    /* rectB top edge */
    refRectB.x = rectA.x;
    refRectB.y = 1 - rectA.h;
    refRectB.w = SDLTest_RandomIntegerInRange(1, refRectA.w - 1);
    refRectB.h = rectA.h;
    rectA = refRectA;
    rectB = refRectB;
    expectedResult.x = refRectB.x;
    expectedResult.y = 0;
    expectedResult.w = refRectB.w;
    expectedResult.h = 1;
    intersection = SDL_IntersectRect(&rectA, &rectB, &result);
    _validateIntersectRectResults(intersection, SDL_TRUE, &rectA, &rectB, &refRectA, &refRectB, &result, &expectedResult);

    return TEST_COMPLETED;
}

/* !
 * \brief Tests SDL_IntersectRect() with 1x1 pixel sized rectangles
 *
 * \sa
 * http://wiki.libsdl.org/SDL_IntersectRect
 */
int rect_testIntersectRectPoint (void *arg)
{
    SDL_Rect refRectA = { 0, 0, 1, 1 };
    SDL_Rect refRectB = { 0, 0, 1, 1 };
    SDL_Rect rectA;
    SDL_Rect rectB;
    SDL_Rect result;
    SDL_bool intersection;
    int offsetX, offsetY;

    /* intersecting pixels */
    refRectA.x = SDLTest_RandomIntegerInRange(1, 100);
    refRectA.y = SDLTest_RandomIntegerInRange(1, 100);
    refRectB.x = refRectA.x;
    refRectB.y = refRectA.y;
    rectA = refRectA;
    rectB = refRectB;
    intersection = SDL_IntersectRect(&rectA, &rectB, &result);
    _validateIntersectRectResults(intersection, SDL_TRUE, &rectA, &rectB, &refRectA, &refRectB, &result, &refRectA);

    /* non-intersecting pixels cases */
    for (offsetX = -1; offsetX <= 1; offsetX++) {
        for (offsetY = -1; offsetY <= 1; offsetY++) {
            if (offsetX != 0 || offsetY != 0) {
                refRectA.x = SDLTest_RandomIntegerInRange(1, 100);
                refRectA.y = SDLTest_RandomIntegerInRange(1, 100);
                refRectB.x = refRectA.x;
                refRectB.y = refRectA.y;
                refRectB.x += offsetX;
                refRectB.y += offsetY;
                rectA = refRectA;
                rectB = refRectB;
                intersection = SDL_IntersectRect(&rectA, &rectB, &result);
                _validateIntersectRectResults(intersection, SDL_FALSE, &rectA, &rectB, &refRectA, &refRectB, (SDL_Rect *)NULL, (SDL_Rect *)NULL);
            }
        }
    }

    return TEST_COMPLETED;
}

/* !
 * \brief Tests SDL_IntersectRect() with empty rectangles
 *
 * \sa
 * http://wiki.libsdl.org/SDL_IntersectRect
 */
int rect_testIntersectRectEmpty (void *arg)
{
    SDL_Rect refRectA;
    SDL_Rect refRectB;
    SDL_Rect rectA;
    SDL_Rect rectB;
    SDL_Rect result;
    SDL_bool intersection;
    SDL_bool empty;

    /* Rect A empty */
    result.w = SDLTest_RandomIntegerInRange(1, 100);
    result.h = SDLTest_RandomIntegerInRange(1, 100);
    refRectA.x = SDLTest_RandomIntegerInRange(1, 100);
    refRectA.y = SDLTest_RandomIntegerInRange(1, 100);
    refRectA.w = SDLTest_RandomIntegerInRange(1, 100);
    refRectA.h = SDLTest_RandomIntegerInRange(1, 100);
    refRectB = refRectA;
    refRectA.w = 0;
    refRectA.h = 0;
    rectA = refRectA;
    rectB = refRectB;
    intersection = SDL_IntersectRect(&rectA, &rectB, &result);
    _validateIntersectRectResults(intersection, SDL_FALSE, &rectA, &rectB, &refRectA, &refRectB, (SDL_Rect *)NULL, (SDL_Rect *)NULL);
    empty = (SDL_bool)SDL_RectEmpty(&result);
    SDLTest_AssertCheck(empty == SDL_TRUE, "Validate result is empty Rect; got: %s", (empty == SDL_TRUE) ? "SDL_TRUE" : "SDL_FALSE");

    /* Rect B empty */
    result.w = SDLTest_RandomIntegerInRange(1, 100);
    result.h = SDLTest_RandomIntegerInRange(1, 100);
    refRectA.x = SDLTest_RandomIntegerInRange(1, 100);
    refRectA.y = SDLTest_RandomIntegerInRange(1, 100);
    refRectA.w = SDLTest_RandomIntegerInRange(1, 100);
    refRectA.h = SDLTest_RandomIntegerInRange(1, 100);
    refRectB = refRectA;
    refRectB.w = 0;
    refRectB.h = 0;
    rectA = refRectA;
    rectB = refRectB;
    intersection = SDL_IntersectRect(&rectA, &rectB, &result);
    _validateIntersectRectResults(intersection, SDL_FALSE, &rectA, &rectB, &refRectA, &refRectB, (SDL_Rect *)NULL, (SDL_Rect *)NULL);
    empty = (SDL_bool)SDL_RectEmpty(&result);
    SDLTest_AssertCheck(empty == SDL_TRUE, "Validate result is empty Rect; got: %s", (empty == SDL_TRUE) ? "SDL_TRUE" : "SDL_FALSE");

    /* Rect A and B empty */
    result.w = SDLTest_RandomIntegerInRange(1, 100);
    result.h = SDLTest_RandomIntegerInRange(1, 100);
    refRectA.x = SDLTest_RandomIntegerInRange(1, 100);
    refRectA.y = SDLTest_RandomIntegerInRange(1, 100);
    refRectA.w = SDLTest_RandomIntegerInRange(1, 100);
    refRectA.h = SDLTest_RandomIntegerInRange(1, 100);
    refRectB = refRectA;
    refRectA.w = 0;
    refRectA.h = 0;
    refRectB.w = 0;
    refRectB.h = 0;
    rectA = refRectA;
    rectB = refRectB;
    intersection = SDL_IntersectRect(&rectA, &rectB, &result);
    _validateIntersectRectResults(intersection, SDL_FALSE, &rectA, &rectB, &refRectA, &refRectB, (SDL_Rect *)NULL, (SDL_Rect *)NULL);
    empty = (SDL_bool)SDL_RectEmpty(&result);
    SDLTest_AssertCheck(empty == SDL_TRUE, "Validate result is empty Rect; got: %s", (empty == SDL_TRUE) ? "SDL_TRUE" : "SDL_FALSE");

    return TEST_COMPLETED;
}

/* !
 * \brief Negative tests against SDL_IntersectRect() with invalid parameters
 *
 * \sa
 * http://wiki.libsdl.org/SDL_IntersectRect
 */
int rect_testIntersectRectParam(void *arg)
{
    SDL_Rect rectA;
    SDL_Rect rectB;
    SDL_Rect result;
    SDL_bool intersection;

    /* invalid parameter combinations */
    intersection = SDL_IntersectRect((SDL_Rect *)NULL, &rectB, &result);
    SDLTest_AssertCheck(intersection == SDL_FALSE, "Check that function returns SDL_FALSE when 1st parameter is NULL");
    intersection = SDL_IntersectRect(&rectA, (SDL_Rect *)NULL, &result);
    SDLTest_AssertCheck(intersection == SDL_FALSE, "Check that function returns SDL_FALSE when 2st parameter is NULL");
    intersection = SDL_IntersectRect(&rectA, &rectB, (SDL_Rect *)NULL);
    SDLTest_AssertCheck(intersection == SDL_FALSE, "Check that function returns SDL_FALSE when 3st parameter is NULL");
    intersection = SDL_IntersectRect((SDL_Rect *)NULL, (SDL_Rect *)NULL, &result);
    SDLTest_AssertCheck(intersection == SDL_FALSE, "Check that function returns SDL_FALSE when 1st and 2nd parameters are NULL");
    intersection = SDL_IntersectRect((SDL_Rect *)NULL, &rectB, (SDL_Rect *)NULL);
    SDLTest_AssertCheck(intersection == SDL_FALSE, "Check that function returns SDL_FALSE when 1st and 3rd parameters are NULL ");
    intersection = SDL_IntersectRect((SDL_Rect *)NULL, (SDL_Rect *)NULL, (SDL_Rect *)NULL);
    SDLTest_AssertCheck(intersection == SDL_FALSE, "Check that function returns SDL_FALSE when all parameters are NULL");

    return TEST_COMPLETED;
}

/* !
 * \brief Tests SDL_HasIntersection() with B fully inside A
 *
 * \sa
 * http://wiki.libsdl.org/SDL_HasIntersection
 */
int rect_testHasIntersectionInside (void *arg)
{
    SDL_Rect refRectA = { 0, 0, 32, 32 };
    SDL_Rect refRectB;
    SDL_Rect rectA;
    SDL_Rect rectB;
    SDL_bool intersection;

    /* rectB fully contained in rectA */
    refRectB.x = 0;
    refRectB.y = 0;
    refRectB.w = SDLTest_RandomIntegerInRange(refRectA.x + 1, refRectA.x + refRectA.w - 1);
    refRectB.h = SDLTest_RandomIntegerInRange(refRectA.y + 1, refRectA.y + refRectA.h - 1);
    rectA = refRectA;
    rectB = refRectB;
    intersection = SDL_HasIntersection(&rectA, &rectB);
    _validateHasIntersectionResults(intersection, SDL_TRUE, &rectA, &rectB, &refRectA, &refRectB);

    return TEST_COMPLETED;
}

/* !
 * \brief Tests SDL_HasIntersection() with B fully outside A
 *
 * \sa
 * http://wiki.libsdl.org/SDL_HasIntersection
 */
int rect_testHasIntersectionOutside (void *arg)
{
    SDL_Rect refRectA = { 0, 0, 32, 32 };
    SDL_Rect refRectB;
    SDL_Rect rectA;
    SDL_Rect rectB;
    SDL_bool intersection;

    /* rectB fully outside of rectA */
    refRectB.x = refRectA.x + refRectA.w + SDLTest_RandomIntegerInRange(1, 10);
    refRectB.y = refRectA.y + refRectA.h + SDLTest_RandomIntegerInRange(1, 10);
    refRectB.w = refRectA.w;
    refRectB.h = refRectA.h;
    rectA = refRectA;
    rectB = refRectB;
    intersection = SDL_HasIntersection(&rectA, &rectB);
    _validateHasIntersectionResults(intersection, SDL_FALSE, &rectA, &rectB, &refRectA, &refRectB);

    return TEST_COMPLETED;
}

/* !
 * \brief Tests SDL_HasIntersection() with B partially intersecting A
 *
 * \sa
 * http://wiki.libsdl.org/SDL_HasIntersection
 */
int rect_testHasIntersectionPartial (void *arg)
{
    SDL_Rect refRectA = { 0, 0, 32, 32 };
    SDL_Rect refRectB;
    SDL_Rect rectA;
    SDL_Rect rectB;
    SDL_bool intersection;

    /* rectB partially contained in rectA */
    refRectB.x = SDLTest_RandomIntegerInRange(refRectA.x + 1, refRectA.x + refRectA.w - 1);
    refRectB.y = SDLTest_RandomIntegerInRange(refRectA.y + 1, refRectA.y + refRectA.h - 1);
    refRectB.w = refRectA.w;
    refRectB.h = refRectA.h;
    rectA = refRectA;
    rectB = refRectB;
    intersection = SDL_HasIntersection(&rectA, &rectB);
    _validateHasIntersectionResults(intersection, SDL_TRUE, &rectA, &rectB, &refRectA, &refRectB);

    /* rectB right edge */
    refRectB.x = rectA.w - 1;
    refRectB.y = rectA.y;
    refRectB.w = SDLTest_RandomIntegerInRange(1, refRectA.w - 1);
    refRectB.h = SDLTest_RandomIntegerInRange(1, refRectA.h - 1);
    rectA = refRectA;
    rectB = refRectB;
    intersection = SDL_HasIntersection(&rectA, &rectB);
    _validateHasIntersectionResults(intersection, SDL_TRUE, &rectA, &rectB, &refRectA, &refRectB);

    /* rectB left edge */
    refRectB.x = 1 - rectA.w;
    refRectB.y = rectA.y;
    refRectB.w = refRectA.w;
    refRectB.h = SDLTest_RandomIntegerInRange(1, refRectA.h - 1);
    rectA = refRectA;
    rectB = refRectB;
    intersection = SDL_HasIntersection(&rectA, &rectB);
    _validateHasIntersectionResults(intersection, SDL_TRUE, &rectA, &rectB, &refRectA, &refRectB);

    /* rectB bottom edge */
    refRectB.x = rectA.x;
    refRectB.y = rectA.h - 1;
    refRectB.w = SDLTest_RandomIntegerInRange(1, refRectA.w - 1);
    refRectB.h = SDLTest_RandomIntegerInRange(1, refRectA.h - 1);
    rectA = refRectA;
    rectB = refRectB;
    intersection = SDL_HasIntersection(&rectA, &rectB);
    _validateHasIntersectionResults(intersection, SDL_TRUE, &rectA, &rectB, &refRectA, &refRectB);

    /* rectB top edge */
    refRectB.x = rectA.x;
    refRectB.y = 1 - rectA.h;
    refRectB.w = SDLTest_RandomIntegerInRange(1, refRectA.w - 1);
    refRectB.h = rectA.h;
    rectA = refRectA;
    rectB = refRectB;
    intersection = SDL_HasIntersection(&rectA, &rectB);
    _validateHasIntersectionResults(intersection, SDL_TRUE, &rectA, &rectB, &refRectA, &refRectB);

    return TEST_COMPLETED;
}

/* !
 * \brief Tests SDL_HasIntersection() with 1x1 pixel sized rectangles
 *
 * \sa
 * http://wiki.libsdl.org/SDL_HasIntersection
 */
int rect_testHasIntersectionPoint (void *arg)
{
    SDL_Rect refRectA = { 0, 0, 1, 1 };
    SDL_Rect refRectB = { 0, 0, 1, 1 };
    SDL_Rect rectA;
    SDL_Rect rectB;
    SDL_bool intersection;
    int offsetX, offsetY;

    /* intersecting pixels */
    refRectA.x = SDLTest_RandomIntegerInRange(1, 100);
    refRectA.y = SDLTest_RandomIntegerInRange(1, 100);
    refRectB.x = refRectA.x;
    refRectB.y = refRectA.y;
    rectA = refRectA;
    rectB = refRectB;
    intersection = SDL_HasIntersection(&rectA, &rectB);
    _validateHasIntersectionResults(intersection, SDL_TRUE, &rectA, &rectB, &refRectA, &refRectB);

    /* non-intersecting pixels cases */
    for (offsetX = -1; offsetX <= 1; offsetX++) {
        for (offsetY = -1; offsetY <= 1; offsetY++) {
            if (offsetX != 0 || offsetY != 0) {
                refRectA.x = SDLTest_RandomIntegerInRange(1, 100);
                refRectA.y = SDLTest_RandomIntegerInRange(1, 100);
                refRectB.x = refRectA.x;
                refRectB.y = refRectA.y;
                refRectB.x += offsetX;
                refRectB.y += offsetY;
                rectA = refRectA;
                rectB = refRectB;
                intersection = SDL_HasIntersection(&rectA, &rectB);
                _validateHasIntersectionResults(intersection, SDL_FALSE, &rectA, &rectB, &refRectA, &refRectB);
            }
        }
    }

    return TEST_COMPLETED;
}

/* !
 * \brief Tests SDL_HasIntersection() with empty rectangles
 *
 * \sa
 * http://wiki.libsdl.org/SDL_HasIntersection
 */
int rect_testHasIntersectionEmpty (void *arg)
{
    SDL_Rect refRectA;
    SDL_Rect refRectB;
    SDL_Rect rectA;
    SDL_Rect rectB;
    SDL_bool intersection;

    /* Rect A empty */
    refRectA.x = SDLTest_RandomIntegerInRange(1, 100);
    refRectA.y = SDLTest_RandomIntegerInRange(1, 100);
    refRectA.w = SDLTest_RandomIntegerInRange(1, 100);
    refRectA.h = SDLTest_RandomIntegerInRange(1, 100);
    refRectB = refRectA;
    refRectA.w = 0;
    refRectA.h = 0;
    rectA = refRectA;
    rectB = refRectB;
    intersection = SDL_HasIntersection(&rectA, &rectB);
    _validateHasIntersectionResults(intersection, SDL_FALSE, &rectA, &rectB, &refRectA, &refRectB);

    /* Rect B empty */
    refRectA.x = SDLTest_RandomIntegerInRange(1, 100);
    refRectA.y = SDLTest_RandomIntegerInRange(1, 100);
    refRectA.w = SDLTest_RandomIntegerInRange(1, 100);
    refRectA.h = SDLTest_RandomIntegerInRange(1, 100);
    refRectB = refRectA;
    refRectB.w = 0;
    refRectB.h = 0;
    rectA = refRectA;
    rectB = refRectB;
    intersection = SDL_HasIntersection(&rectA, &rectB);
    _validateHasIntersectionResults(intersection, SDL_FALSE, &rectA, &rectB, &refRectA, &refRectB);

    /* Rect A and B empty */
    refRectA.x = SDLTest_RandomIntegerInRange(1, 100);
    refRectA.y = SDLTest_RandomIntegerInRange(1, 100);
    refRectA.w = SDLTest_RandomIntegerInRange(1, 100);
    refRectA.h = SDLTest_RandomIntegerInRange(1, 100);
    refRectB = refRectA;
    refRectA.w = 0;
    refRectA.h = 0;
    refRectB.w = 0;
    refRectB.h = 0;
    rectA = refRectA;
    rectB = refRectB;
    intersection = SDL_HasIntersection(&rectA, &rectB);
    _validateHasIntersectionResults(intersection, SDL_FALSE, &rectA, &rectB, &refRectA, &refRectB);

    return TEST_COMPLETED;
}

/* !
 * \brief Negative tests against SDL_HasIntersection() with invalid parameters
 *
 * \sa
 * http://wiki.libsdl.org/SDL_HasIntersection
 */
int rect_testHasIntersectionParam(void *arg)
{
    SDL_Rect rectA;
    SDL_Rect rectB;
    SDL_bool intersection;

    /* invalid parameter combinations */
    intersection = SDL_HasIntersection((SDL_Rect *)NULL, &rectB);
    SDLTest_AssertCheck(intersection == SDL_FALSE, "Check that function returns SDL_FALSE when 1st parameter is NULL");
    intersection = SDL_HasIntersection(&rectA, (SDL_Rect *)NULL);
    SDLTest_AssertCheck(intersection == SDL_FALSE, "Check that function returns SDL_FALSE when 2st parameter is NULL");
    intersection = SDL_HasIntersection((SDL_Rect *)NULL, (SDL_Rect *)NULL);
    SDLTest_AssertCheck(intersection == SDL_FALSE, "Check that function returns SDL_FALSE when all parameters are NULL");

    return TEST_COMPLETED;
}

/* !
 * \brief Test SDL_EnclosePoints() without clipping
 *
 * \sa
 * http://wiki.libsdl.org/SDL_EnclosePoints
 */
int rect_testEnclosePoints(void *arg)
{
    const int numPoints = 16;
    SDL_Point refPoints[16];
    SDL_Point points[16];
    SDL_Rect result;
    SDL_bool anyEnclosed;
    SDL_bool anyEnclosedNoResult;
    SDL_bool expectedEnclosed = SDL_TRUE;
    int newx, newy;
    int minx = 0, maxx = 0, miny = 0, maxy = 0;
    int i;

    /* Create input data, tracking result */
    for (i=0; i<numPoints; i++) {
        newx = SDLTest_RandomIntegerInRange(-1024, 1024);
        newy = SDLTest_RandomIntegerInRange(-1024, 1024);
        refPoints[i].x = newx;
        refPoints[i].y = newy;
        points[i].x = newx;
        points[i].y = newy;
        if (i==0) {
            minx = newx;
            maxx = newx;
            miny = newy;
            maxy = newy;
        } else {
            if (newx < minx) minx = newx;
            if (newx > maxx) maxx = newx;
            if (newy < miny) miny = newy;
            if (newy > maxy) maxy = newy;
        }
    }

    /* Call function and validate - special case: no result requested */
    anyEnclosedNoResult = SDL_EnclosePoints((const SDL_Point *)points, numPoints, (const SDL_Rect *)NULL, (SDL_Rect *)NULL);
    SDLTest_AssertCheck(expectedEnclosed==anyEnclosedNoResult,
        "Check expected return value %s, got %s",
        (expectedEnclosed==SDL_TRUE) ? "SDL_TRUE" : "SDL_FALSE",
        (anyEnclosedNoResult==SDL_TRUE) ? "SDL_TRUE" : "SDL_FALSE");
    for (i=0; i<numPoints; i++) {
        SDLTest_AssertCheck(refPoints[i].x==points[i].x && refPoints[i].y==points[i].y,
            "Check that source point %i was not modified: expected (%i,%i) actual (%i,%i)",
            i, refPoints[i].x, refPoints[i].y, points[i].x, points[i].y);
    }

    /* Call function and validate */
    anyEnclosed = SDL_EnclosePoints((const SDL_Point *)points, numPoints, (const SDL_Rect *)NULL, &result);
    SDLTest_AssertCheck(expectedEnclosed==anyEnclosed,
        "Check return value %s, got %s",
        (expectedEnclosed==SDL_TRUE) ? "SDL_TRUE" : "SDL_FALSE",
        (anyEnclosed==SDL_TRUE) ? "SDL_TRUE" : "SDL_FALSE");
    for (i=0; i<numPoints; i++) {
        SDLTest_AssertCheck(refPoints[i].x==points[i].x && refPoints[i].y==points[i].y,
            "Check that source point %i was not modified: expected (%i,%i) actual (%i,%i)",
            i, refPoints[i].x, refPoints[i].y, points[i].x, points[i].y);
    }
    SDLTest_AssertCheck(result.x==minx && result.y==miny && result.w==(maxx - minx + 1) && result.h==(maxy - miny + 1),
        "Resulting enclosing rectangle incorrect: expected (%i,%i - %i,%i), actual (%i,%i - %i,%i)",
        minx, miny, maxx, maxy, result.x, result.y, result.x + result.w - 1, result.y + result.h - 1);

    return TEST_COMPLETED;
}

/* !
 * \brief Test SDL_EnclosePoints() with repeated input points
 *
 * \sa
 * http://wiki.libsdl.org/SDL_EnclosePoints
 */
int rect_testEnclosePointsRepeatedInput(void *arg)
{
    const int numPoints = 8;
    const int halfPoints = 4;
    SDL_Point refPoints[8];
    SDL_Point points[8];
    SDL_Rect result;
    SDL_bool anyEnclosed;
    SDL_bool anyEnclosedNoResult;
    SDL_bool expectedEnclosed = SDL_TRUE;
    int newx, newy;
    int minx = 0, maxx = 0, miny = 0, maxy = 0;
    int i;

    /* Create input data, tracking result */
    for (i=0; i<numPoints; i++) {
        if (i < halfPoints) {
            newx = SDLTest_RandomIntegerInRange(-1024, 1024);
            newy = SDLTest_RandomIntegerInRange(-1024, 1024);
        } else {
            newx = refPoints[i-halfPoints].x;
            newy = refPoints[i-halfPoints].y;
        }
        refPoints[i].x = newx;
        refPoints[i].y = newy;
        points[i].x = newx;
        points[i].y = newy;
        if (i==0) {
            minx = newx;
            maxx = newx;
            miny = newy;
            maxy = newy;
        } else {
            if (newx < minx) minx = newx;
            if (newx > maxx) maxx = newx;
            if (newy < miny) miny = newy;
            if (newy > maxy) maxy = newy;
        }
    }

    /* Call function and validate - special case: no result requested */
    anyEnclosedNoResult = SDL_EnclosePoints((const SDL_Point *)points, numPoints, (const SDL_Rect *)NULL, (SDL_Rect *)NULL);
    SDLTest_AssertCheck(expectedEnclosed==anyEnclosedNoResult,
        "Check return value %s, got %s",
        (expectedEnclosed==SDL_TRUE) ? "SDL_TRUE" : "SDL_FALSE",
        (anyEnclosedNoResult==SDL_TRUE) ? "SDL_TRUE" : "SDL_FALSE");
    for (i=0; i<numPoints; i++) {
        SDLTest_AssertCheck(refPoints[i].x==points[i].x && refPoints[i].y==points[i].y,
            "Check that source point %i was not modified: expected (%i,%i) actual (%i,%i)",
            i, refPoints[i].x, refPoints[i].y, points[i].x, points[i].y);
    }

    /* Call function and validate */
    anyEnclosed = SDL_EnclosePoints((const SDL_Point *)points, numPoints, (const SDL_Rect *)NULL, &result);
    SDLTest_AssertCheck(expectedEnclosed==anyEnclosed,
        "Check return value %s, got %s",
        (expectedEnclosed==SDL_TRUE) ? "SDL_TRUE" : "SDL_FALSE",
        (anyEnclosed==SDL_TRUE) ? "SDL_TRUE" : "SDL_FALSE");
    for (i=0; i<numPoints; i++) {
        SDLTest_AssertCheck(refPoints[i].x==points[i].x && refPoints[i].y==points[i].y,
            "Check that source point %i was not modified: expected (%i,%i) actual (%i,%i)",
            i, refPoints[i].x, refPoints[i].y, points[i].x, points[i].y);
    }
    SDLTest_AssertCheck(result.x==minx && result.y==miny && result.w==(maxx - minx + 1) && result.h==(maxy - miny + 1),
        "Check resulting enclosing rectangle: expected (%i,%i - %i,%i), actual (%i,%i - %i,%i)",
        minx, miny, maxx, maxy, result.x, result.y, result.x + result.w - 1, result.y + result.h - 1);

    return TEST_COMPLETED;
}

/* !
 * \brief Test SDL_EnclosePoints() with clipping
 *
 * \sa
 * http://wiki.libsdl.org/SDL_EnclosePoints
 */
int rect_testEnclosePointsWithClipping(void *arg)
{
    const int numPoints = 16;
    SDL_Point refPoints[16];
    SDL_Point points[16];
    SDL_Rect refClip;
    SDL_Rect clip;
    SDL_Rect result;
    SDL_bool anyEnclosed;
    SDL_bool anyEnclosedNoResult;
    SDL_bool expectedEnclosed = SDL_FALSE;
    int newx, newy;
    int minx = 0, maxx = 0, miny = 0, maxy = 0;
    int i;

    /* Setup clipping rectangle */
    refClip.x = SDLTest_RandomIntegerInRange(-1024, 1024);
    refClip.y = SDLTest_RandomIntegerInRange(-1024, 1024);
    refClip.w = SDLTest_RandomIntegerInRange(1, 1024);
    refClip.h = SDLTest_RandomIntegerInRange(1, 1024);

    /* Create input data, tracking result */
    for (i=0; i<numPoints; i++) {
        newx = SDLTest_RandomIntegerInRange(-1024, 1024);
        newy = SDLTest_RandomIntegerInRange(-1024, 1024);
        refPoints[i].x = newx;
        refPoints[i].y = newy;
        points[i].x = newx;
        points[i].y = newy;
        if ((newx>=refClip.x) && (newx<(refClip.x + refClip.w)) &&
            (newy>=refClip.y) && (newy<(refClip.y + refClip.h))) {
            if (expectedEnclosed==SDL_FALSE) {
                minx = newx;
                maxx = newx;
                miny = newy;
                maxy = newy;
            } else {
                if (newx < minx) minx = newx;
                if (newx > maxx) maxx = newx;
                if (newy < miny) miny = newy;
                if (newy > maxy) maxy = newy;
            }
            expectedEnclosed = SDL_TRUE;
        }
    }

    /* Call function and validate - special case: no result requested */
    clip = refClip;
    anyEnclosedNoResult = SDL_EnclosePoints((const SDL_Point *)points, numPoints, (const SDL_Rect *)&clip, (SDL_Rect *)NULL);
    SDLTest_AssertCheck(expectedEnclosed==anyEnclosedNoResult,
        "Expected return value %s, got %s",
        (expectedEnclosed==SDL_TRUE) ? "SDL_TRUE" : "SDL_FALSE",
        (anyEnclosedNoResult==SDL_TRUE) ? "SDL_TRUE" : "SDL_FALSE");
    for (i=0; i<numPoints; i++) {
        SDLTest_AssertCheck(refPoints[i].x==points[i].x && refPoints[i].y==points[i].y,
            "Check that source point %i was not modified: expected (%i,%i) actual (%i,%i)",
            i, refPoints[i].x, refPoints[i].y, points[i].x, points[i].y);
    }
    SDLTest_AssertCheck(refClip.x==clip.x && refClip.y==clip.y && refClip.w==clip.w && refClip.h==clip.h,
        "Check that source clipping rectangle was not modified");

    /* Call function and validate */
    anyEnclosed = SDL_EnclosePoints((const SDL_Point *)points, numPoints, (const SDL_Rect *)&clip, &result);
    SDLTest_AssertCheck(expectedEnclosed==anyEnclosed,
        "Check return value %s, got %s",
        (expectedEnclosed==SDL_TRUE) ? "SDL_TRUE" : "SDL_FALSE",
        (anyEnclosed==SDL_TRUE) ? "SDL_TRUE" : "SDL_FALSE");
    for (i=0; i<numPoints; i++) {
        SDLTest_AssertCheck(refPoints[i].x==points[i].x && refPoints[i].y==points[i].y,
            "Check that source point %i was not modified: expected (%i,%i) actual (%i,%i)",
            i, refPoints[i].x, refPoints[i].y, points[i].x, points[i].y);
    }
    SDLTest_AssertCheck(refClip.x==clip.x && refClip.y==clip.y && refClip.w==clip.w && refClip.h==clip.h,
        "Check that source clipping rectangle was not modified");
    if (expectedEnclosed==SDL_TRUE) {
        SDLTest_AssertCheck(result.x==minx && result.y==miny && result.w==(maxx - minx + 1) && result.h==(maxy - miny + 1),
            "Check resulting enclosing rectangle: expected (%i,%i - %i,%i), actual (%i,%i - %i,%i)",
            minx, miny, maxx, maxy, result.x, result.y, result.x + result.w - 1, result.y + result.h - 1);
    }

    /* Empty clipping rectangle */
    clip.w = 0;
    clip.h = 0;
    expectedEnclosed = SDL_FALSE;
    anyEnclosed = SDL_EnclosePoints((const SDL_Point *)points, numPoints, (const SDL_Rect *)&clip, &result);
    SDLTest_AssertCheck(expectedEnclosed==anyEnclosed,
        "Check return value %s, got %s",
        (expectedEnclosed==SDL_TRUE) ? "SDL_TRUE" : "SDL_FALSE",
        (anyEnclosed==SDL_TRUE) ? "SDL_TRUE" : "SDL_FALSE");

    return TEST_COMPLETED;
}

/* !
 * \brief Negative tests against SDL_EnclosePoints() with invalid parameters
 *
 * \sa
 * http://wiki.libsdl.org/SDL_EnclosePoints
 */
int rect_testEnclosePointsParam(void *arg)
{
    SDL_Point points[1];
    int count;
    SDL_Rect clip;
    SDL_Rect result;
    SDL_bool anyEnclosed;

    /* invalid parameter combinations */
    anyEnclosed = SDL_EnclosePoints((SDL_Point *)NULL, 1, (const SDL_Rect *)&clip, &result);
    SDLTest_AssertCheck(anyEnclosed == SDL_FALSE, "Check that functions returns SDL_FALSE when 1st parameter is NULL");
    anyEnclosed = SDL_EnclosePoints((const SDL_Point *)points, 0, (const SDL_Rect *)&clip, &result);
    SDLTest_AssertCheck(anyEnclosed == SDL_FALSE, "Check that functions returns SDL_FALSE when 2nd parameter is 0");
    count = SDLTest_RandomIntegerInRange(-100, -1);
    anyEnclosed = SDL_EnclosePoints((const SDL_Point *)points, count, (const SDL_Rect *)&clip, &result);
    SDLTest_AssertCheck(anyEnclosed == SDL_FALSE, "Check that functions returns SDL_FALSE when 2nd parameter is %i (negative)", count);
    anyEnclosed = SDL_EnclosePoints((SDL_Point *)NULL, 0, (const SDL_Rect *)&clip, &result);
    SDLTest_AssertCheck(anyEnclosed == SDL_FALSE, "Check that functions returns SDL_FALSE when 1st parameter is NULL and 2nd parameter was 0");

    return TEST_COMPLETED;
}

/* !
 * \brief Tests SDL_UnionRect() where rect B is outside rect A
 *
 * \sa
 * http://wiki.libsdl.org/SDL_UnionRect
 */
int rect_testUnionRectOutside(void *arg)
{
    SDL_Rect refRectA, refRectB;
    SDL_Rect rectA, rectB;
    SDL_Rect expectedResult;
    SDL_Rect result;
    int minx, maxx, miny, maxy;
    int dx, dy;

    /* Union 1x1 outside */
    for (dx = -1; dx < 2; dx++) {
        for (dy = -1; dy < 2; dy++) {
            if ((dx != 0) || (dy != 0)) {
                refRectA.x=SDLTest_RandomIntegerInRange(-1024, 1024);
                refRectA.y=SDLTest_RandomIntegerInRange(-1024, 1024);
                refRectA.w=1;
                refRectA.h=1;
                refRectB.x=SDLTest_RandomIntegerInRange(-1024, 1024) + dx*2048;
                refRectB.y=SDLTest_RandomIntegerInRange(-1024, 1024) + dx*2048;
                refRectB.w=1;
                refRectB.h=1;
                minx = (refRectA.x<refRectB.x) ? refRectA.x : refRectB.x;
                maxx = (refRectA.x>refRectB.x) ? refRectA.x : refRectB.x;
                miny = (refRectA.y<refRectB.y) ? refRectA.y : refRectB.y;
                maxy = (refRectA.y>refRectB.y) ? refRectA.y : refRectB.y;
                expectedResult.x = minx;
                expectedResult.y = miny;
                expectedResult.w = maxx - minx + 1;
                expectedResult.h = maxy - miny + 1;
                rectA = refRectA;
                rectB = refRectB;
                SDL_UnionRect(&rectA, &rectB, &result);
                _validateUnionRectResults(&rectA, &rectB, &refRectA, &refRectB, &result, &expectedResult);
            }
        }
    }

    /* Union outside overlap */
    for (dx = -1; dx < 2; dx++) {
        for (dy = -1; dy < 2; dy++) {
            if ((dx != 0) || (dy != 0)) {
                refRectA.x=SDLTest_RandomIntegerInRange(-1024, 1024);
                refRectA.y=SDLTest_RandomIntegerInRange(-1024, 1024);
                refRectA.w=SDLTest_RandomIntegerInRange(256, 512);
                refRectA.h=SDLTest_RandomIntegerInRange(256, 512);
                refRectB.x=refRectA.x + 1 + dx*2;
                refRectB.y=refRectA.y + 1 + dy*2;
                refRectB.w=refRectA.w - 2;
                refRectB.h=refRectA.h - 2;
                expectedResult = refRectA;
                if (dx == -1) expectedResult.x--;
                if (dy == -1) expectedResult.y--;
                if ((dx == 1) || (dx == -1)) expectedResult.w++;
                if ((dy == 1) || (dy == -1)) expectedResult.h++;
                rectA = refRectA;
                rectB = refRectB;
                SDL_UnionRect(&rectA, &rectB, &result);
                _validateUnionRectResults(&rectA, &rectB, &refRectA, &refRectB, &result, &expectedResult);
            }
        }
    }

    return TEST_COMPLETED;
}

/* !
 * \brief Tests SDL_UnionRect() where rect A or rect B are empty
 *
 * \sa
 * http://wiki.libsdl.org/SDL_UnionRect
 */
int rect_testUnionRectEmpty(void *arg)
{
    SDL_Rect refRectA, refRectB;
    SDL_Rect rectA, rectB;
    SDL_Rect expectedResult;
    SDL_Rect result;

    /* A empty */
    refRectA.x=SDLTest_RandomIntegerInRange(-1024, 1024);
    refRectA.y=SDLTest_RandomIntegerInRange(-1024, 1024);
    refRectA.w=0;
    refRectA.h=0;
    refRectB.x=SDLTest_RandomIntegerInRange(-1024, 1024);
    refRectB.y=SDLTest_RandomIntegerInRange(-1024, 1024);
    refRectB.w=SDLTest_RandomIntegerInRange(1, 1024);
    refRectB.h=SDLTest_RandomIntegerInRange(1, 1024);
    expectedResult = refRectB;
    rectA = refRectA;
    rectB = refRectB;
    SDL_UnionRect(&rectA, &rectB, &result);
    _validateUnionRectResults(&rectA, &rectB, &refRectA, &refRectB, &result, &expectedResult);

    /* B empty */
    refRectA.x=SDLTest_RandomIntegerInRange(-1024, 1024);
    refRectA.y=SDLTest_RandomIntegerInRange(-1024, 1024);
    refRectA.w=SDLTest_RandomIntegerInRange(1, 1024);
    refRectA.h=SDLTest_RandomIntegerInRange(1, 1024);
    refRectB.x=SDLTest_RandomIntegerInRange(-1024, 1024);
    refRectB.y=SDLTest_RandomIntegerInRange(-1024, 1024);
    refRectB.w=0;
    refRectB.h=0;
    expectedResult = refRectA;
    rectA = refRectA;
    rectB = refRectB;
    SDL_UnionRect(&rectA, &rectB, &result);
    _validateUnionRectResults(&rectA, &rectB, &refRectA, &refRectB, &result, &expectedResult);

    /* A and B empty */
    refRectA.x=SDLTest_RandomIntegerInRange(-1024, 1024);
    refRectA.y=SDLTest_RandomIntegerInRange(-1024, 1024);
    refRectA.w=0;
    refRectA.h=0;
    refRectB.x=SDLTest_RandomIntegerInRange(-1024, 1024);
    refRectB.y=SDLTest_RandomIntegerInRange(-1024, 1024);
    refRectB.w=0;
    refRectB.h=0;
    result.x=0;
    result.y=0;
    result.w=0;
    result.h=0;
    expectedResult = result;
    rectA = refRectA;
    rectB = refRectB;
    SDL_UnionRect(&rectA, &rectB, &result);
    _validateUnionRectResults(&rectA, &rectB, &refRectA, &refRectB, &result, &expectedResult);

    return TEST_COMPLETED;
}

/* !
 * \brief Tests SDL_UnionRect() where rect B is inside rect A
 *
 * \sa
 * http://wiki.libsdl.org/SDL_UnionRect
 */
int rect_testUnionRectInside(void *arg)
{
    SDL_Rect refRectA, refRectB;
    SDL_Rect rectA, rectB;
    SDL_Rect expectedResult;
    SDL_Rect result;
    int dx, dy;

    /* Union 1x1 with itself */
    refRectA.x=SDLTest_RandomIntegerInRange(-1024, 1024);
    refRectA.y=SDLTest_RandomIntegerInRange(-1024, 1024);
    refRectA.w=1;
    refRectA.h=1;
    expectedResult = refRectA;
    rectA = refRectA;
    SDL_UnionRect(&rectA, &rectA, &result);
    _validateUnionRectResults(&rectA, &rectA, &refRectA, &refRectA, &result, &expectedResult);

    /* Union 1x1 somewhere inside */
    refRectA.x=SDLTest_RandomIntegerInRange(-1024, 1024);
    refRectA.y=SDLTest_RandomIntegerInRange(-1024, 1024);
    refRectA.w=SDLTest_RandomIntegerInRange(256, 1024);
    refRectA.h=SDLTest_RandomIntegerInRange(256, 1024);
    refRectB.x=refRectA.x + 1 + SDLTest_RandomIntegerInRange(1, refRectA.w - 2);
    refRectB.y=refRectA.y + 1 + SDLTest_RandomIntegerInRange(1, refRectA.h - 2);
    refRectB.w=1;
    refRectB.h=1;
    expectedResult = refRectA;
    rectA = refRectA;
    rectB = refRectB;
    SDL_UnionRect(&rectA, &rectB, &result);
    _validateUnionRectResults(&rectA, &rectB, &refRectA, &refRectB, &result, &expectedResult);

    /* Union inside with edges modified */
    for (dx = -1; dx < 2; dx++) {
        for (dy = -1; dy < 2; dy++) {
            if ((dx != 0) || (dy != 0)) {
                refRectA.x=SDLTest_RandomIntegerInRange(-1024, 1024);
                refRectA.y=SDLTest_RandomIntegerInRange(-1024, 1024);
                refRectA.w=SDLTest_RandomIntegerInRange(256, 1024);
                refRectA.h=SDLTest_RandomIntegerInRange(256, 1024);
                refRectB = refRectA;
                if (dx == -1) refRectB.x++;
                if ((dx == 1) || (dx == -1)) refRectB.w--;
                if (dy == -1) refRectB.y++;
                if ((dy == 1) || (dy == -1)) refRectB.h--;
                expectedResult = refRectA;
                rectA = refRectA;
                rectB = refRectB;
                SDL_UnionRect(&rectA, &rectB, &result);
                _validateUnionRectResults(&rectA, &rectB, &refRectA, &refRectB, &result, &expectedResult);
            }
        }
    }

    return TEST_COMPLETED;
}

/* !
 * \brief Negative tests against SDL_UnionRect() with invalid parameters
 *
 * \sa
 * http://wiki.libsdl.org/SDL_UnionRect
 */
int rect_testUnionRectParam(void *arg)
{
    SDL_Rect rectA, rectB;
    SDL_Rect result;

    /* invalid parameter combinations */
    SDL_UnionRect((SDL_Rect *)NULL, &rectB, &result);
    SDLTest_AssertPass("Check that function returns when 1st parameter is NULL");
    SDL_UnionRect(&rectA, (SDL_Rect *)NULL, &result);
    SDLTest_AssertPass("Check that function returns  when 2nd parameter is NULL");
    SDL_UnionRect(&rectA, &rectB, (SDL_Rect *)NULL);
    SDLTest_AssertPass("Check that function returns  when 3rd parameter is NULL");
    SDL_UnionRect((SDL_Rect *)NULL, &rectB, (SDL_Rect *)NULL);
    SDLTest_AssertPass("Check that function returns  when 1st and 3rd parameter are NULL");
    SDL_UnionRect(&rectA, (SDL_Rect *)NULL, (SDL_Rect *)NULL);
    SDLTest_AssertPass("Check that function returns  when 2nd and 3rd parameter are NULL");
    SDL_UnionRect((SDL_Rect *)NULL, (SDL_Rect *)NULL, (SDL_Rect *)NULL);
    SDLTest_AssertPass("Check that function returns  when all parameters are NULL");

    return TEST_COMPLETED;
}

/* !
 * \brief Tests SDL_RectEmpty() with various inputs
 *
 * \sa
 * http://wiki.libsdl.org/SDL_RectEmpty
 */
int rect_testRectEmpty(void *arg)
{
    SDL_Rect refRect;
    SDL_Rect rect;
    SDL_bool expectedResult;
    SDL_bool result;
    int w, h;

    /* Non-empty case */
    refRect.x=SDLTest_RandomIntegerInRange(-1024, 1024);
    refRect.y=SDLTest_RandomIntegerInRange(-1024, 1024);
    refRect.w=SDLTest_RandomIntegerInRange(256, 1024);
    refRect.h=SDLTest_RandomIntegerInRange(256, 1024);
    expectedResult = SDL_FALSE;
    rect = refRect;
    result = (SDL_bool)SDL_RectEmpty((const SDL_Rect *)&rect);
    _validateRectEmptyResults(result, expectedResult, &rect, &refRect);

    /* Empty case */
    for (w=-1; w<2; w++) {
        for (h=-1; h<2; h++) {
            if ((w != 1) || (h != 1)) {
                refRect.x=SDLTest_RandomIntegerInRange(-1024, 1024);
                refRect.y=SDLTest_RandomIntegerInRange(-1024, 1024);
                refRect.w=w;
                refRect.h=h;
                expectedResult = SDL_TRUE;
                rect = refRect;
                result = (SDL_bool)SDL_RectEmpty((const SDL_Rect *)&rect);
                _validateRectEmptyResults(result, expectedResult, &rect, &refRect);
            }
        }
    }

    return TEST_COMPLETED;
}

/* !
 * \brief Negative tests against SDL_RectEmpty() with invalid parameters
 *
 * \sa
 * http://wiki.libsdl.org/SDL_RectEmpty
 */
int rect_testRectEmptyParam(void *arg)
{
    SDL_bool result;

    /* invalid parameter combinations */
    result = (SDL_bool)SDL_RectEmpty((const SDL_Rect *)NULL);
    SDLTest_AssertCheck(result == SDL_TRUE, "Check that function returns TRUE when 1st parameter is NULL");

    return TEST_COMPLETED;
}

/* !
 * \brief Tests SDL_RectEquals() with various inputs
 *
 * \sa
 * http://wiki.libsdl.org/SDL_RectEquals
 */
int rect_testRectEquals(void *arg)
{
    SDL_Rect refRectA;
    SDL_Rect refRectB;
    SDL_Rect rectA;
    SDL_Rect rectB;
    SDL_bool expectedResult;
    SDL_bool result;

    /* Equals */
    refRectA.x=SDLTest_RandomIntegerInRange(-1024, 1024);
    refRectA.y=SDLTest_RandomIntegerInRange(-1024, 1024);
    refRectA.w=SDLTest_RandomIntegerInRange(1, 1024);
    refRectA.h=SDLTest_RandomIntegerInRange(1, 1024);
    refRectB = refRectA;
    expectedResult = SDL_TRUE;
    rectA = refRectA;
    rectB = refRectB;
    result = (SDL_bool)SDL_RectEquals((const SDL_Rect *)&rectA, (const SDL_Rect *)&rectB);
    _validateRectEqualsResults(result, expectedResult, &rectA, &rectB, &refRectA, &refRectB);

    return TEST_COMPLETED;
}

/* !
 * \brief Negative tests against SDL_RectEquals() with invalid parameters
 *
 * \sa
 * http://wiki.libsdl.org/SDL_RectEquals
 */
int rect_testRectEqualsParam(void *arg)
{
    SDL_Rect rectA;
    SDL_Rect rectB;
    SDL_bool result;

    /* data setup */
    rectA.x=SDLTest_RandomIntegerInRange(-1024, 1024);
    rectA.y=SDLTest_RandomIntegerInRange(-1024, 1024);
    rectA.w=SDLTest_RandomIntegerInRange(1, 1024);
    rectA.h=SDLTest_RandomIntegerInRange(1, 1024);
    rectB.x=SDLTest_RandomIntegerInRange(-1024, 1024);
    rectB.y=SDLTest_RandomIntegerInRange(-1024, 1024);
    rectB.w=SDLTest_RandomIntegerInRange(1, 1024);
    rectB.h=SDLTest_RandomIntegerInRange(1, 1024);

    /* invalid parameter combinations */
    result = (SDL_bool)SDL_RectEquals((const SDL_Rect *)NULL, (const SDL_Rect *)&rectB);
    SDLTest_AssertCheck(result == SDL_FALSE, "Check that function returns SDL_FALSE when 1st parameter is NULL");
    result = (SDL_bool)SDL_RectEquals((const SDL_Rect *)&rectA, (const SDL_Rect *)NULL);
    SDLTest_AssertCheck(result == SDL_FALSE, "Check that function returns SDL_FALSE when 2nd parameter is NULL");
    result = (SDL_bool)SDL_RectEquals((const SDL_Rect *)NULL, (const SDL_Rect *)NULL);
    SDLTest_AssertCheck(result == SDL_FALSE, "Check that function returns SDL_FALSE when 1st and 2nd parameter are NULL");

    return TEST_COMPLETED;
}

/* ================= Test References ================== */

/* Rect test cases */

/* SDL_IntersectRectAndLine */
static const SDLTest_TestCaseReference rectTest1 =
        { (SDLTest_TestCaseFp)rect_testIntersectRectAndLine,"rect_testIntersectRectAndLine",  "Tests SDL_IntersectRectAndLine clipping cases", TEST_ENABLED };

static const SDLTest_TestCaseReference rectTest2 =
        { (SDLTest_TestCaseFp)rect_testIntersectRectAndLineInside, "rect_testIntersectRectAndLineInside", "Tests SDL_IntersectRectAndLine with line fully contained in rect", TEST_ENABLED };

static const SDLTest_TestCaseReference rectTest3 =
        { (SDLTest_TestCaseFp)rect_testIntersectRectAndLineOutside, "rect_testIntersectRectAndLineOutside", "Tests SDL_IntersectRectAndLine with line fully outside of rect", TEST_ENABLED };

static const SDLTest_TestCaseReference rectTest4 =
        { (SDLTest_TestCaseFp)rect_testIntersectRectAndLineEmpty, "rect_testIntersectRectAndLineEmpty", "Tests SDL_IntersectRectAndLine with empty rectangle ", TEST_ENABLED };

static const SDLTest_TestCaseReference rectTest5 =
        { (SDLTest_TestCaseFp)rect_testIntersectRectAndLineParam, "rect_testIntersectRectAndLineParam", "Negative tests against SDL_IntersectRectAndLine with invalid parameters", TEST_ENABLED };

/* SDL_IntersectRect */
static const SDLTest_TestCaseReference rectTest6 =
        { (SDLTest_TestCaseFp)rect_testIntersectRectInside, "rect_testIntersectRectInside", "Tests SDL_IntersectRect with B fully contained in A", TEST_ENABLED };

static const SDLTest_TestCaseReference rectTest7 =
        { (SDLTest_TestCaseFp)rect_testIntersectRectOutside, "rect_testIntersectRectOutside", "Tests SDL_IntersectRect with B fully outside of A", TEST_ENABLED };

static const SDLTest_TestCaseReference rectTest8 =
        { (SDLTest_TestCaseFp)rect_testIntersectRectPartial, "rect_testIntersectRectPartial", "Tests SDL_IntersectRect with B partially intersecting A", TEST_ENABLED };

static const SDLTest_TestCaseReference rectTest9 =
        { (SDLTest_TestCaseFp)rect_testIntersectRectPoint, "rect_testIntersectRectPoint", "Tests SDL_IntersectRect with 1x1 sized rectangles", TEST_ENABLED };

static const SDLTest_TestCaseReference rectTest10 =
        { (SDLTest_TestCaseFp)rect_testIntersectRectEmpty, "rect_testIntersectRectEmpty", "Tests SDL_IntersectRect with empty rectangles", TEST_ENABLED };

static const SDLTest_TestCaseReference rectTest11 =
        { (SDLTest_TestCaseFp)rect_testIntersectRectParam, "rect_testIntersectRectParam", "Negative tests against SDL_IntersectRect with invalid parameters", TEST_ENABLED };

/* SDL_HasIntersection */
static const SDLTest_TestCaseReference rectTest12 =
        { (SDLTest_TestCaseFp)rect_testHasIntersectionInside, "rect_testHasIntersectionInside", "Tests SDL_HasIntersection with B fully contained in A", TEST_ENABLED };

static const SDLTest_TestCaseReference rectTest13 =
        { (SDLTest_TestCaseFp)rect_testHasIntersectionOutside, "rect_testHasIntersectionOutside", "Tests SDL_HasIntersection with B fully outside of A", TEST_ENABLED };

static const SDLTest_TestCaseReference rectTest14 =
        { (SDLTest_TestCaseFp)rect_testHasIntersectionPartial,"rect_testHasIntersectionPartial",  "Tests SDL_HasIntersection with B partially intersecting A", TEST_ENABLED };

static const SDLTest_TestCaseReference rectTest15 =
        { (SDLTest_TestCaseFp)rect_testHasIntersectionPoint, "rect_testHasIntersectionPoint", "Tests SDL_HasIntersection with 1x1 sized rectangles", TEST_ENABLED };

static const SDLTest_TestCaseReference rectTest16 =
        { (SDLTest_TestCaseFp)rect_testHasIntersectionEmpty, "rect_testHasIntersectionEmpty", "Tests SDL_HasIntersection with empty rectangles", TEST_ENABLED };

static const SDLTest_TestCaseReference rectTest17 =
        { (SDLTest_TestCaseFp)rect_testHasIntersectionParam, "rect_testHasIntersectionParam", "Negative tests against SDL_HasIntersection with invalid parameters", TEST_ENABLED };

/* SDL_EnclosePoints */
static const SDLTest_TestCaseReference rectTest18 =
        { (SDLTest_TestCaseFp)rect_testEnclosePoints, "rect_testEnclosePoints", "Tests SDL_EnclosePoints without clipping", TEST_ENABLED };

static const SDLTest_TestCaseReference rectTest19 =
        { (SDLTest_TestCaseFp)rect_testEnclosePointsWithClipping, "rect_testEnclosePointsWithClipping", "Tests SDL_EnclosePoints with clipping", TEST_ENABLED };

static const SDLTest_TestCaseReference rectTest20 =
        { (SDLTest_TestCaseFp)rect_testEnclosePointsRepeatedInput, "rect_testEnclosePointsRepeatedInput", "Tests SDL_EnclosePoints with repeated input", TEST_ENABLED };

static const SDLTest_TestCaseReference rectTest21 =
        { (SDLTest_TestCaseFp)rect_testEnclosePointsParam, "rect_testEnclosePointsParam", "Negative tests against SDL_EnclosePoints with invalid parameters", TEST_ENABLED };

/* SDL_UnionRect */
static const SDLTest_TestCaseReference rectTest22 =
        { (SDLTest_TestCaseFp)rect_testUnionRectInside, "rect_testUnionRectInside", "Tests SDL_UnionRect where rect B is inside rect A", TEST_ENABLED };

static const SDLTest_TestCaseReference rectTest23 =
        { (SDLTest_TestCaseFp)rect_testUnionRectOutside, "rect_testUnionRectOutside", "Tests SDL_UnionRect where rect B is outside rect A", TEST_ENABLED };

static const SDLTest_TestCaseReference rectTest24 =
        { (SDLTest_TestCaseFp)rect_testUnionRectEmpty, "rect_testUnionRectEmpty", "Tests SDL_UnionRect where rect A or rect B are empty", TEST_ENABLED };

static const SDLTest_TestCaseReference rectTest25 =
        { (SDLTest_TestCaseFp)rect_testUnionRectParam, "rect_testUnionRectParam", "Negative tests against SDL_UnionRect with invalid parameters", TEST_ENABLED };

/* SDL_RectEmpty */
static const SDLTest_TestCaseReference rectTest26 =
        { (SDLTest_TestCaseFp)rect_testRectEmpty, "rect_testRectEmpty", "Tests SDL_RectEmpty with various inputs", TEST_ENABLED };

static const SDLTest_TestCaseReference rectTest27 =
        { (SDLTest_TestCaseFp)rect_testRectEmptyParam, "rect_testRectEmptyParam", "Negative tests against SDL_RectEmpty with invalid parameters", TEST_ENABLED };

/* SDL_RectEquals */

static const SDLTest_TestCaseReference rectTest28 =
        { (SDLTest_TestCaseFp)rect_testRectEquals, "rect_testRectEquals", "Tests SDL_RectEquals with various inputs", TEST_ENABLED };

static const SDLTest_TestCaseReference rectTest29 =
        { (SDLTest_TestCaseFp)rect_testRectEqualsParam, "rect_testRectEqualsParam", "Negative tests against SDL_RectEquals with invalid parameters", TEST_ENABLED };


/* !
 * \brief Sequence of Rect test cases; functions that handle simple rectangles including overlaps and merges.
 *
 * \sa
 * http://wiki.libsdl.org/CategoryRect
 */
static const SDLTest_TestCaseReference *rectTests[] =  {
    &rectTest1, &rectTest2, &rectTest3, &rectTest4, &rectTest5, &rectTest6, &rectTest7, &rectTest8, &rectTest9, &rectTest10, &rectTest11, &rectTest12, &rectTest13, &rectTest14,
    &rectTest15, &rectTest16, &rectTest17, &rectTest18, &rectTest19, &rectTest20, &rectTest21, &rectTest22, &rectTest23, &rectTest24, &rectTest25, &rectTest26, &rectTest27,
    &rectTest28, &rectTest29, NULL
};


/* Rect test suite (global) */
SDLTest_TestSuiteReference rectTestSuite = {
    "Rect",
    NULL,
    rectTests,
    NULL
};
