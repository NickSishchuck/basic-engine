cmake_minimum_required(VERSION 3.10)
project(PlaneEngine VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Global compiler options
if(MSVC)
    add_compile_options(/W4)
else()
    add_compile_options(-Wall -Wextra)
endif()

# Add the renderer submodule
add_subdirectory(renderer)

# Add engine modules
add_subdirectory(Engine/Common)
add_subdirectory(Engine/Logic)
add_subdirectory(Engine/App)

# Create logs directory
file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/logs)
