#version 450
#extension GL_EXT_debug_printf : enable

#define PASTE(arg) pasted_##arg
#define STRINGIFY(arg) #arg
#define STRINGIFY_TWICE(arg, regular) before, #arg, #arg, after, regular
#define STRINGIFY_WITH_SPACE(arg) debugPrintfEXT( # arg
#define PASTE_WITH_SPACE(arg) pasted_## arg
#define BOTH(arg1,    arg2) debugPrintf##arg1(     #arg2
#define NOARGS(arg) 1 1 1 1

#define REGULAR(arg1, arg2) arg1 == arg2     ? arg2 :arg1
#define STR(a) STRHELP(a)
#define STRHELP(a) #a

#define LEGAL(arg) a######arg

bool before, after;

void main() {
    REGULAR(8,9);
    STRINGIFY(12 3);
    debugPrintfEXT("%d", (STRINGIFY_TWICE(letters_too,       1)));
    STRINGIFY_WITH_SPACE(aaa1));

    int PASTE(symbol);
    float PASTE_WITH_SPACE(symbol2);
    BOTH(EXT, string));
    STRHELP(REGULAR(3, 4));
    STR(REGULAR(3, 4));
    STR(aaa     );
    STR(STR(hello));
    debugPrintfEXT("regular string");
    debugPrintfEXT("");
}
