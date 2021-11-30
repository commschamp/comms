# Integration with Other CMake Project
There are multiple ways to use COMMS library in other projects.

## Pre-built COMMS library
In case the [comms](https://github.com/arobenko/comms) was already built as an external project
it is recommended to use the following construct.
```
list (APPEND CMAKE_PREFIX_PATH /path/to/comms/install/dir)
find_package(LibComms NO_MODULE)

if (NOT TARGET cc::comms)
    message (FATAL_ERROR "COMMS library is not found")
endif ()

# For libraries that use COMMS headers in their headers
target_link_libraries(my_protocol_library INTERFACE cc::comms)

# For binaries that use COMMS library
target_link_libraries(my_binary PRIVATE cc::comms)
```
If the package was successfully found, the target `cc::comms` needs 
to be defined. Linking to `cc::comms` target will add all the relevant
include paths.

## When external COMMS Library is not available
In case the [comms](https://github.com/arobenko/comms) project is not available, the 
[cmake/CC_CommsExternal.cmake](../cmake/CC_CommsExternal.cmake) script provides
multiple macros / functions which could be used for easy 
integration. Please open [it](../cmake/CC_CommsExternal.cmake) for 
the documentation on available macros / functions.

If the [comms](https://github.com/arobenko/comms) 
sources are not attached to the 
project being developed as a submodule, it is recommended to 
prefetch the sources to get an access to the 
[cmake/CC_CommsExternal.cmake](../cmake/CC_CommsExternal.cmake) script during
the cmake execution.

Please copy the [cmake/CC_CommsPrefetch.cmake](../cmake/CC_CommsPrefetch.cmake)
file to your project (or use it as an example) and then 
prefetch the 
[comms](https://github.com/arobenko/comms) 
sources using following (or similar) 
cmake code.
```
include (${PROJECT_SOURCE_DIR}/cmake/CC_CommsPrefetch.cmake)
cc_comms_prefetch(SRC_DIR /path/to/place/for/comms/project)
```
After the prefetching, the provided 
[cmake/CC_CommsExternal.cmake](../cmake/CC_CommsExternal.cmake) becomes available.

### Build and install COMMS library during CMake configuration
When only COMMS library is required (the unittests are not built), the 
build / install process is very light (just copying headers and cmake files to the destination directory), 
it is possible to perform it during CMake execution using `cc_comms_build_during_config()` function.
```
include(/path/to/comms/cmake/CC_CommsExternal.cmake)

set (cc_comms_install_dir ${PROJECT_BINARY_DIR}/comms/install)
cc_comms_build_during_config(
    SRC_DIR /path/to/comms
    CMAKE_ARGS
        -DCMAKE_INSTALL_PREFIX=${cc_comms_install_dir}
        -DCC_COMMS_BUILD_UNIT_TESTS=OFF 
)
```
The code above will install the COMMS library and all the relevant cmake 
scripts to the directory specified in `cc_comms_install_dir` variable. Please 
remember to use default `CMAKE_INSTALL_PREFIX` variable to specify the destination.

After that it is possible to treat the COMMS library as being externally 
available and use `find_package()` described above.
```
list (APPEND CMAKE_PREFIX_PATH ${cc_comms_install_dir})
find_package(LibComms NO_MODULE REQUIRED)
target_link_libraries(my_binary PRIVATE cc::comms)
```
### Build and install as external project
The CMake provides 
[ExternalProject_Add()](https://cmake.org/cmake/help/v3.0/module/ExternalProject.html)
function which can be used to build and install the 
[comms](https://github.com/arobenko/comms) contents during the 
build process. The same [cmake/CC_CommsExternal.cmake](../cmake/CC_CommsExternal.cmake)
script provides different `cc_comms_build_as_external_project()` function,
which provides a convenient interface to build the 
[comms](https://github.com/arobenko/comms) is such way.
It is recommended to be used when the build process of the 
[comms](https://github.com/arobenko/comms) is 
a bit heavy, i.e. building unittests as well.
```
include(/path/to/comms/cmake/CC_CommsExternal.cmake)
cc_comms_build_as_external_project(
    SRC_DIR /path/to/comms/sources
    BUILD_DIR ${PROJECT_BINARY_DIR}/comms
    INSTALL_DIR ${CMAKE_INSTALL_PREFIX}
)
```
The CMake code above will create CMake target (default hidden name of which can be updated
using TGT parameter), which checks out this project from 
[github](https://github.com/arobenko/comms) (the repo url can be updated
using REPO parameter). If this project sources are already checked out, as git submodule
for example, just pass NO_REPO parameter.
```
include(/path/to/comms/cmake/CC_CommsExternal.cmake)
cc_comms_build_as_external_project(
    SRC_DIR /path/to/comms/sources
    BUILD_DIR ${PROJECT_BINARY_DIR}/comms
    INSTALL_DIR ${CMAKE_INSTALL_PREFIX}
    NO REPO                                    # No checkout, assume sources are in SRC_DIR
)
```

The invocation of the function above will also define `cc::comms` 
cmake link target.
```
target_link_libraries(my_app PRIVATE cc::comms)
```
