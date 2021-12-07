# This file contains several helper functions to allow easy 
# inclusion of the CommsChampion project in other cmake project.

# Available functions  are:
#
# ******************************************************
# - Pull comms sources.
#     cc_comms_pull_sources(
#         SRC_DIR <src_dir>
#         [TAG <tag>]
#         [REPO <repo>]
#     )
# - SRC_DIR - A directory where comms sources will end up.
# - TAG - Override the default tag to checkout.
# - REPO - Override the default repository of the comms.
#
# ******************************************************
# - Build comms during cmake configuration stage. Use it 
#   when the compilation process is light, i.e. only comms library is installed.
#     cc_comms_build_during_config(
#         SRC_DIR <src_dir>
#         [BUILD_DIR <build_dir>]
#         [TAG <tag>]
#         [REPO <repo>]
#         [CMAKE_ARGS <arg1> <arg2> ...]
#         [NO_DEFAULT_CMAKE_ARGS]
#         [NO_REPO]
#     )
# - SRC_DIR - A directory where comms sources will end up.
# - BUILD_DIR - A build directory, defaults to ${PROJECT_BINARY_DIR}/comms
# - TAG - Override the default tag to checkout.
# - REPO - Override the default repository of the comms.
# - CMAKE_ARGS - Extra cmake arguments to be passed to the comms project.
#       Use CMAKE_INSTALL_PREFIX to specify where the output needs to be installed.
# - NO_DEFAULT_CMAKE_ARGS - Exclude passing the extra cmake arguments that copy the 
#       following values from the invoking project:
#         * CMAKE_C_COMPILER
#         * CMAKE_CXX_COMPILER
#         * CMAKE_TOOLCHAIN_FILE
#         * CMAKE_GENERATOR
#         * CMAKE_BUILD_TYPE
#         * CMAKE_CXX_STANDARD=${CMAKE_CXX_STANDARD}
#       The default values are passed before ones specified in CMAKE_ARGS, which can overwrite 
#       some of the default values.
# - NO_REPO - Don't checkout sources, SRC_DIR must contain checkout out sources, suitable 
#       for this repo being a submodule.
#
# ******************************************************
# - Build comms as external project during normal build process.
#     cc_comms_build_as_external_project(
#         TGT <target_name>
#         SRC_DIR <src_dir>
#         [BUILD_DIR <build_dir>]
#         [INSTALL_DIR] <install_dir>]
#         [TAG <tag>]
#         [REPO <repo>]
#         [CMAKE_ARGS <arg1> <arg2> ...]
#         [NO_REPO]
#         [UPDATE_DISCONNECTED]
#         [NO_DEFAULT_CMAKE_ARGS]
#     )
# - TGT - Name of CMake target that can be used to establish dependencies.
# - SRC_DIR - A directory where comms sources will end up.
# - BUILD_DIR - A directory where comms will be build.
# - INSTALL_DIR - A directory where comms will be installed, also passed as 
#       CMAKE_INSTALL_PREFIX in addition to provided CMAKE_ARGS.
# - TAG - Override the default tag to checkout (unless NO_REPO param is used).
# - REPO - Override the default repository of the comms (unless NO_REPO param is used).
# - CMAKE_ARGS - Extra cmake arguments to be passed to the comms project.
# - NO_REPO - Don't checkout sources, SRC_DIR must contain checkout out sources, 
#       suitable for this repo being a submodule.
# - UPDATE_DISCONNECTED - Pass "UPDATE_DISCONNECTED 1" to ExternalProject_Add()
# - NO_DEFAULT_CMAKE_ARGS - Exclude passing the extra cmake arguments that copy the 
#       following values from the invoking project:
#         * CMAKE_C_COMPILER
#         * CMAKE_CXX_COMPILER
#         * CMAKE_TOOLCHAIN_FILE
#         * CMAKE_GENERATOR
#         * CMAKE_BUILD_TYPE
#         * CMAKE_CXX_STANDARD=${CMAKE_CXX_STANDARD}
#       The default values are passed before ones specified in CMAKE_ARGS, which can overwrite 
#       some of the copied values.
#
# ******************************************************

set (CC_EXTERNAL_DEFAULT_REPO "https://github.com/commschamp/comms.git")
set (CC_EXTERNAL_DEFAULT_TAG "master")

function (cc_comms_pull_sources)
    set (_prefix CC_PULL)
    set (_options)
    set (_oneValueArgs SRC_DIR REPO TAG)
    set (_mutiValueArgs)
    cmake_parse_arguments(${_prefix} "${_options}" "${_oneValueArgs}" "${_mutiValueArgs}" ${ARGN})

    if (NOT CC_PULL_SRC_DIR)
        message (FATAL_ERROR "The SRC_DIR parameter is not provided")
    endif ()     

    if (NOT CC_PULL_REPO)
        set (CC_PULL_REPO ${CC_EXTERNAL_DEFAULT_REPO})
    endif ()  

    if (NOT CC_PULL_TAG)
        set (CC_PULL_TAG ${CC_EXTERNAL_DEFAULT_TAG})
    endif ()  

    if (NOT GIT_FOUND)
        find_package(Git REQUIRED)
    endif ()

    if (EXISTS "${CC_PULL_SRC_DIR}/.git")
        message (STATUS "Updating existing comms repository")
        execute_process (
            COMMAND ${GIT_EXECUTABLE} pull 
            WORKING_DIRECTORY ${CC_PULL_SRC_DIR}
        )
    
        execute_process (
            COMMAND ${GIT_EXECUTABLE} checkout ${CC_PULL_TAG}
            WORKING_DIRECTORY ${CC_PULL_SRC_DIR}
            RESULT_VARIABLE git_result
        )
        
        if (NOT "${git_result}" STREQUAL "0")
            message (WARNING "git checkout failed")
        endif ()

        return()
    endif()

    message (STATUS "Cloning comms repository from scratch")

    execute_process (
        COMMAND ${CMAKE_COMMAND} -E remove_directory "${CC_PULL_SRC_DIR}"
    )
    
    execute_process (
        COMMAND ${CMAKE_COMMAND} -E make_directory "${CC_PULL_SRC_DIR}"
    )        
    
    execute_process (
        COMMAND 
            ${GIT_EXECUTABLE} clone -b ${CC_PULL_TAG} ${CC_PULL_REPO} ${CC_PULL_SRC_DIR}
        RESULT_VARIABLE git_result
    )

    if (NOT "${git_result}" STREQUAL "0")
        message (WARNING "git clone/checkout failed")
    endif ()
endfunction ()

function (cc_comms_build_during_config)
    set (_prefix CC_BUILD)
    set (_options NO_DEFAULT_CMAKE_ARGS NO_REPO)
    set (_oneValueArgs SRC_DIR BUILD_DIR REPO TAG)
    set (_mutiValueArgs CMAKE_ARGS)
    cmake_parse_arguments(${_prefix} "${_options}" "${_oneValueArgs}" "${_mutiValueArgs}" ${ARGN})

    if (NOT CC_BUILD_SRC_DIR)
        message (FATAL_ERROR "The SRC_DIR parameter is not provided")
    endif () 

    if (NOT CC_BUILD_BUILD_DIR)
        set (CC_BUILD_BUILD_DIR ${PROJECT_BINARY_DIR}/comms)
    endif ()      

    if (NOT CC_BUILD_REPO)
        set (CC_BUILD_REPO ${CC_EXTERNAL_DEFAULT_REPO})
    endif ()  

    if (NOT CC_BUILD_TAG)
        set (CC_BUILD_TAG ${CC_EXTERNAL_DEFAULT_TAG})
    endif () 

    if (NOT CC_BUILD_NO_REPO)
        cc_comms_pull_sources(SRC_DIR ${CC_BUILD_SRC_DIR} REPO ${CC_BUILD_REPO} TAG ${CC_BUILD_TAG})      
    endif ()

    execute_process (
        COMMAND ${CMAKE_COMMAND} -E make_directory "${CC_BUILD_BUILD_DIR}"
    )

    set (extra_cmake_args)
    if (NOT CC_BUILD_NO_DEFAULT_CMAKE_ARGS)
        set (extra_cmake_args
            -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER} 
            -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}
            -DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE}
            -DCMAKE_GENERATOR=${CMAKE_GENERATOR}
            -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
            -DCMAKE_CXX_STANDARD=${CMAKE_CXX_STANDARD}
        )
    endif ()

    execute_process(
        COMMAND ${CMAKE_COMMAND} ${CC_BUILD_SRC_DIR}
            ${extra_cmake_args} ${CC_BUILD_CMAKE_ARGS}
        WORKING_DIRECTORY ${CC_BUILD_BUILD_DIR}
        RESULT_VARIABLE cc_cmake_result
    )

    if (NOT ${cc_cmake_result} EQUAL 0)
        message (FATAL_ERROR "CMake run for comms has failed with result ${cc_cmake_result}")
    endif ()

    execute_process(
        COMMAND ${CMAKE_COMMAND} --build ${CC_BUILD_BUILD_DIR} --target install
        WORKING_DIRECTORY ${CC_BUILD_BUILD_DIR}
        RESULT_VARIABLE cc_build_result
    )

    if (NOT ${cc_build_result} EQUAL 0)
        message (FATAL_ERROR "Build of comms has failed with result ${cc_build_result}")
    endif ()    
endfunction ()

macro (cc_comms_define_external_project_target inst_dir)
    set (CC_ROOT_DIR ${inst_dir})
    set (CC_COMMS_FOUND TRUE)
    set (CC_INCLUDE_DIRS "${inst_dir}/include")

    add_library(cc::comms INTERFACE IMPORTED GLOBAL)
    execute_process(
        COMMAND ${CMAKE_COMMAND} -E make_directory ${CC_INCLUDE_DIRS}
    )    
    target_include_directories(cc::comms INTERFACE ${CC_INCLUDE_DIRS})
endmacro ()

function (cc_comms_build_as_external_project)
    set (_prefix CC_EXTERNAL_PROJ)
    set (_options UPDATE_DISCONNECTED NO_DEFAULT_CMAKE_ARGS NO_REPO)
    set (_oneValueArgs SRC_DIR BUILD_DIR INSTALL_DIR REPO TAG TGT)
    set (_mutiValueArgs CMAKE_ARGS)
    cmake_parse_arguments(${_prefix} "${_options}" "${_oneValueArgs}" "${_mutiValueArgs}" ${ARGN})

    if (NOT CC_EXTERNAL_PROJ_SRC_DIR)
        message (FATAL_ERROR "The SRC_DIR parameter is not provided")
    endif () 

    if (NOT CC_EXTERNAL_PROJ_BUILD_DIR)
        set (CC_EXTERNAL_PROJ_BUILD_DIR ${PROJECT_BINARY_DIR}/comms)
    endif ()   

    if (NOT CC_EXTERNAL_PROJ_INSTALL_DIR)
        set (CC_EXTERNAL_PROJ_INSTALL_DIR ${CC_EXTERNAL_PROJ_BUILD_DIR}/install)
    endif ()  

    set (repo_param)
    set (repo_tag_param)
    if (NOT CC_EXTERNAL_PROJ_NO_REPO)
        if (NOT CC_EXTERNAL_PROJ_REPO)
            set (CC_EXTERNAL_PROJ_REPO ${CC_EXTERNAL_DEFAULT_REPO})
        endif ()  

        if (NOT CC_EXTERNAL_PROJ_TAG)
            set (CC_EXTERNAL_PROJ_TAG ${CC_EXTERNAL_DEFAULT_TAG})
        endif ()            


        set (repo_param GIT_REPOSITORY "${CC_EXTERNAL_PROJ_REPO}")
        set (repo_tag_param GIT_TAG "${CC_EXTERNAL_PROJ_TAG}")
    endif ()

    if (NOT CC_EXTERNAL_PROJ_TGT)
        set (CC_EXTERNAL_PROJ_TGT "cc_external_proj_tgt")
    endif ()

    set (cc_update_disconnected_opt)
    if (CC_EXTERNAL_PROJ_UPDATE_DISCONNECTED)
        set (cc_update_disconnected_opt "UPDATE_DISCONNECTED 1")
    endif ()    

    set (extra_cmake_args)
    if (NOT CC_EXTERNAL_PROJ_NO_DEFAULT_CMAKE_ARGS)
        set (extra_cmake_args
            -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER} 
            -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}
            -DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE}
            -DCMAKE_GENERATOR=${CMAKE_GENERATOR}
            -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
            -DCMAKE_CXX_STANDARD=${CMAKE_CXX_STANDARD}
        )
    endif ()

    include(ExternalProject)
    ExternalProject_Add(
        "${CC_EXTERNAL_PROJ_TGT}"
        PREFIX "${CC_EXTERNAL_PROJ_BUILD_DIR}"
        ${repo_param}
        ${repo_tag_param}
        SOURCE_DIR "${CC_EXTERNAL_PROJ_SRC_DIR}"
        BINARY_DIR "${CC_EXTERNAL_PROJ_BUILD_DIR}"
        INSTALL_DIR "${CC_EXTERNAL_PROJ_INSTALL_DIR}"
        ${cc_update_disconnected_opt}
        CMAKE_ARGS 
            ${extra_cmake_args}
            -DCMAKE_INSTALL_PREFIX=${CC_EXTERNAL_PROJ_INSTALL_DIR}
            ${CC_EXTERNAL_PROJ_CMAKE_ARGS}
    )    
    cc_comms_define_external_project_target(${CC_EXTERNAL_PROJ_INSTALL_DIR})

    if (TARGET cc::comms)
        add_dependencies(cc::comms ${CC_EXTERNAL_PROJ_TGT})
    endif ()
endfunction ()
