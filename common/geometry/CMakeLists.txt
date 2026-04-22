add_library(geometry INTERFACE
    arithmetic_vector.hpp
)

target_include_directories(geometry INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}
)