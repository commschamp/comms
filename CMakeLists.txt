cmake_minimum_required (VERSION 3.1)
project ("comms")

# Available options

option (CC_INSTALL_COMMS_LIB "Install comms library headers." ON)
option (CC_BUILD_UNIT_TESTS "Build unittests." ${CC_INSTALL_COMMS_LIB})
option (CC_UNIT_TESTS_USE_SANITIZERS "Use sanitizers during unittests. Applicable when unittest are enabled." ON)
option (CC_WARN_AS_ERR "Treat warning as error" ON)
option (CC_USE_CCACHE "Use ccache on UNIX systems if it's available" ON)
option (CC_SKIP_CXX_STANDARD_FORCING "Do NOT force C++ standard to C++11, use compiler's default one." ON)
option (CC_STATIC_RUNTIME "Enable/Disable static runtime" OFF)

# Extra variables
# CC_EXTERNALS_DIR - Directory where pull externals, defaults to 
#   ${PROJECT_SOURCE_DIR}/externals

if (CMAKE_TOOLCHAIN_FILE AND EXISTS ${CMAKE_TOOLCHAIN_FILE})
    message(STATUS "Loading toolchain from ${CMAKE_TOOLCHAIN_FILE}")
endif()

if (NOT CC_EXTERNALS_DIR)
    set (CC_EXTERNALS_DIR "${PROJECT_SOURCE_DIR}/externals")
endif ()

set (CMAKE_SCRIPTS_DIR "${PROJECT_SOURCE_DIR}/cmake")

################################################

if ((NOT CMAKE_CXX_STANDARD) AND (NOT CC_SKIP_CXX_STANDARD_FORCING))
    set (CMAKE_CXX_STANDARD 11)
endif()

while (TRUE)
    if (NOT CC_BUILD_UNIT_TESTS)
        message (STATUS "Unittests are disabled")
        break()
    endif ()
    
    if (CMAKE_CROSSCOMPILING)
        message (STATUS "Not building unittests when cross-compiling")
        break()
    endif ()

    include (${CMAKE_SCRIPTS_DIR}/CC_CxxtestFuncs.cmake)
    cc_get_cxxtest(INSTALL_PREFIX ${CC_EXTERNALS_DIR})
    break()
endwhile ()

################################################
# Compiler options
set (warn_opt)
if (CC_WARN_AS_ERR)
    set (warn_opt WARN_AS_ERR)
endif ()

set (static_runtime_opt)
if (CC_STATIC_RUNTIME)
    set (static_runtime_opt STATIC_RUNTIME)
endif ()

set (ccache_opt)
if ((UNIX) AND (CC_USE_CCACHE))
    set (ccache_opt USE_CCACHE)
endif ()

include (${CMAKE_SCRIPTS_DIR}/CC_Compile.cmake)
cc_compile(${warn_opt} ${static_runtime_opt} ${ccache_opt})
cc_msvc_force_warn_opt("/W4")

################################################

include(GNUInstallDirs)
set (INSTALL_NAME "CommsChampion")
set (LIB_INSTALL_DIR ${CMAKE_INSTALL_LIBDIR})
set (BIN_INSTALL_DIR ${CMAKE_INSTALL_BINDIR})
set (INC_INSTALL_DIR ${CMAKE_INSTALL_INCLUDEDIR})
set (DOC_INSTALL_DIR ${CMAKE_INSTALL_DATAROOTDIR}/doc)

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
    set (doc_output_dir "${CMAKE_INSTALL_PREFIX}/${DOC_INSTALL_DIR}/comms")
    
    set (match_str "OUTPUT_DIRECTORY[^\n]*")
    set (replacement_str "OUTPUT_DIRECTORY = ${doc_output_dir}")
    set (output_file "${CMAKE_CURRENT_BINARY_DIR}/doxygen.conf")

    set (config_file "${CMAKE_CURRENT_SOURCE_DIR}/doxygen/doxygen.conf")
    file (READ ${config_file} config_text)
    string (REGEX REPLACE "${match_str}" "${replacement_str}" modified_config_text "${config_text}")
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

if (CC_INSTALL_COMMS_LIB)
    install (
        FILES ${LIB_COMMS_CMAKE_FILES}
        DESTINATION ${LIB_INSTALL_DIR}/LibComms/cmake/
    )

    install(TARGETS comms EXPORT commsExport)
    install(EXPORT commsExport NAMESPACE cc::
        DESTINATION ${LIB_INSTALL_DIR}/LibComms/cmake
    )

    install (
        DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/comms
        DESTINATION ${INC_INSTALL_DIR}
    )
endif ()    

add_subdirectory (test)

