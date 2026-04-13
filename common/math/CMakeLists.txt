add_library(common_math INTERFACE
    vecn.hpp
)

target_include_directories(common_math INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}
)