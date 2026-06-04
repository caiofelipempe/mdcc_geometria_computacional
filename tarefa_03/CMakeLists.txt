add_executable(tarefa_03
    main.cpp
    utils/convex_hull.hpp
)

target_link_libraries(tarefa_03
    PRIVATE
        renderer
        geometry
        result
        sort
        
        OpenGL::GL
        glfw
        imgui::imgui
)