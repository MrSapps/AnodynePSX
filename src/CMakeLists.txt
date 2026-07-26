PROJECT(Anodyne)
cmake_minimum_required(VERSION 3.25)

find_package(SDL2 REQUIRED)

include_directories(${SDL2_INCLUDE_DIR})

add_executable(Anodyne main.c dialog_manager.c)
target_link_libraries(Anodyne ${SDL2_LIBRARIES})

add_executable(CompileContent content_compiler/main.cpp)
set_property(TARGET CompileContent PROPERTY CXX_STANDARD 17)
