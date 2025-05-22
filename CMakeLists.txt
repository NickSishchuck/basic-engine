cmake_minimum_required(VERSION 3.10)
project(PlaneEngine VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Option to enable Windows cross-compilation from Linux
option(BUILD_FOR_WINDOWS "Cross-compile for Windows using MinGW" OFF)

# Detect cross-compilation automatically if using MinGW
if(CMAKE_CROSSCOMPILING AND WIN32)
    set(BUILD_FOR_WINDOWS ON)
    message(STATUS "Detected Windows cross-compilation")
endif()

# Global compiler options
if(MSVC)
    add_compile_options(/W4)
elseif(BUILD_FOR_WINDOWS OR MINGW)
    # MinGW-specific settings
    add_compile_options(-Wall -Wextra)
    message(STATUS "Configuring for Windows build with MinGW")

    # Static linking for easier distribution
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static-libgcc -static-libstdc++")
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -static-libgcc -static-libstdc++")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static-libgcc -static-libstdc++")

    # Help CMake find MinGW libraries
    set(CMAKE_FIND_ROOT_PATH /usr/x86_64-w64-mingw32)
    set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
    set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

    # Windows-specific libraries
    find_library(OPENGL32_LIBRARY opengl32)
    find_library(GDI32_LIBRARY gdi32)
    find_library(USER32_LIBRARY user32)
    find_library(KERNEL32_LIBRARY kernel32)

else()
    add_compile_options(-Wall -Wextra)
endif()

# Platform-specific library handling
if(BUILD_FOR_WINDOWS OR WIN32)
    # Windows library paths for cross-compilation
    if(BUILD_FOR_WINDOWS)
        set(CMAKE_PREFIX_PATH
            ${CMAKE_PREFIX_PATH}
            /usr/x86_64-w64-mingw32
            /usr/x86_64-w64-mingw32/lib
            /usr/x86_64-w64-mingw32/include
        )
    endif()

    # Find Windows-specific OpenGL libraries
    find_package(OpenGL REQUIRED)

elseif(UNIX AND NOT APPLE)
    # Linux-specific libraries
    find_package(OpenGL REQUIRED)
    find_package(X11 REQUIRED)

elseif(APPLE)
    # macOS-specific libraries
    find_package(OpenGL REQUIRED)
endif()

# Add the renderer submodule
add_subdirectory(renderer)

# Add engine modules
add_subdirectory(Engine/Common)
add_subdirectory(Engine/Logic)
add_subdirectory(Engine/App)

# Create logs directory
file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/logs)

# Print build configuration
message(STATUS "=== PlaneEngine Build Configuration ===")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "C++ Standard: ${CMAKE_CXX_STANDARD}")
if(BUILD_FOR_WINDOWS)
    message(STATUS "Target Platform: Windows (Cross-compilation)")
    message(STATUS "Compiler: ${CMAKE_CXX_COMPILER}")
    message(STATUS "Static linking: Enabled")
else()
    message(STATUS "Target Platform: ${CMAKE_SYSTEM_NAME}")
endif()
message(STATUS "========================================")
