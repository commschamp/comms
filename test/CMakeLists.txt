# In order to run the unittests the following conditions must be true:
#   - find_package (CxxTest) was exectued, CXXTEST_FOUND is defined and has true value.

if (NOT BUILD_TESTING)
    return ()
endif ()    

set (COMPONENT_NAME "comms")

#################################################################

function (test_func test_suite_name)
    set (tests "${CMAKE_CURRENT_SOURCE_DIR}/${test_suite_name}.th")

    set (name "${COMPONENT_NAME}.${test_suite_name}Test")

    set (valgrand_args)
    if (VALGRIND_EXECUTABLE)
        set (valgrand_args VALGRIND_EXECUTABLE ${VALGRIND_EXECUTABLE})
    endif ()    

    cc_cxxtest_add_test (NAME ${name} SRC ${tests} ${valgrand_args})

    if ("${CMAKE_CXX_STANDARD}" STREQUAL "23")
        # Visual Studio 2025 doesn't seem to properly detect C++23
        target_compile_definitions(${name} PRIVATE
            $<$<CXX_COMPILER_ID:MSVC>:_SILENCE_ALL_CXX23_DEPRECATION_WARNINGS>
        )
    endif ()
endfunction ()

#################################################################

if ((NOT CC_COMMS_UNIT_TESTS_USE_SANITIZERS) AND CC_COMMS_UNIT_TESTS_USE_VALGRIND)
    if ("${CC_VALGRAND_EXE_NAME}" STREQUAL "")
        set (CC_VALGRAND_EXE_NAME "valgrind")
    endif ()
    find_program(VALGRIND_EXECUTABLE NAMES "${CC_VALGRAND_EXE_NAME}")
endif ()

if (TARGET cxxtest::cxxtest)
    test_func ("Fields")
    test_func ("Fields2")
    test_func ("Message")
    test_func ("MsgDataLayer")
    test_func ("MsgIdLayer")
    test_func ("MsgSizeLayer")
    test_func ("SyncPrefixLayer")
    test_func ("ChecksumLayer")
    test_func ("ChecksumPrefixLayer")
    test_func ("TransportValueLayer")
    test_func ("Util")
    test_func ("CustomMsgIdLayer")
    test_func ("CustomMsgSizeLayer")
    test_func ("CustomTransportValueLayer")
    test_func ("CustomChecksumLayer")
    test_func ("CustomChecksumPrefixLayer")
    test_func ("CustomSyncPrefixLayer")
    test_func ("Dispatch")
    test_func ("MsgFactory")
    test_func ("GenericMessage")
else ()
    message (Warning "Testing is enabled, but cxxtest hasn't been found!")
endif ()
