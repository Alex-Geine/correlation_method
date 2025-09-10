cmake_minimum_required(VERSION 3.15)

project(data_processing)

add_executable(${PROJECT_NAME} main.cpp
                               Generator.cpp
                               Correlator.cpp)