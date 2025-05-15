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

add_custom_command(
    TARGET PlaneApp POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/renderer/shaders ${CMAKE_BINARY_DIR}/Engine/App/shaders
    COMMENT "Copying shaders to executable directory"
)
