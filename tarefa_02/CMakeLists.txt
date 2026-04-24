add_executable(tarefa_02
    main.cpp
    utils.hpp
)

target_link_libraries(tarefa_02
    PRIVATE
        renderer
        geometry
        result
        
        OpenGL::GL
        glfw
        imgui::imgui
)