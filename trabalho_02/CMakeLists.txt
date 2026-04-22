add_executable(trabalho_02
    main.cpp
    utils.hpp
)

target_link_libraries(trabalho_02
    PRIVATE
        renderer
        geometry
        error
        
        OpenGL::GL
        glfw
        imgui::imgui
)