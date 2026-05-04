add_executable(trabalho_01
    main.cpp
    utils/file_selector.hpp
)

target_link_libraries(trabalho_01
    PRIVATE
        renderer
        geometry
        result
        sort
        
        OpenGL::GL
        glfw
        imgui::imgui
)