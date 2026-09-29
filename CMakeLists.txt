cmake_minimum_required(VERSION 3.10)

project(ClassF_Synthesis C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

add_executable(classf_synthesis src/main.c)

target_link_libraries(classf_synthesis PRIVATE m)
