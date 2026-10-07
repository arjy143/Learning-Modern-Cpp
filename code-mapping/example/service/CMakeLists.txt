add_library(service STATIC
    src/OrderService.cpp
)
target_include_directories(service PUBLIC include)
target_link_libraries(service PUBLIC core Threads::Threads)
