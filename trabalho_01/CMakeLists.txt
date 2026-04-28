add_executable(trabalho_01
    main.cpp
)

target_link_libraries(trabalho_01
    PRIVATE
        renderer
        geometry
        result
        
        OpenGL::GL
        glfw
        imgui::imgui
)