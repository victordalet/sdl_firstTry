cmake_minimum_required(VERSION 3.3)
project(SDL2ForWindows)

# C Project
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Werror")

# C++ Project
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
#set(CMAKE_EXE_LINKER_FLAGS "-static-libgcc -static-libstdc++")

include_directories(${PROJECT_SOURCE_DIR}/include)
link_directories(${PROJECT_SOURCE_DIR}/lib)

set(SOURCE_FILES main.c)
add_executable(SDL2ForWindows ${SOURCE_FILES})

target_link_libraries(SDL2ForWindows SDL2main SDL2)
