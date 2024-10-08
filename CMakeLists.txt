cmake_minimum_required (VERSION 3.10)
project ("comms")

# Available options

option (CC_COMMS_BUILD_UNIT_TESTS "Build unittests." OFF)
option (CC_COMMS_UNIT_TESTS_USE_SANITIZERS "Use sanitizers during unittests. Applicable when unittest are enabled." ${CC_COMMS_BUILD_UNIT_TESTS})
option (CC_COMMS_UNIT_TESTS_USE_VALGRIND "Use valgrind to do extra testing. Applicable when unittest are enabled." OFF)
option (CC_COMMS_WARN_AS_ERR "Treat warning as error" ON)
option (CC_COMMS_USE_CCACHE "Use ccache on UNIX systems if it's available" OFF)
option (CC_COMMS_SKIP_CXX_STANDARD_FORCING "Do NOT force C++ standard to C++11, use compiler's default one." ON)
option (CC_COMMS_EXTALL_EXTRA_CONFIGS "Install extra \"comms\" and \"cc_comms\" cmake configs in addition to \"LibComms\"" OFF)

# Extra variables
# CC_COMMS_EXTERNALS_DIR - Directory where pull externals, defaults to ${PROJECT_SOURCE_DIR}/externals
# CC_COMMS_CCACHE_EXECUTABLE - Path to ccache executable

if (CMAKE_TOOLCHAIN_FILE AND EXISTS ${CMAKE_TOOLCHAIN_FILE})
    message(STATUS "Loading toolchain from ${CMAKE_TOOLCHAIN_FILE}")
endif()

if (NOT CC_COMMS_EXTERNALS_DIR)
    set (CC_COMMS_EXTERNALS_DIR "${PROJECT_SOURCE_DIR}/externals")
endif ()

set (CMAKE_SCRIPTS_DIR "${PROJECT_SOURCE_DIR}/cmake")

################################################

if (("${CMAKE_CXX_STANDARD}" STREQUAL "") AND (NOT CC_COMMS_SKIP_CXX_STANDARD_FORCING))
    set (CMAKE_CXX_STANDARD 11)
endif()

while (TRUE)
    if (NOT CC_COMMS_BUILD_UNIT_TESTS)
        message (STATUS "Unittests are disabled")
        break()
    endif ()
    
    if (CMAKE_CROSSCOMPILING)
        message (STATUS "Not building unittests when cross-compiling")
        break()
    endif ()

    include (${CMAKE_SCRIPTS_DIR}/CC_CxxtestFuncs.cmake)
    cc_get_cxxtest(INSTALL_PREFIX ${CC_COMMS_EXTERNALS_DIR})
    break()
endwhile ()

################################################
# Compiler options
set (extra_opts)
if (CC_COMMS_WARN_AS_ERR)
    list (APPEND extra_opts WARN_AS_ERR)
endif ()

if (CC_COMMS_USE_CCACHE)
    list (APPEND extra_opts USE_CCACHE)
    if (NOT "${CC_COMMS_CCACHE_EXECUTABLE}" STREQUAL "")
        list (APPEND extra_opts CCACHE_EXECUTABLE "${CC_COMMS_CCACHE_EXECUTABLE}")
    endif ()
endif ()

if (CC_COMMS_UNIT_TESTS_USE_SANITIZERS)
    list (APPEND extra_opts DEFAULT_SANITIZERS)
endif ()

include (${CMAKE_SCRIPTS_DIR}/CC_Compile.cmake)
cc_compile(${extra_opts})
cc_msvc_force_warn_opt("/W4")

################################################

include(GNUInstallDirs)

file (READ "${PROJECT_SOURCE_DIR}/include/comms/version.h" version_file)
string (REGEX MATCH "COMMS_MAJOR_VERSION ([0-9]*)U*" _ ${version_file})
set (major_ver ${CMAKE_MATCH_1})
string (REGEX MATCH "COMMS_MINOR_VERSION ([0-9]*)U*" _ ${version_file})
set (minor_ver ${CMAKE_MATCH_1})
string (REGEX MATCH "COMMS_PATCH_VERSION ([0-9]*)U*" _ ${version_file})
set (patch_ver ${CMAKE_MATCH_1})
set (COMMS_VERSION "${major_ver}.${minor_ver}.${patch_ver}")

include(CMakePackageConfigHelpers)
write_basic_package_version_file(
    ${CMAKE_BINARY_DIR}/LibCommsConfigVersion.cmake
    VERSION ${COMMS_VERSION}
    COMPATIBILITY AnyNewerVersion)

set (LIB_COMMS_CMAKE_FILES
    ${PROJECT_SOURCE_DIR}/cmake/CC_Compile.cmake
    ${PROJECT_SOURCE_DIR}/cmake/CC_CommsExternal.cmake
    ${PROJECT_SOURCE_DIR}/cmake/CC_CxxtestFuncs.cmake
    ${PROJECT_SOURCE_DIR}/cmake/CC_DocCleanupScript.cmake
    ${PROJECT_SOURCE_DIR}/cmake/CC_CommsPrefetch.cmake
    ${PROJECT_SOURCE_DIR}/cmake/CC_RunWithValgrindScript.cmake
    ${PROJECT_SOURCE_DIR}/cmake/LibCommsConfig.cmake
    ${CMAKE_BINARY_DIR}/LibCommsConfigVersion.cmake
) 

find_package (Doxygen)
if (DOXYGEN_FOUND)
    set (doc_output_dir "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_DATAROOTDIR}/doc/comms")
    set (output_file "${CMAKE_CURRENT_BINARY_DIR}/doxygen.conf")

    set (out_dir_match_str "OUTPUT_DIRECTORY[^\n]*")
    set (out_dir_replacement_str "OUTPUT_DIRECTORY = ${doc_output_dir}")

    set (config_file "${CMAKE_CURRENT_SOURCE_DIR}/doxygen/doxygen.conf")
    file (READ ${config_file} config_text)
    string (REGEX REPLACE "${out_dir_match_str}" "${out_dir_replacement_str}" modified_config_text "${config_text}")

    if (UNIX)
        set (dia_path_match_str "DIA_PATH[^\n]*")
        set (dia_path_replacement_str "DIA_PATH = ${PROJECT_SOURCE_DIR}/script")
        string (REGEX REPLACE "${dia_path_match_str}" "${dia_path_replacement_str}" modified_config_text "${modified_config_text}")
    endif ()

    file (WRITE "${output_file}" "${modified_config_text}")

    add_custom_target ("doc_comms"
            COMMAND ${CMAKE_COMMAND} -E make_directory ${doc_output_dir}
            COMMAND ${DOXYGEN_EXECUTABLE} ${output_file}
            COMMAND ${CMAKE_COMMAND} -DDOC_OUTPUT_DIR="${doc_output_dir}" -P 
                            ${PROJECT_SOURCE_DIR}/cmake/CC_DocCleanupScript.cmake
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
endif ()

add_library(comms INTERFACE)
add_library (cc::comms ALIAS comms)

target_include_directories(comms INTERFACE
  $<INSTALL_INTERFACE:include>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)

install (
    FILES ${LIB_COMMS_CMAKE_FILES}
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/LibComms/cmake/
)

install(TARGETS comms EXPORT commsExport)
install(EXPORT commsExport NAMESPACE cc::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/LibComms/cmake
)

install (
    DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/comms
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

if (CC_COMMS_EXTALL_EXTRA_CONFIGS)
    install(CODE "set (install_lib_dir ${CMAKE_INSTALL_FULL_LIBDIR})")
    install(CODE [[
        message (STATUS "Installing extra cmake configs into ${install_lib_dir}")

        if (UNIX)
            execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink LibComms ${install_lib_dir}/comms)
            execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink LibComms ${install_lib_dir}/cc_comms)
            execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink LibCommsConfig.cmake ${install_lib_dir}/comms/cmake/commsConfig.cmake)
            execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink LibCommsConfig.cmake ${install_lib_dir}/cc_comms/cmake/cc_commsConfig.cmake)
        elseif (WIN32)
            execute_process(COMMAND ${CMAKE_COMMAND} -E copy_directory ${install_lib_dir}/LibComms ${install_lib_dir}/comms)
            execute_process(COMMAND ${CMAKE_COMMAND} -E copy_directory ${install_lib_dir}/LibComms ${install_lib_dir}/cc_comms)
            execute_process(COMMAND ${CMAKE_COMMAND} -E copy ${install_lib_dir}/comms/cmake/LibCommsConfig.cmake ${install_lib_dir}/comms/cmake/commsConfig.cmake)
            execute_process(COMMAND ${CMAKE_COMMAND} -E copy ${install_lib_dir}/cc_comms/cmake/LibCommsConfig.cmake ${install_lib_dir}/cc_comms/cmake/cc_commsConfig.cmake)
        endif ()
    ]])
endif ()

add_subdirectory (test)

