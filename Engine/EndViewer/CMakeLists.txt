cmake_minimum_required(VERSION 3.10)

# EndViewer module for End Dimension ray marching renderer

set(ENDVIEWER_HEADERS
    include/SimplexNoise.h
    include/EndDensity.h
    include/EndCamera.h
    include/EndRenderer.h
)

# Header-only library (all implementations in headers for simplicity)
add_library(EndViewer INTERFACE)

target_include_directories(EndViewer
    INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# EndViewer depends on Common (for Shader, VAO, VBO) and renderer (for ImGui)
target_link_libraries(EndViewer
    INTERFACE
        Common
        renderer
)

# Create the End Viewer executable
set(END_VIEWER_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/end_viewer_main.cpp
)

add_executable(EndViewerApp ${END_VIEWER_SOURCES})

target_include_directories(EndViewerApp
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(EndViewerApp
    PRIVATE
        EndViewer
        Common
        renderer
)

# Copy END VIEWER shaders from this module's shaders folder
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/shaders")
  add_custom_command(TARGET EndViewerApp POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/shaders $<TARGET_FILE_DIR:EndViewerApp>/shaders
        COMMENT "Copying End viewer shaders"
    )
endif()

# Also copy the renderer's default shaders (needed for base functionality)
add_custom_command(TARGET EndViewerApp POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/renderer/shaders $<TARGET_FILE_DIR:EndViewerApp>/shaders
    COMMENT "Copying renderer shaders"
)
