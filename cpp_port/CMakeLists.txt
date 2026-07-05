cmake_minimum_required(VERSION 3.20)
project(AnodyneSharp CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# --------------------------------------------------------------------------
# SDL3 backend
# --------------------------------------------------------------------------
if (LINUX)
    set(SDL3_DIR "${CMAKE_SOURCE_DIR}/SDL3-3.4.12/install/usr/local/lib/cmake/SDL3")
endif()

if (WIN32)
    set(SDL3_ROOT "${CMAKE_SOURCE_DIR}/../SDL3-3.4.10" CACHE PATH "SDL3 root directory")
endif()

find_package(SDL3 REQUIRED CONFIG)

# --------------------------------------------------------------------------
# Compiler flags
# --------------------------------------------------------------------------
if (MSVC)
    add_compile_options(/W3 /wd4100 /wd4201 /wd4244 /wd4267 /wd4458 /wd4456 /FS)
    add_compile_options(/std:c++20)
else()
    add_compile_options(-Wall -Wextra -Wno-unused-parameter)
endif()

# --------------------------------------------------------------------------
# Include directories
# --------------------------------------------------------------------------
include_directories(
    ${CMAKE_SOURCE_DIR}/include
    ${SDL3_ROOT}/include
)

# --------------------------------------------------------------------------
# Source files (recursive glob)
# --------------------------------------------------------------------------
file(GLOB_RECURSE SOURCES
    "${CMAKE_SOURCE_DIR}/src/*.cpp"
)

# --------------------------------------------------------------------------
# Executable target
# --------------------------------------------------------------------------
add_executable(AnodyneSharp ${SOURCES})

target_include_directories(AnodyneSharp PUBLIC
    ${CMAKE_SOURCE_DIR}/include
    ${SDL3_ROOT}/include
)

target_link_libraries(AnodyneSharp PRIVATE SDL3::SDL3)

# --------------------------------------------------------------------------
# Preprocessor defines
# --------------------------------------------------------------------------
target_compile_definitions(AnodyneSharp PRIVATE
    $<$<PLATFORM_ID:Windows>:WINDOWS=1>
    $<$<CONFIG:Release>:NDEBUG=1>
)

# stb_image optional feature
if(EXISTS "${CMAKE_SOURCE_DIR}/include/thirdparty/stb_image.h")
    target_compile_definitions(AnodyneSharp PRIVATE HAS_STB_IMAGE=1)
    message(STATUS "  stb_image.h found — PNG texture loading enabled")
endif()
if(EXISTS "${CMAKE_SOURCE_DIR}/include/thirdparty/dr_mp3.h")
    target_compile_definitions(AnodyneSharp PRIVATE HAS_DR_MP3=1)
    message(STATUS "  dr_mp3.h found — MP3 audio loading enabled")
endif()

# Clang coroutine flag
if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    target_compile_options(AnodyneSharp PRIVATE -fcoroutines)
endif()

# Copy SDL3.dll next to the executable
if (WIN32)
    add_custom_command(TARGET AnodyneSharp POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${SDL3_ROOT}/lib/x64/SDL3.dll"
            "$<TARGET_FILE_DIR:AnodyneSharp>"
        COMMENT "Copying SDL3.dll"
    )
endif()

message(STATUS "AnodyneSharp C++ port build configured.")
message(STATUS "  SDL3 root: ${SDL3_ROOT}")
message(STATUS "  Source files: ${SOURCES}")
