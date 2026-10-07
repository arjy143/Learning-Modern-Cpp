add_library(core STATIC
    src/Order.cpp
    src/ConsoleLogger.cpp
)
target_include_directories(core PUBLIC include)
