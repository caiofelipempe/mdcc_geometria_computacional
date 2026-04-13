add_library(common_error INTERFACE
    error.hpp
)

target_include_directories(common_error INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}
)