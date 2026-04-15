add_library(sort STATIC
    mergesort.cpp
    quicksort.cpp
    selectsort.cpp
)

target_include_directories(sort PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)