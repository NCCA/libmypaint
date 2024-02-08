cmake_minimum_required(VERSION 3.20)
if(NOT DEFINED CMAKE_TOOLCHAIN_FILE AND DEFINED ENV{CMAKE_TOOLCHAIN_FILE})
   set(CMAKE_TOOLCHAIN_FILE $ENV{CMAKE_TOOLCHAIN_FILE})
endif()

# Name of the project
project(RandomBrushBuildAll)
# use C++ 17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)

add_executable(RandomBrush)
find_package(OpenImageIO CONFIG REQUIRED)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
target_sources(RandomBrush PRIVATE 
                            ${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp)
target_link_directories(RandomBrush PRIVATE ${CMAKE_BINARY_DIR})
target_link_libraries(RandomBrush PRIVATE mypaint)
target_link_libraries(RandomBrush PRIVATE OpenImageIO::OpenImageIO OpenImageIO::OpenImageIO_Util)


