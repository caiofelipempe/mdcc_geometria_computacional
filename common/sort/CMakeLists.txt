add_library(sort STATIC
    mergesort.hpp
    quicksort.hpp
    selectsort.hpp
)

target_include_directories(sort PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)