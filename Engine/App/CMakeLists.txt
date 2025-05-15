set(APP_SOURCES
    src/main.cpp
)

add_executable(AircraftSim ${APP_SOURCES})

target_include_directories(AircraftSim
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(AircraftSim
    PRIVATE
        OpenGLRenderer  # Link to the renderer submodule
        Common
        Logic
)
