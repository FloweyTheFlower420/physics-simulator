cmake_minimum_required(VERSION 3.10)
project(phylib VERSION 1.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

file(GLOB_RECURSE PHYLIB_SRC CONFIGURE_DEPENDS src/*.cpp)
add_library(phylib ${PHYLIB_SRC})

find_package(SFML 2.5 COMPONENTS graphics audio REQUIRED)
add_subdirectory(loggerpp)

target_link_libraries(phylib logging sfml-graphics sfml-audio)

target_include_directories(phylib PUBLIC 
    "${PROJECT_BINARY_DIR}" 
    "${PROJECT_SOURCE_DIR}/include" 
    "${PROJECT_SOURCE_DIR}/loggerpp/include" 
)
