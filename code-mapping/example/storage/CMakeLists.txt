add_library(storage SHARED
    src/FileStorage.cpp
    src/CompressedStorage.cpp
)
target_include_directories(storage PUBLIC include)
target_link_libraries(storage PUBLIC core PRIVATE ZLIB::ZLIB)
