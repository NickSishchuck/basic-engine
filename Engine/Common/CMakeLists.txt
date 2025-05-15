set(COMMON_SOURCES
    src/RendererWrapper.cpp
)

set(COMMON_HEADERS
    include/RendererInterface.h
    include/RendererWrapper.h
)

add_library(Common STATIC ${COMMON_SOURCES} ${COMMON_HEADERS})

target_include_directories(Common
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(Common
    PUBLIC
        renderer
)
