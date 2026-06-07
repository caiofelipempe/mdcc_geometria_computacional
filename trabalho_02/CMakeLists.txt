add_executable(trabalho_02
    main.cpp
    utils/file_saver.hpp
    utils/file_selector.hpp
    utils/convex_hull.hpp
    utils/obj_model.hpp
)

target_link_libraries(trabalho_02
    PRIVATE
        renderer
        geometry
        result
        sort
        
        OpenGL::GL
        OpenGL::GLU
        GLEW::GLEW
        glfw
        imgui::imgui
        CGAL::CGAL
)

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    target_compile_options(trabalho_02 PRIVATE -mavx -mavx2 -mfma)
endif()