cmake_minimum_required(VERSION 3.28)
set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_CXX_STANDARD 20)

project(c4lib)

message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "Detected compiler: ${CMAKE_CXX_COMPILER_ID}")
if (WIN32)
    message(STATUS "Target OS: WIN32")
elseif (UNIX)
    message(STATUS "Target OS: UNIX")
endif ()
set(FUZZTEST_ENABLED "$ENV{FUZZTEST_ENABLED}")
if (FUZZTEST_ENABLED)
    message(STATUS "Fuzztest enabled")
endif ()

message(STATUS "Profile enabled = ${PROFILE_ENABLED}")

# Change C4LIB_VERSION to set the version of c4edit and c4lib.
set(C4LIB_VERSION "01.00.00")
configure_file(c4lib-version.hpp.in c4lib-version.hpp)

include(FetchContent)
if (FUZZTEST_ENABLED)
    FetchContent_Declare(
            fuzztest
            # Specify the commit you depend on and update it regularly.
            URL https://github.com/google/fuzztest/archive/refs/tags/2024-10-28.zip
    )
    FetchContent_MakeAvailable(fuzztest)
    enable_testing()
endif ()

set(Boost_VERBOSE ON)
set(Boost_NO_WARN_NEW_VERSIONS ON)
set(BOOST_IOSTREAMS_NO_LIB ON)
if (POLICY CMP0167)
    cmake_policy(SET CMP0167 OLD)
endif ()
find_package(Boost 1.85.0 COMPONENTS system filesystem REQUIRED)
if (Boost_FOUND)
    message(STATUS "Boost found")
endif ()

set(ZLIB_USE_STATIC_LIBS ON)
find_package(ZLIB REQUIRED)
if (ZLIB_FOUND)
    message(STATUS "ZLIB found")
endif ()

# Fix zlib library name.  find_package sets the library name to zlib.lib if WIN32 is defined.  zlib.lib is
# only compatible with the MSVC compiler; for other compilers we need to use libz.a.
if (NOT CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
    string(REPLACE "zlib.lib" "libz.a" ZLIB_LIBRARIES "${ZLIB_LIBRARIES}")
endif ()

set(C4_ROOT ${CMAKE_CURRENT_LIST_DIR})
set(C4_INCLUDE_ROOT ${C4_ROOT})

set(INCLUDE_SOURCE_FILES
        include/c4lib.hpp
        include/exceptions.hpp
        include/logger.hpp
        include/node-attributes.hpp
        include/node-type.hpp
)

set(EXE_SOURCE_FILES
        src/limits.hpp
        src/main.cpp
        src/options-data.hpp
        src/options.hpp
        src/text.hpp
        src/util.hpp
)

set(LIB_SOURCE_FILES
        lib/c4lib/c4lib-internal.hpp
        lib/c4lib/c4lib.cpp
        lib/expression-parser/infix-representation.cpp
        lib/expression-parser/infix-representation.hpp
        lib/expression-parser/parser.cpp
        lib/expression-parser/parser.hpp
        lib/importer/file-manager.cpp
        lib/importer/file-manager.hpp
        lib/importer/importer.cpp
        lib/importer/importer.hpp
        lib/io/io.cpp
        lib/io/io.hpp
        lib/layout/layout.cpp
        lib/layout/layout.hpp
        lib/logger/log-formats.hpp
        lib/logger/logger.cpp
        lib/md5/checksum.cpp
        lib/md5/checksum.cpp
        lib/md5/checksum.hpp
        lib/md5/checksum.hpp
        lib/md5/md5-digest.cpp
        lib/md5/md5-digest.cpp
        lib/md5/md5-digest.hpp
        lib/md5/md5-digest.hpp
        lib/md5/md5.cpp
        lib/md5/md5.cpp
        lib/md5/md5.hpp
        lib/md5/md5.hpp
        lib/native/compiler-support.hpp
        lib/native/path.hpp
        lib/options/exception-formats.hpp
        lib/options/exceptions.hpp
        lib/options/options-manager.cpp
        lib/options/options-manager.hpp
        lib/ptree/base-node-reader.cpp
        lib/ptree/base-node-reader.hpp
        lib/ptree/binary-node-reader.cpp
        lib/ptree/binary-node-reader.hpp
        lib/ptree/binary-node-writer.cpp
        lib/ptree/binary-node-writer.hpp
        lib/ptree/debug.hpp
        lib/ptree/generative-node-source.cpp
        lib/ptree/generative-node-source.hpp
        lib/ptree/internationalization-text.hpp
        lib/ptree/node-reader.hpp
        lib/ptree/node-type.cpp
        lib/ptree/node-writer.hpp
        lib/ptree/null-node-reader.cpp
        lib/ptree/null-node-reader.hpp
        lib/ptree/recursive-node-source.hpp
        lib/ptree/translation-node-writer.cpp
        lib/ptree/translation-node-writer.hpp
        lib/ptree/util.cpp
        lib/ptree/util.hpp
        lib/schema-parser/auto-index.hpp
        lib/schema-parser/def-mem-type.cpp
        lib/schema-parser/def-mem-type.hpp
        lib/schema-parser/def-mem.hpp
        lib/schema-parser/def-tbl.cpp
        lib/schema-parser/def-tbl.hpp
        lib/schema-parser/def-type.cpp
        lib/schema-parser/def-type.hpp
        lib/schema-parser/definition.cpp
        lib/schema-parser/definition.hpp
        lib/schema-parser/parser-phase-one.cpp
        lib/schema-parser/parser-phase-one.hpp
        lib/schema-parser/parser-phase-two.cpp
        lib/schema-parser/parser-phase-two.hpp
        lib/schema-parser/parser.cpp
        lib/schema-parser/parser.hpp
        lib/schema-parser/token-type.cpp
        lib/schema-parser/token-type.hpp
        lib/schema-parser/token.hpp
        lib/schema-parser/tokenizer-constants.hpp
        lib/schema-parser/tokenizer.cpp
        lib/schema-parser/tokenizer.hpp
        lib/util/auto-pop.hpp
        lib/util/constants.hpp
        lib/util/enum-range.hpp
        lib/util/exception-formats.hpp
        lib/util/file-location.hpp
        lib/util/limits.hpp
        lib/util/narrow.hpp
        lib/util/options-data.hpp
        lib/util/options.hpp
        lib/util/schema.cpp
        lib/util/schema.hpp
        lib/util/text.cpp
        lib/util/text.hpp
        lib/util/timer.hpp
        lib/util/tune.hpp
        lib/util/util.hpp
        lib/variable-manager/variable-manager.cpp
        lib/variable-manager/variable-manager.hpp
        lib/zlib/constants.hpp
        lib/zlib/zlib-engine.cpp
        lib/zlib/zlib-engine.hpp
        lib/zlib/zstream.cpp
        lib/zlib/zstream.hpp
)

add_library(c4lib ${LIB_SOURCE_FILES} ${INCLUDE_SOURCE_FILES})
set_target_properties(c4lib PROPERTIES OUTPUT_NAME "c4")
target_include_directories(c4lib SYSTEM PRIVATE ${Boost_INCLUDE_DIRS})
target_include_directories(c4lib PRIVATE ${C4_INCLUDE_ROOT} ${CMAKE_CURRENT_BINARY_DIR})

add_executable(c4edit ${EXE_SOURCE_FILES})
target_include_directories(c4edit SYSTEM PRIVATE ${Boost_INCLUDE_DIRS})
target_include_directories(c4edit PRIVATE ${C4_INCLUDE_ROOT} ${CMAKE_CURRENT_BINARY_DIR})

set(LATEST_BUILD_MANIFEST ${C4_ROOT}/test/data/misc/latest-build.txt)
cmake_path(CONVERT "${LATEST_BUILD_MANIFEST}" TO_NATIVE_PATH_LIST LATEST_BUILD_MANIFEST_NATIVE)
get_target_property(C4EDIT_NAME c4edit NAME)
set(LATEST_EXE_BUILD ${CMAKE_BINARY_DIR}/${C4EDIT_NAME})
cmake_path(CONVERT "${LATEST_EXE_BUILD}" TO_NATIVE_PATH_LIST LATEST_EXE_BUILD_NATIVE)
add_custom_target(record_latest_build
        COMMENT "Recording build"
        COMMAND echo ${LATEST_EXE_BUILD_NATIVE} > ${LATEST_BUILD_MANIFEST_NATIVE}
)
add_dependencies(c4edit
        record_latest_build
)

target_link_libraries(c4edit c4lib ${ZLIB_LIBRARIES} -static-libgcc -static-libstdc++)

# Flags common to both Clang and GNU:
# -D<macroname>=<value>: Adds an implicit #define into the predefines buffer which is read before the source file is preprocessed.
#     -D_FORTIFY_SOURCE: controls hardening of calls into some functions in the GNU C Library
#        See https://www.gnu.org/software/libc/manual/html_node/Source-Fortification.html
# -E: Nothing is done except preprocessing
# -fcf-protection=full: Enable code instrumentation of control-flow transfers to increase program security by checking that target
#     addresses of control-flow transfer instructions (such as indirect function call, function return, indirect jump) are valid.
# -fstack-protector-all: Emit extra code to check for buffer overflows, such as stack smashing attacks.
# -mshstk: enables shadow stack built-in functions from x86 Control-flow Enforcement Technology (CET).
# -M: Instead of outputting the result of preprocessing, output a rule suitable for make describing the dependencies of the main source file.
#     Implies -E.
# -MD: Equivalent to -M -MF file, except that -E is not implied
# -MF <file>: Specified the file to which dependencies are written.
# -MT <target>: Change the target of the rule emitted by dependency generation
# -O0: Disables most optimizations
# -O3: Generate aggressively optimized code performing optimizations that might slow down compilation
# -Wall: This enables all the warnings about constructions that some users consider questionable, and that are easy to avoid.
# -Wconversion: Warn for implicit conversions that may alter a value.
# -Werror: Treat warnings as errors
# -Wextra: This enables some extra warning flags that are not enabled by -Wall
# -Wno-xxxx: Disables warning xxxx
#     -Wno-unknown-pragmas: Disables unknown pragma ignored.  This is necessary b/c we're using pragmas from MSVC.
# -Wpedantic: Issue all the warnings demanded by strict ISO C and ISO C++.
# -Wshadow: Warn whenever a local variable or type declaration shadows another variable, parameter, type,
#      class member (in C++), or instance variable

if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    # Note: Use https://clang.llvm.org/docs/search.html to search for information about Clang including compiler options
    # -flto: Enable link-time optimization.
    # -fno-sanitize-trap=all: Causes CFI to print a diagnostic before the program aborts.  Use only for debug builds.
    # -fsanitize=cfi: Enables Clang control flow integrity schemes.  Relies on -flto and, for some flags, on -fvisibility=hidden.
    # -fvisibility=hidden: Enables CFI checks for certain classes.
    if (FUZZTEST_ENABLED)
        # Google fuzztest sources generate numerous warnings if compiled with -Wall etc.  Skip the extra warnings and disable
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-unknown-pragmas -Wno-sign-conversion")
    else ()
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wconversion -Werror -Wextra -Wpedantic -Wshadow -Wno-unknown-pragmas")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_FORTIFY_SOURCE=3 -fstack-protector-all -mshstk -fcf-protection=full")
        # Unfortunately the CFI flags are causing Clang to crash and are therefore currently disabled.
        # See https://github.com/llvm/llvm-project/issues/122373 which I filed to track this bug.
        # set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=cfi -flto -fvisibility=hidden")
        if (CMAKE_BUILD_TYPE STREQUAL "Debug")
            set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-sanitize-trap=all")
        endif ()
    endif ()

    if (UNIX)
        # Google Sanitizers.
        # Run Memory Sanitizer separately b/c MSAN is not compatible with ASAN.  Memory Sanitizer is available only under clang for Linux.
        # Memory Sanitizer either hangs or is taking far too long to use and is thus disabled.  Additionally, the output MSAN generates
        # is full of false-positives, presumably because the c++ library we're linking against wasn't instrumented with MSAN.
        # Finally, running MSAN may consume a lot of disk space.  MSAN when run under CLion creates a temporary file in
        # C:\Users\<Username>\AppData\Local\Temp\clion-sanitizers<xxx>.  This file grew roughly 300GB when MSAN ran overnight.
        # Fortunately CLion deletes this file on exit; if, however, c-drive disk space runs low after using MSAN, check the
        # temp directory above in case CLion failed to delete the MSAN file.
        # set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=memory -fPIE -fno-omit-frame-pointer -g -fno-optimize-sibling-calls -O1 -fsanitize-memory-track-origins -fsanitize-recover=memory")
    endif ()
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    # Note: See https://gcc.gnu.org/onlinedocs/gcc/Option-Summary.html to look up GCC compiler flags.
    # -Wnrvo: Warn if the compiler does not elide the copy from a local variable to the return value of a function in a context where it is
    #         allowed by [class.copy.elision]. This elision is commonly known as the Named Return Value Optimization.
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wconversion -Werror -Wextra -Wpedantic -Wshadow -Wno-unknown-pragmas")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_FORTIFY_SOURCE=3 -fstack-protector-all -mshstk -fcf-protection=full")

    if (WIN32)
        # Enable -Wnrvo for non-unit test targets.  -Wnrvo generates warnings against Google test-generated code.
        # -Wnrvo requires GCC version 14 or later and my WSL version is 13.3.0.  For now, we'll use -Wnrvo for Windows only.
        target_compile_options(c4edit PRIVATE -Wnrvo)
        target_compile_options(c4lib PRIVATE -Wnrvo)
    endif ()

    if (UNIX)
        # Google Sanitizers.
        # Run Address, Leak and Undefined Sanitizers at the same time. -Wno-maybe-uninitialized is used to suppress warnings stemming from
        # Boost header files which (for some reason) only appear during sanitized builds.
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address,leak,undefined,null -fno-omit-frame-pointer -g -Wno-maybe-uninitialized")
    endif ()
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    # Note: Use https://learn.microsoft.com/en-us/cpp/build/reference/compiler-options?view=msvc-170 to look up MSVC compiler options.
    # /analyze:external- Performs static code analysis, but skips external headers
    # /fsanitize: Enables compilation of sanitizer instrumentation such as AddressSanitizer.
    # /fsanitize-coverage: Enables compilation of code coverage instrumentation for libraries such as LibFuzzer.
    # /guard:cf: Enable Control Flow Guard.  Requires that /guard:cf also be specified for the linker.
    # /O2: Creates fast code
    # /GS: Buffer security check
    # /RTCs: Enables stack frame run-time error checking (this option should only be used for Debug builds according to Microsoft)
    # /sdl: Enable additional security checks
    # /W4 displays level 1, level 2, and level 3 warnings, and all level 4 (informational) warnings that aren't off by default.
    # /Wall: Enable all warnings, including warnings that are disabled by default.  Note that Wall is far too aggressive.  Use /W4 instead.
    # /wdnnnn: Suppresses the compiler warning that is specified by nnnn.
    #    C4068: unknown pragma
    # /Zc:__cplusplus: enables the __cplusplus preprocessor macro to report an updated value for recent C++ language standards support. By default,
    #     Visual Studio always returns the value 199711L for __cplusplus.
    # /Zi: Generates complete debugging information
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4 /wd4068 /WX /Zc:__cplusplus")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /GS /guard:cf /sdl")
    # Enable /analyze warnings for non-unit test targets.  /analyze generates numerous "potential comparison of a constant with
    # another constant" warnings, including from compiler-generated sources forcing us not to enable /analyze warnings for unit tests.
    target_compile_options(c4edit PRIVATE /analyze:external-)
    target_compile_options(c4lib PRIVATE /analyze:external-)

    add_link_options(/guard:cf)

    if (CMAKE_BUILD_TYPE STREQUAL "Debug")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /RTCs")
    endif ()

    if (PROFILE_ENABLED STREQUAL "1")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Zi")
        add_link_options(/PROFILE)
    endif ()
endif ()

message(STATUS "CMAKE_CXX_FLAGS: ${CMAKE_CXX_FLAGS}")

configure_file(${C4_ROOT}/doc/BTS.schema ${C4_ROOT}/dist/c4edit/BTS.schema COPYONLY)
configure_file(${C4_ROOT}/src/config.xml ${C4_ROOT}/dist/c4edit/config.xml COPYONLY)
configure_file(${C4_ROOT}/LICENSE ${C4_ROOT}/dist/c4edit/LICENSE COPYONLY)
configure_file(${C4_ROOT}/QUICKSTART.md ${C4_ROOT}/dist/c4edit/QUICKSTART.md COPYONLY)

configure_file(${C4_ROOT}/API.md ${C4_ROOT}/dist/c4lib/API.md COPYONLY)
configure_file(${C4_ROOT}/LICENSE ${C4_ROOT}/dist/c4lib/LICENSE COPYONLY)
file(COPY ${C4_ROOT}/include/ DESTINATION ${C4_ROOT}/dist/c4lib/include)

if (WIN32)
    if (CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
        if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
            set(C4EDIT_DIST_NAME "c4edit.exe")
            set(C4LIB_DIST_NAME "libc4-clang.a")
        elseif (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
            set(C4LIB_DIST_NAME "libc4-gnu.a")
        elseif (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
            set(C4LIB_DIST_NAME "c4.lib")
        else ()
            message(FATAL_ERROR "Unsupported compiler ${CMAKE_CXX_COMPILER_ID}")
        endif ()
    endif ()
    if (CMAKE_BUILD_TYPE STREQUAL "Debug")
        if (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
            set(C4LIB_DIST_NAME "c4d.lib")
        endif ()
    endif ()
endif ()

if (DEFINED C4EDIT_DIST_NAME)
    add_custom_command(TARGET c4edit
            POST_BUILD
            COMMENT "Copying c4edit to dist"
            COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:c4edit> ${C4_ROOT}/dist/c4edit/${C4EDIT_DIST_NAME}
    )
endif ()
if (DEFINED C4LIB_DIST_NAME)
    add_custom_command(TARGET c4lib
            POST_BUILD
            COMMENT "Copying c4lib to dist"
            COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:c4lib> ${C4_ROOT}/dist/c4lib/${C4LIB_DIST_NAME}
    )
endif ()

cmake_path(CONVERT "${C4_ROOT}/dist/release/" TO_NATIVE_PATH_LIST RELEASE_NATIVE)
cmake_path(CONVERT "${C4_ROOT}/dist/c4edit" TO_NATIVE_PATH_LIST DIST_C4EDIT_NATIVE)
cmake_path(CONVERT "${C4_ROOT}/dist/c4lib" TO_NATIVE_PATH_LIST DIST_C4LIB_NATIVE)
add_custom_target(make_release
        COMMENT "Creating release"
        COMMAND 7z a -r ${RELEASE_NATIVE}c4edit-${C4LIB_VERSION}.zip ${DIST_C4EDIT_NATIVE}
        COMMAND 7z a -r ${RELEASE_NATIVE}c4lib-${C4LIB_VERSION}.zip ${DIST_C4LIB_NATIVE}
)

add_subdirectory(test)
