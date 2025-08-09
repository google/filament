#version 460

// This tests that the preprocessor error
// "error: '#' : (#) can be preceded in its line only by spaces or horizontal tab"
// isn't enforced when inactive (e.g. inside #if 0)

#if 0
#define STRINGIFY(X) #X
#endif

#define C 0

#if 1
#ifdef A
#elif defined B
#elif C
// OK, since preprocessor evaluates to inactive
#define STRINGIFY(X) #X
#endif
#endif

// OK in comments
// #define STRINGIFY(X) #X

void main()
{
    gl_Position = vec4(1.0);
}