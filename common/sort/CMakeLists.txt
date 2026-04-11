add_library(common_sort STATIC
    mergesort.cpp
    quicksort.cpp
    selectsort.cpp
)

target_include_directories(common_sort PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)