cmake_minimum_required(VERSION 3.10)
project(physics-sim VERSION 1.0)

#add_compile_options(-fsanitize=address,undefined)
#add_link_options(-fsanitize=address,undefined)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_subdirectory(phylib)

file(GLOB_RECURSE PHY_SRC CONFIGURE_DEPENDS src/*.cpp)
add_executable(physim src/main.cpp)
target_include_directories(phylib PUBLIC 
    "${PROJECT_BINARY_DIR}" 
    "${PROJECT_SOURCE_DIR}/include" 
    "${PROJECT_SOURCE_DIR}/phylib/include" 
    "${PROJECT_SOURCE_DIR}/phylib/loggerpp/include"
)
target_precompile_headers(phylib PRIVATE
    <fmt/format.h>
    <fmt/core.h>
)
target_link_libraries(physim PUBLIC phylib logging fmt)
