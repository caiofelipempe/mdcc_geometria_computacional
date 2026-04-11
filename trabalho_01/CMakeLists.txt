add_executable(trabalho_01
    main.cpp
)

target_link_libraries(trabalho_01
    PRIVATE
        common
        OpenGL::GL
        glfw
        imgui::imgui
)