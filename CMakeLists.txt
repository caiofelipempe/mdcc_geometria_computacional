cmake_minimum_required(VERSION 3.16)

project(
    geometria_computacional
    VERSION 1.0
    DESCRIPTION "Trabalhos do MDCC em C++ com OpenGL, GLFW e ImGui"
    LANGUAGES CXX
)

# -----------------------------
# Configurações globais
# -----------------------------
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# -----------------------------
# Dependências via vcpkg
# -----------------------------
find_package(OpenGL REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
find_package(imgui CONFIG REQUIRED)

# -----------------------------
# Trabalhos
# -----------------------------
option(BUILD_TRABALHO_01 "Compilar Trabalho 01" OFF)
option(BUILD_TRABALHO_02 "Compilar Trabalho 02" ON)
option(BUILD_TRABALHO_03 "Compilar Trabalho 03" OFF)

target_link_libraries({PROJECT_NAME}
    PUBLIC
        OpenGL::GL
        glfw
        imgui::imgui
)

add_subdirectory(common)

if(BUILD_TRABALHO_01)
    add_subdirectory(trabalho_01)
endif()

if(BUILD_TRABALHO_02)
    add_subdirectory(trabalho_02)
endif()

if(BUILD_TRABALHO_03)
    add_subdirectory(trabalho_03)
endif()