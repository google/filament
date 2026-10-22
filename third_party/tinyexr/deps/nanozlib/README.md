# nanozlib

Nanoscale secure zlib decoding utilizing wuffs `std/zlib`, and somewhat secure zlib compression using stb'z lib, written in portable C/C++(Mostly C).

nanozib provides very limited zlib decoding and encoding functionalities.
nanozib is good to embed zlib support to your C/C++ application.

## Requirements

* C99, C++11 or later

## Advantages and disadvantages

* Advantage(s)
  * Born to be super secure.
    * No assert, No C++ exception. No segfault for corrupted/malcious zlib data.

* Disadvantage(s)
  * Up to 2GB.
  * 50 KB or more in compiled binary(even compiled with `-Os`)
    * If you want picoscale zlib decoder, please take a look at zlib module in stb_image https://github.com/nothings/stb

## Example

```
$ make
# 11 = uncompressed size
$ ./test_nanozdec test/test-000.txt.zz 11
```


## Wuffs version

v0.3.0

## TODO

* [ ] compress using wuffs or fpng's encoder. wuffs doesn't provide zlib encoding feature at the moment.

## License

Apache 2.0 for decoding part.
Public domin for encoding part.

### Third party licenses

* wuffs. Apache 2.0. https://github.com/google/wuffs
* stb zlib. Public domain. https://github.com/nothings/stb
