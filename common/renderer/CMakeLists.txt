add_library(common_renderer STATIC
    renderer.cpp
)

target_include_directories(common_renderer PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(common_renderer PUBLIC
    OpenGL::GL
    glfw
    imgui::imgui
)