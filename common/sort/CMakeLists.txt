add_library(common STATIC
    mergesort.cpp
    quicksort.cpp
    selectsort.cpp
)

target_include_directories(common PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)