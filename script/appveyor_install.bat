IF "%APPVEYOR_BUILD_WORKER_IMAGE%"=="Visual Studio 2015" (
    set TOOLCHAIN=msvc14
    IF "%PLATFORM%"=="x86" (
        echo Performing x86 build in VS2015
        call "C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\vcvarsall.bat" x86
    ) ELSE (
        echo Performing amd64 build in VS2015
        call "C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\vcvarsall.bat" amd64
    )
) ELSE IF "%APPVEYOR_BUILD_WORKER_IMAGE%"=="Visual Studio 2017" (
    set TOOLCHAIN=msvc15
    IF "%PLATFORM%"=="x86" (
        echo Performing x86 build in VS2017
        call "C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\VC\Auxiliary\Build\vcvars32.bat"
    ) ELSE (
        echo Performing amd64 build in VS2017
        call "C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\VC\Auxiliary\Build\vcvars64.bat"
    )
    
) ELSE IF "%APPVEYOR_BUILD_WORKER_IMAGE%"=="Visual Studio 2019" (
    set TOOLCHAIN=msvc16
    IF "%PLATFORM%"=="x86" (
        echo Performing x86 build in VS2019
        call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvars32.bat"
    ) ELSE (
        echo Performing amd64 build in VS2019
        call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvars64.bat"
    )
) ELSE IF "%APPVEYOR_BUILD_WORKER_IMAGE%"=="Visual Studio 2022" (
    set TOOLCHAIN=msvc17
    IF "%PLATFORM%"=="x86" (
        echo Performing x86 build in VS2022
        call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars32.bat"
    ) ELSE (
        echo Performing amd64 build in VS2022
        call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"
    )    
) ELSE (
    echo Toolchain %TOOLCHAIN% is not supported
    exit -1
)

