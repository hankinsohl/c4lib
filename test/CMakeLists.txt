project(c4libtest)

set(TEST_OUT ${C4_ROOT}/test/out)
set(TO_RTT ${TEST_OUT}/round-trip-test)
add_custom_target(clean_test_out
        COMMENT "Cleaning test/out"
        COMMAND ${CMAKE_COMMAND} -E rm -Rf "${TEST_OUT}"
        COMMAND ${CMAKE_COMMAND} -E make_directory "${TEST_OUT}/common"
        COMMAND ${CMAKE_COMMAND} -E make_directory "${TO_RTT}"
        COMMAND ${CMAKE_COMMAND} -E make_directory "${TO_RTT}/Brennus BC-4000"
        COMMAND ${CMAKE_COMMAND} -E make_directory "${TO_RTT}/Brennus BC-4000-2"
        COMMAND ${CMAKE_COMMAND} -E make_directory "${TO_RTT}/LMA"
        COMMAND ${CMAKE_COMMAND} -E make_directory "${TO_RTT}/Mao Zedong_1936-AD_Feb-26-2023_07-31-57"
        COMMAND ${CMAKE_COMMAND} -E make_directory "${TO_RTT}/NC269-Gilgamesh AD-1735"
        COMMAND ${CMAKE_COMMAND} -E make_directory "${TO_RTT}/Parrots"
        COMMAND ${CMAKE_COMMAND} -E make_directory "${TO_RTT}/Play_By_Email"
        COMMAND ${CMAKE_COMMAND} -E make_directory "${TO_RTT}/Tiny-Map-BC-4000"
)

FetchContent_Declare(
        googletest
        # Specify the commit you depend on and update it regularly.
        URL https://github.com/google/googletest/archive/refs/tags/v1.15.0.zip
)

# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

set(TEST_SOURCE_FILES
        integration/round-trip-test.cpp
        unit/definition-table-test.cpp
        unit/expression-parser-test.cpp
        unit/importer-test.cpp
        unit/logger-test.cpp
        unit/md5-test.cpp
        unit/options-manager-test-data.hpp
        unit/options-manager-test.cpp
        unit/path-test.cpp
        unit/recursive-node-source-test.cpp
        unit/schema-parser-p1-test.cpp
        unit/tokenizer-test.cpp
        unit/types-in-test-data.hpp
        unit/types-test.cpp
        unit/write-translation-test.cpp
        unit/zlib-engine-test.cpp
        util/constants.hpp
        util/macros.hpp
        util/util.cpp
        util/util.hpp
)

add_executable(c4libtest ${TEST_SOURCE_FILES})

add_dependencies(c4libtest
        clean_test_out
        c4edit
)
target_link_libraries(c4libtest PRIVATE gtest gtest_main c4lib ${ZLIB_LIBRARIES})
target_include_directories(c4libtest SYSTEM PRIVATE ${Boost_INCLUDE_DIRS} ${gtest_SOURCE_DIR} ${gtest_SOURCE_DIR}/include)
target_include_directories(c4libtest PRIVATE ${C4_INCLUDE_ROOT} ${CMAKE_CURRENT_BINARY_DIR})
if (FUZZTEST_ENABLED)
    link_fuzztest(c4libtest)
endif ()
