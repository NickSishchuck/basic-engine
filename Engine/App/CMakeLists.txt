set(APP_SOURCES
    src/main.cpp
)

add_executable(PlaneApp ${APP_SOURCES})

target_include_directories(PlaneApp
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(PlaneApp
    PRIVATE
        Common
        Logic
)
