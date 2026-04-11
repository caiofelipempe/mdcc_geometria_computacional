add_subdirectory(renderer)
add_subdirectory(sort)

# Interface library para agregar as dependências comuns
add_library(common INTERFACE)
target_link_libraries(common INTERFACE
    common_renderer
    common_sort
)