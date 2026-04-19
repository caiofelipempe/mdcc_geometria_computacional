add_library(geometry INTERFACE
    geometry.hpp
)

target_include_directories(geometry INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}
)