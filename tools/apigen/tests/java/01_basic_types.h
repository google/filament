#include <cstddef>
#include <cstdint>


class Foo {
public:
    char c;
    unsigned char uc;
    short s;
    unsigned short us;
    int i;
    unsigned int ui;
    long l;
    unsigned long ul;
    float f;
    double d;
    bool b;

    int8_t i8;
    uint8_t ui8;
    int16_t i16;
    uint16_t ui16;
    int32_t i32;
    uint32_t ui32;
    int64_t i64;
    uint64_t ui64;

    size_t st;

    uint32_t method(char c, unsigned char uc) noexcept;
};
