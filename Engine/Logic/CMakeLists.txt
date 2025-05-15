set(LOGIC_SOURCES
    src/Entity.cpp
)

set(LOGIC_HEADERS
    include/Component.h
    include/Entity.h
)

add_library(Logic STATIC ${LOGIC_SOURCES} ${LOGIC_HEADERS})

target_include_directories(Logic
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(Logic
    PUBLIC
        Common
)
