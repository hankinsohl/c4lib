# Building c4lib and c4edit

## CLion

c4lib is built with the [CLion IDE](https://www.jetbrains.com/clion/) using CMake and the
corresponding CMakeLists.txt
files. CMakeLists.txt may contain methods bundled with CLion. If you want to build c4lib, CLion
is recommended.

## Thread safety

c4lib is not thread safe.

## Compiler requirements

To build c4lib, you will need a compiler that supports C++ 20. c4lib has been built using the
following compilers:

* GCC 14.2.0
* Clang 19.1.3
* MSVC 16.00.30319.01

## 3rd-party sources

c4lib relies on 3rd-party software not included in the c4lib distribution. To build c4lib you'll
need to install the following 3rd-party software:

[Boost](https://www.boost.org/doc/libs/1_87_0/doc/html/index.html)
c4lib relies on Boost header files. It is not necessary to build any of the Boost libraries.

[utfcpp](https://github.com/nemtrif/utfcpp)
c4lib uses utfcpp to convert UTF-8 to UTF-16 and vice versa.

[zlib](https://github.com/madler/zlib)
c4lib relies on zlib for compression/decompression. You will need to build zlib for the compiler
you want to use.