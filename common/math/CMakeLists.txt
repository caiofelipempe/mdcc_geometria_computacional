add_library(math INTERFACE
    vec.hpp
)

target_include_directories(math INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}
)