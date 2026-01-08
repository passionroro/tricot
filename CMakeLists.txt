cmake_minimum_required(VERSION 3.10)
project(tricot)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(OpenCV REQUIRED)

add_executable(tricot srcs/main.cpp srcs/reader.cpp srcs/verbose.cpp)

target_link_libraries(tricot ${OpenCV_LIBS})

target_include_directories(tricot PRIVATE ${OpenCV_INCLUDE_DIRS})
