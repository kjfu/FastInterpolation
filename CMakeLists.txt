cmake_minimum_required(VERSION 3.10)

project(FastInterpolation)

# SET(CMAKE_C_COMPILER gcc-10)
# SET(CMAKE_CXX_COMPILER g++-10)

aux_source_directory(./src DIR_SRCS)

add_executable(fastInterpolation ${DIR_SRCS}) 

