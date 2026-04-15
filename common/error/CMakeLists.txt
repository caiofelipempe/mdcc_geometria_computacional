add_library(error INTERFACE
    error.hpp
)

target_include_directories(error INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}
)